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

#include "henifig/exception.hpp"

const char* henifig::parse_exception::what() const noexcept {
	return full_error.c_str();
}

henifig::parse_exception::parse_exception(const parse_report& report) {
	full_error = std::string("Parsing error on ") +
		(report.get_error_line() == 0 && report.get_error_index() == 0 ? "<position not given>" :
		(report.get_error_index() == 0 ?
			std::to_string(report.get_error_line()) :
			std::to_string(report.get_error_line()) + ':' + std::to_string(report.get_error_index())) +
		" - " +
			report.get_parse_error() +
			(!report.get_parse_error_details().empty() ? std::string(" (") + report.get_parse_error_details().data() + ')' : std::string{}) +
		std::string(" (code: ") + std::to_string(report.get_error_code()) + ").");
}

const char* henifig::retrieval_exception::what() const noexcept {
	return full_error.c_str();
}

henifig::retrieval_exception::retrieval_exception(const std::string_view error) : full_error(error) {}
