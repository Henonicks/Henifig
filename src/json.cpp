#include "henifig/json.hpp"

std::string get_json_char(const char c) {
	switch (c) {
		case '"': {
			return "\\\"";
		}
		case '\\': {
			return "\\\\";
		}
		case '\b': {
			return "\\b";
		}
		case '\f': {
			return "\\f";
		}
		case '\n': {
			return "\\\n";
		}
		case '\r': {
			return "\\r";
		}
		case '\t': {
			return "\\t";
		}
		case '/': {
			return "\\/";
		}
		default: {
			return std::string{c};
		}
	}
}

std::string get_json_string(const std::string_view s) {
	std::string string;
	for (size_t i = 0; i < s.size(); i++) {
		string += get_json_char(s[i]);
	}
	return string;
}

std::string henifig::config_t::value_to_json(const value_t& value, const size_t& spaces) {
	std::string json_value;
	switch (value.index()) {
		case declaration: {
			return json_value + "null";
		}
		case string: {
			return json_value + '"' + get_json_string(value.get <std::string>()) + '"';
		}
		case character: {
			return json_value + '"' + get_json_char(value.get <char>()) + '"';
		}
		case floating: {
			return json_value + std::to_string(value.get <double>());
		}
		case ulonglong: {
			return json_value + std::to_string(value.get <unsigned long long>());
		}
		case longlong: {
			return json_value + std::to_string(value.get <long long>());
		}
		case boolean: {
			const bool underlying = value.get <bool>();
			return json_value + (underlying ? "true" : "false");
		}
		case array: {
			std::string array_values;
			json_value += "[";
			for (const value_t& x : value.get <value_array>()) {
				json_value += value_to_json(x, spaces) + ", ";
			}
			json_value.pop_back();
			json_value.back() = ']';
			return json_value;
		}
		case map: {
			std::string map_values;
			json_value += "{\n";
			++space_offsets;
			for (const auto& [key, val] : value.get <value_map>()) {
				json_value += get_spaces(spaces) + '"' + key + "\" : " + value_to_json(val, spaces) + ",\n";
			}
			--space_offsets;
			json_value.erase(json_value.size() - 2, 2);
			json_value += '\n' + get_spaces(spaces) + '}';
			return json_value;
		}
		default: {
			return std::move("ERROR: UNKNOWN TYPE");
		}
	}
}

std::string henifig::config_t::to_json(const size_t& spaces) {
	std::string json = "{\n";
	++space_offsets;
	for (const auto& var : vars) {
		json += get_spaces(spaces) + '"' + var + "\" : " + value_to_json(values[var_nums.at(var)], spaces) + ",\n";
		// operator [] fails because of parse_value not being const like I have a fucking choice
	}
	--space_offsets;
	json.erase(json.size() - 2, 1);
	return json + '}';
}
