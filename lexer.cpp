#include "lexer.h"


auto Lexer::token_scan(std::string const &code) -> std::vector<std::pair<std::string, std::string>> const{
    int index = 0;
    std::vector<std::pair<std::string, std::string>> tokens;

    while(index < code.length()){
        std::string ch(1, code[index]);
        // skip space
        if(ch == " "){
            index++;
            continue;
        }
        // delimiters
        if(std::find(delimiters_.begin(), delimiters_.end(), ch) != delimiters_.end()){
            tokens.push_back({ch, "delit"});
            index++;
            continue;
        }

        // operators
        if(index + 1 < code.length() && std::find(operators_.begin(), operators_.end(), ch + code[index + 1]) != operators_.end()) {
            tokens.push_back({ch + code[index], "oper"});
            index += 2;
            continue;
        }
        if(std::find(operators_.begin(), operators_.end(), ch) != operators_.end()){
            tokens.push_back(std::make_pair<std::string, std::string>((std::string)ch, "oper"));
            index++;
            continue;
        }
        
        // digital
        if(isdigit(code[index])) {
            std::string n(1, code[index]);
            index++;
            while(isdigit(code[index])){
                n += code[index++];
            }
            tokens.push_back({n, "liter"});
            continue;
        }

        // string literal
        if(code[index] == '"'){
            std::string str;
            index++;
            while(code[index] != '"'){
                str += code[index++];
            }
            tokens.push_back({str, "liter"});
            index++;
            continue;
        }

        // keywords
        if(isalpha(code[index])){
            std::string key_t(1, code[index]);
            index++;
            while(isalpha(code[index])){
                key_t += code[index++];
            }
            if(std::find(keywords_.begin(), keywords_.end(), key_t) != keywords_.end()) {
                tokens.push_back({key_t, "keywd"});
            }else{
                tokens.push_back({key_t, "idef"});
            }   
            continue;
        }

        index++;
    }

    return tokens;

}