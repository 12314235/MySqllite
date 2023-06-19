

#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <iostream>
#include <fstream>
#include <variant>


class MySQLite {
private:

    enum TYPES {BOOL, INT, FLOAT, DOUBLE, VARCHAR, NUL};

    std::unordered_map<std::string, int> priority = {
            {"NOT", 2},
            {"AND", 1},
            {"OR", 0},
            {"(", -1},
            {")", -1}
    };

    struct table {
        std::string table_name;
        std::vector<TYPES> column_types;

        std::vector<std::pair<std::string, std::pair<std::string, std::string>>> foreign_keys;

        std::vector<std::string> colums;

        std::unordered_map<std::string, int> column_indexes;

        std::vector<std::vector<std::variant<bool, int, float, double, std::string>>> lines;

        table() = default;
        table(const table& other) = default;
        ~table()= default;
    };

    std::unordered_map<std::string, table> tables;

    bool parse_command(const std::string& command);

    void CREATE_TABLE(std::string params, const std::string& table_name);
    void DROP_TABLE(const std::string& table_name);
    void INSERT(const std::string& table_name, const std::string& column_names, const std::vector<std::string>& values);

    bool is_operator(std::string::const_iterator current, std::string::const_iterator end);
    std::vector<std::string> logical_line_parse(const std::string& line);
    bool is_accepted_line(const std::vector<std::variant<bool, int, float, double, std::string>>& line, const std::vector<std::string>& condition, const std::string& table_name);
    std::variant<bool, int, float, double, std::string> type_cast(const std::string& value, TYPES column_type);
    bool condition_accept(const std::string& condition, const std::string& table_name, const std::vector<std::variant<bool, int, float, double, std::string>> &line);
    std::string AND(bool c1, bool c2);
    std::string OR(bool c1, bool c2);
    std::string NOT(bool c);

    std::vector<int> WHERE(const std::string& table_name, const std::string& condition);

    void DELETE(const std::string& table_name, const std::string& condition);
    void UPDATE(const std::string& table_name, const std::string& condition, const std::string& updating);
    void SELECT(const std::string& table_name, std::vector<std::string> colums, const std::string& condition);


    bool join_is_accepted_line(const std::vector<std::variant<bool, int, float, double, std::string>> &line_1,
                               const std::vector<std::variant<bool, int, float, double, std::string>> &line_2,
                               const std::vector<std::string> &condition);
    bool join_condition_accept(const std::string &condition,
                               const std::vector<std::variant<bool, int, float, double, std::string>> &line_1,
                               const std::vector<std::variant<bool, int, float, double, std::string>> &line_2);
    std::vector<std::pair<int, int>> WHERE_JOIN(const std::string& condition);
    void SELECT_JOIN(const std::string &table_name, std::vector<std::string> colums, const std::string &condition, const std::string &other_table, const std::string &join_condition, const std::string& join_type);

    static std::variant<bool, int, float, double, std::string> value_parse(const std::string& value, const TYPES& expected_type);

    TYPES what_type(const std::string& type);

public:
    void UPLOAD(const std::string& file_path);
    void DOWNLOAD(const std::string& file_path);

    bool request(const std::string& commands);

    MySQLite() = default;
    MySQLite(const MySQLite& other) = default;
    ~MySQLite() = default;
};

