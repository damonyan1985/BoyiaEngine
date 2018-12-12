/*
 * CSSParser.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef CssParser_h
#define CssParser_h

#include "CssManager.h"
//#include "kmap.h"
#include "CssRule.h"
#include "InputStream.h"

namespace util
{
/**
 * CSS Parser Engine
 */
class CssParser
{
public:
	CssParser();
	~CssParser();
	
public:	
	void parseCss(InputStream& is);
	CssManager* getCssManager() const;
	
private:
	PropertyMap* parseDeclarations(InputStream& is);
	
	void addSelectorGroup(SelectorGroup* selectors, PropertyMap* declarations);
	void addDeclaration(PropertyMap* properties, PropertyName& prop, PropertyValue& value);

	LInt getCssColor(const String& colorValue);
	void addProperty(CssRule* rule, LUint property, PropertyValue& value);

private:
	CssManager* m_cssManager;
};

}
#endif /* CSSPARSER_H_ */
