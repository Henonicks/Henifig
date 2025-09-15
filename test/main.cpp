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

#include <functional>

#include <henifig/henifig.hpp>
#include "henifig/internal/logger.hpp"

int main() {
	henifig::process_logger::set_enabled(true);
	henifig::config_t cfg("../test.hfg");
	std::vector <std::function <bool()>> tests = {
		[&cfg]() -> bool {
			try {
				return cfg["decl"].isdef();
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["hello"] == std::string("Hello, World!");
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["h"] == 'h';
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["number"] == -1111.1;
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["true"] == true;
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["false"] == false;
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				const henifig::value_array& arr = cfg["arr"];
				if (arr.size() != 4 || arr[0].isnt <henifig::map_t>()) {
					return false;
				}
				henifig::value_map map = arr[0];
				if (map.size() != 3) {
					return false;
				}
				if (map["1"] != true && map["2"].isnt <henifig::map_t>()) {
					return false;
				}
				henifig::value_array arr2 = map["3"];
				if (arr2.size() != 2) {
					return false;
				}
				if (arr2[0] != 1.0 && arr2[1] != false) {
					return false;
				}
				if (arr[1] != -.1 && arr[2] != 'u' && arr[2] != false) {
					return false;
				}
				return true;
			}
			catch (...) {
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				henifig::value_map map = cfg["map"];
				if (map.size() != 2) {
					henifig::cout << "map size: " << map.size() << '\n';
					return false;
				}
				henifig::value_array arr = map["I will"];
				if (arr.size() != 1) {
					henifig::cout << "arr size: " << arr.size() << '\n';
					return false;
				}
				if (arr[0] != std::string("rise")) {
					henifig::cout << "arr[0] is "; cfg.print_value(arr[0]);
					return false;
				}
				if (map["1"] != 2.0) {
					henifig::cout << "map[\"1\"] is "; cfg.print_value(map["1"]);
					return false;
				}
				return true;
			}
			catch (const std::exception& e) {
				henifig::cout << '\n' << e.what() << '\n';
				return false;
			}
		},
	};
	int failed{};
	for (int i = 0; i < tests.size(); i++) {
		henifig::cout << "Test № " << std::to_string(i + 1) << "  ";
		const bool passed{tests[i]()};
		failed += !passed;
		henifig::cout << (passed ? "\nPASSED" : "FAILED") << '\n';
	}
	std::cout << '\n' << (!failed ? "ALL TESTS PASSED" : "TESTS FAILED") << '\n';
	return 0;
}
