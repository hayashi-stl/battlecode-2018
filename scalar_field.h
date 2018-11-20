#pragma once
#include "bc.hpp"
#include <vector>
#include <string>
#include <utility>

struct ScalarField
{
public:
	inline ScalarField(int the_width, int the_height) : _width(the_width), _height(the_height), _field(the_width * the_height) {}
	inline ScalarField(int the_width, int the_height, int init_value) : _width(the_width), _height(the_height), _field(the_width * the_height, init_value) {}
	ScalarField(const ScalarField& the_field) = default;
	ScalarField(ScalarField&& the_field) = default;
	ScalarField& operator=(const ScalarField& the_field) = default;
	inline ScalarField& operator=(ScalarField&& the_field) = default;
	
	inline int field_at(int x, int y) const {return _field[y * _width + x];}
	inline int& field_at(int x, int y) {return _field[y * _width + x];}
	void build_passable_field(bc::Planet planet);
	void build_karbonite_field(bc::Planet planet);
	void update_karbonite_field(bc::Planet planet);
	void build_init_units_field(bc::Planet planet, bc::Team team); //1 represents ally, -1 represents enemy
	ScalarField get_padded(int pad_size, int pad_value) const;
	
	template<int Padding, bool Taxicab = true>
	void build_padded_walking_distance_field(bc::Planet planet, int pos_x, int pos_y);
	template<int Padding, bool Taxicab = true>
	inline void build_padded_walking_distance_field(bc::MapLocation map_location)
		{build_padded_walking_distance_field<Padding, Taxicab>(map_location.get_planet(), map_location.get_x(), map_location.get_y());}
	template<int Padding, bool Taxicab = true>
	void build_padded_multipoint_walking_distance_field(bc::Planet planet, const std::vector<std::pair<int, int>>& points);
	
	template<int Padding, bool Taxicab = true>
	void build_unbuilt_factory_field();
	template<int Padding, bool Taxicab = true>
	void build_earth_spread_field();
	template<int Padding, bool Taxicab = true>
	void build_mars_spread_field();
	template<int Padding, bool Taxicab = true>
	void build_rocket_distance_field();
	template<int Padding, bool Taxicab = true>
	void build_enemy_spread_field(bc::Planet planet);
	template<int Padding>
	void build_enemy_attack_range_field(bc::Planet planet);
	
	template<class Predicate>
	std::vector<std::pair<int, int>> get_points_if(Predicate pred)
	{
		std::vector<std::pair<int, int>> res;
		
		for(int y = 0; y < _height; ++y)
			for(int x = 0; x < _width; ++x)
				if(pred(field_at(x, y)))
					res.push_back(std::pair<int, int>(x, y));
				
		return res;
	}
	
	void operator+=(const ScalarField& other);
	ScalarField operator+(const ScalarField& other) const;
	void operator-=(const ScalarField& other);
	ScalarField operator-(const ScalarField& other) const;
	
	void replace_this(int old_value, int new_value);
	ScalarField replace_new(int old_value, int new_value) const;
	void fill_impassable_this(const ScalarField& passable_field, int value_if_impassable);
	
	bool is_constant_field_with_value(int value) const;
	
	void subtract_n_offset_this(const ScalarField& sub, int offset);
	ScalarField subtract_n_offset_new(const ScalarField& sub, int offset) const;
	void multiply_n_add_multiple_n_offset_this(int mul_this, const ScalarField& add, int mul_add, int offset);
	ScalarField multiply_n_add_multiple_n_offset_new(int mul_this, const ScalarField& add, int mul_add, int offset) const;
	
	void or_circle(int x, int y, int radius_squared, int value);
	
	std::string to_string(int tile_width = 2);
	
	inline int width() const {return _width;}
	inline int height() const {return _height;}
	
private:
	std::vector<int> _field;
	int _width;
	int _height;
	inline int get_index(int x, int y) const {return y * _width + x;}
	template<int Padding, bool Taxicab = false>
	void build_padded_walking_distance_field_internal(bc::Planet planet, int start_walking_distance);
	
public:
	static ScalarField EARTH_PASSABLE;
	static ScalarField MARS_PASSABLE;
	static ScalarField EARTH_KARBONITE;
	static ScalarField MARS_KARBONITE;
	static ScalarField EARTH_INIT_UNITS;
	static ScalarField EARTH_PASSABLE_PAD_1;
	static ScalarField MARS_PASSABLE_PAD_1;
	static ScalarField EARTH_KARBONITE_DISTANCE;
	static ScalarField MARS_KARBONITE_DISTANCE;
	static ScalarField EARTH_UNOCCUPIED_KARBONITE_DISTANCE;
	static ScalarField MARS_UNOCCUPIED_KARBONITE_DISTANCE;
	static ScalarField EARTH_ROCKET_DISTANCE;
	static ScalarField EARTH_SPREAD;
	static ScalarField MARS_SPREAD;
	static ScalarField EARTH_UNBUILT_FACTORY_DISTANCE;
	static ScalarField EARTH_ENEMY_SPREAD;
	static ScalarField MARS_ENEMY_SPREAD;
	static ScalarField EARTH_SPREAD_PLUS_ENEMY_SPREAD;
	static ScalarField MARS_SPREAD_PLUS_ENEMY_SPREAD;
	static ScalarField EARTH_SPREAD_MINUS_ENEMY_SPREAD;
	static ScalarField MARS_SPREAD_MINUS_ENEMY_SPREAD;
	static ScalarField EARTH_ENEMY_ATTACK_RANGE;
	static ScalarField MARS_ENEMY_ATTACK_RANGE;
	static bool UNBUILT_FACTORIES_EXIST;
};