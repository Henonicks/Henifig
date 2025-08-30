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
#include "henifig/internal/logger.hpp"

void henifig::config::operator <<(const std::ifstream& cfg_file) {
	content.str(std::string());
	content << cfg_file.rdbuf() << '\n';
	if (const parse_report report = process_parsing(); report.is_error()) {
		content.str(std::string());
		parsed_content.str(std::string());
		throw parse_exception(report);
	}
}

henifig::parse_report henifig::config::process_parsing() {
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
	size_t hanging_var{}, hanging_comment{}, hanging_escape{}, hanging_quote{}, hanging_apostrophe{};
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
			if (line[i] == '/' && !hanging_var &&
			!hanging_comment && !hanging_quote && !hanging_apostrophe) {
				hanging_var = i;
			}
			if (line[i] == '\\') {
				if (hanging_var && !hanging_comment && !hanging_quote && !hanging_apostrophe) {
					hanging_var = 0;
				}
				else {
					hanging_escape = !hanging_escape ? i : 0;
				}
			}
			if (line[i] == '#' && !hanging_var && !hanging_quote && !hanging_apostrophe) {
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
				if ((line[i] == '"' || line[i] == '\'') && hanging_escape) {
					hanging_escape = 0;
				}
				else if (!hanging_apostrophe && line[i] == '"' && !hanging_escape) {
					hanging_quote = !hanging_quote ? i : 0;
					hanging_quote_line = hanging_quote ? line_num : 0;
				}
				else if (!hanging_quote && line[i] == '\'' && !hanging_escape) {
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
		if (!error_message.empty() || hanging_escape) {
			break;
		}
	}
	if (hanging_var) {
		error_message = "hanging variable declaration";
		i = hanging_var;
	}
	else if (hanging_comment) {
		error_message = R"(hanging multi-line comment ("[#" doesn't match a "#]").)";
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
	else if (hanging_escape) {
		error_message = "hanging escape sequence";
	}
	cout << "---\n" << parsed_content.str() << "---\n\n";
	return parse_report(error_message, line_num, i);
}

henifig::parse_report henifig::config::lex() {
	std::string line;
	size_t hanging_var{}, hanging_quote{}, hanging_apostrophe{}, hanging_escape{};
	std::stack <size_t> hanging_arr, hanging_tuple;
	std::stack <size_t> hanging_arr_line{}, hanging_tuple_line{};
	size_t hanging_comma_line{};
	std::string value, value_str;
	bool var_declared{};
	bool piped{}, afterpipe{};
	std::string error_message;

	size_t i;
	size_t line_num{};
	auto unexpected_expression = [&]() -> bool {
		if ((var_declared && !piped) || (!var_declared && !hanging_var) ||
		(!hanging_var && !value.empty() &&
		!hanging_quote && !hanging_apostrophe && hanging_arr.empty() && hanging_tuple.empty()) &&

		(line[i] >= '0' && line[i] <= '9' &&
		line[i - 1] <= '0' && line[i - 1] >= '9' &&
		line[i - 1] != '.' && (line[i - 1] != '-' || (line[i - 1] == '-' && value.size() > 1))) ||

		(line[i] <= '0' && line[i] >= '9')) {
			cout << "UNEXPECTED EXPRESSION " << line_num << ' ' << i << '\n';
			return true;
		}
		return false;
	};
	while (std::getline(parsed_content, line)) {
		cout << "VALUE STR " << line_num << " `" << value_str << "`\n";
		++line_num;
		const size_t first_index = line.find_first_not_of(' ');
		for (i = first_index; i < line.size(); i++) {
			if (line[i] != ';' && line[i] != ' ' && line[i] != '/' && line[i] != '|') {
				if (unexpected_expression()) {
					error_message = "unexpected expression";
					break;
				}
			}
			if (line[i] == '\\') {
				if (hanging_var) {
					if (!hanging_escape) {
						if ((i != line.size() - 1 && line[i + 1] == '|' || line[i + 1] == ' ') ||
						(i != first_index && line[i - 1] == ']' || line[i - 1] == '}') ||
						i == line.size() - 1) {
							// If this backslash is to complete a declaration
							hanging_var = 0;
							if (!afterpipe) {
								vars.push_back(value);
								cout << "AFTERPIPE PUSHING: " << line_num << ' ' << i << " `" << value << "`\n";
								var_declared = true;
							}
							else {
								// This is a /var()\-like declaration
								if (!hanging_arr.empty()) {
									error_message = "hanging array declaration";
									line_num = hanging_arr_line.top();
									i = hanging_arr.top();
									break;
								}
								if (!hanging_tuple.empty()) {
									error_message = "hanging tuple declaration";
									line_num = hanging_tuple_line.top();
									i = hanging_tuple.top();
									break;
								}
								values_str.push_back(value);
							}
							cout << "/VAR()\\, CLEARING " << line_num << ' ' << i << '\n';
							value.clear();
							value_str.clear();
						}
						else {
							hanging_escape = i;
						}
					}
					else {
						value += '\\';
					}
				}
				else {
					hanging_escape = i;
				}
				cout << "HIT BACKSLASH " << line_num << ' ' << i << ' ' << hanging_escape << '\n';
			}
			else if (line[i] == '/') {
				if (!hanging_quote && !hanging_apostrophe) {
					if (!var_declared && hanging_var) {
						value += '/';
					}
					else if (afterpipe && i != first_index) {
						error_message = "expected ';' before the '/'";
						break;
					}
					else if (!afterpipe && piped) {
						error_message = "expected expression after the '|'";
						break;
					}
					else {
						if (vars.size() > values_str.size()) {
							values_str.push_back(value);
						}
						cout << "NEW DECL, CLEARING " << line_num << ' ' << i << '\n';
						value.clear();
						value_str.clear();
						hanging_var = i;
						var_declared = false;
						piped = false;
						afterpipe = false;
					}
				}
			}
			else if (line[i] == '|') {
				if (!afterpipe) {
					if (!piped) {
						piped = true;
						cout << "PIPE HIT, CLEARING " << line_num << ' ' << i << '\n';
						value.clear();
						value_str.clear();
					}
					else {
						error_message = "expected expression after the '|' sign";
						break;
					}
				}
				else {
					error_message = "unexpected '|' - no variable declared";
					break;
				}
			}
			else if (line[i] == '\"' || line[i] == '\'' ||
			line[i] == '[' || line[i] == ']' || line[i] == '{' || line[i] == '}' || line[i] == ',' ||
			((line[i] >= '0' && line[i] <= '9') || line[i] == '.') || (line[i] == 't' || line[i] == 'f') ||
			line[i] == ' ') {
				if (line[i] != ' ' && afterpipe) {
					cout << line_num << ' ' << i << " `" << line << "` `" << value << "` " << hanging_quote << ' ' << hanging_apostrophe << "\n";
					if ((!hanging_quote && !hanging_apostrophe) && (
					(!value.empty() && line[i] == '"' && *value.rbegin() != '"' && *value.rbegin() != ',' && *value.rbegin() != '[' && *value.rbegin() != '{') ||
					line[i] != '"' && (
					// No, cuz like, I didn't pretend to study competitive programming to have shit like this with somehow more parentheses in my code (in just a singular if statement!) than rejections in my life actually fucking work, somehow manage to! Somehow this passes every single of my fucking test. And I don't even know how! Fucking piece of shit code, I just want to move onto the parser!
					(!value.empty()) &&
					((isdigit(line[i]) && !isdigit(*value.rbegin()) && *value.rbegin() != ',' && *value.rbegin() != '[' && *value.rbegin() != '{') ||
					(!isdigit(line[i]) &&  isdigit(*value.rbegin()) && line[i] != ',')) ||
					((!hanging_arr.empty() || !hanging_tuple.empty()) &&
					(line[i] != '}' && line[i] != ']' && line[i] != ',' && *value.rbegin() != ',' && *value.rbegin() != '[' && *value.rbegin() != '{') ||
					line[i] == ',' && (*value.rbegin() == '[' || *value.rbegin() == '{'))
					))) {
						error_message = "unexpected expression";
						break;
					}
				}
				else {
					cout << line_num << ' ' << i << " `" << line[i] << "` " << afterpipe << "\n";
				}
				if (line[i] == '\"') {
					if (!hanging_escape && !hanging_apostrophe) {
						if (!hanging_quote) {
							hanging_quote = i;
						}
						else {
							value_str.clear();
							hanging_quote = 0;
						}
					}
					else if (hanging_escape) {
						value += '\\';
						hanging_escape = 0;
						if (hanging_apostrophe) {
							value_str += '"';
						}
					}
					value += '\"';
				}
				else if (line[i] == '\'') {
					if (hanging_escape || hanging_quote) {
						if (hanging_escape) {
							value += '\\';
							hanging_escape = 0;
						}
						if (hanging_apostrophe) {
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
						value_str.clear();
						hanging_apostrophe = 0;
					}
					value += '\'';
				}
				else if (line[i] == '[' || line[i] == ']' || line[i] == '{' || line[i] == '}') {
					if (line[i] == '[' || line[i] == '{') {
						if ((piped || afterpipe) && hanging_arr.empty() && hanging_tuple.empty()) {
							error_message = fmt::format("unexpected '{}'", line[i]);
							break;
						}
						if (!piped && !hanging_quote && !hanging_apostrophe && hanging_arr.empty() && hanging_tuple.empty()) {
							var_declared = true;
							piped = true;
							vars.push_back(value);
							cout << "ARRAY AFTERPIPE, PUSHING, CLEARING " << line_num << ' ' << i << " `" << value << "`\n";
							value.clear();
							value_str.clear();
						}
					}
					if (line[i] == '[') {
						value += '[';
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += '[';
							}
						}
						else {
							hanging_arr.push(i);
							hanging_arr_line.push(line_num);
						}
					}
					else if (line[i] == ']') {
						value += ']';
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += ']';
							}
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
						hanging_arr_line.pop();
					}
					else if (line[i] == '{') {
						value += '{';
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += '{';
							}
						}
						else {
							hanging_tuple.push(i);
							hanging_tuple_line.push(line_num);
						}
					}
					else if (line[i] == '}') {
						value += '}';
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += '}';
							}
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
						hanging_tuple_line.pop();
					}
				}
				else if (line[i] == ',') {
					if (!hanging_quote && !hanging_apostrophe) {
						if (hanging_arr.empty() && hanging_tuple.empty()) {
							error_message = "unexpected ','";
							break;
						}
						if (value.size() > 1 && *value.rbegin() == ',') {
							error_message = "expected expression, got ','";
							break;
						}
					}
					value += ',';
				}
				else if (line[i] == 't') {
					if (i <= line.size() - 4 - 1 && line.find("true", i) != std::string::npos) {
						value += "true";
						if (hanging_apostrophe) {
							value_str += "true";
						}
						i += 4 - 1;
					}
					else if ((!hanging_var || (!hanging_arr.empty() || !hanging_tuple.empty())) && !hanging_quote && !hanging_apostrophe) {
						if (!hanging_arr.empty() || !hanging_tuple.empty()) {
							error_message = "unexpected expression";
							break;
						}
					}
					else {
						value += 't';
						if (hanging_apostrophe) {
							value_str += 't';
						}
					}
				}
				else if (line[i] == 'f') {
					if (i <= line.size() - 5 - 1 && line.find("false", i) == i) {
						value += "false";
						if (hanging_apostrophe) {
							value_str += "false";
						}
						i += 5 - 1;
					}
					else if ((!hanging_var || (!hanging_arr.empty() || !hanging_tuple.empty())) && !hanging_quote && !hanging_apostrophe) {
						if (!hanging_arr.empty() || !hanging_tuple.empty()) {
							error_message = "unknown expression";
							break;
						}
					}
					else {
						value += 'f';
						if (hanging_apostrophe) {
							value_str += 'f';
						}
					}
				}
				else if (line[i] == ' ') {
					cout << "SPACE HIT " << line_num << ' ' << i << '\n';
					cout << hanging_var << ' ' << var_declared << ' ' << hanging_quote << ' ' << hanging_apostrophe << '\n';
					if ((hanging_var && !var_declared) || (hanging_quote || hanging_apostrophe)) {
						value += ' ';
						value_str += ' ';
						cout << "ADDING SPACE " << line_num << ' ' << i << '\n';
					}
				}
				else {
					value += line[i];
				}
				if (piped && line[i] != ' ' && !afterpipe) {
					afterpipe = true;
				}
			}
			else if ((!hanging_var || (!hanging_arr.empty() || !hanging_tuple.empty())) && !hanging_quote && !hanging_apostrophe) {
				error_message = "unknown expression";
				if (line[i] == '#') {
					error_message += fmt::format(".\n           Note: a comment seems to have made "
					"its way through to the lexing stage. Are you missing a '{}'?", !hanging_arr.empty() ? ']' : '}');
				}
				break;
			}
			else {
				cout << "ELSE: " << line_num << ' ' << i << " `" << line[i] << "`\n";
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
						error_message = fmt::format(R"(unexpected escape sequence: "\{}")", line[i]);
						break;
					}
					hanging_escape = 0;
				}
				else {
					to_add = line[i];
				}
				cout << "ADDING `" << to_add << "`\n";
				value += to_add;
				if (hanging_apostrophe) {
					value_str += to_add;
				}
			}
			if (line[i] == ';') {
				if (piped) {
					error_message = "expected expression after '|'";
					break;
				}
				var_declared = false;
				piped = false;
				afterpipe = false;
			}
		}
		if (!error_message.empty()) {
			break;
		}
		if (hanging_var && hanging_arr.empty() && hanging_tuple.empty()) {
			error_message = "hanging variable declaration";
			i = hanging_var;
		}
		else if (hanging_quote) {
			error_message = "hanging quote";
			i = hanging_quote;
		}
		else if (hanging_apostrophe) {
			error_message = "hanging apostrophe";
			i = hanging_apostrophe;
		}
		else if (hanging_escape) {
			error_message = "hanging escape sequence";
			i = hanging_escape;
		}
		if (!error_message.empty()) {
			break;
		}
	}
	values_str.push_back(value);
	cout << "\n----\n";
	for (size_t i = 0; i < vars.size(); i++) {
		cout << '`' << vars[i] << "` | `" << (values_str.size() > i ? values_str[i] : "") << "`\n";
	}
	cout << "----\n";
	return parse_report(error_message, line_num, i);
}

henifig::parse_report henifig::config::parse() {

}
