#include "embed.h"

namespace lisp
{
    std::optional<List> flatten(const List &_args, PtrEnvir &_envir)
    {
        List list;

        for(auto &cell : _args){
            auto newCell = evaluate(cell, _envir);
            if(!newCell){
                return std::nullopt;
            }

            list.push_back(newCell.value());
        }

        return list;
    }

    Embedded wrap(Embedded _embed)
    {
        return [f = _embed](const List &_args, PtrEnvir &_envir) -> std::optional<Cell>
            {
                auto args = flatten(_args, _envir);
                
                if(!args){
                    std::cerr << "wrap: fail to eval args" << std::endl;
                    return std::nullopt;
                }

                auto ret = f(args.value(), _envir);

                if(!ret){
                    std::cerr << "wrap: args mismatch" << std::endl;
                }

                return ret;
            };
    }

    Embedded makeOverloadSub(std::initializer_list<Embedded> _fs)
    {
        return [fs = std::vector<Embedded>(_fs)](const List &_args, PtrEnvir &_envir) -> std::optional<Cell>
            {
                for(auto &f : fs){
                    auto ret = f(_args, _envir);

                    if(ret){
                        return ret;
                    }
                }

                return std::nullopt;
            };
    }
}