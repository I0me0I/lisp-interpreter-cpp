#include "lisp.h"
#include "lispbase.h"

namespace lisp
{
    std::optional<Embedded> Environment::lookupEmbedsLocal(const std::string &_name) const
    {
        auto it = embeds.find(_name);

        if(it != embeds.end()){
            return it->second;
        }

        return std::nullopt;
    }

    std::optional<Cell> Environment::lookupVarsLocal(const std::string &_name) const
    {
        auto it = vars.find(_name);

        if(it != vars.end()){
            return it->second;
        }

        return std::nullopt;
    }

    std::optional<Embedded> Environment::lookupEmbeds(const std::string &_name) const
    {
        for(auto env = this; env != nullptr; env = env->parent.get()){
            auto it = env->embeds.find(_name);

            if(it != env->embeds.end()){
                return it->second;
            }
        }

        return globalEnvir.lookupEmbedsLocal(_name);
    }

    std::optional<Cell> Environment::lookupVars(const std::string &_name) const
    {
        for(auto env = this; env != nullptr; env = env->parent.get()){
            auto it = env->vars.find(_name);

            if(it != env->vars.end()){
                return it->second;
            }
        }

        return globalEnvir.lookupVarsLocal(_name);
    }

    bool Environment::extend(const std::string &_name, Embedded _embed)
    {
        if(lookupEmbedsLocal(_name) || lookupVarsLocal(_name)){
            return false;
        }

        embeds.insert({_name, _embed});
        return true;
    }

    bool Environment::extend(const std::string &_name, const Cell &_cell)
    {
        if(lookupEmbedsLocal(_name) || lookupVarsLocal(_name)){
            return false;
        }

        vars.insert({_name, _cell});
        return true;
    }

    bool Environment::bind(const std::vector<std::string> &_params, const List &_args)
    {
        if(_params.size() != _args.size()){
            return false;
        }

        auto pit = _params.begin();
        auto ait = _args.begin();
        for(; pit != _params.end(); pit++, ait++){
            if(!extend(*pit, *ait)){
                return false;
            }
        }

        return true;
    }

    bool Environment::setVar(const std::string &_name, const Cell &_cell)
    {
        for(auto env = this; env != nullptr; env = env->parent.get()){
            auto it = env->vars.find(_name);

            if(it != env->vars.end()){
                it->second = _cell;
                return true;
            }
        }

        return false;
    }

    Environment Environment::globalEnvir;

    void Environment::initGlobalEnvir()
    {
        auto &env = globalEnvir;

        env.embeds.insert(
            {
                {"if", buildinIf},
                {"cond", buildinCond},
                {"lambda", buildinLambda},
                {"define", buildinDefine},
                {"begin", buildinBegin},
                {"let", buildinLet},
                {"atom?", buildinAtom},
                {"set!", buildinSet},

                {"+", plus},
                {"-", minus},
                {"*", multiplies},
                {"/", divides},
                {"mod", modulus},
                {"=", equal},
                {"<", less},
                {">", greater},
                {"<=", lessEqual},
                {">=", greaterEqual},
                {"not", logicalNot},
                {"and", logicalAnd},
                {"or", logicalOr},
            }
        );

        env.vars.insert(
            {
                {"true", true},
                {"false", false},
                {"cons", buildinCons},
                {"car", buildinCar},
                {"cdr", buildinCdr}
            }
        );
    }
}