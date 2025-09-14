/**************************************************************************
 * Copyright 2025 Ramskyi Roman
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 * http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
***************************************************************************/

#pragma once

#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <variant>
#include <map>
#include <stack>

#include <fmt/format.h>

#include "henifig/errors.hpp"

namespace henifig {
	/**
	 * @brief All the supported data types.
	 */
	enum data_types : uint8_t {
		unset,			// nothing
		declaration,	// undefined, can be used to check if a variable exist in case we don't need it to have a value
		string,			// "string"
		character,		// 'char'
		number,			// double
		boolean,		// bool
		array,			// [array]
		map,			// {map}
	};
	enum index_types : uint8_t {
		VAR,			// variable
		ARR,			// array
		MAP,			// map
		ARR_ITEM,		// array item
		MAP_KEY,		// a key inside a map
		MAP_VALUE,		// a value inside a map
	};
	constexpr size_t NPOS = -1;
	struct declaration_t{};
	struct unset_t{};
	struct array_index {
		size_t index{};
	};
	struct map_index {
		size_t index{};
	};
	using index_t = std::variant
	<size_t, std::string>;
	// Either an array index or a map key

	struct depth_t {
		size_t arr_index{NPOS}, map_index{NPOS};
		index_types index_type{};
		index_types environment_type{};
	};

	using value_variant = std::variant
	<unset_t, declaration_t, std::string, unsigned char, double, bool, array_index, map_index>;
	class parse_report {
		const error_codes error_code{};
		const size_t error_line{};
		const size_t error_index{};
		const std::string error_details;
	public:
		parse_report() = default;
		parse_report(const error_codes& error_code);
		parse_report(const error_codes& error_code, const size_t& error_line, const size_t& error_index = 0, std::string_view error_details = "");
		[[nodiscard]] bool is_error() const noexcept;
		[[nodiscard]] error_codes get_error_code() const noexcept;
		[[nodiscard]] const char* get_parse_error() const noexcept;
		[[nodiscard]] size_t get_error_line() const noexcept;
		[[nodiscard]] size_t get_error_index() const noexcept;
		[[nodiscard]] std::string_view get_parse_error_details() const noexcept;
	};
	class config {
		std::string filename;
		std::vector <std::string> vars;
		std::vector <std::string> values_str;
		std::vector <value_variant> values;
		std::vector <std::vector <value_variant>> arrs;
		std::vector <std::map <std::string, value_variant>> maps;
		std::map <std::string, size_t> line_nums;
		std::stack <size_t> arr_indexes, map_indexes;
		std::map <size_t, std::string> map_keys;
		std::stringstream content, parsed_content;
		parse_report process_parsing();
		parse_report remove_comments();
		parse_report lex();
		parse_report parse();
		size_t parse_value(const size_t& var_num, const size_t& pos = 0, depth_t depth = {});
		error_codes append(depth_t& depth, const value_variant& value = declaration_t{});
		size_t spaces{};
		error_codes print_value(const value_variant& x);
		void print_spaces() const;
		error_codes print_array(const std::vector <value_variant>& x);
		error_codes print_map(const std::map <std::string, value_variant>& x);
	public:
		config() = default;
		explicit config(std::string_view filename);
		void operator <<(std::string_view new_content);
		void operator <<(const std::ifstream& cfg_file);
	};
}
