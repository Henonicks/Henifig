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

void henifig::config::operator <<(const std::ifstream& cfg_file) {
	content.str(std::string());
	content << cfg_file.rdbuf() << "\n";
	if (const parse_report report = parse(); report.is_error()) {
		content.str(std::string());
		parsed_content.str(std::string());
		throw parse_exception(report);
	}
}

henifig::parse_report henifig::config::parse() {
	if (const parse_report report = remove_comments(); report.is_error()) {
		return report;
	}
	if (const parse_report report = lex(); report.is_error()) {
		return report;
	}
	return {};
}

henifig::parse_report henifig::config::remove_comments() {
	std::string error_message;
	std::string line;
	size_t hanging_comment{}, hanging_quote{}, hanging_apostrophe{};
	size_t hanging_comment_line{}, hanging_quote_line{}, hanging_apostrophe_line{};

	std::string buffer;

	size_t line_num{};
	size_t i{};

	while (std::getline(content, line)) {
		line = ' ' + line + ' ';
		++line_num;
		const size_t first_index = line.find_first_not_of(' ');
		// We'll start from the first position of whatever could be important.
		for (i = first_index; i < line.size(); i++) {
			if (line[i] == '#' && !hanging_quote && !hanging_apostrophe) {
				bool ml_comment_begin{};
				// Is this the beginning of a multi-line comment?
				if (i != first_index && line[i - 1] == '[' && !hanging_comment) {
					// We're beginning to read a multi-line comment.
					hanging_comment = i;
					hanging_comment_line = line_num;
					ml_comment_begin = true;
					line[i - 1] = ' ';
					line[i] = ' ';
				}
				if (i < line.size() - 1 && line[i + 1] == ']') {
					// This is the end of the currently hanging comment.
					if (ml_comment_begin) {
						// This is the end of the comment that was started with the same '#'.
						error_message = "unexpected ']' right after \"[#\"";
						break;
					}
					if (!hanging_comment) {
						error_message = "unexpected \"#]\"";
						break;
					}
					hanging_comment = 0;
					line[i] = ' ';
					line[i + 1] = ' ';
				}
				else if (!hanging_comment) {
					// We're in a single-line comment.
					line = line.replace(i, line.size() - i, "");
					break;
				}
			}
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
			else {
				line[i] = ' ';
			}
		}
		if (std::count(line.begin(), line.end(), ' ') == line.size()) {
			line.clear();
		}
		buffer += line;
		buffer += '\n';
		if (!line.empty()) {
			if (!hanging_comment || hanging_comment != first_index) {
				parsed_content << buffer;
				buffer.clear();
			}
		}
		if (!error_message.empty()) {
			break;
		}
	}
	if (hanging_comment) {
		error_message = R"(hanging multi-line comment ( "[#" doesn't match a "#]" ).)";
		line_num = hanging_comment_line;
		i = hanging_comment;
	}
	else if (hanging_quote) {
		line_num = hanging_quote_line;
		i = hanging_quote;
	}
	else if (hanging_apostrophe) {
		line_num = hanging_apostrophe_line;
		i = hanging_apostrophe;
	}
	std::cout << "---\n" << parsed_content.str() << "---\n\n";
	return parse_report(error_message, line_num, i);
}

henifig::parse_report henifig::config::lex() {
	std::string line;
	size_t hanging_var{}, hanging_quote{}, hanging_apostrophe{}, hanging_escape{};
	std::stack <size_t> hanging_arr, hanging_tuple;
	size_t hanging_comma{};
	size_t hanging_var_line{}, hanging_quote_line{}, hanging_apostrophe_line{}, hanging_escape_line{};
	size_t hanging_arr_line{}, hanging_tuple_line{};
	size_t hanging_comma_line{};
	std::string value, value_str;
	bool var_declared{}, var_defined{};
	bool piped{}, afterpipe{};
	std::string error_message;

	size_t i;
	size_t line_num{};
	while (std::getline(parsed_content, line)) {
		++line_num;
		const size_t first_index = line.find_first_not_of(' ');
		for (i = first_index; i < line.size(); i++) {
			if (line[i] == '\\') {
				if (hanging_var) {
					if (!hanging_escape) {
						if ((i != line.size() - 1 && line[i + 1] == '|' || line[i + 1] == ' ') || i == line.size() - 1) {
							// If this backslash is to complete a declaration
							hanging_var = 0;
							vars.push_back(value);
							value.clear();
							var_declared = true;
						}
						else {
							hanging_escape = i;
						}
					}
					else {
						value += '\\';
					}
				}
			}
			else if (line[i] == '/') {
				if (!hanging_quote && !hanging_apostrophe) {
					hanging_var = i;
				}
			}
			else if (line[i] == '|') {
				if (var_declared) {
					if (!piped) {
						piped = true;
						value.clear();
						value_str.clear();
					}
					else {
						error_message = "expected expression after the '|' sign";
						break;
					}
				}
				else {
					error_message = "the '|' sign needs to be used after a declared variable";
					break;
				}
			}
			else if (line[i] == '\"' || line[i] == '\'' ||
				line[i] == '[' || line[i] == ']' || line[i] == '{' || line[i] == '}' ||
			(line[i] >= '0' && line[i] <= '9') || (line[i] == 't' || line[i] == 'f')) {
				if (piped && !afterpipe) {
					afterpipe = true;
				}
				if (line[i] == '\"') {
					value += '\"';
					if ((hanging_escape || hanging_apostrophe) || (!hanging_arr.empty() || !hanging_tuple.empty())) {
						if (hanging_escape || hanging_apostrophe) {
							value_str += '\"';
						}
					}
					else if (!hanging_quote) {
						hanging_quote = i;
					}
					else {
						hanging_quote = 0;
					}
				}
				else if (line[i] == '\'') {
					value += '\'';
					if ((hanging_escape || hanging_quote) || (!hanging_arr.empty() || !hanging_tuple.empty())) {
						if (hanging_escape || hanging_quote) {
							value_str += '\'';
						}
					}
					else if (!hanging_apostrophe) {
						hanging_apostrophe = i;
					}
					else if (value_str.size() > 1) {
						error_message = "multiple characters in a char literal";
						break;
					}
					else if (value_str.empty()) {
						error_message = "empty char literal";
						break;
					}
					else {
						hanging_apostrophe = 0;
					}
				}
				else if (line[i] == '[' || line[i] == ']' || line[i] == '{' || line[i] == '}') {
					if (line[i] == '[' || line[i] == '{') {
						if (!piped && !hanging_quote && !hanging_apostrophe) {
							piped = true;
							vars.push_back(value);
							value.clear();
							var_declared = true;
						}
					}
					if (line[i] == '[') {
						value += '[';
						if (hanging_quote || hanging_apostrophe) {
							value_str += '[';
						}
						else {
							hanging_arr.push(i);
						}
					}
					else if (line[i] == ']') {
						value += ']';
						if (hanging_quote || hanging_apostrophe) {
							value_str += ']';
						}
						else if (hanging_arr.empty()) {
							error_message = "missing '[' to complete with the ']'";
							break;
						}
						else if (!hanging_tuple.empty() && hanging_tuple.top() > hanging_arr.top()) {
							error_message = "can't complete the hanging '{' with the ']'";
							break;
						}
						hanging_arr.pop();
					}
					else if (line[i] == '{') {
						value += '{';
						if (hanging_quote || hanging_apostrophe) {
							value_str += '{';
						}
						else {
							hanging_tuple.push(i);
						}
					}
					else if (line[i] == '}') {
						value += '}';
						if (hanging_quote || hanging_apostrophe) {
							value_str += '}';
						}
						else if (hanging_tuple.empty()) {
							error_message = "missing '{' to complete with the '}'";
							break;
						}
						else if (!hanging_arr.empty() && hanging_arr.top() > hanging_tuple.top()) {
							error_message = "can't complete the hanging '[' with the '}'";
							break;
						}
						hanging_tuple.pop();
					}
				}
				else if (line[i] == 't') {
					if (i <= line.size() - 4 - 1 && line.find("true", i) != std::string::npos) {
						value += "true";
						value_str += "true";
						i += 4 - 1;
					}
					else if (!hanging_var && !hanging_quote && !hanging_apostrophe) {
						error_message = "unknown expression";
						break;
					}
					else {
						value += 't';
						value_str += 't';
					}
				}
				else if (line[i] == 'f') {
					if (i <= line.size() - 5 - 1 && line.find("false", i) != std::string::npos) {
						value += "false";
						value_str += "false";
						i += 5 - 1;
					}
					else if (!hanging_var && !hanging_quote && !hanging_apostrophe) {
						error_message = "unknown expression";
						break;
					}
					else {
						value += 'f';
						value_str += 'f';
					}
				}
				else {
					value += line[i];
				}
			}
			else if (hanging_var || hanging_quote || hanging_apostrophe || hanging_escape || !hanging_arr.empty() || !hanging_tuple.empty()) {
				char to_add;
				if (hanging_escape) {
					if (line[i] == 'n') {
						to_add = '\n';
					}
					else if (line[i] == ' ') {
						error_message = "every non-declaration '\\' needs to be escaped";
						break;
					}
					else {
						error_message = fmt::format("unexpected escape sequence: '\\{}\"", line[i]);
						break;
					}
					hanging_escape = 0;
				}
				else {
					to_add = line[i];
				}
				value += to_add;
				value_str += to_add;
			}
			if (hanging_arr.empty() && hanging_tuple.empty() && !hanging_quote && !hanging_apostrophe && afterpipe) {
				var_defined = true;
				piped = false;
				values_str.push_back(value);
			}
			if (line[i] == ';') {
				if (piped && !var_defined) {
					error_message = "expected expression after '|'";
					break;
				}
			}
		}
		if (var_declared && !piped) {
			value.clear();
			var_declared = false;
			afterpipe = false;
			var_defined = false;
		}
		if (!error_message.empty()) {
			break;
		}
		if (hanging_var) {
			error_message = "hanging variable declaration";
		}
		else if (hanging_quote) {
			error_message = "hanging quote";
		}
		else if (hanging_apostrophe) {
			error_message = "hanging apostrophe";
		}
		else if (hanging_escape) {
			error_message = "hanging escape sequence";
		}
		if (!error_message.empty()) {
			break;
		}
	}
	std::cout << "\n----\n";
	for (i = 0; i < vars.size(); i++) {
		std::cout << '`' << vars[i] << "` | `" << (values_str.size() > i ? values_str[i] : "") << "`\n";
	}
	std::cout << "----\n";
	return parse_report(error_message, line_num, i);
}
