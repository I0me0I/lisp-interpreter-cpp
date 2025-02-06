#include "lisp.h"
#include <functional>
#include <sstream>
#include <fstream>

void printCell(const lisp::Cell &_cell)
{
    if(_cell.isType<lisp::List>()){
        auto list = _cell.get<lisp::List>();
        std::cout << '(';
        
        auto it = list.begin();
        while(it != list.end()){
            printCell(*it);
            it++;

            if(it != list.end()){
                std::cout << ' ';
            }
        }

        std::cout << ')';
    }
    else{
        _cell.visit(
            [](auto &_argu){
                using T = std::decay_t<decltype(_argu)>;
                if constexpr(std::is_same_v<T, bool>){
                    std::cout << 'b' << _argu;
                }
                else if constexpr(std::is_same_v<T, int>){
                    std::cout << 'i' << _argu;
                }
                else if constexpr(std::is_same_v<T, float>){
                    std::cout << 'f' << _argu;
                }
                else if constexpr(std::is_same_v<T, std::string>){
                    std::cout << '\'' << _argu << '\'';
                }
                else if constexpr(std::is_same_v<T, lisp::Quotation>){
                    std::cout << '\"' << _argu.str() << '\"';
                }
                else if constexpr(std::is_same_v<T, lisp::PtrProc>){
                    std::cout << "Procedure";
                }
            }
        );
    }
}

int main(int argc, char *argv[])
{
    lisp::Environment::initGlobalEnvir();
    auto env = lisp::Environment::createEnvir();

    std::ifstream fin;
    std::istream *input;

    if(argc > 1){
        std::cout << argv[1] << std::endl;
        fin.open(argv[1]);
        input = &fin;
    }
    else{
        input = &std::cin;
    }

    while(true){
        auto cell = lisp::parseInput(*input);

        if(cell){
            printCell(cell.value());
            std::cout << std::endl;
            auto value = lisp::evaluate(cell.value(), env);

            if(value){
                printCell(value.value());
                std::cout << std::endl;
            }
            else{
                std::cout << "eval fail" << std::endl;
            }
        }
        else{
            std::cout << "parse fail "
                      << input->fail()
                      << input->bad()
                      << input->eof()
                      << std::endl;
            system("pause");
            return 0;
        }

        std::cout << "---------- ---------- ---------- ----------" << std::endl;
    }

    std::cin.get();
    return 0;
}
