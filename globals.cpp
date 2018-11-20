#include "globals.h"
#include "utility.h"
#include "scalar_field.h"

namespace gb
{
	uint32_t MIN_REPLICATE_COOLDOWN_1_WORKER = 10;
	uint32_t MAX_REPLICATE_COOLDOWN_1_WORKER = 15;
	
	std::default_random_engine generator;
	std::uniform_int_distribution<int32_t> distribution_0_to_9 (0,NUM_DIRECTIONS - 1);
	std::uniform_int_distribution<int32_t> distribution_0_to_8 (0,NUM_NONZERO_DIRECTIONS - 1);
	decltype(std::bind(distribution_0_to_9, generator)) direction_dice         = std::bind(distribution_0_to_9, generator);
	decltype(std::bind(distribution_0_to_8, generator)) nonzero_direction_dice = std::bind(distribution_0_to_8, generator);
	
	std::array<int, 5> robot_relative_probabilities {0, 0, 0, 0, 0};
	
	unit_behav::MegaStrategies mega_strategy = unit_behav::MS_NONE;
	
	bc::GameController gc;
	bc::Team my_team = gc.get_team();
	bc::Team opposite_team = utility::opposite_team(my_team);
	
	std::vector<std::pair<int, int>> mars_points;
	std::vector<bc::Unit> enemy_init_units;
	
	bool save_karbonite_for_rocket = false;
	bool rocket_built_or_launched_this_turn = false;
	
	int num_production = 0;
	
	std::vector<bc::RocketLanding> all_rocket_landings;
	
	std::map<uint32_t, ExtraUnitData> extra_unit_datas;
}