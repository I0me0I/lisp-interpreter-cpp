#include "lispbase.h"
#include <cctype>
#include <sstream>

namespace lisp
{
    static bool isLegalChar(char _c)
    {
        if(isalnum(_c)){
            return true;
        }

        std::string chars = "_.+-*/=<>!?";

        return chars.find(_c) != std::string::npos;
    }

    static std::optional<Cell> parseInt(const std::string &_str)
    {
        std::istringstream sin(_str);
        int value;

        if((sin >> value) && (sin.eof())){
            return value;
        }
        
        return std::nullopt;
    }

    static std::optional<Cell> parseFloat(const std::string &_str)
    {
        std::istringstream sin(_str);
        float value;

        if((sin >> value) && (sin.eof())){
            return value;
        }

        return std::nullopt;
    }

    static std::optional<Cell> parseIdentifier(const std::string &_str)
    {
        return std::string(_str);
    }

    static std::string splitToken(std::istream &_in)
    {
        std::string value;

        while(true){
            auto ch = _in.get();

            if(_in.good() && isLegalChar(ch)){
                value.push_back(ch);
            }
            else{
                _in.unget();
                break;
            }
        }

        return value;
    }

    static std::optional<Cell> parseAtom(std::istream &_in, bool quoted)
    {
        auto value = splitToken(_in);
        if(value.empty()){
            return std::nullopt;
        }

        if(quoted){
            return Quotation(value);
        }

        auto cell = parseInt(value);
        if(cell){
            return cell;
        }

        cell = parseFloat(value);
        if(cell){
            return cell;
        }

        return parseIdentifier(value);
    }

    std::optional<Cell> parseInput(std::istream &_in, bool quoted)
    {
        while(isspace(_in.get()));

        if(!_in.good()){
            return std::nullopt;
        }

        _in.unget();

        if(_in.peek() == '('){
            _in.get();
            lisp::List list;

            while(true){
                auto cell = parseInput(_in, quoted);

                if(!cell){
                    break;
                }

                list.push_back(cell.value());
            }

            if(_in.get() == ')'){
                return list;
            }

            return std::nullopt;
        }

        if(_in.peek() == ')'){
            return std::nullopt;
        }

        if(_in.peek() == '\''){
            _in.get();

            if(quoted){
                return std::nullopt;
            }

            return parseInput(_in, true);
        }

        return parseAtom(_in, quoted);
    }
}