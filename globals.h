#pragma once
#include "bc.hpp"
#include <map>
#include <random>
#include <array>
#include "scalar_field.h"
#include <memory>

namespace unit_behav
{
	struct State
	{
		void(*main)(const bc::Unit& unit);
		bool(*please_move)(const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth);
	};
	
	void st_attack_main (const bc::Unit& unit);
	void st_defend_main (const bc::Unit& unit);
	void st_heal_main   (const bc::Unit& unit);
	void st_build_main  (const bc::Unit& unit);
	void st_harvest_main(const bc::Unit& unit);
	void st_unload_main (const bc::Unit& unit);
	void st_launch_main (const bc::Unit& unit);
	
	bool st_attack_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	bool st_defend_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	bool st_heal_please_move   (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	bool st_build_please_move  (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	bool st_harvest_please_move(const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	bool st_unload_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	bool st_launch_please_move (const bc::Unit& unit, const bc::Unit& requester, bc::Direction requester_move_dir, int stack_depth = 0);
	
	enum States
	{
		ST_ATTACK,
		ST_DEFEND,
		ST_HEAL,
		ST_BUILD,
		ST_HARVEST,
		ST_UNLOAD,
		ST_LAUNCH,
		
		NUM_STATES
	};
	
	const std::array<State, NUM_STATES> states
	{
		State{&st_attack_main , &st_attack_please_move },
		State{&st_defend_main , &st_defend_please_move },
		State{&st_heal_main   , &st_heal_please_move   },
		State{&st_build_main  , &st_build_please_move  },
		State{&st_harvest_main, &st_harvest_please_move},
		State{&st_unload_main , &st_unload_please_move },
		State{&st_launch_main , &st_launch_please_move }
	};
	
	enum MegaStrategies
	{
		MS_NONE,
		MS_KARBONITE_META,
		MS_KNIGHT_RUSH,
		MS_RANGER_RUSH,
		MS_KNIGHT_RANGER_MIX,
		MS_TRUE_VARIETY,
		MS_MAGE_RUSH,
		MS_ROCKET_MASS_LAUNCH
	};
}

namespace gb
{
	extern uint32_t MIN_REPLICATE_COOLDOWN_1_WORKER;
	extern uint32_t MAX_REPLICATE_COOLDOWN_1_WORKER;
}

struct ExtraUnitData
{
	unit_behav::States state;
	uint32_t replicate_cooldown; //for workers
	uint32_t blueprint_cycle; //for workers
	std::vector<bc::UnitType> garrison_unit_types; //for rockets
	
	ExtraUnitData() : state(unit_behav::States(-1)), replicate_cooldown(0), blueprint_cycle(0), garrison_unit_types() {}
};

namespace gb
{
	constexpr int32_t NUM_NONZERO_DIRECTIONS = 8;
	constexpr int32_t NUM_DIRECTIONS = 9;
	constexpr int32_t MAX_MAP_SIZE = 50;
	
	constexpr int32_t TURN_ROCKET_LAUNCH_INIT = 550;
	constexpr int32_t TURN_ROCKET_COORDINATED_LAUNCH = 700;
	constexpr int32_t MIN_TROOP_ROCKET_LAUNCH_INIT = 35;
	
	extern std::vector<bc::RocketLanding> all_rocket_landings;
	
	//using move_validation_func = std::function<bool, const bc::Unit&, bc::Direction dir>;
	using int_pair = std::pair<int, int>;
	constexpr std::array<int_pair, gb::NUM_NONZERO_DIRECTIONS> nonzero_dir_array
	{
		int_pair( 0,  1),
		int_pair( 1,  1),
		int_pair( 1,  0),
		int_pair( 1, -1),
		int_pair( 0, -1),
		int_pair(-1, -1),
		int_pair(-1,  0),
		int_pair(-1,  1)
	};
	extern std::vector<std::pair<int, int>> mars_points;
	
	extern unit_behav::MegaStrategies mega_strategy;
	
	extern std::array<int, 5> robot_relative_probabilities;
	extern std::vector<bc::Unit> enemy_init_units;
	
	constexpr std::array<bc::UnitType, 10> FACTORY_SPAWN_DISTRIBUTION
	{
		Knight, Knight, Knight, Knight, Knight,
		//Ranger, Ranger, Ranger, Ranger, Ranger,
		//Mage  , Mage  , Mage  ,
		Healer, Healer, Healer, Healer, Healer,
		//Worker, Worker
	};
	
	extern std::default_random_engine generator;
	extern std::uniform_int_distribution<int32_t> distribution_0_to_9;
	extern std::uniform_int_distribution<int32_t> distribution_0_to_8;
	extern decltype(std::bind(distribution_0_to_9, generator)) direction_dice;
	extern decltype(std::bind(distribution_0_to_8, generator)) nonzero_direction_dice;
	
	extern int num_production;
	
	extern bc::GameController gc;
	extern bc::Team my_team;
	extern bc::Team opposite_team;
	
	extern bool save_karbonite_for_rocket;
	extern bool rocket_built_or_launched_this_turn;
	
	extern std::map<uint32_t, ExtraUnitData> extra_unit_datas;
}