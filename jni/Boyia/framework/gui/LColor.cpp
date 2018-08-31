/*
 * Color.cpp
 *
 *  Created on: 2011-6-22
 *      Author: yanbo
 */

#include "LColor.h"
#include "StringUtils.h"

namespace util
{

LUint LColor::rgb(LInt red, LInt green, LInt blue, LInt alpha)
{
	if (red < 0 || red > 255 ||
			green < 0 || green > 255 ||
			blue < 0 || blue > 255 ||
			alpha < 0 || alpha > 255)
	{
	    return 0;
	}

	LUint value = ((alpha << 24) | (red << 16) | (green << 8) | blue);
	KFORMATLOG("rgb value=%x", value);
	return value;
	//return (red << 16) | (green << 8) | blue;
}

LRgb LColor::parseRgbInt(LUint rgbValue)
{
	LUint8 r = LGetRValue(rgbValue);
	LUint8 g = LGetGValue(rgbValue);
	LUint8 b = LGetBValue(rgbValue);
	return LRgb(r, g, b);
}

LRgb LColor::parseArgbInt(LUint argbValue)
{
	LUint8 a = (argbValue >> 24) & 255;
	LUint8 r = (argbValue >> 16) & 255;
	LUint8 g = (argbValue >> 8) & 255;
	LUint8 b = argbValue & 255;
	return LRgb(r, g, b, a);
}

LUint LColor::parseRgbString(const String& rgbString)
{
	LInt r = StringUtils::stringToInt(rgbString.Mid(1, 2), 16);
	LInt g = StringUtils::stringToInt(rgbString.Mid(3, 2), 16);
	LInt b = StringUtils::stringToInt(rgbString.Mid(5, 2), 16);
	LInt a = 0xFF;
	if (rgbString.GetLength() > 8) {
		a = StringUtils::stringToInt(rgbString.Mid(7, 2), 16);
	}
	return rgb(r,g,b,a);
}

}
