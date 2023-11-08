#include "parser.h"


void Parser::analyze_syntax() {
    // state状态栈，symbol符号栈
    std::vector<std::string> state, symbol;
    state.push_back("0");
    symbol.push_back("#");
    tokens_.push_back({"#", "#"});

    int token_idx = 0;
    int step = 1;
    printer_->print_ln("tokens:");
    for(auto const &token : tokens_) {
        printer_ -> print("[" + token.first +" " + token.second +"] ");
    }

    printer_->print_ln("analyze syntax:");
    auto get_token = [this](int const idx) {
        if(idx >= tokens_.size()){
            return tokens_[tokens_.size() - 1].first;
        }
        const auto &pr = tokens_[idx];
        if(pr.second == "idef" || pr.second == "liter"){
            return pr.second;
        }else{
            return pr.first;
        }
    };

    std::string tk = get_token(token_idx);
    while(true) {
        std::string const s = state.back();
        
        printer_->print("Step" + std::to_string(step++) +":");
        printer_->print(" state stack: ");
        for(auto const& st : state) {
            printer_->print(st + " ");
        }
        printer_->print(" symbol stack: ");
        for(auto const& sy : symbol) {
            printer_->print(sy + " ");
        }
        
        if(auto action = action_table_[{s, tk}]; action[0] == 's'){
            // 移进操作,将状态（去掉开头s）压入状态栈
            state.push_back(action.erase(0, 1));
            symbol.push_back(tk);
            printer_ -> print_ln("Shift " + tk);
            tk = get_token(++token_idx);
        }else if(action[0] == 'r'){
            // 对第i个产生式进行规约
            const int p_idx = std::stoi(action.erase(0, 1));
            Production p = G_.P_[p_idx];
            printer_->print_ln("Reduce " + std::to_string(p_idx));
            for(int i = 0; i < p.R_.size(); ++i) {
                if(!state.empty()){
                    state.pop_back();
                    symbol.pop_back();
                }
            }
            // 令S'是现在的栈顶状态: 把A和goto[S',A]先后压入栈中
            std::string const t = state.back();
            state.push_back(goto_table_[{t, p.L_}]);
            symbol.push_back(p.L_);
        }else if(action == "acc") {
            std::cerr<<"acc!"<<std::endl;
            printer_->print_ln("ACC!");
            break;
        }else{
            break;
        }
    }

}

void Parser::get_first() {
    // if (X∈T) then FIRST(X):= {X} 
    for (auto const& t : G_.T_) {
        first_set_[t] = { t };
    }
    // if X∈V then
    for (auto const& v: G_.V_) {
        first_set_[v] = {};
    }
    for(auto const& p : G_.P_) {
        // if (X→ε∈P) then FIRST(X):= FIRST(X)∪{ε}
        if(std::find(p.R_.begin(), p.R_.end(), constants::RPSLION) != p.R_.end()){
            first_set_[p.L_].insert(constants::RPSLION);
        }
        // FIRST(X):= FIRST(X)∪{a|X→a…∈P}
        if(std::find(G_.T_.begin(), G_.T_.end(), p.R_[0]) != G_.T_.end()) {
            first_set_[p.L_].insert(p.R_[0]);
        }
    }

    bool changed;
    do{
        changed = false;
        for(auto const& X : G_.V_) {
            const int size = first_set_[X].size();
            // 找出X为左部的产生式
            auto find_X_products = [this](std::string const &X){
                std::vector<Production> x_products;

                for(auto &p : G_.P_){
                    if(p.L_ == X){x_products.push_back(p);}
                }
                return x_products;
            };
            auto x_products = find_X_products(X);
            // f (X→Y…∈P and Y∈V) then FIRST(X):= FIRST(X)∪(FIRST(Y)-{ε})；
            for(auto const &p : x_products) {
                const int size = first_set_[X].size();

                std::string const &s = p.R_[0];
                if(std::find(G_.V_.begin(), G_.V_.end(), s) != G_.V_.end()){
                    for(auto const &t : first_set_[s]){
                        if(t != constants::RPSLION){
                            first_set_[X].insert(t);
                            
                        }
                    }
                }

                if(size != first_set_[X].size()){changed = true;}
            }
            // if (X→Y1…Yn∈P and Y1...Yi-1 => ε) then for k=2 to i do FIRST(X):= FIRST(X)∪(FIRST(Yk)-{ε});
            auto is_V = [this](std::string const &R) {
                return std::find(G_.V_.begin(), G_.V_.end(), R) != G_.V_.end();
            };
            for(auto const &p : x_products) {
                if(is_V(p.R_[0])){
                    int idx;
                    for(idx = 1; idx <p.R_.size(); ++idx) {
                        std::string const &s = p.R_[idx];
                        if(is_V(s) && first_set_[s].count(constants::RPSLION) > 0){
                            for(auto const &t : first_set_[s]){
                                if(t != constants::RPSLION){
                                    first_set_[X].insert(t);
                                    changed = true;
                                }
                            }
                        }
                    }
                    // if Y1...Yn ε then FIRST(X):=FIRST(X)∪{ε}
                    if(idx == p.R_.size() - 1) {
                        first_set_[X].insert(constants::RPSLION);
                        changed = true;
                    }
                }    

            }
        }

    }while(changed);

    printer_->print_ln("First Set:");
    for(auto const& pr : first_set_) {
        printer_->print("FIRST[" + pr.first + "]: {" );
        for(auto const& s : pr.second) {
            printer_->print("\"" +s + "\" ");
        }
        printer_->print_ln("}");
    }

}

void Parser::get_follow() {

    // initalization FOLLOW(V)
    for(auto const &v : G_.V_) {
        follow_set_[v] = {};
    }
    // FOLLOW(S) := {#}
    follow_set_[G_.S_] = {"#"};
    bool changed;
    do{
        changed = false;
        for(auto const &p : G_.P_) {
            for(int i = 0; i <= p.R_.size() - 1; ++i) {
                if(std::find(G_.V_.begin(), G_.V_.end(), p.R_[i]) != G_.V_.end()) {
                    const int size = first_set_[p.R_[i]].size();
                    if(i < p.R_.size() - 1) {
                        const int size = first_set_[p.R_[i]].size();
                        [&]() {
                            // 若A→αBβ∈P，则 FOLLOW(B):=FOLLOW(B)∪(FIRST(β)–{ε})
                            for(int j = i + 1; j <= p.R_.size() -1; ++j) {
                                if(first_set_[p.R_[j]].find(constants::RPSLION) == first_set_[p.R_[j]].end()){
                                    for(auto const& f : first_set_[p.R_[j]]) {
                                        follow_set_[p.R_[i]].insert(f);
                                    }
                                    return;
                                }
                            }
                            // 若A→αB或A→αBβ∈P且β ε，A≠B，则FOLLOW(B):=FOLLOW(B)∪FOLLOW(A)
                            for(const auto&f : follow_set_[p.L_]) {
                                follow_set_[p.R_[i]].insert(f);
                            }
                        }();
                        
                    }
                    else{
                        // A→αB 则FOLLOW(B):=FOLLOW(B)∪FOLLOW(A)
                        for(auto const& f :follow_set_[p.L_]) {
                            follow_set_[p.R_[i]].insert(f);
                        }
                    }
                    changed = first_set_[p.R_[i]].size() != size || changed;
                }
            }
        }
    }while(changed);

    printer_->print_ln("Follow Set:");
    for(auto const& pr : follow_set_) {
        printer_->print("FOLLOW[" + pr.first + "]: { ");
        for(auto const &s : pr.second) {
            printer_->print("\"" +s + "\" ");
        }
        printer_->print_ln("}");
    }

}

void Parser::get_SLR1_table() {
    // 扩展文法加入S' -> S
    G_.S_ = "S'";
    G_.V_.push_back(G_.S_);
    G_.P_.push_back({G_.S_, {"S"}});
    // I0= CLOSURE({S' →.S})
    I_Closure I0 = {{{static_cast<int>(G_.P_.size()-1), 0}}};

    Collection C = {{I0.closure(G_)}};
    bool changed = true;
    while(changed){
        changed = false;
        const int size = C.I_.size();
        std::set<I_Closure> t = {};
        for(auto const& I : C.I_) {
            for(auto const& V : G_.V_) {
                t.insert(I.go(G_, V));
            }   
            for(auto const& T : G_.T_) {
                t.insert(I.go(G_, T));
            }    
        }

        for(auto const& I : t) {
            if(I.items_.size() != 0){
                C.I_.insert({I.items_});
            }
        }

        changed = C.I_.size() != size || changed;
    };

    // I0,I1,I2...In
    std::map<I_Closure, int> map_I_idx;
    int idx = 0;
    for(auto const& I: C.I_){
        map_I_idx[I] = idx++;
    }

    printer_->print_ln("C = {I1, I2...In}:");
    for(auto const& I : C.I_) {
        printer_->print("I");  
        printer_->print(std::to_string(map_I_idx[I]) + ":  ");
        for(auto const &t : I.items_) {
            printer_->print("p:" + std::to_string(t.p_idx_) + "," +"dot:" + std::to_string(t.dot_idx_) +"|");
        } 
        printer_->print_ln("");
        for(auto const& item : I.items_) {
            auto const p = G_.P_[item.p_idx_];
            printer_->print(p.L_);
            printer_->print("->");
            for(int i = 0 ; i < p.R_.size(); ++i) {
                if(i == item.dot_idx_) {
                    printer_->print(". ");
                }
                printer_->print(p.R_[i] + " ");
            }
            if(item.dot_idx_ == p.R_.size()) {
                printer_->print(". ");
            }
            printer_->print_ln("");
        }
    }

    // create action table
    for(auto const& I : C.I_) {
        const int idx = map_I_idx[I];
        for(auto const& item : I.items_) {
            int p_idx = item.p_idx_;
            int dot_idx = item.dot_idx_;
            if(dot_idx < G_.P_[p_idx].R_.size()){
                auto const a = G_.P_[p_idx].R_[dot_idx];
                // if A→α.aβ∈Ik & a∈T & GO(Ik, a)=Ij then action[k,a]:=Sj
                if(std::find(G_.T_.begin(), G_.T_.end(), a) != G_.T_.end()) {
                    action_table_[{std::to_string(idx), a}] = "s" + std::to_string(map_I_idx[I.go(G_, a)]);
                }
            }
            else{
                // if S'→S.∈Ik then action[k,#]:=acc
                auto const p = G_.P_[p_idx];
                if(p.L_ == "S'" && p.R_[0] == "S" && dot_idx == 1){
                    action_table_[{std::to_string(idx), "#"}] = "acc";
                }
                else{
                // if A→α.∈Ik & A→α为G的第j个产生式，for ∀a∈T∪{#}(改为follow(A)) do action[k,a]:=rj;
                    auto const follow_A = follow_set_[p.L_];
                    for(auto const &a : follow_A) {
                        action_table_[{std::to_string(idx), a}] = "r" + std::to_string(p_idx);
                    }
                }
            }
        }
    }

    // print action table
    printer_->print_ln("ACTION TABLE:");
    printer_->print("  ");
    for(auto const& t : G_.T_) {
        std::string out_str;
        std::ostringstream oss;
        oss << std::left <<std::setw(constants::FIXED_LEN) << t;
        printer_->print(oss.str());
        oss.clear();
    }
    for(int i = 0; i < idx; ++i) {
        printer_->print("I" + std::to_string(i));
        printer_->print("\t");
        for(auto const& t : G_.T_) {
            std::string out_str;
            std::ostringstream oss;
            oss << std::left <<std::setw(constants::FIXED_LEN) << action_table_[{std::to_string(i), t}];
            printer_->print( oss.str());
            oss.clear();
        }
        printer_->print_ln("");
    }

    // create goto table
    for(auto const &I : C.I_) {
        const int idx = map_I_idx[I];
        for(auto const &v : G_.V_) {
            // if A→α.Bβ∈Ik & B∈V & GO(Ik, B)=Ij then goto[k,B]:=j;
            if(auto const go = I.go(G_, v); map_I_idx.find(go) != map_I_idx.end()) {
                int const j = map_I_idx[go];
                goto_table_[{std::to_string(idx), v}] = std::to_string(j);
            }
        }
    }

    // print goto table
    printer_->print_ln("GOTO TABLE:");
    printer_->print("            ");
    for(auto const& v : G_.V_) {
        std::string out_str2;
        std::ostringstream oss;
        oss << std::left << std::setw(constants::FIXED_LEN_V) << v;
        printer_->print(oss.str());
        oss.clear();
    }
    for(int i = 0; i < idx; ++i) {
        printer_->print("I" + std::to_string(i));
        for(auto const& v : G_.V_) {
            std::string out_str2;
            std::ostringstream oss;
            oss << std::left <<std::setw(constants::FIXED_LEN_V) << goto_table_[{std::to_string(i), v}];
            printer_->print( oss.str());
            oss.clear();
        }
        printer_->print_ln("");
    }

}