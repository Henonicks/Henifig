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

#include "henifig/henifig.hpp"

int main() {
	henifig::process_logger::set_enabled(true);
	henifig::config_t cfg("../test.hfg");
	const std::vector <std::function <bool()>> tests = {
		[&cfg]() -> bool {
			try {
				return cfg["decl"].isdef();
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["hello"] == "Hello, World!";
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["h"] == 'h';
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["number"] == -1111.1;
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["true"] == true;
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				return cfg["false"] == false;
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				const henifig::value_array& arr = cfg["arr"];
				if (arr.size() != 4 || !arr[0].is <henifig::map_t>()) {
					return false;
				}
				const henifig::value_map& map = arr[0];
				if (map.size() != 3) {
					return false;
				}
				if (map.at("1") != true || map.at("2").isndef()) {
					return false;
				}
				const henifig::value_array& arr2 = map.at("3");
				if (arr2.size() != 2) {
					return false;
				}
				if (arr2[0] != 1 || arr2[1] != false) {
					return false;
				}
				if (arr[1] != -.1 || arr[2] != 'u' || arr[3] != false) {
					return false;
				}
				return true;
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				return false;
			}
		},
		[&cfg]() -> bool {
			try {
				const henifig::value_map& map = cfg["map"];
				if (map.size() != 2) {
					std::cout << "map size: " << map.size() << '\n';
					return false;
				}
				const henifig::value_array& arr = map.at("I will");
				if (arr.size() != 1) {
					std::cout << "arr size: " << arr.size() << '\n';
					return false;
				}
				if (arr[0] != "rise") {
					std::cout << "arr[0] is "; cfg.print_value(arr[0]);
					return false;
				}
				if (map.at("1") != 2) {
					std::cout << "map[\"1\"] is "; cfg.print_value(map.at("1"));
					return false;
				}
				return true;
			}
			catch (const std::exception& e) {
				std::cout << '\n' << e.what() << '\n';
				return false;
			}
		},
	};
	int failed{};
	for (int i = 0; i < tests.size(); i++) {
		std::cout << "Test № " << std::to_string(i + 1) << "  ";
		const bool passed{tests[i]()};
		failed += !passed;
		std::cout << (passed ? "\nPASSED" : "FAILED") << '\n';
	}
	std::cout << '\n' << (!failed ? "ALL TESTS PASSED" : "TESTS FAILED") << '\n';
	return 0;
}
