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

#include "henifig/parser.hpp"

template <typename T>
constexpr T index(const T& i) {
	return i + 1;
}

void henifig::config::operator <<(const std::ifstream& cfg_file) {
	content.clear();
	content << cfg_file.rdbuf() << "\n";
	if (const parse_report report = parse(content); report.is_error()) {
		content.clear();
		parsed_content.clear();
		throw parse_exception(report);
	}
}

henifig::parse_report henifig::config::parse(std::stringstream& cfg) {
	std::string error_message;
	std::string line;
	size_t hanging_comment{}, hanging_quote{}, hanging_apostrophe{};
	size_t hanging_comment_line{}, hanging_quote_line{}, hanging_apostrophe_line{};
	size_t line_num{};
	while (std::getline(cfg, line)) {
		++line_num;
		const size_t first_index = line.find_first_not_of(' ');
		std::string buffer = line;
		// We'll start from the first position of whatever could be important.
		for (size_t i = first_index; i < line.size(); i++) {
			if (!hanging_comment) {
				if (!hanging_apostrophe && line[i] == '"') {
					hanging_quote = !hanging_quote ? i : 0;
					hanging_quote_line = hanging_quote ? line_num : 0;
				}
				if (!hanging_quote && line[i] == '\'') {
					hanging_apostrophe = !hanging_apostrophe ? i : 0;
					hanging_apostrophe_line = hanging_apostrophe ? line_num : 0;
				}
			}
			if (line[i] == '#' && !hanging_quote && !hanging_apostrophe) {
				bool ml_comment_begin{};
				// Is this the beginning of a multi-line comment?
				if (i != first_index && line[i - 1] == '[' && !hanging_comment) {
					// We're beginning to read a multi-line comment.
					hanging_comment = i;
					hanging_comment_line = line_num;
					ml_comment_begin = true;
					buffer = buffer.substr(first_index, i - first_index - 1);
				}
				if (i != line.size() - 1 && line[i + 1] == ']') {
					// This is the end of the currently hanging comment.
					if (ml_comment_begin) {
						// This is the end of the comment that was started with the same '#'.
						error_message = "unexpected ']'";
						break;
					}
					if (!hanging_comment) {
						error_message = "unexpected '#]'";
						break;
					}
					hanging_comment = 0;
					hanging_comment_line = 0;
					buffer = buffer.substr(first_index, i - first_index);
				}
				else if (!hanging_comment) {
					// We're in a single-line comment.
					buffer = buffer.substr(first_index, i - first_index);
					break;
				}
			}
		}
		if (!hanging_comment && !buffer.empty()) {
			parsed_content << buffer << '\n';
		}
		if (!error_message.empty()) {
			break;
		}
	}
	if (hanging_comment) {
		error_message = "hanging multi-line comment ( [# doesn't match a #] ).";
	}
	std::cout << "---\n" << parsed_content.str() << "---\n" << std::endl << hanging_comment << ' ' << hanging_apostrophe << ' ' << hanging_apostrophe << std::endl;
	if (hanging_comment) {
		return parse_report{error_message, hanging_comment_line, hanging_comment};
	}
	if (hanging_quote) {
		return parse_report(error_message, hanging_quote_line, hanging_quote);
	}
	if (hanging_apostrophe) {
		return parse_report(error_message, hanging_apostrophe_line, hanging_apostrophe);
	}
	return {};
}
