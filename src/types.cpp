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

#include "henifig/types.hpp"

henifig::parse_report::parse_report(const error_codes& error_code) : error_code(error_code), error_line(0), error_index(0) {}

henifig::parse_report::parse_report(const error_codes error_code, const size_t& error_line, const size_t& error_index) :
error_code(error_code), error_line(error_line), error_index(error_index) {}

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
