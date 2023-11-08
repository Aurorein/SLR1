#pragma once

#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <map>
#include <functional>
#include <cstdlib>
#include <memory>
#include <iomanip>
#include "printer.h"

// 函数对象用于计算pair作为key的hash值
struct pair_hash{
    std::size_t operator() (const std::pair<std::string, std::string> &p) const {
        return std::hash<std::string>{}(p.first) ^ std::hash<std::string>{}(p.second);
    }
};

using first_set_t = std::unordered_map<std::string, std::set<std::string>>;   // FIRST集
using follow_set_t = std::unordered_map<std::string, std::set<std::string>>;    // FOLLOW集
using action_table_t = std::unordered_map<std::pair<std::string, std::string>, std::string, pair_hash>;   // action表
using goto_table_t = std::unordered_map<std::pair<std::string, std::string>, std::string, pair_hash>;    // go表


// 产生式
struct Production{
    // 产生式左部
    std::string L_;
    // 产生式右部
    std::vector<std::string> R_;

    bool operator < (Production const &other) const {
        if(L_ != other.L_) {
            return L_ < other.L_;
        }
        return R_ < other.R_;
    }
};

// 文法
struct Grammer {
    std::string S_;     // 开始符号
    std::vector<std::string> V_;    // 变量
    std::vector<std::string> T_;    // 终结符
    std::vector<Production> P_;     // 产生式

};

// 项目
struct Item {
    int p_idx_; // 产生式序号
    int dot_idx_; // 圆点下标位置

    bool operator< (Item const &other) const{
        return p_idx_ == other.p_idx_ ? dot_idx_ < other.dot_idx_ : p_idx_ < other.p_idx_; 
    }
};

// I(闭包)
struct I_Closure{
    std::set<Item> items_; // 项目集
    bool operator < (I_Closure const& other) const {
        return items_ < other.items_;
    }  
    // 计算闭包
    I_Closure closure(Grammer const &G) const{
        I_Closure J = {items_};
        bool changed = true;
        while(changed){
            changed = false;
            auto const size = J.items_.size();
            for(auto const I : J.items_){
                const int dot_idx = I.dot_idx_;
                const int p_idx = I.p_idx_;
                if(dot_idx == G.P_[p_idx].R_.size()){continue;}
                auto const B = G.P_[p_idx].R_[dot_idx];
                if(std::find(G.V_.begin(), G.V_.end(), B) != G.V_.end()){
                    for(int i = 0; i < G.P_.size(); ++i){
                        const auto& p = G.P_[i];
                        if(p.L_ == B){
                            if(J.items_.find({i, 0}) == J.items_.end()){J.items_.insert({i, 0});}
                        }
                    }
                }
            }

            changed = J.items_.size() != size || changed;
        }

        return J;

    }

    I_Closure go(Grammer const &G, std::string const &X) const{
        I_Closure go = {}; 
        for(auto const& I : items_) {
            const int dot_idx = I.dot_idx_;
            const int p_idx = I.p_idx_;
            if(dot_idx == G.P_[p_idx].R_.size()){continue;}
            auto const B = G.P_[p_idx].R_[dot_idx];
            // I中每个形如A→α.Xβ的项目 J:=J∪{A→αX.β}
            if(B == X) {
               go.items_.insert({p_idx, dot_idx + 1});
            }
        }

        return go.closure(G);
    }


};

// C 项目集规范族
struct Collection{
    std::set<I_Closure> I_;
};


class Parser {
private:
    Grammer G_;    // 文法
    std::vector<std::pair<std::string, std::string>> tokens_;    // lexer执行后的词法
    first_set_t first_set_;    // FIRST集
    follow_set_t follow_set_;     // FOLLOW集
    action_table_t action_table_;    // action table
    goto_table_t goto_table_;     // go table
    Collection C_;
    std::unique_ptr<Printer> printer_;

    void get_first();
    void get_follow();
    void get_SLR1_table();

public:
    Parser() = delete;
    template <typename T>
    explicit Parser(T &&tokens, std::unique_ptr<Printer> printer) : tokens_(std::forward<T>(tokens)), printer_(std::move(printer)) {
        G_.S_ = "S";
        G_.V_ = {"S", "FuncD", "ArgL", "BS",
               "Type", "AExpr", "BExpr",
               "AOper", "COper", "FunId", "AArgL"};
        G_.T_ = {"idef", "liter",
               "(", ")", "{", "}", ",", ";", "=",
               "while", "if", "else", "return",
               "int", "string",
               "==", "!=", "-", "+", "*", "/"};
        G_.P_ = {{"S", {"FuncD"}},

                {"FuncD", {"FuncD", "FuncD"}},
                {"FuncD", {"Type", "idef", "(", ")", "{", "BS", "}"}},
                {"FuncD", {"Type", "idef", "(", "ArgL", ")", "{", "BS", "}"}},

                {"ArgL", {"Type", "idef"}},
                {"ArgL", {"Type", "idef", ",", "ArgL"}},

                {"FunId", {"idef", "(", ")"}},
                {"FunId", {"idef", "(", "AArgL", ")"}},

                {"AArgL", {"AExpr"}},
                {"AArgL", {"AExpr", ",", "AArgL"}},

                {"BS", {"BS", "BS"}},

                {"BS", {"Type", "idef", ";"}},
                {"BS", {"Type", "idef", "=", "AExpr", ";"}},
                {"BS", {"idef", "=", "AExpr", ";"}},

                {"BS", {"Type", "idef", "=", "FunId", ";"}},
                {"BS", {"idef", "=", "FunId", ";"}},

                {"BS", {"while", "(", "BExpr", ")", "{", "BS", "}"}},
                {"BS", {"if", "(", "BExpr", ")", "{", "BS", "}"}},
                {"BS", {"if", "(", "BExpr", ")", "{", "BS", "}", "else", "{", "BS", "}"}},
                {"BS", {"return", ";"}},
                {"BS", {"return", "AExpr", ";"}},

                {"Type", {"int"}},
                {"Type", {"string"}},

                {"AExpr", {"AExpr", "AOper", "AExpr"}},
                {"AExpr", {"-", "AExpr"}},
                {"AExpr", {"(", "AExpr", ")"}},
                {"AExpr", {"idef"}},
                {"AExpr", {"liter"}},

                {"BExpr", {"AExpr", "COper", "AExpr"}},
                {"BExpr", {"(", "BExpr", ")"}},

                {"COper", {"=="}},
                {"COper", {"!="}},

                {"AOper", {"+"}},
                {"AOper", {"-"}},
                {"AOper", {"*"}},
                {"AOper", {"/"}} };

        get_first();
        get_follow();
        get_SLR1_table();
    }
    ~Parser() = default;
    Parser(const Parser& other) = default;
    Parser& operator = (const Parser &other) = default;
    Parser(Parser&& other) noexcept = default;
    Parser& operator = (Parser &&other) noexcept = default;

    void analyze_syntax();

};