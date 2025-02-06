#pragma once
#include <iostream>
#include <variant>
#include <list>
#include <optional>
#include <functional>
#include <unordered_map>
#include <sstream>
#include <memory>

namespace lisp
{
    class Quotation{
        private:
            std::string value;
        
        public:
            Quotation(const char *_c) : value(_c) {}
            Quotation(const std::string &_s) : value(_s) {}
            bool operator==(const Quotation &_q) const {return value == _q.value;}
            bool operator!=(const Quotation &_q) const {return value != _q.value;}
            std::string str() const {return value;}
    };
    class Procedure;
    using PtrProc = std::shared_ptr<Procedure>;
    class Cell;
    using List = std::list<Cell>;
    class Cell{
        private:
            std::variant<bool, int, float, std::string, Quotation, List, PtrProc> value;
        
        public:
            Cell(bool _b) : value(_b) {}
            Cell(int _i) : value(_i) {}
            Cell(float _f) : value(_f) {}
            Cell(const std::string &_s) : value(_s) {}
            Cell(const Quotation &_q) : value(_q) {}
            Cell(const List &_l) : value(_l) {}
            Cell(const PtrProc &_p) : value(_p) {}
            template<typename T>
            bool isType() const {return std::holds_alternative<T>(value);}
            template<typename T>
            T get() const {return std::get<T>(value);}
            template<typename T>
            void visit(T _f) const {std::visit(_f, value);}
    };

    class Environment;
    using PtrEnvir = std::shared_ptr<Environment>;
    using Embedded = std::function<std::optional<Cell> (const List &_args, PtrEnvir &_envir)>;
    class Environment{
        private:
            std::unordered_map<std::string, Embedded> embeds;
            std::unordered_map<std::string, Cell> vars;
            const PtrEnvir parent;
            
            Environment(const PtrEnvir &_parent = nullptr) : parent(_parent) {}
            std::optional<Embedded> lookupEmbedsLocal(const std::string &_name) const;
            std::optional<Cell> lookupVarsLocal(const std::string &_name) const;
        
        public:
            std::optional<Embedded> lookupEmbeds(const std::string &_name) const;
            std::optional<Cell> lookupVars(const std::string &_name) const;
            bool extend(const std::string &_name, Embedded _embed);
            bool extend(const std::string &_name, const Cell &_cell);
            bool bind(const std::vector<std::string> &_params, const List &_args);
            bool setVar(const std::string &_name, const Cell &_cell);

            static PtrEnvir createEnvir(const PtrEnvir &_parent = nullptr)
            {return PtrEnvir(new Environment(_parent));}
            static Environment globalEnvir;
            static void initGlobalEnvir();
    };

    std::optional<Cell> parseInput(std::istream &_in, bool quoted = false);
    std::optional<Cell> evaluate(const Cell &_expr, PtrEnvir &_envir);
    std::optional<Cell> apply(const Cell &_operat, const List &_operands, PtrEnvir &_envir);

    inline std::optional<Cell> parseString(const std::string &_str)
    {
        std::istringstream sin(_str);
        auto value = parseInput(sin);
        return value;
    }

    class Procedure{
        private:
            std::vector<std::string> params;
            Cell body;
            const PtrEnvir envir;

        public:
            Procedure(const std::vector<std::string> &_params, const Cell &_body, const PtrEnvir &_envir)
            : params(_params), body(_body), envir(_envir) {}
            std::optional<Cell> operator()(const List &_args, PtrEnvir &_envir);
    };
}