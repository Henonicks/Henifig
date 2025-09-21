#pragma once

#include "henifig/types.hpp"

namespace henifig {
	namespace json {
		std::string get_json_char(char c);
		std::string get_json_string(std::string_view s);
	}
}
