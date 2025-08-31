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
	full_error = fmt::format("Parsing error on {0}:{1} - {2} (code: {3}).",
		report.get_error_line(), report.get_error_index(),
		report.get_parse_error(), static_cast<uint16_t>(report.get_error_code())
	);
}
