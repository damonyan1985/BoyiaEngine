#include "PlatformLib.h"

#ifndef BoyiaError_h
#define BoyiaError_h

enum ErrorMsg {
    SYNTAX,
    UNBAL_PARENS,
    NO_EXP,
    EQUALS_EXPECTED,
    NOT_VAR,
    PARAM_ERR,
    SEMI_EXPECTED,
    UNBAL_BRACES, // 大括号
    FUNC_UNDEF,
    TYPE_EXPECTED,
    NEST_FUNC,
    RET_NOCALL,
    PAREN_EXPECTED,
    WHILE_EXPECTED,
    QUOTE_EXPECTED,
    NOT_TEMP,
    TOO_MANY_LVARS,
    DIV_BY_ZERO,
    ARRAY_LEN_ERROR,
    ARRAY_BOUNDARY_ERROR,
    VALUE_NOT_OBJECT,
    LPTR_EXPECTED,
    RPTR_EXPECTED,
    RUNTIME_NO_PROP,
    CREATE_MAP_ERROR,
    MAP_KEY_VALUE_ERROR,
};

LVoid SntxError(LInt error, LInt lineNum);
LVoid PrintErrorKey(LUintPtr key, LInt error, LInt lineNum, LVoid* vm);

#endif // !MiniError_h
