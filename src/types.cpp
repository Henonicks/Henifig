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

#include <utility>

#include "henifig/types.hpp"

henifig::config_t::brace_t::brace_t(const size_t& num, const size_t& i) : num(num), i(i) {}

henifig::parse_report::parse_report(const error_codes& error_code, const std::string_view error_filename) : error_code(error_code), error_filename(error_filename) {}

henifig::parse_report::parse_report(const error_codes& error_code, const size_t& error_line, const size_t& error_index, const std::string_view error_filename, const std::string_view error_details) :
error_code(error_code), error_line(error_line), error_index(error_index), error_filename(error_filename), error_details(error_details) {}

bool henifig::parse_report::is_error() const noexcept {
	return error_code != OK;
}

henifig::error_codes henifig::parse_report::get_error_code() const noexcept {
	return error_code;
}

const char* henifig::parse_report::get_parse_error() const noexcept {
	return error_messages[error_code];
}

size_t henifig::parse_report::get_error_line() const noexcept {
	return error_line;
}

size_t henifig::parse_report::get_error_index() const noexcept {
	return error_index;
}

std::string_view henifig::parse_report::get_error_filename() const noexcept {
	return error_filename;
}

std::string_view henifig::parse_report::get_parse_error_details() const noexcept {
	return error_details;
}

henifig::value_t::value_t(value_variant value) : value(std::move(value)) {}

std::size_t henifig::value_t::index() const {
	return value.index();
}

henifig::value_t::operator const henifig::value_variant&() const {
	return value;
}

henifig::array_t::operator const value_array&() const {
	return cfg->get_arr(index);
}

const std::vector <henifig::value_t>& henifig::array_t::get() const {
	return cfg->get_arr(index);
}

henifig::map_t::operator const value_map&() const {
	return cfg->get_map(index);
}

const henifig::value_map& henifig::map_t::get() const {
	return cfg->get_map(index);
}

bool henifig::value_t::isdef() const {
	return value.index() == declaration;
}

bool henifig::value_t::isndef() const {
	return value.index() != declaration;
}

henifig::value_t henifig::value_t::operator[](const std::size_t& index) const {
	return get <value_array>()[index];
}

henifig::config_t::operator value_map() const {
	value_map res;
	for (const std::string& x : vars) {
		res[x] = operator[](x);
	}
	return res;
}
