#include "scalar_field.h"
#include "globals.h"
#include <sstream>
#include <iomanip>
#include <array>
#include <utility>
#include <algorithm>
#include <exception>
#include <stdexcept>

ScalarField ScalarField::EARTH_PASSABLE(gb::gc.get_earth_map().get_width(), gb::gc.get_earth_map().get_height());
ScalarField ScalarField::MARS_PASSABLE (gb::gc.get_mars_map ().get_width(), gb::gc.get_mars_map ().get_height());

ScalarField ScalarField::EARTH_PASSABLE_PAD_1(0, 0); //will be initialized later
ScalarField ScalarField::MARS_PASSABLE_PAD_1(0, 0);

ScalarField ScalarField::EARTH_KARBONITE(gb::gc.get_earth_map().get_width(), gb::gc.get_earth_map().get_height());
ScalarField ScalarField::MARS_KARBONITE (gb::gc.get_mars_map ().get_width(), gb::gc.get_mars_map ().get_height());

ScalarField ScalarField::EARTH_INIT_UNITS(gb::gc.get_earth_map().get_width(), gb::gc.get_earth_map().get_height());

ScalarField ScalarField::EARTH_ROCKET_DISTANCE = ScalarField::EARTH_PASSABLE.get_padded(1, -1);

ScalarField ScalarField::EARTH_KARBONITE_DISTANCE = ScalarField::EARTH_PASSABLE.get_padded(1, -1);
ScalarField ScalarField::MARS_KARBONITE_DISTANCE  = ScalarField::MARS_PASSABLE .get_padded(1, -1);

ScalarField ScalarField::EARTH_UNOCCUPIED_KARBONITE_DISTANCE = ScalarField::EARTH_PASSABLE.get_padded(1, -1);
ScalarField ScalarField::MARS_UNOCCUPIED_KARBONITE_DISTANCE  = ScalarField::MARS_PASSABLE .get_padded(1, -1);

ScalarField ScalarField::EARTH_SPREAD = ScalarField::EARTH_PASSABLE.get_padded(1, -1);
ScalarField ScalarField::MARS_SPREAD  = ScalarField::MARS_PASSABLE .get_padded(1, -1);

ScalarField ScalarField::EARTH_UNBUILT_FACTORY_DISTANCE = ScalarField::EARTH_PASSABLE.get_padded(1, -1);

ScalarField ScalarField::EARTH_ENEMY_SPREAD = ScalarField::EARTH_PASSABLE.get_padded(1, -1);
ScalarField ScalarField::MARS_ENEMY_SPREAD  = ScalarField::MARS_PASSABLE .get_padded(1, -1);

ScalarField ScalarField::EARTH_SPREAD_PLUS_ENEMY_SPREAD = ScalarField::EARTH_PASSABLE.get_padded(1, -1);
ScalarField ScalarField::MARS_SPREAD_PLUS_ENEMY_SPREAD  = ScalarField::MARS_PASSABLE .get_padded(1, -1);

ScalarField ScalarField::EARTH_SPREAD_MINUS_ENEMY_SPREAD = ScalarField::EARTH_PASSABLE.get_padded(1, -1);
ScalarField ScalarField::MARS_SPREAD_MINUS_ENEMY_SPREAD  = ScalarField::MARS_PASSABLE .get_padded(1, -1);

ScalarField ScalarField::EARTH_ENEMY_ATTACK_RANGE = ScalarField::EARTH_PASSABLE.get_padded(1, 0);
ScalarField ScalarField::MARS_ENEMY_ATTACK_RANGE  = ScalarField::MARS_PASSABLE .get_padded(1, 0);

bool ScalarField::UNBUILT_FACTORIES_EXIST = false;

namespace
{
	
	std::array<gb::int_pair, gb::MAX_MAP_SIZE * gb::MAX_MAP_SIZE + 1> expand_points_0;
	std::array<gb::int_pair, gb::MAX_MAP_SIZE * gb::MAX_MAP_SIZE + 1> expand_points_1;
	
	
}

void ScalarField::build_passable_field(bc::Planet planet)
{
	const bc::PlanetMap& planet_map = planet == Mars ? gb::gc.get_mars_map() : gb::gc.get_earth_map();
	bc::MapLocation map_location(planet, 0, 0);
	
	for(int y = 0; y < _height; ++y)
		for(int x = 0; x < _width; ++x)
			//map_location.set_x(x);
			//map_location.set_y(y);
			//printf("Planet: %d, Position: (%2d, %2d), Passable: %d\n", map_location.get_planet(), map_location.get_x(), map_location.get_y(), planet_map.is_passable_terrain_at(map_location));
			if(planet_map.is_passable_terrain_at(bc::MapLocation(planet, x, y)))
				field_at(x, y) = 1;
			else
				field_at(x, y) = 0;
		
}

void ScalarField::build_karbonite_field(bc::Planet planet)
{
	const bc::PlanetMap& planet_map = planet == Mars ? gb::gc.get_mars_map() : gb::gc.get_earth_map();
	bc::MapLocation map_location(planet, 0, 0);
	
	for(int y = 0; y < _height; ++y)
		for(int x = 0; x < _width; ++x)
			field_at(x, y) = planet_map.get_initial_karbonite_at(bc::MapLocation(planet, x, y));
}

void ScalarField::update_karbonite_field(bc::Planet planet)
{
	const bc::PlanetMap& planet_map = planet == Mars ? gb::gc.get_mars_map() : gb::gc.get_earth_map();
	
	for(int y = 0; y < _height; ++y)
		for(int x = 0; x < _width; ++x)
		{
			bc::MapLocation map_location(planet, x, y);
			if(gb::gc.can_sense_location(map_location))
				field_at(x, y) = gb::gc.get_karbonite_at(map_location);
		}
}

void ScalarField::build_init_units_field(bc::Planet planet, bc::Team team)
{
	const bc::PlanetMap& planet_map = planet == Mars ? gb::gc.get_mars_map() : gb::gc.get_earth_map();
	std::fill(_field.begin(), _field.end(), 0);
	
	auto initial_units = planet_map.get_initial_units();
	for(const bc::Unit& unit : initial_units)
		field_at(unit.get_map_location().get_x(), unit.get_map_location().get_y()) = unit.get_team() == team ? 1 : -1;
}

ScalarField ScalarField::get_padded(int pad_size, int pad_value) const
{
	ScalarField result(_width + 2 * pad_size, _height + 2 * pad_size, pad_value);
	
	for(int y = pad_size; y < _height + pad_size; ++y)
		std::copy_n(_field.begin() + get_index(0, y - pad_size), _width, result._field.begin() + result.get_index(pad_size, y));
	
	return result;
}

template<int Padding, bool Taxicab>
void ScalarField::build_padded_walking_distance_field_internal(bc::Planet planet, int start_walking_distance)
{
	const ScalarField& padded_planet_field = planet == Mars ? MARS_PASSABLE_PAD_1 : EARTH_PASSABLE_PAD_1;
	auto curr_list = &expand_points_0[0];
	auto next_list = &expand_points_1[0];
	int walking_distance = start_walking_distance;
	
	while(true)
	{
		bool continue_expansion = false;
		
		for(int point_id = 0; curr_list[point_id].first >= 0; ++point_id)
		{
			bool found_point = false;
			for(int dir_id = 0; dir_id < gb::NUM_NONZERO_DIRECTIONS; dir_id += Taxicab ? 2 : 1)
			{
				int x = curr_list[point_id].first  + gb::nonzero_dir_array[dir_id].first;
				int y = curr_list[point_id].second + gb::nonzero_dir_array[dir_id].second;
				
				if(field_at(x + Padding, y + Padding) != -1)
				{
					if(field_at(x + Padding, y + Padding) >= walking_distance)
						found_point = true;
				}
				else if(padded_planet_field.field_at(x + 1, y + 1) != 0)
				{
					field_at(x + Padding, y + Padding) = walking_distance;
					*next_list = gb::int_pair(x, y);
					++next_list;
					continue_expansion = true;
					found_point = true;
				}
				
				if(Taxicab && dir_id == 6 && !found_point)
					dir_id = -1;
			}
		}
		
		if(!continue_expansion)
			break;
		
		next_list->first = -1;
		next_list = curr_list == &expand_points_0[0] ? &expand_points_0[0] : &expand_points_1[0];
		curr_list = curr_list == &expand_points_0[0] ? &expand_points_1[0] : &expand_points_0[0];
		++walking_distance;
	}
}

template<int Padding, bool Taxicab>
void ScalarField::build_padded_walking_distance_field(bc::Planet planet, int pos_x, int pos_y)
{
	std::fill(_field.begin(), _field.end(), -1);
	
	expand_points_0[0] = gb::int_pair(pos_x, pos_y);
	expand_points_0[1].first = -1;
	field_at(pos_x + Padding, pos_y + Padding) = 0;
	
	build_padded_walking_distance_field_internal<Padding, Taxicab>(planet, 1);
}

template<int Padding, bool Taxicab>
void ScalarField::build_padded_multipoint_walking_distance_field(bc::Planet planet, const std::vector<gb::int_pair>& points)
{
	std::fill(_field.begin(), _field.end(), -1);
	
	auto end_it = std::copy(points.cbegin(), points.cend(), expand_points_0.begin());
	end_it->first = -1;
	std::for_each(points.begin(), points.end(), [this](gb::int_pair pair){this->field_at(pair.first + Padding, pair.second + Padding) = 0;});
	
	build_padded_walking_distance_field_internal<Padding, Taxicab>(planet, 1);
}

template<int Padding, bool Taxicab>
void ScalarField::build_unbuilt_factory_field()
{
	auto factories = gb::gc.get_my_units();
	factories.erase(std::remove_if(factories.begin(), factories.end(),
		[](const bc::Unit& unit){return unit.get_unit_type() != Factory || unit.structure_is_built();}), factories.end());
		
	UNBUILT_FACTORIES_EXIST = factories.size() > 0;
	
	std::vector<gb::int_pair> points(factories.size());
	std::transform(factories.begin(), factories.end(), points.begin(), [](const bc::Unit& unit)
		{return gb::int_pair(unit.get_map_location().get_x(), unit.get_map_location().get_y());});
	
	build_padded_multipoint_walking_distance_field<Padding, Taxicab>(Earth, points);
}

template<int Padding, bool Taxicab>
void ScalarField::build_earth_spread_field()
{
	auto factories = gb::gc.get_my_units();
	factories.erase(std::remove_if(factories.begin(), factories.end(), [](const bc::Unit& unit){return unit.get_unit_type() != Factory;}), factories.end());
	
	std::vector<gb::int_pair> points(factories.size());
	std::transform(factories.begin(), factories.end(), points.begin(), [](const bc::Unit& unit)
		{return gb::int_pair(unit.get_map_location().get_x(), unit.get_map_location().get_y());});
	
	build_padded_multipoint_walking_distance_field<Padding, Taxicab>(Earth, points);
}

template<int Padding, bool Taxicab>
void ScalarField::build_mars_spread_field()
{
	auto rocket_landings = gb::all_rocket_landings;
	
	std::vector<gb::int_pair> points(rocket_landings.size());
	std::transform(rocket_landings.begin(), rocket_landings.end(), points.begin(), [](const bc::RocketLanding& landing)
		{return gb::int_pair(landing.get_destination().get_x(), landing.get_destination().get_y());});
		
	build_padded_multipoint_walking_distance_field<Padding, Taxicab>(Mars, points);
}

template<int Padding, bool Taxicab>
void ScalarField::build_enemy_spread_field(bc::Planet planet)
{
	auto enemy_units = planet == Mars ? gb::gc.get_mars_map().get_initial_units() : gb::enemy_init_units;
	auto visible_units = gb::gc.get_units();
	enemy_units.insert(enemy_units.begin(), visible_units.begin(), visible_units.end());
	enemy_units.erase(std::remove_if(enemy_units.begin(), enemy_units.end(), [](const bc::Unit& unit)
		{return unit.get_team() == gb::my_team || !unit.get_location().is_on_map();}), enemy_units.end());
	
	std::vector<gb::int_pair> points(enemy_units.size());
	std::transform(enemy_units.begin(), enemy_units.end(), points.begin(), [](const bc::Unit& unit)
		{return gb::int_pair(unit.get_map_location().get_x(), unit.get_map_location().get_y());});
	
	build_padded_multipoint_walking_distance_field<Padding, Taxicab>(planet, points);
}

template<int Padding, bool Taxicab>
void ScalarField::build_rocket_distance_field()
{
	auto rockets = gb::gc.get_my_units();
	rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const bc::Unit& unit){return unit.get_unit_type() != Rocket;}), rockets.end());
	
	std::vector<gb::int_pair> points(rockets.size());
	std::transform(rockets.begin(), rockets.end(), points.begin(), [](const bc::Unit& unit)
		{return gb::int_pair(unit.get_map_location().get_x(), unit.get_map_location().get_y());});
	
	build_padded_multipoint_walking_distance_field<Padding, Taxicab>(Earth, points);
}

template<int Padding>
void ScalarField::build_enemy_attack_range_field(bc::Planet planet)
{
	std::fill(_field.begin(), _field.end(), 0);
	
	auto enemy_units = gb::gc.get_units();
	enemy_units.erase(std::remove_if(enemy_units.begin(), enemy_units.end(), [](const bc::Unit& unit)
		{return unit.get_team() == gb::my_team || !unit.get_location().is_on_map();}), enemy_units.end());
	
	std::sort(enemy_units.begin(), enemy_units.end(), [](const bc::Unit& a, const bc::Unit& b){return int(a.get_unit_type()) < int(b.get_unit_type());});
	
	auto begin_it = enemy_units.begin();
	for(int i = 0; i < 7; ++i)
	{
		bc::UnitType unit_type = bc::UnitType(i);
		int range = unit_type == Rocket ? 2 : unit_type == Factory ? 0 : -1;
		
		auto end_it = std::find_if(begin_it, enemy_units.end(), [i](const bc::Unit& unit){return int(unit.get_unit_type()) > i;});
		//if(gb::gc.get_round() % 50 == 1)
		//	printf("Unit type %d, Count %d\n", unit_type, int(end_it - begin_it));
		
		std::for_each(begin_it, end_it, [this, range, i](const bc::Unit& unit)
			{this->or_circle(unit.get_map_location().get_x() + Padding, unit.get_map_location().get_y() + Padding, range >= 0 ? range : unit.get_attack_range(), 1 << i);});
		begin_it = end_it;
	}
	
}

template void ScalarField::build_padded_walking_distance_field_internal<0, false>(bc::Planet planet, int start_walking_distance);
template void ScalarField::build_padded_walking_distance_field_internal<1, false>(bc::Planet planet, int start_walking_distance);
template void ScalarField::build_padded_walking_distance_field_internal<0, true>(bc::Planet planet, int start_walking_distance);
template void ScalarField::build_padded_walking_distance_field_internal<1, true>(bc::Planet planet, int start_walking_distance);

template void ScalarField::build_padded_walking_distance_field<0, false>(bc::Planet planet, int pos_x, int pos_y);
template void ScalarField::build_padded_walking_distance_field<1, false>(bc::Planet planet, int pos_x, int pos_y);
template void ScalarField::build_padded_walking_distance_field<0, true>(bc::Planet planet, int pos_x, int pos_y);
template void ScalarField::build_padded_walking_distance_field<1, true>(bc::Planet planet, int pos_x, int pos_y);

template void ScalarField::build_padded_multipoint_walking_distance_field<0, false>(bc::Planet planet, const std::vector<std::pair<int, int>>& points);
template void ScalarField::build_padded_multipoint_walking_distance_field<1, false>(bc::Planet planet, const std::vector<std::pair<int, int>>& points);
template void ScalarField::build_padded_multipoint_walking_distance_field<0, true>(bc::Planet planet, const std::vector<std::pair<int, int>>& points);
template void ScalarField::build_padded_multipoint_walking_distance_field<1, true>(bc::Planet planet, const std::vector<std::pair<int, int>>& points);

template void ScalarField::build_unbuilt_factory_field<0, false>();
template void ScalarField::build_unbuilt_factory_field<1, false>();
template void ScalarField::build_unbuilt_factory_field<0, true>();
template void ScalarField::build_unbuilt_factory_field<1, true>();

template void ScalarField::build_earth_spread_field<0, false>();
template void ScalarField::build_earth_spread_field<1, false>();
template void ScalarField::build_earth_spread_field<0, true>();
template void ScalarField::build_earth_spread_field<1, true>();

template void ScalarField::build_mars_spread_field<0, false>();
template void ScalarField::build_mars_spread_field<1, false>();
template void ScalarField::build_mars_spread_field<0, true>();
template void ScalarField::build_mars_spread_field<1, true>();

template void ScalarField::build_rocket_distance_field<0, false>();
template void ScalarField::build_rocket_distance_field<1, false>();
template void ScalarField::build_rocket_distance_field<0, true>();
template void ScalarField::build_rocket_distance_field<1, true>();

template void ScalarField::build_enemy_spread_field<0, false>(bc::Planet planet);
template void ScalarField::build_enemy_spread_field<1, false>(bc::Planet planet);
template void ScalarField::build_enemy_spread_field<0, true>(bc::Planet planet);
template void ScalarField::build_enemy_spread_field<1, true>(bc::Planet planet);

template void ScalarField::build_enemy_attack_range_field<0>(bc::Planet planet);
template void ScalarField::build_enemy_attack_range_field<1>(bc::Planet planet);

void ScalarField::operator+=(const ScalarField& other)
{
	if(_width != other._width || _height != other._height)
		throw std::domain_error(std::string("ScalarField widths and heights must equal for operator+=.\n") +
			"SF1 Width: " + std::to_string(_width)  + ", SF2 Width: " + std::to_string(other._width) +
			"\nSF1 Height: " + std::to_string(_height)  + ", SF2 Height: " + std::to_string(other._height));
			
	std::transform(other._field.begin(), other._field.end(), _field.begin(), _field.begin(), [](int a, int b){return a + b;});
}

ScalarField ScalarField::operator+(const ScalarField& other) const
{
	ScalarField res = *this;
	res += other;
	return res;
}

void ScalarField::operator-=(const ScalarField& other)
{
	if(_width != other._width || _height != other._height)
		throw std::domain_error(std::string("ScalarField widths and heights must equal for operator-=.\n") +
			"SF1 Width: " + std::to_string(_width)  + ", SF2 Width: " + std::to_string(other._width) +
			"\nSF1 Height: " + std::to_string(_height)  + ", SF2 Height: " + std::to_string(other._height));
			
	std::transform(other._field.begin(), other._field.end(), _field.begin(), _field.begin(), [](int a, int b){return a - b;});
}

ScalarField ScalarField::operator-(const ScalarField& other) const
{
	ScalarField res = *this;
	res -= other;
	return res;
}

void ScalarField::replace_this(int old_value, int new_value)
{	
	std::replace(_field.begin(), _field.end(), old_value, new_value);
}

ScalarField ScalarField::replace_new(int old_value, int new_value) const
{	
	ScalarField res = *this;
	res.replace_this(old_value, new_value);
	return res;
}

void ScalarField::fill_impassable_this(const ScalarField& passable_field, int value_if_impassable)
{
	if(_width != passable_field._width || _height != passable_field._height)
		throw std::domain_error(std::string("ScalarField widths and heights must equal for fill_impassable_this.\n") +
			"SF1 Width: " + std::to_string(_width)  + ", SF2 Width: " + std::to_string(passable_field._width) +
			"\nSF1 Height: " + std::to_string(_height)  + ", SF2 Height: " + std::to_string(passable_field._height));
			
	std::transform(_field.begin(), _field.end(), passable_field._field.begin(), _field.begin(), [value_if_impassable](int a, int b){return b != 0 ? a : value_if_impassable;});
}

bool ScalarField::is_constant_field_with_value(int value) const
{
	return std::all_of(_field.begin(), _field.end(), [value](int x){return x == value;});
}

void ScalarField::subtract_n_offset_this(const ScalarField& sub, int offset)
{
	if(_width != sub._width || _height != sub._height)
		throw std::domain_error(std::string("ScalarField widths and heights must equal for subtract_n_offset_this.\n") +
			"SF1 Width: " + std::to_string(_width)  + ", SF2 Width: " + std::to_string(sub._width) +
			"\nSF1 Height: " + std::to_string(_height)  + ", SF2 Height: " + std::to_string(sub._height));
			
	std::transform(_field.begin(), _field.end(), sub._field.begin(), _field.begin(), [offset](int a, int b){return a - b + offset;});
}

ScalarField ScalarField::subtract_n_offset_new(const ScalarField& sub, int offset) const
{
	ScalarField res = *this;
	res.subtract_n_offset_this(sub, offset);
	return res;
}

void ScalarField::multiply_n_add_multiple_n_offset_this(int mul_this, const ScalarField& add, int mul_add, int offset)
{
	if(_width != add._width || _height != add._height)
		throw std::domain_error(std::string("ScalarField widths and heights must equal for multiply_n_add_multiple_n_offset_this.\n") +
			"SF1 Width: " + std::to_string(_width)  + ", SF2 Width: " + std::to_string(add._width) +
			"\nSF1 Height: " + std::to_string(_height)  + ", SF2 Height: " + std::to_string(add._height));
			
	std::transform(_field.begin(), _field.end(), add._field.begin(), _field.begin(),
		[mul_this, mul_add, offset](int a, int b){return a * mul_this + b * mul_add + offset;});
}

void ScalarField::or_circle(int x, int y, int radius_squared, int value)
{
	int bottom = std::max(y - (int)std::sqrt(radius_squared), 0);
	int top = std::min(y + (int)std::sqrt(radius_squared), _height - 1);
	for(int curr_y = bottom; curr_y <= top; ++curr_y)
	{
		int left = std::max(x - (int)std::sqrt(radius_squared - (curr_y - y) * (curr_y - y)), 0);
		int right = std::min(x + (int)std::sqrt(radius_squared - (curr_y - y) * (curr_y - y)), _width - 1);
		
		std::transform(_field.begin() + get_index(left, curr_y), _field.begin() + get_index(right + 1, curr_y), _field.begin() + get_index(left, curr_y),
			[value](int eger){return eger | value;});
	}
}

ScalarField ScalarField::multiply_n_add_multiple_n_offset_new(int mul_this, const ScalarField& add, int mul_add, int offset) const
{
	ScalarField res = *this;
	res.multiply_n_add_multiple_n_offset_this(mul_this, add, mul_add, offset);
	return res;
}

std::string ScalarField::to_string(int tile_width)
{
	std::ostringstream stream;
	for(int y = _height - 1; y >= 0; --y)
	{
		for(int x = 0; x < _width; ++x)
			stream << std::setw(tile_width) << field_at(x, y);
		stream << std::endl;
	}
	return stream.str();
}