#include "BoyiaError.h"
#include <stdio.h>

extern void jsLog(const char* format, ...);
// error
LVoid SntxError(LInt error, LInt lineNum) {
	static const LInt8 *errorMap[] = {
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
	};

	char info[50];
	sprintf(info, "Error=%s in line %d\n", errorMap[error], lineNum);
	jsLog("BoyiaVM Error=%s \n", info);
}
