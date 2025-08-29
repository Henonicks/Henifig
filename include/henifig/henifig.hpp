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

#include <vector>
#include <variant>
#include <tuple>

namespace henifig {
	class process_logger {
		static bool LOG_PROCESS;
	public:
		static void set_enabled(const bool& enabled) {
			LOG_PROCESS = enabled;
		}
		[[nodiscard]] static bool is_enabled() {
			return LOG_PROCESS;
		}
		process_logger(const process_logger&) = delete;
		process_logger& operator=(const process_logger&) = delete;
		process_logger(process_logger&&) = delete;
		process_logger& operator=(process_logger&&) = delete;
	};
}

#include "henifig/types.hpp"
#include "henifig/exception.hpp"
#include "henifig/parser.hpp"
