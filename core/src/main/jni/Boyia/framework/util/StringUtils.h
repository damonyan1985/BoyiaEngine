/*
 * StringUtils.h
 *
 *  Created on: 2011-6-25
 *      Author: yanbo
 */

#ifndef StringUtils_h
#define StringUtils_h

#include "UtilString.h"
#include "KVector.h"
namespace util
{

// string utility tools
class StringUtils
{
public:
	// delete the begin and end blank string from the source
	static String             skipBlank(const String& src);
	
	// convert string to int
	static LInt                stringToInt(const String& src, LInt radix = 10);
	
	// return heap data need to be deleted when it's not needed any more
	static KVector<String>*   split(const String& src, const String& splitSrc);
	
	static void                strTostrW(const String& src, StringW& dest);

	static void                strWtoStr(const StringW& src, String& dest);

	static LUint               hashCode(const String& keyName);

	//static LUint               genIdentify(const String& keyName);

	static LBool               isspace(LInt ch);
	static LBool               isdigit(LInt ch);
	static LBool               isalpha(LInt ch);
	static LBool               isalnum(LInt ch);
	static LInt                tolower(LInt ch);
	static LInt                toupper(LInt ch);

	static void                replaceSpecialChars(util::String& text);
};

}

using util::StringUtils;
#endif /* STRINGUTILS_H_ */
