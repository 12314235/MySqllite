#include "gtest/gtest.h"
#include "../bin/MySQLite.h"
#include <iostream>
#include <fstream>


TEST(MySQLite, Test) {
    MySQLite bd = MySQLite();
    bd.DOWNLOAD("D:\\labwork12\\labwork-12-12314235\\tests\\bd.txt");

    std::ofstream outputFile("D:\\labwork12\\labwork-12-12314235\\tests\\output.txt");
    std::streambuf* coutBuf = std::cout.rdbuf();
    std::cout.rdbuf(outputFile.rdbuf());

    std::string inp = " CREATE TABLE Persons (    PersonID int,    LastName varchar(255),    FirstName varchar(255),    Address varchar(255), City varchar(255),    FOREIGN KEY (PersonID) REFERENCES Students (PersonID));CREATE TABLE Students (    PersonID int, LastName varchar(255),    FirstName varchar(255),    Smth bool);INSERT INTO Persons (PersonID, LastName, FirstName, Address, City) VALUES (367158, 'Name', 'Surname', 'Vyazm', 'SaintP' ), (36718, 'Name', 'Surname', 'Vyazma', 'Sain' ), (8, 'Name', 'Surname', 'Vyazm', 'SaintP');INSERT INTO Students (PersonID, LastName, FirstName, Smth) VALUES (367158, 'Name', 'Name', true);SELECT * FROM Persons WHERE City = 'SaintP' AND LastName = 'Name';UPLOAD(D:\\\\labwork12\\\\labwork-12-12314235\\\\bd.txt);";
    ASSERT_TRUE(bd.request(inp));
}
