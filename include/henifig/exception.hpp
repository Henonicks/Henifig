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

#include "henifig/types.hpp"

namespace henifig {
	class parse_exception final : public std::exception {
		std::string full_error;
	public:
		parse_exception() = delete;
		explicit parse_exception(const parse_report& report);
		[[nodiscard]] const char* what() const noexcept override;
	};
}
