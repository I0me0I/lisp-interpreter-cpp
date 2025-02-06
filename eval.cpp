#include "lispbase.h"

namespace lisp
{
    std::optional<Cell> evaluate(const Cell &_expr, PtrEnvir &_envir)
    {
        if(_expr.isType<std::string>()){
            auto name = _expr.get<std::string>();
            auto var = _envir->lookupVars(name);
            if(var){
                return evaluate(var.value(), _envir);
            }

            auto embed = _envir->lookupEmbeds(name);
            if(embed){
                return name;
            }

            std::cerr << "eval: undefined indentifier '" << name << '\''<< std::endl;
            return std::nullopt;
        }

        if(_expr.isType<List>()){
            auto list = _expr.get<List>();

            if(list.empty()){
                std::cerr << "eval: empty list" << std::endl;
                return std::nullopt;
            }

            auto operat = list.front();
            list.pop_front();
            return apply(operat, list, _envir);
        }

        return _expr;
    }

    std::optional<Cell> apply(const Cell &_operat, const List &_operands, PtrEnvir &_envir)
    {
        auto cell = evaluate(_operat, _envir);
        if(!cell){
            return std::nullopt;
        }

        auto operat = cell.value();

        if(operat.isType<PtrProc>()){
            auto ptr = operat.get<PtrProc>();

            if(!ptr){
                std::cerr << "apply: null procedure" << std::endl;
                return std::nullopt;
            }

            auto proc = ptr.get();
            return proc->operator()(_operands, _envir);
        }

        if(!operat.isType<std::string>()){
            std::cerr << "apply: invalid operator" << std::endl;
            return std::nullopt;
        }

        auto operName = operat.get<std::string>();
        auto embed = _envir->lookupEmbeds(operName);
        if(embed){
            return embed.value()(_operands, _envir);
        }

        // auto var = _envir.lookupVar(operName);
        // if(var){
        //     return evaluate(var.value(), _envir);
        // }

        std::cerr << "apply: cannot apply operator" << std::endl;
        return std::nullopt;
    }
}