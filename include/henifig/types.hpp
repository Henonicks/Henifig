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
#include <fmt/format.h>

namespace henifig {
	/**
	 * @brief All the supported data types.
	 */
	enum class data_type : uint8_t {
		string,			// string
		character,		// char
		integer,		// int/long
		number,			// double
		boolean,		// bool
		declaration,	// undefined, serves the same purpose as "#define FMT_HEADER_ONLY"
	};
	class parse_report {
		const std::string parse_error;
		const size_t error_line{};
		const size_t error_index{};
	public:
		parse_report() = default;
		explicit parse_report(std::string_view parse_error, const size_t& error_line, const size_t& error_index);
		[[nodiscard]] bool is_error() const noexcept;
		[[nodiscard]] std::string get_parse_error() const noexcept;
		[[nodiscard]] size_t get_error_line() const noexcept;
		[[nodiscard]] size_t get_error_index() const noexcept;
	};
	class config {
		std::vector <std::string> vars;
		std::vector <std::string> values_str;
		std::vector <data_type> types;
		std::stringstream content, parsed_content;
		parse_report parse();
		parse_report remove_comments();
		parse_report lex();
	public:
		void operator <<(const std::ifstream& cfg_file);
	};
}
