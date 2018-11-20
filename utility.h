#pragma once
#include "bc.hpp"
#include "globals.h"
#include "scalar_field.h"

namespace utility
{
	inline bc::Team opposite_team(bc::Team team) {return team == Red ? Blue : Red;}
	
	uint32_t closest_unit_id_to(const bc::MapLocation& location, const std::vector<bc::Unit>& units, bool include_curr_location = false);
	bc::Unit closest_unit_to(const bc::MapLocation& location, const std::vector<bc::Unit>& units, bool include_curr_location = false); //use only if there's at least 1 unit
	void chase_and_attack(const bc::Unit& unit, const bc::Unit& target);
	void attempt_move_attacker(const bc::Unit& unit);
	void chase_and_heal(const bc::Unit& unit, const bc::Unit& target);
	void chase_and_build(const bc::Unit& unit, const bc::Unit& target);
	uint32_t count_team_workers();
	void attempt_replication(const bc::Unit& worker);
	
	template <bool Backwards = false, class MoveValidationFunction>
	bool follow_walking_distance_field(const bc::Unit& unit, const ScalarField& field, MoveValidationFunction mvf, bool* moved_ptr = nullptr)
	{
		uint32_t id = unit.get_id();
		bc::MapLocation map_location = unit.get_map_location();
		
		//if(field.field_at(map_location.get_x() + 1, map_location.get_y() + 1) == 0)
		//	return true;
		if(!gb::gc.is_move_ready(id))
			return false;
		
		bc::Planet planet = map_location.get_planet();
		bc::Direction move_dir = bc::Direction(8);
		bc::Direction push_dir = bc::Direction(8);
		int max_diff = 0;
		int push_max_diff = 0;
		bc::Unit blocker;
		
		for(int i = 0; i < gb::NUM_NONZERO_DIRECTIONS; ++i)
		{
			int x = map_location.get_x() + gb::nonzero_dir_array[i].first;
			int y = map_location.get_y() + gb::nonzero_dir_array[i].second;
			bc::Direction dir = bc::Direction(i);
			
			int diff = field.field_at(x + 1, y + 1) - field.field_at(map_location.get_x() + 1, map_location.get_y() + 1);
			
			if(field.field_at(x + 1, y + 1) >= 0
				&& (( Backwards && (diff > max_diff || diff >= max_diff && max_diff == 0)) ||
					(!Backwards && (diff < max_diff || diff <= max_diff && max_diff == 0)))
				&& mvf(unit, dir, x, y) && (planet == Mars ? ScalarField::MARS_PASSABLE_PAD_1 : ScalarField::EARTH_PASSABLE_PAD_1).field_at(x + 1, y + 1) != 0)
			{
				/*if(unit.get_unit_type() == Knight && map_location.get_x() == 2 && map_location.get_y() == 3)
					printf("Moving knight on WDF from (%d, %d) (field = %d) to (%d, %d) (field = %d)!\n",
						map_location.get_x(),
						map_location.get_y(),
						field.field_at(map_location.get_x() + 1, map_location.get_y() + 1),
						x,
						y,
						field.field_at(x, y));*/
				/*if(moved_ptr)
					*moved_ptr = true;
				gb::gc.move_robot(id, dir);
				if(!Backwards && field.field_at(x + 1, y + 1) == 0)
					return true;*/
				if(!gb::gc.has_unit_at_location(bc::MapLocation(planet, x, y)))
				{
					move_dir = dir;
					max_diff = diff;
				}
				else if(gb::gc.sense_unit_at_location(bc::MapLocation(planet, x, y)).get_team() == gb::my_team)
				{
					push_dir = dir;
					push_max_diff = diff;
					blocker = gb::gc.sense_unit_at_location(bc::MapLocation(planet, x, y));
				}
			}
		}
		
		if(int(move_dir) != 8)
		{
			if(moved_ptr)
				*moved_ptr = true;
			gb::gc.move_robot(id, move_dir);
			map_location = map_location + move_dir;
			if(!Backwards && field.field_at(map_location.get_x() + 1, map_location.get_y() + 1) == 0)
				return true;
		}
		else if(int(push_dir) != 8 && gb::extra_unit_datas.find(blocker.get_id()) != gb::extra_unit_datas.end())
		{
			(*unit_behav::states[gb::extra_unit_datas[blocker.get_id()].state].please_move)(blocker, unit, push_dir, 0);
		}
		
		return false;
	}
	template <bool Backwards = false>
	bool follow_walking_distance_field(const bc::Unit& unit, const ScalarField& field, bool* moved_ptr = nullptr)
	{
		return follow_walking_distance_field<Backwards>(unit, field, [](const bc::Unit&, bc::Direction, int, int) -> bool {return true;}, moved_ptr);
	}
}