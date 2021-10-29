#include "BoyiaError.h"
#include "SalLog.h"
#include "BoyiaValue.h"

extern LVoid GetIdentName(LUintPtr key, BoyiaStr* str, LVoid* vm);

static const char* FindErrorInfo(LInt error)
{
    static const LInt8* errorMap[] = {
        "syntax error",
        "unbalanced parentheses",
        "no expression present",
        "equals sign expected",
        "not a variable",
        "parameter error",
        "semicolon expected",
        "unbalanced braces",
        "function undefined",
        "type specifier expected",
        "too many nested function calls",
        "return without call",
        "parentheses expected",
        "while expected",
        "closing quote expected",
        "not a string",
        "too many local variables",
        "operator div by zero",
        "array length must be digit",
        "out of array boundary",
        "value is not a object",
        "left parentheses expected",
        "right parentheses expected",
        "is not a prop of the object"
    };

    return errorMap[error];
}
// error
LVoid SntxError(LInt error, LInt lineNum)
{
    BOYIA_LOG("BoyiaVM Error=%s lineno=%d", FindErrorInfo(error), lineNum);
}

LVoid PrintErrorKey(LUintPtr key, LInt error, LInt lineNum, LVoid* vm)
{
    BoyiaStr name;
    GetIdentName(key, &name, vm);
    BOYIA_LOG("BoyiaVM Error=%s lineno=%d key=%s", FindErrorInfo(error), lineNum, name.mPtr);
}
