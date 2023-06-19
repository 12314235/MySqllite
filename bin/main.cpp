#include <iostream>
#include <string>
#include <regex>
#include "MySQLite.h"


int main() {

    std::string commands;
    std::string buffer;

    while(std::getline(std::cin, buffer)) {
        if(buffer == "quit") break;
        commands += buffer;
    }

    auto* sql = new MySQLite();

    sql->request(commands);

    return 0;
}