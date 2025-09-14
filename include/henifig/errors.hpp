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

namespace henifig {
	enum error_codes : uint8_t {
		OK,
		HANGING_ESCAPE,
		UNDEFINED_ESCAPE,
		HANGING_COMMENT,
		IMPROPERLY_CLOSED_COMMENT,
		NO_OPENED_COMMENT,
		HANGING_QUOTE,
		HANGING_APOSTROPHE,
		HANGING_VAR,
		HANGING_PIPE,
		NAKED_PIPE,
		HANGING_ARR,
		UNEXPECTED_ARR,
		UNEXPECTED_ARR_END,
		MAP_COMPLETED_WITH_ARR,
		HANGING_MAP,
		UNEXPECTED_MAP,
		UNEXPECTED_MAP_END,
		ARR_COMPLETED_WITH_MAP,
		EXPECTED_DOLLAR,
		HANGING_DOLLAR,
		UNEXPECTED_DOLLAR,
		REPEATED_DOLLAR,
		PIPED_KEY,
		PIPED_VALUE,
		MULTIPLE_CHARS,
		NO_CHARS,
		UNEXPECTED_COMMA,
		HANGING_COMMA,
		MINUS_IN_MIDDLE,
		HANGING_DOT,
		REPEATED_DOT,
		EXPECTED_EXPRESSION,
		UNEXPECTED_EXPRESSION,
		UNKNOWN_EXPRESSION,
		WRONG_EXPRESSION,
		MISSING_SEMICOLON,
		UNKNOWN_TYPE,
		REDECLARED_VAR,
		REDECLARED_KEY,
		FILE_OPEN_FAILED,
	};

	inline const char* error_messages[] = {
		"",
		"hanging escape sequence",
		"undefined escape sequence",
		R"(hanging multi-line comment (the "[#" doesn't match a "#]"))",
		R"(unexpected ']' right after "[#")",
		R"(unexpected "#]")",
		"hanging quote",
		"hanging apostrophe",
		"hanging variable declaration",
		"expected expression after '|'",
		"unexpected '|' - no variable declared",
		"hanging array declaration",
		"unexpected array declaration",
		"missing '[' to complete with the ']'",
		"can't complete the hanging '{' with the ']'",
		"hanging map declaration",
		"unexpected map declaration",
		"missing '{' to complete with the '}'",
		"can't complete the hanging '[' with the '}'",
		"expected '$'",
		"hanging '$'",
		"unexpected '$'",
		"repeated '$'",
		"unexpected key name after the '|'",
		"unexpected '|' after the expression",
		"multiple characters in a char literal",
		"empty char literal",
		"unexpected ','",
		"hanging ','",
		"hit '-' in the middle of a number",
		"hanging '.'",
		"repeated '.' in a number",
		"expected expression",
		"unexpected expression",
		"unknown expression",
		"couldn't handle the expression",
		"expected ';' before the '/'",
		"unknown type reached during the parsing process",
		"redeclared variable",
		"redeclared map value key",
		"failed to open the file"
	};
}
