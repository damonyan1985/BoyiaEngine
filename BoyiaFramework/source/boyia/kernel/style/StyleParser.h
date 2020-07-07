/*
 * StyleParser.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef StyleParser_h
#define StyleParser_h

#include "StyleManager.h"
//#include "kmap.h"
#include "InputStream.h"
#include "StyleRule.h"

namespace util {
/**
 * CSS Parser Engine
 */
class StyleParser {
public:
    StyleParser();
    ~StyleParser();

public:
    void parseCss(InputStream& is);
    StyleManager* getStyleManager() const;

private:
    PropertyMap* parseDeclarations(InputStream& is);

    void addSelectorGroup(SelectorGroup* selectors, PropertyMap* declarations);
    void addDeclaration(PropertyMap* properties, PropertyName& prop, PropertyValue& value);

    LInt getCssColor(const String& colorValue);
    void addProperty(StyleRule* rule, LInt property, PropertyValue& value);

private:
    StyleManager* m_styleManager;
};
}
#endif /* StyleParser_H_ */
