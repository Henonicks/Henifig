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
		TUPLE_COMPLETED_WITH_ARR,
		HANGING_TUPLE,
		UNEXPECTED_TUPLE,
		UNEXPECTED_TUPLE_END,
		ARR_COMPLETED_WITH_TUPLE,
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
		"hanging tuple declaration",
		"unexpected tuple declaration",
		"missing '{' to complete with the '}'",
		"can't complete the hanging '[' with the '}'",
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
		"unexpected/unknown expression",
		"expected ';' before the '/'",
	};
}
