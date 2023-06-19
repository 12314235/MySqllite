

#include "MySQLite.h"

bool MySQLite::request(const std::string &commands) {
    std::regex e("([^;]+;)");

    std::sregex_iterator iter(commands.begin(), commands.end(), e);
    std::sregex_iterator end;

    while (iter != end) {
        if(!parse_command((*iter)[0])) return false;
        ++iter;
    }
    return true;
}

bool MySQLite::parse_command(const std::string &command) {
    std::regex params(R"(\(([^;]+)\))");
    std::regex name("([^\\s;,]+)");
    std::sregex_iterator end;
    std::sregex_iterator iter;
    std::sregex_iterator iter2;
    try {
        if (std::regex_search(command, std::regex("CREATE TABLE"))) {
            iter = std::sregex_iterator (command.begin(), command.end(), params);
            iter2 = std::sregex_iterator (command.begin(), command.end(), name);
            iter2++;
            iter2++;
            CREATE_TABLE((*iter)[0], (*iter2)[0]);
        } else if (std::regex_search(command, std::regex("DROP TABLE"))) {
            iter2 = std::sregex_iterator (command.begin(), command.end(), name);
            iter2++;
            iter2++;
            while(iter2 != end) {
                DROP_TABLE((*iter2)[0]);
                iter2++;
            }
        }
        else if (std::regex_search(command, std::regex("INSERT INTO"))) {
            std::regex p(R"(\(([^;\(\)]+)\))");
            iter = std::sregex_iterator (command.begin(), command.end(), p);
            iter2 = std::sregex_iterator (command.begin(), command.end(), name);
            iter2++;
            iter2++;
            std::string table_name = (*iter2)[0];
            std::string column_names = (*iter)[0];
            std::vector<std::string> new_values;
            iter++;

            while(iter != end) {
                new_values.emplace_back((*iter)[0]);
                ++iter;
            }

            INSERT(table_name, column_names, new_values);
        } else if(std::regex_search(command, std::regex("DELETE FROM"))) {
            iter = std::sregex_iterator (command.begin(), command.end(), name);
            ++iter;
            ++iter;
            std::string table_name = (*iter)[0];
            ++iter;
            ++iter;
            std::string condition;
            while(iter != end) {
                condition += (*iter)[0];
                condition += ' ';
                ++iter;
            }

            DELETE(table_name, condition);
        } else if (std::regex_search(command, std::regex("UPDATE"))) {
            iter = std::sregex_iterator (command.begin(), command.end(), name);
            ++iter;
            std::string table_name = (*iter)[0];
            ++iter;
            ++iter;
            std::string updating;
            while(iter != end && (*iter)[0] != "WHERE") {
                updating += (*iter)[0];
                updating += ' ';
                ++iter;
            }
            ++iter;

            std::string condition;
            while(iter != end) {
                condition += (*iter)[0];
                condition += ' ';
                ++iter;
            }

            UPDATE(table_name, condition, updating);
        } else if(std::regex_search(command, std::regex("SELECT"))) {
            iter = std::sregex_iterator (command.begin(), command.end(), name);
            ++iter;
            std::vector<std::string> colums;
            while((*iter)[0] != "FROM") {
                colums.emplace_back((*iter)[0]);
                ++iter;
            }
            ++iter;
            std::string table_name = (*iter)[0];
            std::string condition;
            std::string ON_CONDITION;
            std::string first_table;
            std::string type_join;
            bool have_join = false;
            if(std::regex_search(command, std::regex("JOIN"))) {
                ++iter;
                type_join = (*iter)[0];
                ++iter;
                ++iter;
                first_table = (*iter)[0];
                ++iter;
                ++iter;
                while ((*iter)[0] != "WHERE" && iter != end) {
                    ON_CONDITION += (*iter)[0];
                    ON_CONDITION += ' ';
                    ++iter;
                }
                have_join = true;
            }
            if(std::regex_search(command, std::regex("WHERE"))) {
                while((*iter)[0] != "WHERE") {
                    ++iter;
                }
                ++iter;
                while(iter != end) {
                    condition += (*iter)[0];
                    condition += ' ';
                    ++iter;
                }
            }

            if(have_join) {
                SELECT_JOIN(table_name, colums, condition, first_table, ON_CONDITION, type_join);
            } else {
                SELECT(table_name, colums, condition);
            }
        } else if(std::regex_search(command, std::regex("UPLOAD"))) {
            std::regex a(R"(([^\s\(\)]+))");
            iter = std::sregex_iterator (command.begin(), command.end(), a);
            ++iter;
            UPLOAD((*iter)[0]);
        } else if(std::regex_search(command, std::regex("DOWNLOAD"))) {
            std::regex a(R"(([^\s\(\)]+))");
            iter = std::sregex_iterator (command.begin(), command.end(), a);
            ++iter;
            DOWNLOAD((*iter)[0]);
        } else {
            throw std::runtime_error(command);
        }
    } catch(const std::exception& e) {
        std::cout << "Wrong command: " << e.what() << '\n';
        return false;
    }

    return true;
}

void MySQLite::CREATE_TABLE(std::string params, const std::string &table_name) {
    std::regex a(R"(([^\s\(\);,]+\s+[^\s,]+[,\)]))");
    std::regex foreign("(FOREIGN\\s+KEY\\s+[^,;]+)");
    std::sregex_iterator foreign_iter(params.begin(), params.end(), foreign);
    std::sregex_iterator end;

    std::string buff;

    std::regex b("([^\\s;,]+)");

    table new_table;

    new_table.table_name = table_name;

    int counter = 0;

    std::string foreign_key;

    while(foreign_iter != end) {
        foreign_key = (*foreign_iter)[0];
        std::regex sp(R"(([^\s\(\)]+))");
        std::sregex_iterator it(foreign_key.begin(), foreign_key.end(), sp);
        ++it;
        ++it;
        std::string column_name = (*it)[0];
        ++it;
        ++it;
        std::string ref_table = (*it)[0];
        ++it;
        new_table.foreign_keys.emplace_back(column_name, std::make_pair(ref_table, (*it)[0]));
        ++foreign_iter;
    }

    params = std::regex_replace(params, foreign, "");

    std::sregex_iterator iter(params.begin(), params.end(), a);

    while (iter != end) {
        buff = (*iter)[0];
        std::sregex_iterator it(buff.begin(), buff.end(), b);
        new_table.colums.emplace_back((*it)[0]);
        new_table.column_indexes.insert({(*it)[0], counter});
        it++;
        new_table.column_types.push_back(what_type((*it)[0]));

        ++iter;
        ++counter;
    }

    this->tables.insert({new_table.table_name, new_table});
}

MySQLite::TYPES MySQLite::what_type(const std::string &type) {
    try {
        if (std::regex_search(type, std::regex("(I|i)(N|n)(T|t)"))) {
            return INT;
        } else if (std::regex_search(type, std::regex("(B|b)(O|o){2}(L|l)"))) {
            return BOOL;
        } else if (std::regex_search(type, std::regex("(F|f)(L|l)(O|o)(A|a)(T|t)"))) {
            return FLOAT;
        } else if (std::regex_search(type, std::regex("(D|d)(O|o)(U|u)(B|b)(L|l)(E|e)"))) {
            return DOUBLE;
        } else if (std::regex_search(type, std::regex(R"((V|v)(A|a)(R|r)(C|c)(H|h)(A|a)(R|r)\s?+\(\d+)"))) {
            return VARCHAR;
        } else {
            throw std::runtime_error("Wrong type " + type);
        }
    } catch (const std::exception &e) {
        std::cout << "Create table error: " << e.what() << '\n';
    }
}

void MySQLite::DROP_TABLE(const std::string &table_name) {
    try {
        this->tables.erase(table_name);
    } catch(const std::exception& e) {
        std::cout << "DROP TABLE Error, wrong table name: " << table_name << ". Error message: " << e.what() << '\n';
    }
}

void MySQLite::INSERT(const std::string &table_name, const std::string &column_names, const std::vector<std::string> &values) {
    std::regex reg(R"(([^\s\(\),]+))");

    for(auto v : values) {
        std::sregex_iterator colums(column_names.begin(), column_names.end(), reg);
        std::sregex_iterator val(v.begin(), v.end(), reg);
        std::sregex_iterator end;

        std::vector<std::variant<bool, int, float, double, std::string>> new_line(
                this->tables.at(table_name).column_types.size());

        int ind;
        try {
            while (colums != end && val != end) {
                ind = this->tables.at(table_name).column_indexes.at((*colums)[0]);
                new_line[ind] = value_parse((*val)[0], this->tables.at(table_name).column_types[ind]);
                ++colums;
                ++val;
            }

            if (colums != end || val != end) {
                throw std::runtime_error("INSERT ERROR");
            }

            this->tables.at(table_name).lines.emplace_back(new_line);
        } catch (const std::exception &e) {
            std::cout << e.what() << '\n';
        }

    }
}

std::variant<bool, int, float, double, std::string> MySQLite::value_parse(const std::string &value, const TYPES& expected_type) {
    try {
        if (std::regex_search(value, std::regex("^[Tt][Rr][Uu][Ee]$"))) {
            if (expected_type != BOOL) throw std::runtime_error("Wrong type");
            return true;
        } else if (std::regex_search(value, std::regex("^[Ff][Aa][Ll][Ss][Ee]$"))) {
            if (expected_type != BOOL) throw std::runtime_error("Wrong type");
            return false;
        } else if (std::regex_search(value, std::regex("^\\d+$"))) {
            if (expected_type != INT) throw std::runtime_error("Wrong type");
            return std::stoi(value);
        } else if (std::regex_search(value, std::regex(R"(^\d+\.\d+$)"))) {
            if (expected_type != FLOAT) throw std::runtime_error("Wrong type");
            return std::stof(value);
        } else if (std::regex_search(value, std::regex(R"(^\d+\.\d+$)"))) {
            if (expected_type != DOUBLE) throw std::runtime_error("Wrong type");
            return std::stod(value);
        } else if(std::regex_search(value, std::regex("^'.+'$"))) {
            if (expected_type != VARCHAR) throw std::runtime_error("Wrong type");
            return value;
        } else {
            throw std::runtime_error("Wrong type");
        }
    } catch(const std::exception& e) {
        std::cout << e.what() << '\n';
    }
}

std::vector<std::string> MySQLite::logical_line_parse(const std::string& line) {
    std::vector<std::string> stack;
    std::vector<std::string> result;

    std::string buff;

    try {
        for (std::string::const_iterator i = line.begin(); i != line.end(); ++i) {
            if (*i == '(') {
                stack.emplace_back(std::to_string(*i));
            } else if (*i == ')') {
                if(!buff.empty()) {
                    result.emplace_back(buff);
                    buff.clear();
                }
                while (stack.back() != std::to_string('(')) {
                    result.emplace_back(stack.back());
                    stack.erase(stack.end() - 1);
                }
                stack.erase(stack.end() - 1);
            } else if (is_operator(i, line.end())) {
                if(!buff.empty()) {
                    result.emplace_back(buff);
                    buff.clear();
                }
                while (*i == ' ') ++i;
                while (*i != ' ') {
                    buff += *i;
                    ++i;
                }
                if (stack.empty() or stack.back() == std::to_string('(')) {
                    stack.emplace_back(buff);
                } else if (priority.at(buff) > priority.at(stack.back())) {
                    stack.emplace_back(buff);
                } else if (priority.at(buff) <= priority.at(stack.back())) {
                    while (priority.at(stack.back()) >= priority.at(buff)) {
                        result.emplace_back(stack.back());
                        stack.erase(stack.end() - 1);
                    }
                    stack.emplace_back(buff);
                }
                buff.clear();
            } else {
                buff += (*i);
            }
        }

        if(!buff.empty()) result.emplace_back(buff);

        while (!stack.empty()) {
            result.emplace_back(stack.back());
            stack.erase(stack.end() - 1);
        }
    } catch (const std::exception& e) {
        std::cout << "Wrong logical argument " << line << '\n';
    }

    return result;
}

bool MySQLite::is_operator(std::string::const_iterator current,
                       std::string::const_iterator end) {
    int count_without_spaces = 0;
    std::string buff;
    bool f = false;
    for(auto i = current; i != end; ++i) {
        if(*i == ' ' && !f) continue;
        if(*i == ' ' && f) break;
        f = true;
        buff += *i;
        ++count_without_spaces;
        if(count_without_spaces == 2) {
            if(buff != "OR" && buff != "AN" && buff != "NO") return false;
        }
    }

    return buff == "OR" || buff == "AND" || buff == "NOT";
}

bool MySQLite::is_accepted_line(const std::vector<std::variant<bool, int, float, double, std::string>> &line,
                                const std::vector<std::string> &condition, const std::string& table_name) {
    std::vector<std::string> stack;
    std::string condition1;
    std::string condition2;

    for(auto i : condition) {
        if(!is_operator(i.begin(), i.end())) {
            stack.emplace_back(i);
        } else {
            if(i == "NOT") {
                condition1 = stack.back();
                stack.erase(stack.end() - 1);
                stack.emplace_back(NOT(condition_accept(condition1, table_name, line)));
            } else {
                condition1 = stack.back();
                stack.erase(stack.end() - 1);
                condition2 = stack.back();
                stack.erase(stack.end() - 1);
                if(i == "AND") {
                    stack.emplace_back(AND(condition_accept(condition1, table_name, line), condition_accept(condition2, table_name, line)));
                } else if(i == "OR") {
                    stack.emplace_back(OR(condition_accept(condition1, table_name, line), condition_accept(condition2, table_name, line)));
                }
            }

        }
    }

    if(stack[0] != "true" || stack[0] != "false") stack[0] = (condition_accept(stack[0], table_name, line) ? "true" : "false");

    return stack[0] == "true";
}

bool MySQLite::condition_accept(const std::string &condition, const std::string &table_name, const std::vector<std::variant<bool, int, float, double, std::string>> &line) {
    if(std::regex_search(condition, std::regex("true"))) {
        return true;
    } else if(std::regex_search(condition, std::regex("false"))) {
        return false;
    } else {
        std::regex a(R"(([^;\s\(\)]+))");
        std::sregex_iterator iter(condition.begin(), condition.end(), a);
        std::string column_name = (*iter)[0];
        ++iter;
        std::string operation = (*iter)[0];
        ++iter;
        std::string rhs = (*iter)[0];
        std::variant<bool, int, float, double, std::string> rhs_casted;
        TYPES column_type = this->tables.at(table_name).column_types[this->tables.at(table_name).column_indexes.at(column_name)];
        rhs_casted = type_cast(rhs, column_type);
        if(operation == ">") {
            return line[this->tables.at(table_name).column_indexes.at(column_name)] > rhs_casted;
        } else if(operation == ">=") {
            return line[this->tables.at(table_name).column_indexes.at(column_name)] >= rhs_casted;
        } else if(operation == "=") {
            return line[this->tables.at(table_name).column_indexes.at(column_name)] == rhs_casted;
        } else if(operation == "<=") {
            return line[this->tables.at(table_name).column_indexes.at(column_name)] <= rhs_casted;
        } else if(operation == "<") {
            return line[this->tables.at(table_name).column_indexes.at(column_name)] < rhs_casted;
        } else if(operation == "!=") {
            return line[this->tables.at(table_name).column_indexes.at(column_name)] != rhs_casted;
        }
    }
}

std::string MySQLite::AND(bool c1, bool c2) {
    return (c1 && c2 ? "true" : "false");
}

std::string MySQLite::OR(bool c1, bool c2) {
    return (c1 || c2 ? "true" : "false");
}

std::string MySQLite::NOT(bool c) {
    return (c ? "true" : "false");
}

std::vector<int> MySQLite::WHERE(const std::string& table_name, const std::string &condition) {
    std::vector<int> result;
    for(int i = 0; i < this->tables.at(table_name).lines.size(); i++) {
        if(is_accepted_line(this->tables.at(table_name).lines[i], logical_line_parse(condition), table_name)) {
            result.push_back(i);
        }
    }

    return result;
}

void MySQLite::DELETE(const std::string &table_name, const std::string &condition) {
    std::vector<int> delete_rows = this->WHERE(table_name, condition);
    auto table = this->tables.at(table_name).lines.begin();
    for(auto i : delete_rows) {
        this->tables.at(table_name).lines.erase(table + i);
    }
}



void MySQLite::UPDATE(const std::string &table_name, const std::string &condition, const std::string &updating) {
    std::vector<int> updating_lines = WHERE(table_name, condition);
    std::regex r("([^\\s;,]+)");
    std::sregex_iterator iter(updating.begin(), updating.end(), r);
    std::sregex_iterator end;
    std::string column_name;

    while(iter != end) {
        column_name = (*iter)[0];
        ++iter;
        ++iter;
        for(auto i : updating_lines) {
            this->tables.at(table_name).lines[i][this->tables.at(table_name).column_indexes.at(column_name)] = type_cast((*iter)[0], this->tables.at(table_name).column_types[this->tables.at(table_name).column_indexes.at(column_name)]);
        }
        ++iter;
    }
}

std::variant<bool, int, float, double, std::string> MySQLite::type_cast(const std::string &rhs, TYPES column_type) {
    if(column_type == INT) {
        return std::stoi(rhs);
    } else if (column_type == FLOAT) {
        return std::stof(rhs);
    } else if (column_type == DOUBLE) {
        return std::stod(rhs);
    } else if (column_type == BOOL) {
        if(std::regex_search(rhs, std::regex("^[Tt][Rr][Uu][Ee]$"))) {
            return true;
        } else if(std::regex_search(rhs, std::regex("^[Ff][Aa][Ll][Ss][Ee]$"))) {
            return false;
        }
    } else if(column_type == VARCHAR) {
        return rhs;
    }
}

void MySQLite::SELECT(const std::string &table_name, std::vector<std::string> colums, const std::string& condition) {
    std::vector<int> selected_lines = WHERE(table_name, condition);
    table* current_table = &this->tables.at(table_name);
    std::cout << "SELECT " << condition << " FROM " << table_name << '\n';
    std::cout << "------------------------------------------------------------\n";
    if(colums[0] == "*") {
        colums.assign(current_table->colums.begin(), current_table->colums.end());
    }
    for(auto k : colums) {
        std::cout << k << ' ';
    }
    std::cout << '\n';
    for(auto i : selected_lines) {
        for(auto j : colums) {
            if(current_table->column_types[current_table->column_indexes.at(j)] == INT) {
                std::cout << std::get<int>(current_table->lines[i][current_table->column_indexes.at(j)]) << ' ';
            } else if(current_table->column_types[current_table->column_indexes.at(j)] == BOOL) {
                std::cout << std::get<bool>(current_table->lines[i][current_table->column_indexes.at(j)]) << ' ';
            } else if(current_table->column_types[current_table->column_indexes.at(j)] == FLOAT) {
                std::cout << std::get<float>(current_table->lines[i][current_table->column_indexes.at(j)]) << ' ';
            } else if(current_table->column_types[current_table->column_indexes.at(j)] == DOUBLE) {
                std::cout << std::get<double>(current_table->lines[i][current_table->column_indexes.at(j)]) << ' ';
            } else if(current_table->column_types[current_table->column_indexes.at(j)] == VARCHAR) {
                std::cout << std::get<std::string>(current_table->lines[i][current_table->column_indexes.at(j)]) << ' ';
            } else if(current_table->column_types[current_table->column_indexes.at(j)] == NUL) {
                std::cout << "NULL" << ' ';
            }
        }
        std::cout << '\n';
    }
    std::cout << "------------------------------------------------------------\n";
}

void MySQLite::UPLOAD(const std::string &file_path) {
    std::ofstream file;
    file.open(file_path, std::ios::binary);

    file << this->tables.size() << '\n';

    for(auto& current_table : this->tables) {
        file << current_table.second.lines.size() + 3 << ' ' << current_table.second.column_types.size() << ' ' << current_table.second.lines.size() << '\n';
        file << current_table.first << '\n';
        for(auto i : current_table.second.colums) {
            file << i << '\t';
        }
        file << '\n';
        for(auto i : current_table.second.column_types) {
            if(i == INT) {
                file << "INT" << '\t';
            } else if(i == BOOL) {
                file << "BOOL" << '\t';
            } else if(i == FLOAT) {
                file << "FLOAT" << '\t';
            } else if(i == DOUBLE) {
                file << "DOUBLE" << '\t';
            } else if(i == VARCHAR) {
                file << "VARCHAR" << '\t';
            } else if(i == NUL) {
                file << "NULL" << '\t';
            }
        }
        file << '\n';
        for(auto i : current_table.second.lines) {
            for(int j = 0; j < i.size(); j++) {
                if(current_table.second.column_types[j] == INT) {
                    file << std::get<int>(i[j]) << '\t';
                } else if(current_table.second.column_types[j] == BOOL) {
                    file << std::get<BOOL>(i[j]) << '\t';
                } else if(current_table.second.column_types[j] == FLOAT) {
                    file << std::get<float>(i[j]) << '\t';
                } else if(current_table.second.column_types[j] == DOUBLE) {
                    file << std::get<double>(i[j]) << '\t';
                } else if(current_table.second.column_types[j] == VARCHAR) {
                    file << std::get<std::string>(i[j]) << '\t';
                } else if(current_table.second.column_types[j] == NUL) {
                    file << "NULL" << '\t';
                }
            }
            file << '\n';
        }
    }
    file.close();
}

void MySQLite::DOWNLOAD(const std::string &file_path) {
    std::ifstream file;
    file.open(file_path, std::ios::binary);

    int count;
    int columns_count;
    int lines_count;
    int tables_count;
    std::string buff;

    file >> tables_count;

    for(int l = 0; l < tables_count; l++) {
        file >> count >> columns_count >> lines_count;
        table new_table;
        file >> new_table.table_name;

        for(int i = 0; i < columns_count; i++) {
            file >> buff;
            new_table.column_indexes.insert({buff, i});
            new_table.colums.emplace_back(buff);
        }

        for(int i = 0; i < columns_count; i++) {
            file >> buff;
            if(buff == "INT") {
                new_table.column_types.emplace_back(INT);
            } else if(buff == "BOOL") {
                new_table.column_types.emplace_back(BOOL);
            } else if(buff == "FLOAT") {
                new_table.column_types.emplace_back(FLOAT);
            } else if(buff == "DOUBLE") {
                new_table.column_types.emplace_back(DOUBLE);
            } else if(buff == "VARCHAR") {
                new_table.column_types.emplace_back(VARCHAR);
            } else if(buff == "NULL") {
                new_table.column_types.emplace_back(NUL);
            }
        }

        for(int k = 0; k < lines_count; k++) {
            new_table.lines.emplace_back();
            for (int i = 0; i < columns_count; i++) {
                file >> buff;
                if (new_table.column_types[i] == INT) {
                    new_table.lines.back().emplace_back(std::stoi(buff));
                } else if (new_table.column_types[i] == BOOL) {
                    if (buff == "1") {
                        new_table.lines.back().emplace_back(true);
                    } else {
                        new_table.lines.back().emplace_back(false);
                    }
                } else if (new_table.column_types[i] == FLOAT) {
                    new_table.lines.back().emplace_back(std::stof(buff));
                } else if (new_table.column_types[i] == DOUBLE) {
                    new_table.lines.back().emplace_back(std::stod(buff));
                } else if (new_table.column_types[i] == VARCHAR) {
                    new_table.lines.back().emplace_back(buff);
                } else if (new_table.column_types[i] == NUL) {
                    new_table.lines.back().emplace_back("NULL");
                }
            }
        }
        this->tables.insert({new_table.table_name, new_table});
    }
}

void MySQLite::SELECT_JOIN(const std::string &table_name, std::vector<std::string> colums, const std::string &condition,
                           const std::string &other_table, const std::string &join_condition, const std::string& join_type) {
    std::vector<std::pair<int, int>> selected = WHERE_JOIN(join_condition);
    std::vector<int> selected_by_where = WHERE(table_name, condition);
    if(colums[0] == "*") {
        colums.assign(this->tables.at(table_name).colums.begin(), this->tables.at(table_name).colums.end());
        for(auto i : this->tables.at(other_table).colums) {
            colums.emplace_back(i);
        }
    }

    std::cout << "-------------------------------------------------------------------------------------------------------------\n";
    if(join_type == "INNER") {
        for(auto i : selected) {
            if(std::find(selected_by_where.begin(), selected_by_where.end(), i.first) != selected_by_where.end()) {
                for(auto j : this->tables.at(table_name).lines[i.first]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                for(auto j : this->tables.at( other_table).lines[i.second]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                std::cout << '\n';
            }
        }
    } else if(join_type == "RIGHT") {
        int counter = 0;
        for(int k = 0; k < this->tables.at(other_table).lines.size(); k++) {
            if(selected[counter].second == k) {
                for(auto j : this->tables.at(table_name).lines[selected[counter].first]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                for(auto j : this->tables.at( other_table).lines[selected[counter].second]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                std::cout << '\n';
                counter++;
            } else {
                for(auto j : this->tables.at( other_table).lines[k]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                std::cout << '\n';
            }
        }
    } else if(join_type == "LEFT") {
        int counter = 0;
        for(int k = 0; k < this->tables.at(other_table).lines.size(); k++) {
            if(selected[counter].second == k) {
                for(auto j : this->tables.at(table_name).lines[selected[counter].first]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                for(auto j : this->tables.at( other_table).lines[selected[counter].second]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                std::cout << '\n';
                counter++;
            } else {
                for(auto j : this->tables.at(  table_name).lines[k]) {
                    if(std::holds_alternative<int>(j)) {
                        std::cout << std::get<int>(j) << ' ';
                    } else if (std::holds_alternative<bool>(j)) {
                        std::cout << std::get<bool>(j) << ' ';
                    } else if (std::holds_alternative<float>(j)) {
                        std::cout << std::get<float>(j) << ' ';
                    } else if (std::holds_alternative<double>(j)) {
                        std::cout << std::get<double>(j) << ' ';
                    } else if (std::holds_alternative<std::string>(j)) {
                        std::cout << std::get<std::string>(j) << ' ';
                    }
                }
                std::cout << '\n';
            }
        }
    }
    std::cout << "-------------------------------------------------------------------------------------------------------------\n";
}



std::vector<std::pair<int, int>> MySQLite::WHERE_JOIN(const std::string &condition) {
    std::vector<std::pair<int, int>> res;
    std::regex a(R"(([^;\s\(\)\.]+))");
    std::sregex_iterator iter(condition.begin(), condition.end(), a);
    std::string table_name_lhs = (*iter)[0];
    ++iter;
    std::string column_name_lhs = (*iter)[0];
    ++iter;
    ++iter;
    std::string table_name_rhs = (*iter)[0];
    ++iter;
    std::string column_name_rhs = (*iter)[0];
    for(int i = 0; i < this->tables.at(table_name_lhs).lines.size(); i++) {
        for(int j = 0; j < this->tables.at(table_name_rhs).lines.size(); j++) {
            if(join_is_accepted_line(this->tables.at(table_name_lhs).lines[i], this->tables.at(table_name_rhs).lines[j],logical_line_parse(condition))) {
                res.emplace_back(i, j);
            }
        }
    }

    return res;
}

bool MySQLite::join_condition_accept(const std::string &condition,
                                     const std::vector<std::variant<bool, int, float, double, std::string>> &line_1,
                                     const std::vector<std::variant<bool, int, float, double, std::string>> &line_2) {
    if(condition == "true") {
        return true;
    } else if(condition == "false") {
        return false;
    } else {
        std::regex a(R"(([^;\s\(\)\.]+))");
        std::sregex_iterator iter(condition.begin(), condition.end(), a);
        std::string table_name_lhs = (*iter)[0];
        ++iter;
        std::string column_name_lhs = (*iter)[0];
        ++iter;
        std::string operation = (*iter)[0];
        ++iter;
        std::string table_name_rhs = (*iter)[0];
        ++iter;
        std::string column_name_rhs = (*iter)[0];
        std::variant<bool, int, float, double, std::string> rhs_casted;
        rhs_casted = line_2[this->tables.at(table_name_rhs).column_indexes.at(column_name_rhs)];
        if (operation == ">") {
            return line_1[this->tables.at(table_name_lhs).column_indexes.at(column_name_lhs)] > rhs_casted;
        } else if (operation == ">=") {
            return line_1[this->tables.at(table_name_lhs).column_indexes.at(column_name_lhs)] >= rhs_casted;
        } else if (operation == "=") {
            return line_1[this->tables.at(table_name_lhs).column_indexes.at(column_name_lhs)] == rhs_casted;
        } else if (operation == "<=") {
            return line_1[this->tables.at(table_name_lhs).column_indexes.at(column_name_lhs)] <= rhs_casted;
        } else if (operation == "<") {
            return line_1[this->tables.at(table_name_lhs).column_indexes.at(column_name_lhs)] < rhs_casted;
        } else if (operation == "!=") {
            return line_1[this->tables.at(table_name_lhs).column_indexes.at(column_name_lhs)] != rhs_casted;
        }
    }
}

bool MySQLite::join_is_accepted_line(const std::vector<std::variant<bool, int, float, double, std::string>> &line_1,
                                     const std::vector<std::variant<bool, int, float, double, std::string>> &line_2,
                                     const std::vector<std::string> &condition) {
    std::vector<std::string> stack;
    std::string condition1;
    std::string condition2;

    for(auto i : condition) {
        if(!is_operator(i.begin(), i.end())) {
            stack.emplace_back(i);
        } else {
            if(i == "NOT") {
                condition1 = stack.back();
                stack.erase(stack.end() - 1);
                stack.emplace_back(NOT(join_condition_accept(condition1, line_1, line_2)));
            } else {
                condition1 = stack.back();
                stack.erase(stack.end() - 1);
                condition2 = stack.back();
                stack.erase(stack.end() - 1);
                if(i == "AND") {
                    stack.emplace_back(AND(join_condition_accept(condition1, line_1, line_2), join_condition_accept(condition2, line_1, line_2)));
                } else if(i == "OR") {
                    stack.emplace_back(OR(join_condition_accept(condition1, line_1, line_2), join_condition_accept(condition2, line_1, line_2)));
                }
            }

        }
    }

    if(stack[0] != "true" || stack[0] != "false") stack[0] = (join_condition_accept(stack[0], line_1, line_2) ? "true" : "false");

    return stack[0] == "true";
}
