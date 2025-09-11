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
#include <fmt/format.h>

#include "henifig/errors.hpp"

namespace henifig {
	/**
	 * @brief All the supported data types.
	 */
	enum data_types : uint8_t {
		declaration = 0,	// undefined, can be used to check if a variable exist in case we don't need it to have a value
		string = 1,			// "string"
		character = 2,		// 'char'
		integer = 3,		// int/long
		number = 3,			// double
		boolean = 4,		// bool
		array = 5,			// [array]
		map = 6,			// {map}
	};
	enum index_types : bool {
		VAR_ARR,		// variable/array
		MAP,			// map
	};
	constexpr size_t NPOS = -1;
	struct declaration_t{};
	class array_index {
		size_t index{};
	public:
		array_index(const size_t& index) : index(index) {}
	};
	class map_index {
		size_t index{};
	public:
		map_index(const size_t& index) : index(index) {}
	};
	using index_t = std::variant
	<size_t, std::string>;
	// Either an array index or a map key

	using value_variant = std::variant
	<declaration_t, std::string, unsigned char, double, bool, array_index, map_index>;
	class parse_report {
		const error_codes error_code{};
		const size_t error_line{};
		const size_t error_index{};
	public:
		parse_report() = default;
		parse_report(const error_codes& error_code);
		parse_report(error_codes error_code, const size_t& error_line, const size_t& error_index);
		[[nodiscard]] bool is_error() const noexcept;
		[[nodiscard]] error_codes get_error_code() const noexcept;
		[[nodiscard]] const char* get_parse_error() const noexcept;
		[[nodiscard]] size_t get_error_line() const noexcept;
		[[nodiscard]] size_t get_error_index() const noexcept;
	};
	class config {
		std::vector <std::string> vars;
		std::vector <std::string> values_str;
		std::vector <value_variant> values;
		std::vector <std::vector <value_variant>> arrs;
		std::vector <std::pair <std::string, value_variant>> maps;
		std::vector <size_t> values_index;
		std::vector <data_types> types;
		std::stringstream content, parsed_content;
		parse_report process_parsing();
		parse_report remove_comments();
		parse_report lex();
		parse_report parse();
		size_t parse_value(const size_t& var_num, const size_t& pos = 0, const size_t& index = NPOS);
		void append(const index_t& index, const value_variant& value);
	public:
		void operator <<(const std::ifstream& cfg_file);
	};
}
