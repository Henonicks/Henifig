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

#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <variant>
#include <map>
#include <stack>
#include <type_traits>

#include "henifig/errors.hpp"

namespace henifig {
	/**
	 * @brief All the supported data types.
	 */
	enum data_types : uint8_t {
		unset,			// nothing
		declaration,	// undefined, can be used to check if a variable exist in case we don't need it to have a value
		string,			// "string"
		character,		// 'char'
		floating,		// double
		ulonglong,		// unsigned long long
		longlong,		// long long
		boolean,		// bool
		array,			// [array]
		map,			// {map}
	};

	/**
	 * @brief The type of the value at the <current index>
	 */
	enum index_types : uint8_t {
		VAR,			// variable
		ARR,			// array
		MAP,			// map
		ARR_ITEM,		// array item
		MAP_KEY,		// a key inside a map
		MAP_VALUE,		// a value inside a map
	};
	constexpr size_t NPOS = -1;
	struct declaration_t{};
	struct unset_t{};
	class config_t;
	struct array_t;
	struct map_t;
	class value_t;

	using value_map = std::map <std::string, value_t>;
	using value_variant = std::variant
	<unset_t, declaration_t, std::string, char, double, unsigned long long, long long, bool, array_t, map_t>;

	using value_array = std::vector <value_t>;

	struct array_t {
		size_t index{};
		const config_t* cfg{};
		operator const value_array&() const;
		[[nodiscard]] const value_array& get() const;
	};
	struct map_t {
		size_t index{};
		const config_t* cfg{};
		operator const value_map&() const;
		[[nodiscard]] const value_map& get() const;
	};

	namespace detail {
		template <typename T>
		struct type_identity {
			using type = T;
		};
		template <typename... Args, typename T>
		constexpr bool is_alternative(type_identity <std::variant <Args...>>, type_identity <T>) {
			return (std::is_same_v <Args, T> || ...);
		}
	}

	template<typename T>
	constexpr inline bool in_variant = detail::is_alternative(detail::type_identity <value_variant>{}, detail::type_identity <T>{});

	template <typename T, typename... Args>
	constexpr inline bool convertible_to_variant = in_variant <T> || detail::is_alternative(detail::type_identity <std::variant <Args...>>{}, detail::type_identity <T>{});

	template <typename T>
	constexpr inline bool convertible_to_ref = convertible_to_variant <T, value_array, value_map>;

	class value_t {
	public:
		value_variant value;
		value_t() = default;
		value_t(value_variant value);
		template <typename T>
		value_t(T value) : value(std::move(value)) {}
		operator const value_variant&() const;

		/**
		 * @brief Get a reference to the underlying value.
		 * @tparam T The type to get a reference to.
		 * @return A reference to the underlying value.
		 * @exception std::bad_variant_access If a reference to T can't refer to the underlying variable.
		 */
		template <typename T, typename = std::enable_if_t <convertible_to_ref <T>>>
		[[nodiscard]] const T& get() const {
			if constexpr (std::is_same_v <T, value_array>) {
				return std::get <array_t>(value).get();
			}
			else if constexpr (std::is_same_v <T, value_map>) {
				return std::get <map_t>(value).get();
			}
			else {
				return std::get <T>(value);
			}
		}

		/**
		 * @brief Convert self to the type of the underlying value through @ref get.
		 * @tparam T The type to convert to.
		 */
		template <typename T, typename = std::enable_if_t <convertible_to_ref <T>>>
		[[nodiscard]] operator const T&() const {
			return get <T>();
		}

		/**
		 * @brief Convert self to the type compatible with that of the underlying value through @ref get_val.
		 * @tparam T The type to convert to.
		 */
		template <typename T, typename = std::enable_if_t <!convertible_to_ref <T>>>
		[[nodiscard]] operator T() const {
			if constexpr (!std::is_same_v <T, bool> &&
			!std::is_same_v <T, char> &&
			std::is_convertible_v <T, int>) {
				if constexpr (std::is_floating_point <T>()) {
					return std::get <double>(value);
				}
				else if (value.index() == ulonglong) {
					return std::get <unsigned long long>(value);
				}
				else {
					return std::get <long long>(value);
				}
			}
			else if constexpr (std::is_convertible_v <const char*, T> && !std::is_same_v <T, std::string>) {
				return std::get <std::string>(value).data();
			}
			else {
				static_assert(std::is_convertible_v <T, value_variant>, "Can't cast T to the types in value_t.");
				return std::get <T>(value);
			}
		}
		template <typename T>
		value_t& operator =(const T& val) {
			value = val;
			return *this;
		}
		template <typename T>
		[[nodiscard]] bool operator ==(const T& val) const {
			if constexpr (std::is_convertible_v <T, const char*> && !std::is_same_v <T, std::string>) {
				return !strcmp(*this, val);
			}
			else {
				return static_cast <T>(*this) == val;
			}
		}
		template <typename T>
		[[nodiscard]] bool operator !=(const T& val) const {
			return !(*this == val);
		}
		[[nodiscard]] std::size_t index() const;
		[[nodiscard]] bool isdef() const;
		[[nodiscard]] bool isndef() const;
		template <typename T>
		[[nodiscard]] bool is() const {
			return std::holds_alternative <T>(value);
		}
		const value_t& operator [](const std::size_t& index) const;
		template <typename T>
		const value_t& operator [](const T& index) const {
			return get <value_map>().at(static_cast <std::string>(index));
		}
	};
	struct depth_t {
		size_t arr_index{NPOS}, map_index{NPOS};
		index_types index_type{};
		index_types environment_type{};
	};

	class parse_report {
		const error_codes error_code{};
		const size_t error_line{};
		const size_t error_index{};
		const std::string error_filename;
		const std::string error_details;
	public:
		parse_report() = default;
		parse_report(const error_codes& error_code, std::string_view error_filename);
		parse_report(const error_codes& error_code, const size_t& error_line, const size_t& error_index = 0, std::string_view error_filename = "", std::string_view error_details = "");
		[[nodiscard]] bool is_error() const noexcept;
		[[nodiscard]] error_codes get_error_code() const noexcept;
		[[nodiscard]] const char* get_parse_error() const noexcept;
		[[nodiscard]] size_t get_error_line() const noexcept;
		[[nodiscard]] size_t get_error_index() const noexcept;
		[[nodiscard]] std::string_view get_error_filename() const noexcept;
		[[nodiscard]] std::string_view get_parse_error_details() const noexcept;
	};
	class config_t {
		struct brace_t {
			size_t num{}, i{};
			brace_t(const size_t& num, const size_t& i);
		};
		const config_t* cfg_self = this;
		std::string filename;
		std::vector <std::string> vars;
		std::map <std::string, size_t> var_nums;
		std::vector <std::string> values_str;
		value_array values;
		std::vector <value_array> arrs;
		std::vector <value_map> maps;
		std::map <std::string, size_t> line_nums;
		std::stack <size_t> arr_indexes, map_indexes;
		std::map <size_t, std::string> map_keys;
		std::stringstream content, parsed_content;
		parse_report process_parsing();
		parse_report remove_comments();
		parse_report lex();
		parse_report parse();
		size_t parse_value(const size_t& var_num, const size_t& pos = 0, depth_t depth = {});
		error_codes append(depth_t& depth, const value_t& value = declaration_t{});
		size_t space_offsets{};
		std::string get_spaces(const size_t& offset = 2) const;
		error_codes print_array(const value_array& x);
		error_codes print_map(const value_map& x);
		std::string value_to_json(const value_t& value, const size_t& spaces);
	public:
		config_t() = default;
		void clear();
		explicit config_t(std::string_view filename);
		void operator <<(std::string_view new_content);
		void operator <<(const std::ifstream& cfg_file);
		void open(std::string_view new_filename);
		error_codes print_value(const value_t& x);
		const value_t& operator [](std::string_view key) const;
		const value_array& get_arr(const size_t& index) const;
		const value_map& get_map(const size_t& index) const;
		std::string to_json(const size_t& spaces = 4);
		operator value_map() const;
	};
}
