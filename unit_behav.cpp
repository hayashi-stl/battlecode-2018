#include "unit_behav.h"
#include "utility.h"
#include "globals.h"
#include <algorithm>
#include <numeric>

namespace unit_behav
{
	void st_attack_main(const bc::Unit& unit)
	{
		if(!unit.get_location().is_on_map())
			return;
		
		bc::MapLocation map_location = unit.get_map_location();
		auto visible = gb::gc.sense_nearby_units_by_team(map_location, unit.get_vision_range(), gb::opposite_team);
		auto too_close = gb::gc.sense_nearby_units_by_team(map_location, 10, gb::opposite_team);
		auto nearby = visible;
		if(unit.get_unit_type() == Ranger)
			nearby.erase(std::remove_if(nearby.begin(), nearby.end(), [&map_location](const bc::Unit enemy)
				{return enemy.get_map_location().distance_squared_to(map_location) <= 10;}), nearby.end());
		
		//if(unit.get_unit_type() == Ranger || unit.get_unit_type() == Mage)
		//{
		//	auto factory_split = std::stable_partition(nearby.begin(), nearby.end(), [](const bc::Unit& enemy){return enemy.get_unit_type() == Factory;});
		//	if(factory_split != nearby.begin())
		//		nearby.erase(factory_split, nearby.end());
		//}
		
		uint32_t unit_id = unit.get_id();
		uint32_t enemy_id = utility::closest_unit_id_to(map_location, nearby);
		
		ExtraUnitData& extra_unit_data = gb::extra_unit_datas[unit_id];
		
		bool could_attack = false;
		if(enemy_id != uint32_t(-1))
		{
			if(gb::gc.can_attack(unit_id, enemy_id))
			{
				could_attack = true;
				if(gb::gc.is_attack_ready(unit_id))
					gb::gc.attack(unit_id, enemy_id);
			}
			//bc::Unit enemy = gb::gc.get_unit(enemy_id);
			//	utility::chase_and_attack(unit, enemy);
		}
		
		if(!could_attack)
		{
			/*if(!extra_unit_data.use_walking_distance_field && extra_unit_data.walking_distance_field.field_at(0, 0) == 0x7fffffff
				&& map_location.get_planet() == Earth)
			{
				auto init_units = gb::gc.get_earth_map().get_initial_units();
				init_units.erase(std::remove_if(init_units.begin(), init_units.end(), [](const bc::Unit& the_unit){return the_unit.get_team() == gb::my_team;}), init_units.end());
				
				bc::MapLocation enemy_location = utility::closest_unit_to(map_location, init_units).get_map_location();
				extra_unit_data.use_walking_distance_field = true;
				extra_unit_data.walking_distance_field.build_padded_walking_distance_field<1>(enemy_location);
				
				//if(unit.get_team() == Red && unit.get_map_location().get_planet() == Earth)
				//	printf("Field for Unit Type %d, ID %d:\n%s\n", unit.get_unit_type(), unit_id, extra_unit_data.walking_distance_field.to_string().c_str());
			}
			
			if(extra_unit_data.use_walking_distance_field)
			{
				if(utility::follow_walking_distance_field(unit, extra_unit_data.walking_distance_field))
					extra_unit_data.use_walking_distance_field = false;
			}
			else
			{
				bc::Direction dir = bc::Direction(gb::nonzero_direction_dice());
				if(gb::gc.can_move(unit_id, dir) && gb::gc.is_move_ready(unit_id))
					gb::gc.move_robot(unit_id, dir);
			}*/
			
			bool moved = false;
			if(gb::mega_strategy == MS_ROCKET_MASS_LAUNCH && map_location.get_planet() == Earth)
				utility::follow_walking_distance_field(unit,
					ScalarField::EARTH_ROCKET_DISTANCE, &moved);
			if(!moved)
			{
				if(too_close.size() > 0 && unit.get_unit_type() == Ranger)
					utility::follow_walking_distance_field<true>(unit,
						unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD, &moved);
				else if(visible.size() > 0 && unit.get_unit_type() == Ranger)
					; //no moving
				else
					utility::follow_walking_distance_field(unit,
						unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD, &moved);
			}
			if(!moved)
				utility::follow_walking_distance_field<true>(unit,
					unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_SPREAD : ScalarField::EARTH_SPREAD);
		}
		
	}
	
	void st_defend_main(const bc::Unit& unit)
	{
		
	}
	
	void st_heal_main(const bc::Unit& unit)
	{
		if(!unit.get_location().is_on_map())
			return;
		
		//auto nearby = gb::gc.sense_nearby_units_by_team(unit.get_location().get_map_location(), unit.get_vision_range(), gb::my_team);
		auto visible = gb::gc.sense_nearby_units_by_team(unit.get_map_location(), unit.get_vision_range(), gb::opposite_team);
		auto nearby = gb::gc.sense_nearby_units_by_team(unit.get_map_location(), unit.get_attack_range(), gb::my_team);
		nearby.erase(std::remove_if(nearby.begin(), nearby.end(), [](const bc::Unit& unit){return unit.is_structure();}), nearby.end());
		
		uint32_t unit_id = unit.get_id();
		auto heal_unit_it = std::min_element(nearby.begin(), nearby.end(),
			[](const bc::Unit& a, const bc::Unit& b){return int(a.get_health()) - int(a.get_max_health()) < int(b.get_health()) - int(b.get_max_health());});
		bc::MapLocation map_location = unit.get_map_location();
		
		if(gb::gc.get_research_info().get_level(Healer) >= 3)
		{
			std::vector<bc::Unit> overchargeable = nearby;
			overchargeable.erase(std::remove_if(overchargeable.begin(), overchargeable.end(), [](const bc::Unit& unit){return unit.get_unit_type() == Healer;}), overchargeable.end());
			
			auto overcharge_unit_it = std::min_element(overchargeable.begin(), overchargeable.end(), [](const bc::Unit& a, const bc::Unit& b)
				{return a.get_attack_cooldown() - a.get_attack_heat() < b.get_attack_cooldown() - b.get_attack_heat();});
			
			if(overcharge_unit_it != nearby.end() && overcharge_unit_it->get_attack_heat() >= 10)
			{
				uint32_t overcharge_id = overcharge_unit_it->get_id();
				if(gb::gc.can_overcharge(unit_id, overcharge_id) && gb::gc.is_overcharge_ready(unit_id))
				{
					gb::gc.overcharge(unit_id, overcharge_id);
					if(gb::extra_unit_datas.find(overcharge_id) != gb::extra_unit_datas.end())
						(*states[gb::extra_unit_datas[overcharge_id].state].main)(*overcharge_unit_it);
				}
			}
		}
		
		/*if(heal_unit_it != nearby.end())
		{
			printf("Healer %d at (%d, %d) tries to heal robot %d at (%d, %d), HP %d\n", unit_id, unit.get_map_location().get_x(), unit.get_map_location().get_y(),
				heal_unit_it->get_id(), heal_unit_it->get_map_location().get_x(), heal_unit_it->get_map_location().get_y(), heal_unit_it->get_health());
		}*/
		
		if(heal_unit_it != nearby.end() && heal_unit_it->get_health() < heal_unit_it->get_max_health())
		{
			//utility::chase_and_heal(unit, *heal_unit_it);
			if(gb::gc.can_heal(unit_id, heal_unit_it->get_id()) && gb::gc.is_heal_ready(unit_id))
				gb::gc.heal(unit_id, heal_unit_it->get_id());
		}
		//else
		//{
			//bc::Direction dir = bc::Direction(gb::nonzero_direction_dice());
			//if(gb::gc.can_move(unit_id, dir) && gb::gc.is_move_ready(unit_id))
			//	gb::gc.move_robot(unit_id, dir);
			bc::Planet planet = unit.get_map_location().get_planet();
			auto dont_get_too_close = [planet](const bc::Unit& unit, bc::Direction dir, int new_x, int new_y) -> bool
			{
				return (planet == Mars ? ScalarField::MARS_ENEMY_ATTACK_RANGE : ScalarField::EARTH_ENEMY_ATTACK_RANGE).field_at(new_x + 1, new_y + 1) == 0;
			};
			
			bool moved = false;
			if(gb::mega_strategy == MS_ROCKET_MASS_LAUNCH && map_location.get_planet() == Earth)
				utility::follow_walking_distance_field(unit,
					ScalarField::EARTH_ROCKET_DISTANCE, &moved);
			if(!moved)
			{
				if(visible.size() > 0)
					utility::follow_walking_distance_field<true>(unit,
						unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD, &moved);
				else
					utility::follow_walking_distance_field(unit,
						unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD, &moved);
			}
			if(!moved)
				utility::follow_walking_distance_field<true>(unit,
					planet == Mars ? ScalarField::MARS_SPREAD : ScalarField::EARTH_SPREAD, dont_get_too_close);
		//}
	}
	
	void st_build_main(const bc::Unit& unit)
	{
		if(!unit.get_location().is_on_map())
			return;
		
		uint32_t unit_id = unit.get_id();
		ExtraUnitData& extra_unit_data = gb::extra_unit_datas[unit_id];
		bc::MapLocation map_location = unit.get_map_location();
		
		utility::attempt_replication(unit);
		
		if(map_location.get_planet() == Mars)
		{
			extra_unit_data.state = ST_HARVEST;
			return;
		}
		
		const ScalarField& enemy_spread_field = unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD;
		
		auto nearby = gb::gc.sense_nearby_units_by_team(unit.get_location().get_map_location(), unit.get_vision_range(), gb::my_team);
		nearby.erase(std::remove_if(nearby.begin(), nearby.end(), [](const bc::Unit& unit){return !unit.is_structure() || unit.get_health() == unit.get_max_health();}), nearby.end());
		
		uint32_t structure_id = utility::closest_unit_id_to(unit.get_location().get_map_location(), nearby);
		//if(structure_id != uint32_t(-1))
		//	utility::chase_and_build(unit, gb::gc.get_unit(structure_id));
		bool can_build = structure_id != uint32_t(-1) ? gb::gc.can_build(unit_id, structure_id) : false;
		bool can_repair = structure_id != uint32_t(-1) ? gb::gc.can_repair(unit_id, structure_id) : false;
		/*if(map_location.get_x() == 5 && map_location.get_y() == 11 && structure_id != -1)
		{
			printf("Worker %d trying to build structure at (%d, %d), has worker acted: %d\n", unit_id,
				gb::gc.get_unit(structure_id).get_map_location().get_x(), gb::gc.get_unit(structure_id).get_map_location().get_y(), unit.worker_has_acted() ? 1 : 0);
				
			printf("Condition Check: Using many functions: %d, Using umbrella function: %d\n",
				unit.is_on_map() && !unit.worker_has_acted() && unit.get_map_location().is_adjacent_to(gb::gc.get_unit(structure_id).get_map_location()) ? 1 : 0,
				gb::gc.can_build(unit_id, structure_id) ? 1 : 0);
		}*/
				
		bool built_or_repaired = false;
		if(can_build || can_repair)
		{
			bool built = false;
			bc::Unit structure = gb::gc.get_unit(structure_id);
			if(can_build)
			{
				gb::gc.build(unit_id, structure_id);
				built = true;
				built_or_repaired = true;
			}
			if(can_repair && structure.get_unit_type() != Rocket && structure.get_health() < structure.get_max_health())
			{
				gb::gc.repair(unit_id, structure_id);
				built_or_repaired = true;
			}
			
			if(built && structure.get_unit_type() == Rocket && structure.get_health() >= 200 - unit.get_worker_build_health())
				gb::rocket_built_or_launched_this_turn = true;
		}
		
		if(!built_or_repaired)
		{
			bc::UnitType type = gb::save_karbonite_for_rocket || gb::mega_strategy == MS_ROCKET_MASS_LAUNCH ? Rocket : extra_unit_data.blueprint_cycle % 3 == 2 ? Rocket : Factory;
			bool blueprinted = false;
			
			if(gb::gc.get_karbonite() > bc::unit_type_get_blueprint_cost(type))
			{
				std::vector<bc::Direction> allowed_directions;
				std::vector<bc::Direction> blocked_directions;
				for(int i = 0; i < gb::NUM_NONZERO_DIRECTIONS; ++i)
				{
					bc::Direction dir = bc::Direction(i);
					//int x = unit.get_map_location().get_x() + gb::nonzero_dir_array[i].first;
					//int y = unit.get_map_location().get_y() + gb::nonzero_dir_array[i].second;
					bc::MapLocation new_location = map_location + dir;
					int x = new_location.get_x();
					int y = new_location.get_y();
					
					//printf("Location (%d, %d), Direction %d, Unit exists: %d, Passable: %d, RDF: %d, Blueprintable: %d\n",
					//map_location.get_x(), map_location.get_y(), int(dir),
					//	gb::gc.has_unit_at_location(new_location) ? 1 : 0,
					//	ScalarField::EARTH_PASSABLE_PAD_1.field_at(new_location.get_x(), new_location.get_y()),
					//	ScalarField::EARTH_ROCKET_DISTANCE.field_at(x + 1, y + 1),
					//	gb::gc.can_blueprint(unit_id, type, dir) ? 1 : 0);
						
					if(gb::gc.has_unit_at_location(new_location) || ScalarField::EARTH_PASSABLE_PAD_1.field_at(new_location.get_x(), new_location.get_y()) == 0)
						blocked_directions.push_back(dir);
					else if ((ScalarField::EARTH_ROCKET_DISTANCE.field_at(x + 1, y + 1) >= 2 || ScalarField::EARTH_ROCKET_DISTANCE.field_at(x + 1, y + 1) == -1)
							&& gb::gc.can_blueprint(unit_id, type, dir))
						allowed_directions.push_back(dir);
				}
				
				//printf("Location (%d, %d), allowed direction list size: %d\n", map_location.get_x(), map_location.get_y(), int(allowed_directions.size()));
				if(allowed_directions.size() > 0)
				{
					std::sort(allowed_directions.begin(), allowed_directions.end(), [&map_location, &enemy_spread_field](bc::Direction a, bc::Direction b)
					{
						bc::MapLocation new_location_a = map_location + a;
						bc::MapLocation new_location_b = map_location + b;
						return enemy_spread_field.field_at(new_location_a.get_x() + 1, new_location_a.get_y() + 1) <
							   enemy_spread_field.field_at(new_location_b.get_x() + 1, new_location_b.get_y() + 1);
					});
					
					gb::gc.blueprint(unit_id, type, allowed_directions[0]);
					blueprinted = true;
					if(gb::save_karbonite_for_rocket && gb::generator() % 2 == 0) //sometimes we want to build multiple rockets
						gb::save_karbonite_for_rocket = false;
				}
				else if(type == Rocket && blocked_directions.size() == gb::NUM_NONZERO_DIRECTIONS) //rockets are high priority!
				{
					for(int i = 0; i < gb::NUM_NONZERO_DIRECTIONS; ++i)
					{
						bc::Direction dir = bc::Direction(i);
						bc::MapLocation new_location = map_location + dir;
						if(gb::gc.has_unit_at_location(new_location))
						{
							bc::Unit blocker = gb::gc.sense_unit_at_location(new_location);
							if(blocker.get_team() == gb::my_team && !blocker.is_structure())
							{
								printf("Disintegrating unit type %d at (%d, %d)\n", blocker.get_unit_type(), new_location.get_x(), new_location.get_y());
								gb::gc.disintegrate_unit(blocker.get_id());
								if(gb::gc.can_blueprint(unit_id, type, dir))
								{
									gb::gc.blueprint(unit_id, type, dir);
									blueprinted = true;
									if(gb::save_karbonite_for_rocket && gb::generator() % 2 == 0) //sometimes we want to build multiple rockets
										gb::save_karbonite_for_rocket = false;
								}
							}
						}
					}
				}
			}
			
			for(int i = 0; i < 9; ++i)
			{
				bc::Direction dir = bc::Direction(i);
				if(gb::gc.can_harvest(unit_id, dir))
				{
					gb::gc.harvest(unit_id, dir);
					break;
				}
			}
			
			if(!blueprinted)
			{
				utility::follow_walking_distance_field(unit,
					unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_UNOCCUPIED_KARBONITE_DISTANCE : ScalarField::EARTH_UNOCCUPIED_KARBONITE_DISTANCE);
			}
		}
		
	}
	
	void st_harvest_main(const bc::Unit& unit)
	{
		if(!unit.get_location().is_on_map())
			return;
		uint32_t id = unit.get_id();
		
		utility::attempt_replication(unit);
		
		bool harvested_or_built = false;
		
		auto nearby = gb::gc.sense_nearby_units_by_team(unit.get_location().get_map_location(), 2, gb::my_team);
		nearby.erase(std::remove_if(nearby.begin(), nearby.end(), [](const bc::Unit& unit){return !unit.is_structure() || unit.get_health() == unit.get_max_health();}), nearby.end());
		
		uint32_t structure_id = utility::closest_unit_id_to(unit.get_location().get_map_location(), nearby);
		//if(structure_id != uint32_t(-1))
		//	utility::chase_and_build(unit, gb::gc.get_unit(structure_id));
		bool can_build = structure_id != uint32_t(-1) ? gb::gc.can_build(id, structure_id) : false;
		bool can_repair = structure_id != uint32_t(-1) ? gb::gc.can_repair(id, structure_id) : false;
		if(can_build || can_repair)
		{
			bc::Unit structure = gb::gc.get_unit(structure_id);
			if(can_build)
			{
				gb::gc.build(id, structure_id);
				harvested_or_built = true;
			}
			if(can_repair && structure.get_unit_type() != Rocket && structure.get_health() < structure.get_max_health())
			{
				gb::gc.repair(id, structure_id);
				harvested_or_built = true;
			}
			
			if(harvested_or_built && structure.get_unit_type() == Rocket && structure.get_health() >= 200 - unit.get_worker_build_health())
				gb::rocket_built_or_launched_this_turn = true;
		}
		
		for(int i = 0; i < 9; ++i)
		{
			bc::Direction dir = bc::Direction(i);
			if(gb::gc.can_harvest(id, dir))
			{
				gb::gc.harvest(id, dir);
				//harvested_or_built = true;
				break;
			}
		}
		
		if(!harvested_or_built)
		{
			/*bc::Direction dir = bc::Direction(gb::direction_dice());
				if(gb::gc.can_move(id, dir) && gb::gc.is_move_ready(id))
					gb::gc.move_robot(id, dir);*/
			utility::follow_walking_distance_field(unit,
				unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_UNOCCUPIED_KARBONITE_DISTANCE : ScalarField::EARTH_UNOCCUPIED_KARBONITE_DISTANCE);
		}
		
		if(gb::mega_strategy == MS_ROCKET_MASS_LAUNCH)
			gb::extra_unit_datas[id].state = ST_BUILD;
	}
	
	void st_unload_main(const bc::Unit& unit)
	{
		
		uint32_t id = unit.get_id();
		auto garrison = unit.get_structure_garrison();
		std::vector<bc::Direction> allowed_directions;
		
		//if(garrison.size() == 0 && unit.get_location().is_on_map() && unit.get_map_location().get_planet() == Mars)
		//	gb::gc.disintegrate_unit(id);
					
		bc::MapLocation map_location = unit.get_map_location();
		ScalarField& enemy_spread_field = map_location.get_planet() == Mars ? ScalarField::MARS_ENEMY_SPREAD : ScalarField::EARTH_ENEMY_SPREAD;
		
		if(garrison.size() > 0)
		{
			for(int i = 0; i < gb::NUM_NONZERO_DIRECTIONS; ++i)
			{
				bc::Direction dir = bc::Direction(i);
				
				if (gb::gc.can_unload(id, dir))
					allowed_directions.push_back(dir);
			}
			
			//printf("Unloaded a robot!");
			if(allowed_directions.size() > 0)
			{
				std::sort(allowed_directions.begin(), allowed_directions.end(), [&map_location, &enemy_spread_field](bc::Direction dir_a, bc::Direction dir_b)
					{
						bc::MapLocation new_location_a = map_location + dir_a;
						bc::MapLocation new_location_b = map_location + dir_b;
						return enemy_spread_field.field_at(new_location_a.get_x() + 1, new_location_a.get_y() + 1) <
						       enemy_spread_field.field_at(new_location_b.get_x() + 1, new_location_b.get_y() + 1);
					});
				gb::gc.unload(id, allowed_directions[0]);
			}
		}
		
		//bc::UnitType type_to_produce = gb::FACTORY_SPAWN_DISTRIBUTION[gb::generator() % gb::FACTORY_SPAWN_DISTRIBUTION.size()];
		int distribution_size = std::accumulate(gb::robot_relative_probabilities.begin(), gb::robot_relative_probabilities.end(), 0);
		if(distribution_size == 0)
			return;
		
		int unit_type_int = 0;
		int random_number = gb::generator() % distribution_size;
		while(random_number >= gb::robot_relative_probabilities[unit_type_int])
		{
			random_number -= gb::robot_relative_probabilities[unit_type_int];
			++unit_type_int;
		}
		
		bc::UnitType type_to_produce = bc::UnitType(unit_type_int);
		auto factories = gb::gc.sense_nearby_units_by_type(map_location, 10, Factory);
		if(std::any_of(factories.begin(), factories.end(), [](const bc::Unit& factory){return factory.get_team() != gb::my_team;}))
			type_to_produce = Knight;
		
		if(gb::num_production < 3)
			type_to_produce = Knight;
		else if(gb::num_production % 7 == 6 || gb::num_production % 7 == 2)
			type_to_produce = Healer;
		
		//printf("Generated %d, producing type %d.\n", random_number, type_to_produce);
		
		if(!gb::save_karbonite_for_rocket && gb::gc.can_produce_robot(id, type_to_produce))
		{
			//printf("Produced a robot!");
			gb::gc.produce_robot(id, type_to_produce);
			++gb::num_production;
		}
	}
	
	void st_launch_main(const bc::Unit& unit)
	{
		if(unit.get_location().is_in_space())
			return;
		
		auto garrison = unit.get_structure_garrison();
		uint32_t id = unit.get_id();
		
		ExtraUnitData& extra_unit_data = gb::extra_unit_datas[id];
		
		//printf("Rocket Planet %d, (%2d, %2d)", unit.get_map_location().get_planet(), unit.get_map_location().get_x(), unit.get_map_location().get_y());
		
		if(unit.get_location().get_map_location().get_planet() == Earth)
		{
			auto nearby = gb::gc.sense_nearby_units_by_team(unit.get_map_location(), 2, gb::my_team);
			auto nearby_enemies = gb::gc.sense_nearby_units_by_team(unit.get_map_location(), 2, gb::opposite_team);
			nearby.erase(std::remove_if(nearby.begin(), nearby.end(), [](const bc::Unit& unit){return unit.is_structure();}), nearby.end());
			
			if(garrison.size() >= 8 || (garrison.size() >= 4 && nearby.size() == 0) || nearby_enemies.size() > 0 || gb::gc.get_round() >= 745)
			{	
				if(gb::mars_points.size() == 0)
					return;
				
				gb::int_pair location_pair = gb::mars_points[gb::generator() % gb::mars_points.size()];
				bc::MapLocation location(Mars, location_pair.first, location_pair.second);
					
				if(gb::gc.can_launch_rocket(id, location))
				{
					gb::gc.launch_rocket(id, location);
					gb::rocket_built_or_launched_this_turn = true;
				}
			}
			else
			{
				for(const bc::Unit& other : nearby)
					if((gb::mega_strategy != MS_ROCKET_MASS_LAUNCH ||
						other.get_unit_type() != Worker || std::count(extra_unit_data.garrison_unit_types.begin(), extra_unit_data.garrison_unit_types.end(), Worker) < 1) &&
						gb::gc.can_load(id, other.get_id()))
					{
						gb::gc.load(id, other.get_id());
						extra_unit_data.garrison_unit_types.push_back(other.get_unit_type());
					}
			}
		}
		else
		{
			gb::extra_unit_datas[id].state = ST_UNLOAD;
		}
	}
	
	bool st_attack_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		return false;
	}
	
	bool st_defend_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		return false;
	}
	
	bool st_heal_please_move   (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		return false;
	}
	
	bool st_build_please_move  (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		bc::MapLocation new_location = unit.get_map_location() + requester_move_dir;
		
		if(stack_depth >= 50 || !gb::gc.is_move_ready(unit.get_id()) ||
			(unit.get_map_location().get_planet() == Mars ? ScalarField::MARS_PASSABLE_PAD_1 : ScalarField::EARTH_PASSABLE_PAD_1).field_at(new_location.get_x() + 1, new_location.get_y() + 1) == 0)
			return false;
		
		if(gb::gc.has_unit_at_location(new_location))
		{
			bc::Unit blocker = gb::gc.sense_unit_at_location(new_location);
			if(blocker.get_team() == gb::my_team && gb::extra_unit_datas.find(blocker.get_id()) != gb::extra_unit_datas.end() &&
				(*states[gb::extra_unit_datas[blocker.get_id()].state].please_move)(blocker, unit, requester_move_dir, stack_depth + 1))
			{
				gb::gc.move_robot(unit.get_id(), requester_move_dir);
				return true;
			}
			else
				return false;
		}
		else
		{
			gb::gc.move_robot(unit.get_id(), requester_move_dir);
			return true;
		}
	}
	
	bool st_harvest_please_move(const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		return st_build_please_move(unit, requester, requester_move_dir, stack_depth);
	}
	
	bool st_unload_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		return false;
	}
	
	bool st_launch_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth)
	{
		return false;
	}
};