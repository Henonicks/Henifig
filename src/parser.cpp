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
	if (const parse_report report = parse(); report.is_error()) {
		return report;
	}
	return {};
}

henifig::parse_report henifig::config::remove_comments() {
	error_codes error_code{};
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
			if (line[i] == ' ' && hanging_escape) {
				error_code = HANGING_ESCAPE;
				break;
			}
			if (line[i] == '/' && !hanging_var && !hanging_comment && !hanging_quote && !hanging_apostrophe) {
				hanging_var = i;
			}
			if (line[i] == '\\') {
				if (!hanging_comment && line[i + 1] != '|' && line[i + 1] != ' ') {
					hanging_escape = !hanging_escape ? i : 0;
				}
				if (hanging_var && !hanging_escape && !hanging_comment && !hanging_quote && !hanging_apostrophe) {
					hanging_var = 0;
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
						error_code = IMPROPERLY_CLOSED_COMMENT;
						break;
					}
					if (!hanging_comment) {
						error_code = NO_OPENED_COMMENT;
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
				if (!hanging_apostrophe && line[i] == '"' && !hanging_escape) {
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
			if (hanging_escape != i) {
				hanging_escape = 0;
			}
			if (line[i] == '	') {
				if (!hanging_quote && !hanging_apostrophe) {
					line[i] = ' ';
				}
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
		if (error_code != OK) {
			break;
		}
		if (hanging_quote) {
			error_code = HANGING_QUOTE;
			line_num = hanging_quote_line;
			i = hanging_quote;
		}
		else if (hanging_apostrophe) {
			error_code = HANGING_APOSTROPHE;
			line_num = hanging_apostrophe_line;
			i = hanging_apostrophe;
		}
		else if (hanging_escape) {
			error_code = HANGING_ESCAPE;
			i = hanging_escape;
		}
		if (error_code != OK) {
			break;
		}
	}
	if (hanging_var) {
		error_code = HANGING_VAR;
		i = hanging_var;
	}
	else if (hanging_comment) {
		error_code = HANGING_COMMENT;
		line_num = hanging_comment_line;
		i = hanging_comment;
	}
	cout << "---\n" << parsed_content.str() << "---\n\n";
	return {error_code, line_num, i};
}

henifig::parse_report henifig::config::lex() {
	std::string line;
	size_t hanging_var{}, hanging_quote{}, hanging_apostrophe{}, hanging_escape{};
	std::stack <size_t> hanging_arr, hanging_map;
	std::stack <size_t> hanging_arr_line{}, hanging_map_line{};
	bool is_double{};
	size_t hanging_comma_line{};
	std::string value, value_str;
	bool var_declared{};
	bool piped{}, afterpipe{};
	size_t map_keys_amount{}, map_pipes_amount{};
	error_codes error_code{};

	size_t i;
	size_t line_num{};
	auto unexpected_expression = [&var_declared, &piped, &hanging_var, &hanging_quote,
	&line, &value, &hanging_apostrophe, &hanging_arr, &hanging_map, &i, &line_num]() -> bool {
		if ((var_declared && !piped) || (!var_declared && !hanging_var) ||
		(!hanging_var && !value.empty() &&
		!hanging_quote && !hanging_apostrophe && hanging_arr.empty() && hanging_map.empty()) &&

		(line[i] >= '0' && line[i] <= '9' &&
		line[i - 1] < '0' && line[i - 1] > '9' &&
		line[i - 1] != '.' && (line[i - 1] != '-' || (line[i - 1] == '-' && value.size() > 1))) ||
		(line[i] < '0' && line[i] > '9') // for some fucking reason replacing this with !isdigit(line[i]) breaks it
		) {
			cout << "UNEXPECTED EXPRESSION " << line_num << ' ' << i << '\n';
			return true;
		}
		return false;
	};
	auto is_arr = [&hanging_arr, &hanging_map]() -> bool {
		return !hanging_arr.empty() && (hanging_map.empty() || hanging_arr.top() > hanging_map.top());
	};
	auto is_map = [&hanging_arr, &hanging_map]() -> bool {
		return !hanging_map.empty() && (hanging_arr.empty() || hanging_map.top() > hanging_arr.top());
	};
	auto line_num_exists = [&value, &line_num, this, &error_code, &i]() -> bool {
		size_t& num = line_nums[value];
		if (num) {
			i -= value.size() + 1;
			error_code = REDECLARED_VAR;
			return true;
		}
		num = line_num;
		return false;
	};
	while (std::getline(parsed_content, line)) {
		cout << "VALUE STR " << line_num << " `" << value_str << "`\n";
		++line_num;
		const size_t first_index = line.find_first_not_of(' ');
		for (i = first_index; i < line.size(); i++) {
			if (line[i] != ';' && line[i] != ' ' && line[i] != '/' && line[i] != '|') {
				if (unexpected_expression()) {
					error_code = UNEXPECTED_EXPRESSION;
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
								cout << "AFTERPIPE PUSHING: " << line_num << ' ' << i << " `" << value << "`\n";
								vars.push_back(value);
								if (line_num_exists()) {
									break;
								}
								var_declared = true;
							}
							else {
								// This is a /var()\-like declaration
								if (!hanging_arr.empty()) {
									error_code = HANGING_ARR;
									line_num = hanging_arr_line.top();
									i = hanging_arr.top();
									break;
								}
								if (!hanging_map.empty()) {
									error_code = HANGING_MAP;
									line_num = hanging_map_line.top();
									i = hanging_map.top();
									break;
								}
								values_str.push_back(value);
							}
							cout << "/VAR()\\, CLEARING " << line_num << ' ' << i << '\n';
							value.clear();
							value_str.clear();
							is_double = false;
						}
						else {
							hanging_escape = i;
						}
					}
					else {
						cout << "ADDING BACKSLASH " << line_num << ' ' << i << '\n';
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
						error_code = MISSING_SEMICOLON;
						break;
					}
					else if (!afterpipe && piped) {
						error_code = HANGING_PIPE;
						break;
					}
					else {
						if (vars.size() > values_str.size()) {
							values_str.push_back(value);
						}
						cout << "NEW DECL, CLEARING " << line_num << ' ' << i << '\n';
						cout << hanging_arr.empty() << '\n';
						value.clear();
						value_str.clear();
						hanging_var = i;
						var_declared = false;
						piped = false;
						afterpipe = false;
						is_double = false;
					}
				}
			}
			else if (line[i] == '|') {
				if (is_map()) {
					value += '|';
					++map_pipes_amount;
					if (map_keys_amount != map_pipes_amount) {
						cout << "OOPSIE, MAP KEYS AND PIPES DIFFERENCE " << line_num << ' ' << i << ' ' << map_keys_amount << ' ' << map_pipes_amount << '\n';
						error_code = PIPED_VALUE;
						break;
					}
				}
				else if (!afterpipe) {
					if (!piped) {
						piped = true;
						cout << "PIPE HIT, CLEARING " << line_num << ' ' << i << '\n';
						value.clear();
						value_str.clear();
						is_double = false;
					}
					else {
						error_code = HANGING_PIPE;
						break;
					}
				}
			}
			else if (line[i] == '\"' || line[i] == '\'' ||
			line[i] == '[' || line[i] == ']' || line[i] == '{' || line[i] == '}' || line[i] == ',' || line[i] == '$' ||
			((line[i] >= '0' && line[i] <= '9') || line[i] == '.') || (line[i] == 't' || line[i] == 'f') ||
			line[i] == ' ' || line[i] == '-' || line[i] == '.') {
				if (line[i] != ' ' && afterpipe) {
					const bool is_string = hanging_quote || hanging_apostrophe;
					bool quote_after_expr{};
					bool num_after_expr{};
					bool expr_after_num{};
					bool expr_after_expr{};
					bool in_arr{};
					bool comma_in_begin{};
					bool comma_in_end{};
					bool comma_around_pipe{};
					bool middle_minus{};
					const bool escaped_num = (isdigit(line[i]) || line[i] == '.' || line[i] == '-') && hanging_escape;
					bool hanging_dot{};
					bool hanging_dollar{};
					bool unexpected_dollar{};
					bool repeated_dollar{};
					bool expected_dollar{};
					bool piped_key{};
					bool piped_value{};
					if (!hanging_escape) {
						quote_after_expr = !value.empty() && line[i] == '"' && *value.rbegin() != '"' && *value.rbegin() != ',' && *value.rbegin() != '[' && *value.rbegin() != '{' && *value.rbegin() != '|' && (line[i - 1] != '$' || line[i - 1] == '$' && !is_map()) && !hanging_quote;
						num_after_expr =   isdigit(line[i]) && !isdigit(*value.rbegin()) && *value.rbegin() != ',' && *value.rbegin() != '[' && *value.rbegin() != '{' && *value.rbegin() != '-' && *value.rbegin() != '.' && *value.rbegin() != '|';
						expr_after_num =  !isdigit(line[i]) &&  isdigit(*value.rbegin()) && line[i] != ',' && line[i] != '-' && line[i] != '.' && line[i] != ']' && line[i] != '}';
						expr_after_expr = !isdigit(line[i]) && line[i] != '"' && line[i] != ']' && line[i] != '}' && line[i] != ',' && line[i] != '-' && line[i] != '.' && *value.rbegin() != ',' && *value.rbegin() != '[' && *value.rbegin() != '{' && *value.rbegin() != '|';
						in_arr = !hanging_arr.empty() || !hanging_map.empty();
						comma_in_begin = line[i] == ',' && (*value.rbegin() == '[' || *value.rbegin() == '{');
						comma_in_end = line[i] == ',' && (*value.rbegin() == '[' || *value.rbegin() == '{');
						comma_around_pipe = line[i] == ',' && (var_declared && !piped) || (piped && value.empty());
						middle_minus = line[i] == '-' && line[i - 1] == '-';
						hanging_dot = line[i] == '.' && !isdigit(line[i + 1]);
						hanging_dollar = line[i] == '$' && line[i + 1] != '"';
						unexpected_dollar = line[i] == '$' && (hanging_map.empty() || (!hanging_arr.empty() && hanging_map.top() < hanging_arr.top()));
						repeated_dollar = line[i] == '$' && line[i - 1] == '$';
						expected_dollar = line[i] != '$' && line[i - 1] != '$' && line[i] != ',' && line[i] != '}' && (value.empty() || !value.empty() && *value.rbegin() != '|') && is_map();
						piped_key = line[i] == '$' && (!value.empty() && *value.rbegin() != '$' && *value.rbegin() != '{' && *value.rbegin() != ',');
					}
					// I didn't say I had a lot of tests
					if (!is_string && ((quote_after_expr ||
					line[i] != '"' && (!value.empty() &&
					((num_after_expr || expr_after_num || expr_after_expr) ||
					(in_arr && comma_in_begin || comma_in_end))
					) || comma_around_pipe) ||
					middle_minus || escaped_num || hanging_dot ||
					hanging_dollar || unexpected_dollar || repeated_dollar || expected_dollar || piped_key || piped_value)) {
						cout << "BIG-ASS CHECK " << line_num << ' ' << i << '\n';
						if (quote_after_expr || num_after_expr || expr_after_num || expr_after_expr) {
							error_code = UNEXPECTED_EXPRESSION;
						}
						else if (comma_in_begin || comma_in_end || comma_around_pipe) {
							error_code = UNEXPECTED_COMMA;
						}
						else if (middle_minus) {
							error_code = MINUS_IN_MIDDLE;
						}
						else if (escaped_num) {
							error_code = UNDEFINED_ESCAPE;
						}
						else if (hanging_dot) {
							error_code = HANGING_DOT;
						}
						else if (hanging_dollar) {
							error_code = HANGING_DOLLAR;
						}
						else if (unexpected_dollar) {
							error_code = UNEXPECTED_DOLLAR;
						}
						else if (repeated_dollar) {
							error_code = REPEATED_DOLLAR;
						}
						else if (expected_dollar) {
							 error_code = EXPECTED_DOLLAR;
						}
						else if (piped_key) {
							error_code = PIPED_KEY;
						}
						else {
							error_code = WRONG_EXPRESSION;
						}
						break;
					}
				}
				if (line[i] == '\"') {
					if (!hanging_escape && !hanging_apostrophe) {
						if (!hanging_quote) {
							hanging_quote = i;
						}
						else {
							hanging_quote = 0;
						}
					}
					else if (hanging_escape) {
						if (piped) {
							value += '\\';
						}
						if (hanging_apostrophe) {
							value_str += '"';
						}
					}
					value += '"';
				}
				else if (line[i] == '\'') {
					if (!hanging_escape && !hanging_quote) {
						if (!hanging_apostrophe) {
							hanging_apostrophe = i;
						}
						else if (value_str.size() > 1) {
							error_code = MULTIPLE_CHARS;
							break;
						}
						else if (value_str.empty()) {
							error_code = NO_CHARS;
							break;
						}
						else {
							value_str.clear();
							hanging_apostrophe = 0;
						}
					}
					else {
						if (hanging_escape) {
							if (afterpipe) {
								value += '\\';
							}
						}
						if (hanging_apostrophe) {
							value_str += '\'';
						}
					}
					value += '\'';
				}
				else if (line[i] == '[' || line[i] == ']' || line[i] == '{' || line[i] == '}') {
					if (line[i] == '[' || line[i] == '{') {
						if ((piped || afterpipe) && hanging_arr.empty() && hanging_map.empty()) {
							if (line[i] == '[') {
								error_code = UNEXPECTED_ARR;
							}
							else {
								error_code = UNEXPECTED_MAP;
							}
							break;
						}
						if (!piped && !hanging_escape && !hanging_quote && !hanging_apostrophe && hanging_arr.empty() && hanging_map.empty()) {
							cout << "ARRAY AFTERPIPE, PUSHING, CLEARING " << line_num << ' ' << i << " `" << value << "`\n";
							var_declared = true;
							piped = true;
							vars.push_back(value);
							if (line_num_exists()) {
								break;
							}
							value.clear();
							value_str.clear();
							is_double = false;
						}
					}
					if (line[i] == '[') {
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += '[';
							}
						}
						else if (!hanging_escape) {
							hanging_arr.push(i);
							hanging_arr_line.push(line_num);
						}
						else {
							if (afterpipe) {
								value += '\\';
							}
						}
						value += '[';
					}
					else if (line[i] == ']') {
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += ']';
							}
						}
						else if (!hanging_escape) {
							if (hanging_arr.empty()) {
								error_code = UNEXPECTED_ARR_END;
								break;
							}
							if (!hanging_map.empty() && hanging_map.top() > hanging_arr.top()) {
								error_code = MAP_COMPLETED_WITH_ARR;
								break;
							}
							if (*value.rbegin() == ',') {
								error_code = HANGING_COMMA;
								break;
							}
							hanging_arr.pop();
							hanging_arr_line.pop();
						}
						value += ']';
					}
					else if (line[i] == '{') {
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += '{';
							}
						}
						else if (!hanging_escape) {
							hanging_map.push(i);
							hanging_map_line.push(line_num);
						}
						else {
							if (afterpipe) {
								value += '\\';
							}
						}
						value += '{';
					}
					else if (line[i] == '}') {
						if (hanging_quote || hanging_apostrophe) {
							if (hanging_apostrophe) {
								value_str += '}';
							}
						}
						else if (!hanging_escape) {
							if (hanging_map.empty()) {
								error_code = UNEXPECTED_MAP_END;
								break;
							}
							if (!hanging_arr.empty() && hanging_arr.top() > hanging_map.top()) {
								error_code = MAP_COMPLETED_WITH_ARR;
								break;
							}
							if (*value.rbegin() == ',') {
								error_code = HANGING_COMMA;
								break;
							}
							hanging_map.pop();
							hanging_map_line.pop();
						}
						value += '}';
					}
				}
				else if (line[i] == ',') {
					if (!hanging_quote && !hanging_apostrophe) {
						if (hanging_arr.empty() && hanging_map.empty()) {
							error_code = UNEXPECTED_COMMA;
							break;
						}
						if (value.size() > 1 && *value.rbegin() == ',') {
							error_code = EXPECTED_EXPRESSION;
							break;
						}
						if (is_map()) {
							if (map_pipes_amount >= map_keys_amount) {
								--map_pipes_amount;
							}
							--map_keys_amount;
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
					else if ((!hanging_var || (!hanging_arr.empty() || !hanging_map.empty())) && !hanging_quote && !hanging_apostrophe) {
						if (!hanging_arr.empty() || !hanging_map.empty()) {
							error_code = UNEXPECTED_EXPRESSION;
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
					else if ((!hanging_var || (!hanging_arr.empty() || !hanging_map.empty())) && !hanging_quote && !hanging_apostrophe) {
						if (!hanging_arr.empty() || !hanging_map.empty()) {
							error_code = UNKNOWN_EXPRESSION;
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
				else if (line[i] == '.') {
					if (is_double) {
						error_code = REPEATED_DOT;
						break;
					}
					if (!hanging_quote && !hanging_apostrophe) {
						is_double = true;
						if (!isdigit(*value.rbegin())) {
							value += '0';
						}
					}
					value += '.';
				}
				else if (line[i] == ' ') {
					cout << "SPACE HIT " << line_num << ' ' << i << '\n';
					cout << hanging_var << ' ' << var_declared << ' ' << hanging_quote << ' ' << hanging_apostrophe << '\n';
					if (hanging_escape) {
						cout << "HANGING ESCAPE SEQUENCE\n";
						error_code = HANGING_ESCAPE;
						break;
					}
					if ((hanging_var && !var_declared) || (hanging_quote || hanging_apostrophe)) {
						value += ' ';
						if (hanging_apostrophe) {
							value_str += ' ';
						}
						cout << "ADDING SPACE " << line_num << ' ' << i << '\n';
					}
				}
				else if (line[i] == '$') {
					if (hanging_quote || hanging_apostrophe) {
						value += '$';
						if (hanging_apostrophe) {
							value_str += '$';
						}
					}
					else if (is_map()) {
						++map_keys_amount;
					}
				}
				else {
					value += line[i];
				}
				if (piped && line[i] != ' ' && !afterpipe) {
					afterpipe = true;
				}
			}
			else if (line[i] != ';' && (!hanging_var || (!hanging_arr.empty() || !hanging_map.empty())) && !hanging_quote && !hanging_apostrophe) {
				cout << "UNKNOWN EXPRESSION\n";
				error_code = UNKNOWN_EXPRESSION;
				break;
			}
			else if (line[i] != ';') {
				cout << "ELSE: " << line_num << ' ' << i << " `" << line[i] << "`\n";
				char to_add;
				if (hanging_escape) {
					if (line[i] == 'n') {
						to_add = '\n';
					}
					else if (line[i] == ' ') {
						error_code = HANGING_ESCAPE;
						break;
					}
					else {
						error_code = UNDEFINED_ESCAPE;
						break;
					}
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
				if (!hanging_quote && !hanging_apostrophe && !hanging_var) {
					if (piped && !afterpipe) {
						error_code = HANGING_PIPE;
						break;
					}
					var_declared = false;
					piped = false;
					afterpipe = false;
				}
				else {
					value += ';';
					if (hanging_apostrophe) {
						value_str += ';';
					}
				}
			}
			if (hanging_escape != i) {
				hanging_escape = 0;
			}
		}
		if (error_code != OK) {
			break;
		}
		if (hanging_var && hanging_arr.empty() && hanging_map.empty()) {
			error_code = HANGING_VAR;
			i = hanging_var;
		}
		else if (hanging_quote) {
			error_code = HANGING_QUOTE;
			i = hanging_quote;
		}
		else if (hanging_apostrophe) {
			error_code = HANGING_APOSTROPHE;
			i = hanging_apostrophe;
		}
		else if (hanging_escape) {
			error_code = HANGING_ESCAPE;
			i = hanging_escape;
		}
		if (error_code != OK) {
			break;
		}
	}
	values_str.push_back(value);
	cout << "\n----\n";
	for (size_t i = 0; i < vars.size(); i++) {
		cout << '`' << vars[i] << "` | `" << (values_str.size() > i ? values_str[i] : "") << "`\n";
	}
	cout << "----\n";
	if (error_code == OK && (piped && !afterpipe)) {
		error_code = HANGING_PIPE;
		i -= 2;
	}
	return {error_code, line_num, i};
}

henifig::error_codes henifig::config::print_value(const value_variant& x) {
	switch (x.index()) {
		case declaration: {
			cout << "<declaration>\n";
			break;
		}
		case string: {
			cout << "string(" << std::get <std::string>(x) << ")\n";
			break;
		}
		case character: {
			cout << "character(" << std::get <unsigned char>(x) << ")\n";
			break;
		}
		case number: {
			cout << "number(" << std::get <double>(x) << ")\n";
			break;
		}
		case boolean: {
			cout << "boolean(" << (std::get <bool>(x) == 1 ? "true" : "false") << ")\n";
			break;
		}
		case array: {
			print_array(arrs[std::get <array_index>(x).index]);
			break;
		}
		case map: {
			print_map(maps[std::get <map_index>(x).index]);
			break;
		}
		default: {
			cout << "<unknown>\n";
			return UNKNOWN_TYPE;
		}
	}
	return OK;
}

void henifig::config::print_spaces() const {
	for (int i = 0; i < spaces; i++) {
		cout << ' ';
	}
}

henifig::error_codes henifig::config::print_array(const std::vector <value_variant>& x) {
	error_codes error_code = OK;
	cout << "array[";
	if (x.empty()) {
		cout << "]\n";
		return error_code;
	}
	cout << '\n';
	spaces += 2;
	for (const value_variant& y : x) {
		print_spaces(); error_code = print_value(y);
	}
	spaces -= 2;
	print_spaces(); cout << "]\n";
	return error_code;
}

henifig::error_codes henifig::config::print_map(const std::map <std::string, value_variant>& x) {
	error_codes error_code = OK;
	cout << "map{";
	if (x.empty()) {
		cout << "}\n";
		return error_code;
	}
	cout << '\n';
	spaces += 2;
	for (const auto& y : x) {
		print_spaces();
		cout << "key(" << y.first << ") | ";
		error_code = print_value(y.second);
	}
	spaces -= 2;
	print_spaces(); cout << "}\n";
	return error_code;
}

henifig::parse_report henifig::config::parse() {
	error_codes error_code{};
	for (size_t i = 0; i < vars.size(); i++) {
		parse_value(i);
	}
	cout << "-------\n";
	for (const value_variant& x : values) {
		error_code = print_value(x);
	}
	cout << "-------\n";
	return error_code;
}
#define append_to_map(original_value, new_value) \
	value_variant& map_value_ref = original_value; \
	if (map_value_ref.index() != unset) { \
		print_value(original_value); \
		print_value(new_value); \
		return REDECLARED_KEY; \
	} \
	map_value_ref = new_value

henifig::error_codes henifig::config::append(depth_t& depth, const value_variant& value) {
	switch (depth.index_type) {
		case VAR: {
			values.push_back(value);
			break;
		}
		case ARR: {
			if (depth.environment_type == VAR) {
				values.emplace_back(array_index{arrs.size()});
			}
			else if (depth.environment_type == ARR) {
				arrs[arr_indexes.top()].emplace_back(array_index{arrs.size()});
			}
			else {
				maps[map_indexes.top()][map_keys[depth.map_index]] = array_index{arrs.size()};
			}
			arr_indexes.push(arrs.size());
			arrs.emplace_back();
			depth.environment_type = ARR;
			break;
		}
		case MAP: {
			if (depth.environment_type == VAR) {
				values.emplace_back(map_index{maps.size()});
			}
			else if (depth.environment_type == ARR) {
				arrs[arr_indexes.top()].emplace_back(map_index{maps.size()});
			}
			else {
				maps[map_indexes.top()][map_keys[depth.map_index]] = map_index{maps.size()};
			}
			map_indexes.push(maps.size());
			maps.emplace_back();
			depth.environment_type = MAP;
			break;
		}
		case ARR_ITEM: {
			arrs[arr_indexes.top()].emplace_back(value);
			break;
		}
		case MAP_KEY: {
			cout << "MAP_KEY " << depth.map_index << ' ';
			print_value(value);
			append_to_map(maps[map_indexes.top()][std::get <std::string>(value)], declaration_t{});
			map_keys[depth.map_index] = std::get <std::string>(value);
			break;
		}
		case MAP_VALUE: {
			cout << "MAP_VALUE " << depth.map_index << ' ' << map_keys[depth.map_index] << ' ';
			print_value(value);
			maps[map_indexes.top()][map_keys[depth.map_index]] = value;
			break;
		}
		default: break;
	}
	return OK;
}

#undef append_to_map

#define appender(depth, value) \
	error_codes error_code = append(depth, value); \
	if (error_code != OK) \
		throw parse_exception(parse_report(error_code, line_nums[vars[var_num]])) \

#define container_appender(depth) \
	appender(depth, unset_t{})

size_t henifig::config::parse_value(const size_t& var_num, const size_t& pos, depth_t depth) {
	const std::string_view line = values_str[var_num];
	if (pos >= line.size()) {
		if (pos == 0) {
			append(depth);
		}
		return pos;
	}
	size_t i = pos;
	switch (line[i]) {
		case '"': {
			size_t hanging_escape{};
			std::string value;
			for (++i; i < line.size(); i++) {
				switch (line[i]) {
					case '\\': {
						if (hanging_escape) {
							value += '\\';
							hanging_escape = 0;
						}
						else {
							hanging_escape = i;
						}
						break;
					}
					case '"': {
						if (hanging_escape) {
							hanging_escape = 0;
							value += line[i];
						}
						else {
							if (i < line.size() - 1 && line[i + 1] == '"') {
								++i;
								break;
							}
							appender(depth, value);
							return parse_value(var_num, i + 1, depth);
						}
						break;
					}
					default: {
						if (hanging_escape) {
							hanging_escape = 0;
							if (line[i] == 'n') {
								value += '\n';
							}
						}
						else {
							value += line[i];
						}
						break;
					}
				}
			}
			break;
		}
		case '\'': {
			unsigned char value = line[i + 1];
			if (value == '\\') {
				if (line[i + 2] == 'n') {
					value = '\n';
				}
				else {
					value = line[i + 2];
				}
			}
			appender(depth, value);
			return parse_value(var_num, i + 4, depth);
		}
		case '0' ... '9': case '-': {
			std::string value;
			for (;i < line.size(); i++) {
				if (isdigit(line[i]) || line[i] == '.' || line[i] == '-') {
					value += line[i];
				}
				else {
					break;
				}
			}
			appender(depth, std::stod(value));
			return parse_value(var_num, i, depth);
		}
		case 't': {
			appender(depth, true);
			return parse_value(var_num, i + 4, depth);
		}
		case 'f': {
			appender(depth, false);
			return parse_value(var_num, i + 5, depth);
		}
		case '[': {
			depth_t new_depth = depth;
			++new_depth.arr_index;
			new_depth.index_type = ARR;
			container_appender(new_depth);
			new_depth.index_type = ARR_ITEM;
			size_t new_pos = parse_value(var_num, pos + 1, new_depth);
			return parse_value(var_num, new_pos, depth);
		}
		case '{': {
			depth_t new_depth = depth;
			++new_depth.map_index;
			new_depth.index_type = MAP;
			container_appender(new_depth);
			new_depth.index_type = MAP_KEY;
			size_t new_pos = parse_value(var_num, pos + 1, new_depth);
			return parse_value(var_num, new_pos, depth);
		}
		case ',': {
			if (depth.index_type == MAP_VALUE) {
				depth.index_type = MAP_KEY;
			}
			return parse_value(var_num, pos + 1, depth);
		}
		case '|': {
			depth.index_type = MAP_VALUE;
			return parse_value(var_num, pos + 1, depth);
		}
		case ']': {
			arr_indexes.pop();
			return i + 1;
		}
		case '}': {
			map_indexes.pop();
			return i + 1;
		}
		default: break;
	}
	return i;
}

#undef appender
#undef container_appender
