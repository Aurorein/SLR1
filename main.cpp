#include <memory>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "printer.h"


int main(int argc, char** argv) {
    if(argc < 2){
        std::cerr<<"please input the code path!"<<std::endl;
        exit(0);
    }

    std::string file_path = argv[1];
    std::string code;
    std::ifstream file(file_path);
    if(!file){
        std::cerr<<"open code file error!"<<std::endl;
        exit(0);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    code = buffer.str();

    std::unique_ptr<Printer> printer = std::make_unique<Printer>(constants::FILE_NAME);

    // 产生式信息对照：
    printer->print_ln("abbrev mapping:");
    printer->print_ln("FuncD -> 函数定义");
    printer->print_ln("ArgL -> 参数列表");
    printer->print_ln("AArgL -> 实参列表");
    printer->print_ln("FunId -> 函数");
    printer->print_ln("AExpr -> 算术表达式");
    printer->print_ln("BExpr -> 布尔表达式");
    printer->print_ln("AOper -> 算术运算符");
    printer->print_ln("COper -> 比较运算符");
    printer->print_ln("BS -> 语句块");
    printer->print_ln("idef -> 标识符");
    printer->print_ln("liter -> 字面量");
     

    auto lexer = std::make_unique<Lexer>();
    auto const tokens = lexer->token_scan(code);
    
    auto parser = std::make_unique<Parser>(std::move(tokens), std::move(printer));
    parser->analyze_syntax();
    return 0;

}