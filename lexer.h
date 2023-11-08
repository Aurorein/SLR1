#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

class Lexer {
private:
    // 关键词
    const std::vector<std::string> keywords_ = {"int", "string" ,"if", "else", "for", "while", "return"};
    // 运算符
    const std::vector<std::string> operators_ = {"+", "-", "*", "/", "=", "==", "!="};
    // 定界符
    const std::vector<std::string> delimiters_ = {"(", ")", "{", "}", ",", ";"};
public:
    Lexer() = default;
    auto token_scan(std::string const &code) -> std::vector<std::pair<std::string, std::string>> const; 
    ~Lexer() = default;

};