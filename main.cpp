#include <cstdio>
#include <cstdint>
#include <cstdbool>
#include <cstdlib>
#include <cassert>
#include <random>
#include <functional>
#include <iostream>
#include <exception>
#include <csignal>
#include <memory>
#include <string>
#include <chrono>
#include <algorithm>

#include "globals.h"
#include "unit_behav.h"
#include "utility.h"
#include "bc.hpp"
#include "scalar_field.h"

using namespace bc;

//TODO:
//Move units away from rockets as they launch
//Move units away from factories and try not to include factories in the walking distance fields
//Implement knight, ranger, mage, and healer micro

int32_t main()
{
	printf("Player C++ bot starting\n");
	printf("Connecting to manager...\n");
	// Most methods return pointers; methods returning integers or enums are the only exception.
	printf("Team: %d\n", gb::my_team);
	printf("Enemy Team: %d\n", gb::opposite_team);
	
	//fill up research
	gb::gc.queue_research(Worker); // 25
	//gb::gc.queue_research(Knight); // 25
	gb::gc.queue_research(Ranger); // 25
	gb::gc.queue_research(Healer); // 25
	gb::gc.queue_research(Healer); //100
	gb::gc.queue_research(Rocket); // 50
	gb::gc.queue_research(Healer); //100
	gb::gc.queue_research(Mage);   // 25
	gb::gc.queue_research(Mage);   // 75
	gb::gc.queue_research(Mage);   //100
	gb::gc.queue_research(Mage);   // 75
	//gb::gc.queue_research(Knight); // 75
	gb::gc.queue_research(Ranger); //100
	gb::gc.queue_research(Ranger); //200
	//then this is round 925; go for the finishing triple team snipe
	
	ScalarField::EARTH_PASSABLE.build_passable_field(Earth);
	ScalarField::MARS_PASSABLE .build_passable_field(Mars);
	ScalarField::EARTH_PASSABLE_PAD_1 = ScalarField::EARTH_PASSABLE.get_padded(1, 0);
	ScalarField::MARS_PASSABLE_PAD_1  = ScalarField::MARS_PASSABLE .get_padded(1, 0);
	ScalarField::EARTH_KARBONITE.build_karbonite_field(Earth);
	ScalarField::MARS_KARBONITE .build_karbonite_field(Mars);
	ScalarField::EARTH_INIT_UNITS.build_init_units_field(Earth, gb::my_team);
	//Mars can't have initial units
	
	gb::enemy_init_units = gb::gc.get_earth_map().get_initial_units();
	gb::enemy_init_units.erase(std::remove_if(gb::enemy_init_units.begin(), gb::enemy_init_units.end(), [](const bc::Unit& unit){return unit.get_team() == gb::my_team;}), gb::enemy_init_units.end());
	
	if(gb::gc.get_planet() == Earth)
		gb::mars_points = ScalarField::MARS_PASSABLE.get_points_if([](int value){return value != 0;});
	
	//ScalarField test = ScalarField::EARTH_PASSABLE_PAD_1;
	//test.build_padded_walking_distance_field(Earth, 14, 14);
	
	if(gb::gc.get_team() == Red && gb::gc.get_planet() == Earth)
	{
		//printf("Earth Passable:\n%s\nMars Passable:\n%s\n", ScalarField::EARTH_PASSABLE.to_string().c_str(), ScalarField::MARS_PASSABLE.to_string().c_str());
		//printf("Earth Karbonite:\n%s\nMars Karbonite:\n%s\n", ScalarField::EARTH_KARBONITE.to_string().c_str(), ScalarField::MARS_KARBONITE.to_string().c_str());
		//printf("Earth Init Units:\n%s\n", ScalarField::EARTH_INIT_UNITS.to_string().c_str());
	}
	
	int has_err = bc_has_err();
	if (has_err) {
		// If there was an error creating gb::gc, just die.
		printf("Failed, dying.\n");
		exit(1);
	}
	printf("Connected!\n");
	
	bool mars_exists = !ScalarField::MARS_PASSABLE.is_constant_field_with_value(-1);
	bool prev_worker_was_harvester = false;
	unit_behav::MegaStrategies dest_mega_strategy = unit_behav::MS_RANGER_RUSH;
	
	if(gb::gc.get_planet() == Mars)
		gb::mega_strategy = unit_behav::MS_RANGER_RUSH;
	else
		gb::mega_strategy = unit_behav::MS_KARBONITE_META;

	// loop through the whole game.
	while (true) {
		try{
			/*auto t0 = std::chrono::high_resolution_clock::now();
			for(int i = 0; i < 10000; ++i)
			{
				test.build_walking_distance_field(Earth, 14, 14);
			}
			auto t1 = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(t1 - t0);
			printf("Pathing time: %f seconds\n", duration.count());*/
			
			uint32_t round = gb::gc.get_round();
			printf("Round: %d, Karbonite: %d, Time Left: %d ms\n", round, gb::gc.get_karbonite(), gb::gc.get_time_left_ms());
			if(gb::gc.get_time_left_ms() < 300)
			{
				printf("Skipped\n");
				fflush(stdout);
				gb::gc.next_turn();
				continue;
			}
			
			
			/*if(round == 1)
			{
				gb::KNIGHT_RNG_LESS_THAN =  2000;
				gb::RANGER_RNG_LESS_THAN =  7000;
				gb::MAGE_RNG_LESS_THAN   =  7000;
				gb::HEALER_RNG_LESS_THAN = 10000;
				gb::WORKER_RNG_LESS_THAN = 10000;
			}
			else if(round == 1)
			{
				gb::KNIGHT_RNG_LESS_THAN =  1000;
				gb::RANGER_RNG_LESS_THAN =  8000;
				gb::MAGE_RNG_LESS_THAN   =  8000;
				gb::HEALER_RNG_LESS_THAN = 10000;
				gb::WORKER_RNG_LESS_THAN = 10000;
			}
			else if(round == 450)
			{
				gb::KNIGHT_RNG_LESS_THAN =  1000;
				gb::RANGER_RNG_LESS_THAN =  5000;
				gb::MAGE_RNG_LESS_THAN   =  7500;
				gb::HEALER_RNG_LESS_THAN = 10000;
				gb::WORKER_RNG_LESS_THAN = 10000;
			}*/
			
			
			
			if(gb::gc.get_planet() == Earth)
			{
				ScalarField::EARTH_SPREAD.build_earth_spread_field<1>();
				ScalarField::EARTH_ENEMY_SPREAD.build_enemy_spread_field<1>(Earth);
				//ScalarField::EARTH_SPREAD_PLUS_ENEMY_SPREAD = ScalarField::EARTH_SPREAD + ScalarField::EARTH_ENEMY_SPREAD;
				//ScalarField::EARTH_SPREAD_MINUS_ENEMY_SPREAD =
				//	ScalarField::EARTH_ENEMY_SPREAD.multiply_n_add_multiple_n_offset_new(3, ScalarField::EARTH_SPREAD, -1, 30000);
				//ScalarField::EARTH_SPREAD_MINUS_ENEMY_SPREAD.fill_impassable_this(ScalarField::EARTH_PASSABLE_PAD_1, -1);
				ScalarField::EARTH_ENEMY_ATTACK_RANGE.build_enemy_attack_range_field<1>(Earth);
				ScalarField::EARTH_KARBONITE.update_karbonite_field(Earth);
				
				auto karbonite_points = ScalarField::EARTH_KARBONITE.get_points_if([](int value){return value > 0;});
				ScalarField::EARTH_KARBONITE_DISTANCE.build_padded_multipoint_walking_distance_field<1, true>(Earth, karbonite_points);
				karbonite_points.erase(std::remove_if(karbonite_points.begin(), karbonite_points.end(), [](gb::int_pair coordinates)
				{
					bc::MapLocation map_location(Earth, coordinates.first, coordinates.second);
					if(!gb::gc.has_unit_at_location(map_location))
						return false;
					bc::Unit unit = gb::gc.sense_unit_at_location(map_location);
					return unit.get_team() == gb::my_team && unit.get_unit_type() == Worker;
				}), karbonite_points.end());
				ScalarField::EARTH_UNOCCUPIED_KARBONITE_DISTANCE.build_padded_multipoint_walking_distance_field<1, true>(Earth, karbonite_points);
				
				ScalarField::EARTH_UNBUILT_FACTORY_DISTANCE.build_unbuilt_factory_field<1, false>();
					
				if(round == 1 || gb::rocket_built_or_launched_this_turn)
				{
					ScalarField::EARTH_ROCKET_DISTANCE.build_rocket_distance_field<1>();
					//printf("Earth Rocket Distance, Round %d:\n%s\n", round, ScalarField::EARTH_ROCKET_DISTANCE.to_string(3).c_str());
				}
					
				if(round % 50 == 1)
				{
					//printf("Earth Unbuilt Factory Distance Field, Round %d:\n%s\n", round, ScalarField::EARTH_UNBUILT_FACTORY_DISTANCE.to_string().c_str());
					//printf("Earth Enemy Spread Field, Round %d:\n%s\n", round, ScalarField::EARTH_ENEMY_SPREAD.to_string().c_str());
					//printf("Earth Spread + Enemy Spread Field, Round %d:\n%s\n", round, ScalarField::EARTH_SPREAD_PLUS_ENEMY_SPREAD.to_string().c_str());
					//printf("Earth Spread - Enemy Spread Field, Round %d:\n%s\n", round, ScalarField::EARTH_SPREAD_MINUS_ENEMY_SPREAD.to_string(5).c_str());
					//printf("Earth Karbonite, Round %d:\n%s\n", round, ScalarField::EARTH_KARBONITE.to_string(3).c_str());
					printf("Earth Unoccupied Karbonite Distance, Round %d:\n%s\n", round, ScalarField::EARTH_UNOCCUPIED_KARBONITE_DISTANCE.to_string(3).c_str());
				}
				
				gb::enemy_init_units.erase(std::remove_if(gb::enemy_init_units.begin(), gb::enemy_init_units.end(), [](const bc::Unit& unit)
					{
						bc::MapLocation map_location = unit.get_map_location();
						return gb::gc.has_unit_at_location(map_location) && gb::gc.sense_unit_at_location(map_location).get_team() == gb::my_team;
					}),
					gb::enemy_init_units.end());
			}
			else if(round > 1)
			{
				ScalarField::MARS_ENEMY_SPREAD.build_enemy_spread_field<1>(Mars);
				auto prev_rocket_landings = gb::gc.get_rocket_landings().get_landings_on_round(round - 1);
				if(prev_rocket_landings.size() > 0)
				{
					gb::all_rocket_landings.insert(gb::all_rocket_landings.end(), prev_rocket_landings.begin(), prev_rocket_landings.end());
					ScalarField::MARS_SPREAD.build_mars_spread_field<1>();
					//printf("Mars Spread Field, Round %d:\n%s\n", round, ScalarField::MARS_SPREAD.to_string().c_str());
				}
				int sub_repl = -10000;
				int sub_offset = -1 - (sub_repl + 1);
				//ScalarField::MARS_SPREAD_PLUS_ENEMY_SPREAD = ScalarField::MARS_SPREAD + ScalarField::MARS_ENEMY_SPREAD;
				//ScalarField::MARS_SPREAD_MINUS_ENEMY_SPREAD =
				//	ScalarField::MARS_ENEMY_SPREAD.multiply_n_add_multiple_n_offset_new(3, ScalarField::MARS_SPREAD, -1, 30000);
				//ScalarField::MARS_SPREAD_MINUS_ENEMY_SPREAD.fill_impassable_this(ScalarField::MARS_PASSABLE_PAD_1, -1);
				ScalarField::MARS_ENEMY_ATTACK_RANGE.build_enemy_attack_range_field<1>(Mars);
				ScalarField::MARS_KARBONITE.update_karbonite_field(Mars);
				
				auto karbonite_points = ScalarField::MARS_KARBONITE.get_points_if([](int value){return value > 0;});
				ScalarField::MARS_KARBONITE_DISTANCE.build_padded_multipoint_walking_distance_field<1, true>(Mars, karbonite_points);
				karbonite_points.erase(std::remove_if(karbonite_points.begin(), karbonite_points.end(), [](gb::int_pair coordinates)
				{
					bc::MapLocation map_location(Mars, coordinates.first, coordinates.second);
					if(!gb::gc.has_unit_at_location(map_location))
						return false;
					bc::Unit unit = gb::gc.sense_unit_at_location(map_location);
					return unit.get_team() == gb::my_team && unit.get_unit_type() == Worker;
				}), karbonite_points.end());
				ScalarField::MARS_UNOCCUPIED_KARBONITE_DISTANCE.build_padded_multipoint_walking_distance_field<1, true>(Mars, karbonite_points);
					
				//if(round % 50 == 1)
				//{
				//	printf("Mars Karbonite, Round %d:\n%s\n", round, ScalarField::MARS_KARBONITE.to_string(3).c_str());
				//	printf("Mars Karbonite Distance, Round %d:\n%s\n", round, ScalarField::MARS_KARBONITE_DISTANCE.to_string(3).c_str());
				//}
			}
			
			const ScalarField& spread_field = gb::gc.get_planet() == Mars ? ScalarField::MARS_SPREAD : ScalarField::EARTH_SPREAD;
			auto units = gb::gc.get_my_units();
			std::sort(units.begin(), units.end(), [&spread_field](const Unit& u, const Unit& v)
			{
				if(!u.get_location().is_on_map() || !v.get_location().is_on_map())
					return false;
				const MapLocation& map_location_u = u.get_map_location();
				const MapLocation& map_location_v = v.get_map_location();
				return spread_field.field_at(map_location_u.get_x() + 1, map_location_u.get_y() + 1) < spread_field.field_at(map_location_v.get_x() + 1, map_location_v.get_y() + 1);
			});
			
			//Order of update: Rocket, Everything else, Healer, Factory
			//auto partition_middle = std::stable_partition(units.begin(), units.end(), [](const Unit& unit){return unit.get_unit_type() != Healer && unit.get_unit_type() != Factory;});
			//std::stable_partition(units.begin(), partition_middle, [](const Unit& unit){return unit.get_unit_type() == Rocket;});
			//std::stable_partition(partition_middle, units.end(), [](const Unit& unit){return unit.get_type() == Healer;});
			
			uint32_t worker_count = utility::count_team_workers();
			if(mars_exists && round < 750 && gb::gc.get_research_info().get_level(Rocket) >= 1 && worker_count > 0)
			{
				if(gb::mega_strategy == unit_behav::MS_ROCKET_MASS_LAUNCH)
				{
					if(worker_count >= 4)
						gb::save_karbonite_for_rocket = true;
					else
						gb::save_karbonite_for_rocket = false;
				}
				else
				{
					if(!gb::save_karbonite_for_rocket && gb::generator() % 15 == 0)
						gb::save_karbonite_for_rocket = true;
				
					//just in case the worker can't build a rocket for some other reason (blocked off, etc.)
					else if(gb::save_karbonite_for_rocket && gb::generator() % 10 == 0)
						gb::save_karbonite_for_rocket = false;
				}
			}
			else
				gb::save_karbonite_for_rocket = false; //can't build rockets on Mars, and Earth is flooded
			
			if(gb::gc.get_planet() == Earth)
			{
				int troop_count = std::count_if(units.begin(), units.end(), [](const bc::Unit& unit){return !unit.is_structure() && unit.get_unit_type() != Worker;});
				
				if((round >= 350 && gb::enemy_init_units.size() == 0) || (troop_count >= gb::MIN_TROOP_ROCKET_LAUNCH_INIT && round >= gb::TURN_ROCKET_LAUNCH_INIT) || round >= 650)
				{
					gb::mega_strategy = unit_behav::MS_ROCKET_MASS_LAUNCH;
					printf("Massively building rockets...\n");
				}
				
				if(gb::mega_strategy == unit_behav::MS_KARBONITE_META &&
					(round >= 25 || std::count_if(units.begin(), units.end(), [](const bc::Unit& unit){return unit.get_unit_type() == Factory;}) >= 2))
				{
					gb::mega_strategy = unit_behav::MS_RANGER_RUSH;
				}
				
				if(gb::mega_strategy == unit_behav::MS_KARBONITE_META)
				{
					gb::MIN_REPLICATE_COOLDOWN_1_WORKER = 2;
					gb::MAX_REPLICATE_COOLDOWN_1_WORKER = 2;
				}
				else
				{
					gb::MIN_REPLICATE_COOLDOWN_1_WORKER = 10;
					gb::MAX_REPLICATE_COOLDOWN_1_WORKER = 15;
				}
				
				printf("Current mega strategy: %d\n", int(gb::mega_strategy));
				
				gb::robot_relative_probabilities[int(Worker)] = 0;
				gb::robot_relative_probabilities[int(Knight)] = 0;
				gb::robot_relative_probabilities[int(Ranger)] = 0;
				gb::robot_relative_probabilities[int(Mage  )] = 0;
				gb::robot_relative_probabilities[int(Healer)] = 0;
				switch(gb::mega_strategy)
				{
					case unit_behav::MS_KARBONITE_META:
						gb::robot_relative_probabilities[int(Worker)] = 0;
						//gb::robot_relative_probabilities[int(Ranger)] = 1;
						break;
						
					case unit_behav::MS_KNIGHT_RUSH:
						gb::robot_relative_probabilities[int(Knight)] = 100;
						gb::robot_relative_probabilities[int(Ranger)] =  25;
						break;
						
					case unit_behav::MS_RANGER_RUSH:
						gb::robot_relative_probabilities[int(Knight)] =  10;
						gb::robot_relative_probabilities[int(Ranger)] = 100;
						break;
						
					case unit_behav::MS_MAGE_RUSH:
						gb::robot_relative_probabilities[int(Ranger)] =  25;
						gb::robot_relative_probabilities[int(Mage  )] = 100;
						break;
						
					case unit_behav::MS_KNIGHT_RANGER_MIX:
						gb::robot_relative_probabilities[int(Knight)] =  60;
						gb::robot_relative_probabilities[int(Ranger)] =  60;
						break;
						
					case unit_behav::MS_TRUE_VARIETY:
						gb::robot_relative_probabilities[int(Knight)] =  45;
						gb::robot_relative_probabilities[int(Ranger)] =  45;
						gb::robot_relative_probabilities[int(Mage  )] =  45;
						break;
						
					case unit_behav::MS_ROCKET_MASS_LAUNCH:
						gb::robot_relative_probabilities[int(Worker)] =   1;
						break;
				}
				if(gb::gc.get_research_info().get_level(Mage) >= 2 && (gb::mega_strategy == unit_behav::MS_RANGER_RUSH || gb::mega_strategy == unit_behav::MS_KNIGHT_RANGER_MIX))
					gb::robot_relative_probabilities[int(Mage  )] = 25;
				
				if(gb::mega_strategy != unit_behav::MS_KARBONITE_META && gb::mega_strategy != unit_behav::MS_ROCKET_MASS_LAUNCH)
				{
					//int fighter_count = std::count_if(units.begin(), units.end(), [](const bc::Unit& unit)
					//	{return !unit.is_structure() && unit.get_unit_type() != Worker && unit.get_unit_type() != Healer;});
					//
					//printf("Fighter Count: %d\n", fighter_count);
					//
					//if (fighter_count >= 2)
					//	gb::robot_relative_probabilities[int(Healer)] = 30;
				}
			}
			
			gb::rocket_built_or_launched_this_turn = false;
			for (const auto unit : units)
			{
				try
				{
					const uint32_t id = unit.get_id();
				
					if(gb::extra_unit_datas.find(id) == gb::extra_unit_datas.end())
					{
						ExtraUnitData data;
						if(unit.get_unit_type() == Knight || unit.get_unit_type() == Ranger || unit.get_unit_type() == Mage)
							data.state = unit_behav::ST_ATTACK;
						
						else if(unit.get_unit_type() == Healer)
							data.state = unit_behav::ST_HEAL;
						
						else if(unit.get_unit_type() == Worker)
						{
							data.state = gb::mega_strategy != unit_behav::MS_ROCKET_MASS_LAUNCH && ((round > 1 && !prev_worker_was_harvester) ||
								(gb::gc.get_planet() == Mars))
								? unit_behav::ST_HARVEST
								: unit_behav::ST_BUILD;
							if(round > 1)
								prev_worker_was_harvester = !prev_worker_was_harvester;
							
							data.replicate_cooldown = round == 1 ? 0 :
								std::uniform_int_distribution<uint32_t>(gb::MIN_REPLICATE_COOLDOWN_1_WORKER * worker_count,
																		gb::MAX_REPLICATE_COOLDOWN_1_WORKER * worker_count)(gb::generator);
						}
						
						else if(unit.get_unit_type() == Factory)
							data.state = unit_behav::ST_UNLOAD;
						
						else if(unit.get_unit_type() == Rocket)
							data.state = unit_behav::ST_LAUNCH;
						
						ScalarField& field = gb::gc.get_planet() == Mars ? ScalarField::MARS_PASSABLE_PAD_1 : ScalarField::EARTH_PASSABLE_PAD_1;
						gb::extra_unit_datas[id] = std::move(data);
					}
					
					ExtraUnitData& extra_unit_data = gb::extra_unit_datas[id];
					
					if(extra_unit_data.state != -1)
						(*unit_behav::states[extra_unit_data.state].main)(unit);
				}
				catch(std::exception ex)
				{
					printf("%s", ex.what());
				}
			}
			
			if(gb::mega_strategy != unit_behav::MS_ROCKET_MASS_LAUNCH && gb::gc.get_planet() != Mars)
			{
				auto workers = units;
				workers.erase(std::remove_if(workers.begin(), workers.end(), [](const bc::Unit& unit){return unit.get_unit_type() != Worker;}), workers.end());
				//std::stable_sort(workers.begin(), workers.end(),
				//	[](const bc::Unit& a, const bc::Unit& b){return a.get_ability_heat() < b.get_ability_heat();});
				
				auto halfway = std::stable_partition(workers.begin(), workers.end(), [](const bc::Unit& unit){return unit.get_ability_heat() <= 30;});
				
				//int halfway = (workers.size() + 1) / 2;
				std::for_each(workers.begin(), halfway, [](const bc::Unit& unit){gb::extra_unit_datas[unit.get_id()].state = unit_behav::ST_BUILD;});
				std::for_each(halfway,   workers.end(), [](const bc::Unit& unit){gb::extra_unit_datas[unit.get_id()].state = unit_behav::ST_HARVEST;});
			}
			
			
			// this line helps the output logs make more sense by forcing output to be sent
			// to the manager.
			// it's not strictly necessary, but it helps.
			// pause and wait for the next turn.
			fflush(stdout);
			gb::gc.next_turn();
		} 
		catch(std::exception ex)
		{
			printf("%s", ex.what());
		}
	}
	// I'm convinced C++ is the better option :)
}
