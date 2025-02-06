#pragma once
#include "lispbase.h"
#include "embed.h"

namespace lisp
{
    // primary
    std::optional<Cell> buildinIf(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinCond(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinLambda(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinDefine(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinBegin(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinLet(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinAtom(const List &_args, PtrEnvir &_envir);
    std::optional<Cell> buildinSet(const List &_args, PtrEnvir &_envir);

    // Arithmetic
    extern Embedded plus, minus, multiplies, divides, modulus;
    // Comparisons
    extern Embedded equal, less, greater, lessEqual, greaterEqual;
    // Logical
    extern Embedded logicalNot, logicalAnd, logicalOr;
    // List Operate
    extern Cell buildinCons, buildinCar, buildinCdr;
}