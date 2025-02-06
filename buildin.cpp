#include "lisp.h"

namespace lisp
{
    std::optional<Cell> lisp::Procedure::operator()(const List &_args, PtrEnvir &_envir)
    {
        auto args = flatten(_args, _envir);
        if(!args){
            std::cerr << "Procedure: fail to eval args" << std::endl;
            return std::nullopt;
        }

        auto newEnvir = Environment::createEnvir(envir);

        if(!newEnvir->bind(params, args.value())){
            std::cerr << "Procedure: fail to bind args" << std::endl;
            return std::nullopt;
        }

        return evaluate(body, newEnvir);
    }

    // primary
    // (if <cond> <expr1> <expr2>)
    std::optional<Cell> buildinIf(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() != 3){
            std::cerr << "if: need 3 args" << std::endl;
            return std::nullopt;
        };

        auto it = _args.begin();
        auto cond = evaluate(*it, _envir);

        if(!(cond && cond.value().isType<bool>())){
            std::cerr << "if: invalid condition" << std::endl;
            return std::nullopt;
        }

        auto &expr1 = *(++it);
        auto &expr2 = *(++it);
        return evaluate(cond.value().get<bool>() ? expr1 : expr2, _envir);
    }

    //(cond (<cond1> <expr1>) (<cond2> <expr2>) ... (<condn> <exprn>) <default>)
    // <=> (if <cond1> <expr1> (cond (<cond2> <expr2>) ... (<condn> <exprn>) <default>))
    // (if <cond> <expr1> <expr2>) <=> (cond (<cond> <expr1>) <expr2>)
    std::optional<Cell> buildinCond(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() < 2){
            std::cerr << "cond: too less args" << std::endl;
            return std::nullopt;
        }

        auto last = --_args.end();
        for(auto it = _args.begin(); it != _args.end(); it++){
            if(it == last){
                return evaluate(*last, _envir);
            }

            if(!it->isType<List>()){
                std::cerr << "cond: invalid args" << std::endl;
                return std::nullopt;
            }

            auto list = it->get<List>();
            if(list.size() != 2){
                std::cerr << "cond: invalid args" << std::endl;
                return std::nullopt;
            }

            auto cond = evaluate(list.front(), _envir);
            if(!(cond && cond.value().isType<bool>())){
                std::cerr << "cond: invalid condition" << std::endl;
                return std::nullopt;
            }

            if(cond.value().get<bool>()){
                return evaluate(list.back(), _envir);
            }
        }

        return std::nullopt;
    }

    // (lambda (<param1> ... <paramn>) <body>)
    std::optional<Cell> buildinLambda(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() != 2){
            std::cerr << "lambda: need 2 args" << std::endl;
            return std::nullopt;
        }

        auto &first = _args.front();
        if(!first.isType<List>()){
            std::cerr << "lambda: invalid param list" << std::endl;
            return std::nullopt;
        }

        std::vector<std::string> paramList;
        for(auto param : first.get<List>()){
            if(!param.isType<std::string>()){
                std::cerr << "lambda: invalid param" << std::endl;
                return std::nullopt;
            }

            paramList.push_back(param.get<std::string>());
        }

        Procedure proc(paramList, _args.back(), _envir);
        auto ptr = std::make_shared<Procedure>(std::move(proc));
        return Cell(ptr);
    }

    // (define <name> <value>)
    std::optional<Cell> buildinDefine(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() != 2){
            std::cerr << "define: need 2 args" << std::endl;
            return std::nullopt;
        };

        auto &name = _args.front();
        if(!name.isType<std::string>()){
            std::cerr << "define: invalid name" << std::endl;
            return std::nullopt;
        };

        auto value = evaluate(_args.back(), _envir);
        if(!value){
            std::cerr << "define: invalid value" << std::endl;
            return std::nullopt;
        }

        if(!_envir->extend(name.get<std::string>(), value.value())){
            std::cerr << "define: name conflict" << std::endl;
            return std::nullopt;
        }

        return name.get<std::string>();
    }

    std::optional<Cell> buildinBegin(const List &_args, PtrEnvir &_envir)
    {
        if(_args.empty()){
            std::cerr << "begin: empty args" << std::endl;
            return std::nullopt;
        }

        auto it = _args.begin();
        while(true){
            auto cell = evaluate(*it, _envir);
            it++;
            
            if((!cell) || (it == _args.end())){
                return cell;
            }
        }
    }

    //(let (<var1> <expr1>) ... (<varn> <exprn>) <body>)
    // => ((lambda (<var1> ... <varn>) <body>) <expr1> ... <exprn>)
    std::optional<Cell> buildinLet(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() < 2){
            std::cerr << "let: too less args" << std::endl;
            return std::nullopt;
        }

        std::vector<std::string> vars;
        List values;

        auto last = --_args.end();
        for(auto it = _args.begin(); it != last; it++){
            if(!it->isType<List>()){
                std::cerr << "let: invalid args" << std::endl;
                return std::nullopt;
            }

            auto list = it->get<List>();
            if(list.size() != 2){
                std::cerr << "let: invalid args" << std::endl;
                return std::nullopt;
            }

            auto var = list.front();
            if(!var.isType<std::string>()){
                std::cerr << "let: invalid var name" << std::endl;
                return std::nullopt;
            }

            auto value = evaluate(list.back(), _envir);
            if(!value){
                std::cerr << "let: invalid value" << std::endl;
                return std::nullopt;
            }

            vars.push_back(var.get<std::string>());
            values.push_back(value.value());
        }

        auto newEnvir = Environment::createEnvir(_envir);
        if(!newEnvir->bind(vars, values)){
            std::cerr << "let: fail to bind vars" << std::endl;
            return std::nullopt;
        }

        return evaluate(*last, newEnvir);
    }

    std::optional<Cell> buildinAtom(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() != 1){
            std::cerr << "atom?: need 1 arg" << std::endl;
            return std::nullopt;
        }

        return !_args.front().isType<List>();
    }

    // (set! <name> <value>)
    std::optional<Cell> buildinSet(const List &_args, PtrEnvir &_envir)
    {
        if(_args.size() != 2){
            std::cerr << "set!: need 2 args" << std::endl;
            return std::nullopt;
        };

        auto &name = _args.front();
        if(!name.isType<std::string>()){
            std::cerr << "set!: invalid name" << std::endl;
            return std::nullopt;
        };

        auto value = evaluate(_args.back(), _envir);
        if(!value){
            std::cerr << "set!: invalid value" << std::endl;
            return std::nullopt;
        }

        if(!_envir->setVar(name.get<std::string>(), value.value())){
            std::cerr << "set!: fail, maybe var not exist" << std::endl;
            return std::nullopt;
        }

        return name.get<std::string>();
    }

    // Arithmetic
    Embedded plus = makeOverloadReducer<int, float>(
        std::plus<int>(),
        std::plus<float>()
    ),
    minus = makeOverload
    <
        int (int), float (float),
        int (int, int), float (float, float)
    >(
        std::negate<int>(),
        std::negate<float>(),
        std::minus<int>(),
        std::minus<float>()
    ),
    multiplies = makeOverloadReducer<int, float>(
        std::multiplies<int>(),
        std::multiplies<float>()
    ),
    divides = makeOverload<int (int, int), float (float, float)>(
        std::divides<int>(),
        std::divides<float>()
    ),
    modulus = makeEmbed<int (int, int)>(std::modulus<int>());

    // Comparisons
    Embedded equal = makeOverload
    <
        bool (bool, bool), bool (int, int), bool (float, float),
        bool (std::string, std::string),
        bool (Quotation, Quotation)
    >(
        std::equal_to<bool>(),
        std::equal_to<int>(),
        std::equal_to<float>(),
        std::equal_to<std::string>(),
        std::equal_to<Quotation>()
    ),
    less = makeOverload<bool (int, int), bool (float, float)>(
        std::less<int>(),
        std::less<float>()
    ),
    greater = makeOverload<bool (int, int), bool (float, float)>(
        std::greater<int>(),
        std::greater<float>()
    ),
    lessEqual = makeOverload<bool (int, int), bool (float, float)>(
        std::less_equal<int>(),
        std::less_equal<float>()
    ),
    greaterEqual = makeOverload<bool (int, int), bool (float, float)>(
        std::greater_equal<int>(),
        std::greater_equal<float>()
    );

    // Logical
    Embedded logicalNot = makeEmbed<bool (bool)>(std::logical_not<bool>()),
    logicalAnd = makeEmbed<bool (bool, bool)>(std::logical_and<bool>()),
    logicalOr = makeEmbed<bool (bool, bool)>(std::logical_or<bool>());

    //List Operate
    Cell buildinCons = parseString(
        "(lambda (x y) (lambda (s) (if (= s 1) x y)))"
    ).value(),
    buildinCar = parseString("(lambda (x) (x 1))").value(),
    buildinCdr = parseString("(lambda (x) (x 2))").value();
}