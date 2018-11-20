#include "utility.h"
#include <algorithm>
#include "scalar_field.h"

namespace utility
{
	uint32_t closest_unit_id_to(const bc::MapLocation& location, const std::vector<bc::Unit>& units, bool include_curr_location)
	{
		uint32_t closest_dist_squared = 10000;
		uint32_t closest_id = uint32_t(-1); //unit id max is 65535
		
		for(const bc::Unit& unit : units)
		{
			uint32_t distance = unit.get_location().get_map_location().distance_squared_to(location);
			if((distance != 0 || include_curr_location) && distance < closest_dist_squared)
			{
				closest_dist_squared = distance;
				closest_id = unit.get_id();
			}
		}
		
		return closest_id;
	}
	
	bc::Unit closest_unit_to(const bc::MapLocation& location, const std::vector<bc::Unit>& units, bool include_curr_location)
	{
		uint32_t closest_dist_squared = 10000;
		auto closest = units.begin();
		
		for(auto unit_it = units.begin() + 1; unit_it != units.end(); ++unit_it)
		{
			uint32_t distance = unit_it->get_location().get_map_location().distance_squared_to(location);
			if((distance != 0 || include_curr_location) && distance < closest_dist_squared)
			{
				closest_dist_squared = distance;
				closest = unit_it;
			}
		}
		
		return *closest;
	}
	
	void chase_and_attack(const bc::Unit& unit, const bc::Unit& target)
	{
		if(!target.get_location().is_on_map())
			return;
		
		const bc::MapLocation& unit_location = unit.get_location().get_map_location();
		const bc::MapLocation& target_location = target.get_location().get_map_location();
		
		uint32_t unit_id = unit.get_id();
		uint32_t target_id = target.get_id();
		
		if(gb::gc.can_attack(unit_id, target_id))
		{
			if(gb::gc.is_attack_ready(unit_id))
				gb::gc.attack(unit_id, target_id);
		}
		//else// if(unit.get_unit_type() != Ranger || target.is_structure())
		//{
			/*bc::Direction dir = unit_location.direction_to(target_location);
			
			//Rangers should not move into other robot's ranges
			if(unit.get_unit_type() != Ranger || target.is_structure() ||
				(unit_location + dir).distance_squared_to(target_location) > target.get_attack_range())
			{
				if(gb::gc.can_move(unit_id, dir) && gb::gc.is_move_ready(unit_id))
					gb::gc.move_robot(unit_id, dir);
			}*/
			attempt_move_attacker(unit);
		//}
	}
	
	void attempt_move_attacker(const bc::Unit& unit)
	{
		bool moved;
		utility::follow_walking_distance_field(unit,
			unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD, &moved);
		if(!moved)
			utility::follow_walking_distance_field<true>(unit,
				unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_SPREAD : ScalarField::EARTH_SPREAD);
	}
	
	void chase_and_heal(const bc::Unit& unit, const bc::Unit& target)
	{
		//if(!target.get_location().is_on_map())
		//	return;
		
		uint32_t unit_id = unit.get_id();
		uint32_t target_id = target.get_id();
		
		bool can_heal = gb::gc.can_heal(unit_id, target_id);
		
		if(can_heal && gb::gc.is_heal_ready(unit_id))
			gb::gc.heal(unit_id, target_id);
		
		bc::Direction dir = unit.get_location().get_map_location().direction_to(target.get_location().get_map_location());
		if(gb::gc.can_move(unit_id, dir) && gb::gc.is_move_ready(unit_id))
			gb::gc.move_robot(unit_id, dir);
	}
	
	void chase_and_build(const bc::Unit& unit, const bc::Unit& target)
	{
		if(!target.get_location().is_on_map())
			return;
		
		uint32_t unit_id = unit.get_id();
		uint32_t target_id = target.get_id();
		
		if(gb::gc.can_build(unit_id, target_id))
			gb::gc.build(unit_id, target_id);
		else
		{
			bc::Direction dir = unit.get_location().get_map_location().direction_to(target.get_location().get_map_location());
			if(gb::gc.can_move(unit_id, dir) && gb::gc.is_move_ready(unit_id))
				gb::gc.move_robot(unit_id, dir);
		}
	}
	
	uint32_t count_team_workers()
	{
		auto units = gb::gc.get_my_units();
		return std::count_if(units.begin(), units.end(), [](const bc::Unit& unit){return unit.get_unit_type() == Worker;});
	}
	
	void attempt_replication(const bc::Unit& worker)
	{
		if(!worker.get_location().is_on_map())
			return;
		
		uint32_t id = worker.get_id();
		ExtraUnitData& extra_unit_data = gb::extra_unit_datas[id];
		if(!ScalarField::UNBUILT_FACTORIES_EXIST && gb::gc.get_round() < 750 && extra_unit_data.replicate_cooldown != 0)
		{
			--extra_unit_data.replicate_cooldown;
			return;
		}
		if(gb::save_karbonite_for_rocket)
			return;
		
		std::vector<bc::Direction> allowed_directions;
		int karbonite = gb::gc.get_karbonite();
		bc::MapLocation map_location = worker.get_map_location();
		bc::Planet planet = map_location.get_planet();
		
		for(int i = 0; i < gb::NUM_NONZERO_DIRECTIONS; ++i)
		{
			bc::Direction dir = bc::Direction(i);
			bc::MapLocation new_location = map_location + dir;
			
			if (gb::gc.can_replicate(id, dir) && (planet == Mars || karbonite >= 400 || !ScalarField::UNBUILT_FACTORIES_EXIST ||
					ScalarField::EARTH_UNBUILT_FACTORY_DISTANCE.field_at(new_location.get_x() + 1, new_location.get_y() + 1) == 1))
				allowed_directions.push_back(dir);
		}
			
		if(allowed_directions.size() == 0)
			return;
		
		gb::gc.replicate(id, allowed_directions[gb::generator() % allowed_directions.size()]);
		
		uint32_t worker_count = count_team_workers();
		extra_unit_data.replicate_cooldown =
			std::uniform_int_distribution<uint32_t>(gb::MIN_REPLICATE_COOLDOWN_1_WORKER * worker_count, gb::MAX_REPLICATE_COOLDOWN_1_WORKER * worker_count)(gb::generator);
			
		//printf("Replicated worker! Count = %d, cooldown = %d", worker_count, extra_unit_data.replicate_cooldown);
	}
	
	/*template <bool Backwards>
	bool follow_walking_distance_field(const bc::Unit& unit, const ScalarField& field, bool* moved_ptr, const gb::move_validation_func& mvf)
	{
		uint32_t id = unit.get_id();
		bc::MapLocation map_location = unit.get_map_location();
		if(moved_ptr)
			*moved_ptr = false;
		
		if(!gb::gc.is_move_ready(id))
			return false;
		
		for(int i = 0; i < gb::NUM_NONZERO_DIRECTIONS; ++i)
		{
			int x = map_location.get_x() + gb::nonzero_dir_array[i].first;
			int y = map_location.get_y() + gb::nonzero_dir_array[i].second;
			bc::Direction dir = bc::Direction(i);
			
			if(field.field_at(x + 1, y + 1) >= 0
				&& (( Backwards && field.field_at(x + 1, y + 1) > field.field_at(map_location.get_x() + 1, map_location.get_y() + 1)) ||
					(!Backwards && field.field_at(x + 1, y + 1) < field.field_at(map_location.get_x() + 1, map_location.get_y() + 1)))
				&& mvf(unit, dir) && gb::gc.can_move(id, dir))
			{
				/*if(unit.get_unit_type() == Knight && map_location.get_x() == 2 && map_location.get_y() == 3)
					printf("Moving knight on WDF from (%d, %d) (field = %d) to (%d, %d) (field = %d)!\n",
						map_location.get_x(),
						map_location.get_y(),
						field.field_at(map_location.get_x() + 1, map_location.get_y() + 1),
						x,
						y,
						field.field_at(x, y));*./
				if(moved_ptr)
					*moved_ptr = true;
				gb::gc.move_robot(id, dir);
				if(!Backwards && field.field_at(x + 1, y + 1) == 0)
					return true;
				
				break;
			}
		}
		
		return false;
	}
	template bool follow_walking_distance_field< true>(const bc::Unit& unit, const ScalarField& walking_distance_field, bool* moved_ptr);
	template bool follow_walking_distance_field<false>(const bc::Unit& unit, const ScalarField& walking_distance_field, bool* moved_ptr);*/
}