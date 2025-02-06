#pragma once
#include "lispbase.h"
#include <vector>
#include <numeric>

namespace lisp
{
    std::optional<List> flatten(const List &_args, PtrEnvir &_envir);
    Embedded wrap(Embedded _embed);
    Embedded makeOverloadSub(std::initializer_list<Embedded> _fs);
    
    template<typename R, typename T1, typename... T>
    Embedded makeEmbedSub(std::function<R (T1, T...)> _f)
    {
        if constexpr(sizeof...(T) == 0){
            return [f = _f](const List &_args, PtrEnvir &_envir) -> std::optional<Cell>
                {
                    if(_args.size() == 1){
                        auto &cell = _args.front();
                        if(cell.isType<T1>()){
                            return Cell(f(cell.get<T1>()));
                        }
                    }

                    return std::nullopt;
                };
        }
        else{
            return [f = _f](const List &_args, PtrEnvir &_envir) -> std::optional<Cell>
                {
                    if(!_args.empty()){
                        auto &cell = _args.front();
                        
                        if(cell.isType<T1>()){
                            auto arg1 = cell.get<T1>();
                            auto embed = makeEmbedSub(
                                (std::function<R (T...)>)
                                [f, arg1](T... _args) -> R
                                {
                                    return f(arg1, _args...);
                                }
                            );
                            
                            List remain = _args;
                            remain.pop_front();
                            return embed(remain, _envir);
                        }

                    }

                    return std::nullopt;
                };
        }
    }

    template<typename T>
    Embedded makeReducerSub(std::function<T (T, T)> _f)
    {
        return [f = _f](const List &_args, PtrEnvir &_envir) -> std::optional<Cell>
            {
                if(_args.size() < 2){
                    return std::nullopt;
                }

                std::vector<T> args;

                for(auto &cell : _args){
                    if(!cell.isType<T>()){
                        return std::nullopt;
                    }

                    args.push_back(cell.get<T>());
                }

                auto init = args.back();
                args.pop_back();
                // 'reduce' is not a member of 'std'gcc
                return std::accumulate(args.begin(), args.end(), init, f);
            };
    }

    template<typename T>
    Embedded makeEmbed(std::function<T> _f)
    {
        return wrap(makeEmbedSub(_f));
    }

    template<typename T>
    Embedded makeReducer(std::function<T (T, T)> _f)
    {
        return wrap(makeReducerSub(_f));
    }

    template<typename... T>
    Embedded makeOverload(std::function<T>... _fs)
    {
        return wrap(makeOverloadSub({makeEmbedSub(_fs)...}));
    }

    template<typename... T>
    Embedded makeOverloadReducer(std::function<T (T, T)>... _fs)
    {
        return wrap(makeOverloadSub({makeReducerSub(_fs)...}));
    }
}