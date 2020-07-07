/*
 * StyleParser.cpp
 *
 *  Created on: 2011-6-23
 * Description: CSS Parser Engine
 *      Author: yanbo
 */

#include "StyleParser.h"
#include "KVector.h"
#include "LColor.h"
#include "LGdi.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "Style.h"
#include "StyleTags.h"

namespace util {

const LInt kStringBufferLen = 30;

StyleParser::StyleParser()
    : m_styleManager(kBoyiaNull)
{
}

StyleParser::~StyleParser()
{
    if (m_styleManager) {
        delete m_styleManager;
    }
}

void StyleParser::parseCss(InputStream& is)
{
    if (!m_styleManager) {
        m_styleManager = new StyleManager();
    }

    SelectorGroup* selectorGroup = new SelectorGroup(0, 20);
    Selector* selector = new Selector(0, 20);

    String selectText((LUint8)0, kStringBufferLen);

    LBool done = LFalse;
    LBool inComment = LFalse;
    LCharA prevChar = 0;

    LInt i = is.read();
    while (i != -1 && !done) {
        if (i >= 32) {
            LCharA c = (LCharA)i;
            if (inComment) {
                if (c == '/' && prevChar == '*') {
                    inComment = LFalse;
                }
            } else {
                switch (c) {
                case '<': {
                    done = LTrue;
                    return;
                }
                case '{': {

                    PropertyMap* declarations = parseDeclarations(is);
                    if (selectText.GetLength() > 0) {
                        SimpleSelector* realSelecor = new SimpleSelector();
                        realSelecor->setSelectorText(selectText);
                        selector->addElement(realSelecor);
                    }
                    selectorGroup->addElement(selector);
                    addSelectorGroup(selectorGroup, declarations);

                    selector = new Selector();
                    selectText.ClearBuffer();
                    selectorGroup = new SelectorGroup();

                    if (declarations) {
                        delete declarations;
                    }

                } break;
                case ',': {
                    SimpleSelector* realSelecor = new SimpleSelector();
                    realSelecor->setSelectorText(selectText);
                    selector->addElement(realSelecor);
                    selectorGroup->addElement(selector);
                    selectText.ClearBuffer();
                    selector = new Selector();
                } break;
                case ' ': {
                    if (selectText.GetLength() > 0) {
                        SimpleSelector* realSelecor = new SimpleSelector();
                        realSelecor->setSelectorText(selectText);
                        selector->addElement(realSelecor);
                        selectText.ClearBuffer();
                    }
                } break;
                case '/': {
                } break;
                case '*': {
                    if (prevChar == '/') {
                        inComment = LTrue;
                    }
                } break;
                default: {
                    selectText += (LUint8)c;
                } break;
                }
            }
            prevChar = c;
        }

        i = is.read();
        if (is.isEnd()) {
            done = LTrue;
        }
    }

    // last judge the new selector
    if (selector->size() == 0) {
        delete selector;
    }

    if (selectorGroup->size() == 0) {
        delete selectorGroup;
    }

    KFORMATLOG("ParseCSS = %s", "END");
}

PropertyMap* StyleParser::parseDeclarations(InputStream& is)
{
    LBool done = LFalse;
    PropertyMap* declarations = new PropertyMap();
    LBool inPropertyField = LTrue;

    PropertyName property((LUint8)0, kStringBufferLen);
    PropertyValue value((LUint8)0, kStringBufferLen);

    do {
        LInt i = is.read();
        if (i < 0) {
            done = LTrue;
        } else if (i >= 32) {
            LCharA c = (LCharA)i;
            switch (c) {
            case '}': {
                addDeclaration(declarations, property, value);
                done = LTrue;
            } break;
            case ':': {
                if (inPropertyField) {
                    inPropertyField = !inPropertyField;
                } else {
                    value += (LUint8)c;
                }
            } break;
            case ';': {
                addDeclaration(declarations, property, value);
                property.ClearBuffer();
                value.ClearBuffer();
                inPropertyField = LTrue;
            } break;
            case ' ': {
                if (!inPropertyField) {
                    value += (LUint8)c;
                }
            } break;
            default: {
                if (inPropertyField) {
                    property += (LUint8)c;
                } else {
                    value += (LUint8)c;
                }
            } break;
            }
        }
    } while (!done);

    return declarations;
}

void StyleParser::addDeclaration(PropertyMap* properties, PropertyName& prop, PropertyValue& value)
{
    if (prop.GetLength() > 0 && value.GetLength() > 0) {
        // 除去空白
        PropertyName realProp = StringUtils::skipBlank(prop);
        // 计算处理了大小写的字符串的标识符
        LInt type = StyleTags::getInstance()->symbolAsInt(realProp.ToLower());
        properties->put(type, value);
    }
}

void StyleParser::addSelectorGroup(SelectorGroup* selectors, PropertyMap* declarations)
{
    KLOG("StyleParser::addSelectorGroup");
    StyleRule* rule = StyleRule::New();
    //StyleTags* tags = StyleTags::getInstance();

    PropertyMap::Iterator iter = declarations->begin();
    PropertyMap::Iterator iterEnd = declarations->end();

    for (; iter != iterEnd; ++iter) {
        LUint property = (*iter)->getKey();
        PropertyValue value = (*iter)->getValue();

        value = StringUtils::skipBlank(value);
        KSTRLOG8(property);
        KSTRLOG8(value);
        addProperty(rule, property, value);
        KFORMATLOG("StyleParser:: PropertyValue = %s", (const char*)value.GetBuffer());
    }

    if (selectors->size()) {
        rule->setSelectorGroup(selectors);
        m_styleManager->addStyleRule(rule);
    }

    KLOG("StyleParser:: addSelectorGroup end");
}

LInt StyleParser::getCssColor(const String& colorValue)
{
    if (colorValue.GetLength() == 4) //Shorthand Hexadecimal Colors e.g #fff
    {
        KFORMATLOG("getCssColor = %s", (const char*)colorValue.GetBuffer());

        LUint8* colorBuffer = new LUint8[7];
        colorBuffer[0] = colorValue.CharAt(0);
        colorBuffer[1] = colorValue.CharAt(1);
        colorBuffer[2] = colorValue.CharAt(1);
        colorBuffer[3] = colorValue.CharAt(2);
        colorBuffer[4] = colorValue.CharAt(2);
        colorBuffer[5] = colorValue.CharAt(3);
        colorBuffer[6] = colorValue.CharAt(3);

        String colorStr(colorBuffer, LTrue, 7);

        return LColor::parseRgbString(colorStr);
    } else {
        BOYIA_LOG("getCssColor = %s", GET_STR(colorValue));
        return LColor::parseRgbString(colorValue);
    }
}

void StyleParser::addProperty(StyleRule* rule, LInt cssTag, PropertyValue& value)
{
    //StyleTags* tags = StyleTags::getInstance();
    //LInt cssTag = property;
    switch (cssTag) {
    case StyleTags::ALIGN: {
        if (value.CompareNoCase(_CS("top"))) {
            rule->addProperty(cssTag, Style::ALIGN_TOP);
        } else if (value.CompareNoCase(_CS("left"))) {
            rule->addProperty(cssTag, Style::ALIGN_LEFT);
        } else if (value.CompareNoCase(_CS("right"))) {
            rule->addProperty(cssTag, Style::ALIGN_RIGHT);
        } else if (value.CompareNoCase(_CS("bottom"))) {
            rule->addProperty(cssTag, Style::ALIGN_BOTTOM);
        } else if (value.CompareNoCase(_CS("center"))) {
            rule->addProperty(cssTag, Style::ALIGN_CENTER);
        }
    } break;
    case StyleTags::FONT_WEIGHT: {
        if (value.CompareNoCase(_CS("bold"))) {
            rule->addProperty(cssTag, LFont::FONT_STYLE_BOLD);
        }
    } break;
    case StyleTags::FONT_STYLE: {
        if (value.CompareNoCase(_CS("italic"))) {
            rule->addProperty(cssTag, LFont::FONT_STYLE_ITALIC);
        } else if (value.CompareNoCase(_CS("bold"))) {
            rule->addProperty(cssTag, LFont::FONT_STYLE_BOLD);
        } else if (value.CompareNoCase(_CS("underline"))) {
            rule->addProperty(cssTag, LFont::FONT_STYLE_UNDERLINE);
        }
    } break;
    case StyleTags::WIDTH:
    case StyleTags::HEIGHT:
    case StyleTags::TOP:
    case StyleTags::LEFT:
    case StyleTags::MARGIN_TOP:
    case StyleTags::MARGIN_LEFT:
    case StyleTags::MARGIB_BOTTOM:
    case StyleTags::MARGIN_RIGHT:
    case StyleTags::PADDING_BOTTOM:
    case StyleTags::PADDING_LEFT:
    case StyleTags::PADDING_RIGHT:
    case StyleTags::PADDING_TOP: {
        if (value.EndWithNoCase(_CS("px"))) {
            LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength() - 2));
            rule->addProperty(cssTag, intValue);
        }
    } break;
    case StyleTags::POSITION: {
        if (value.CompareNoCase(_CS("relative"))) {
            rule->addProperty(cssTag, Style::RELATIVEPOSITION);
        } else if (value.CompareNoCase(_CS("absolute"))) {
            rule->addProperty(cssTag, Style::ABSOLUTEPOSITION);
        } else if (value.CompareNoCase(_CS("fixed"))) {
            rule->addProperty(cssTag, Style::FIXEDPOSITION);
        } else {
            rule->addProperty(cssTag, Style::STATICPOSITION);
        }
    } break;
    case StyleTags::PADDING: {
        //    	    KVector<String>* values = StringUtils::split(value, _CS(" "));
        //    	    LInt size = values->size();
        //    	    for (LInt i=0; i<size; i++)
        //    	    {
        //    	        String oneValue = values->elementAt(i);
        //
        //    	        if (oneValue.EndWithNoCase(_CS("px")))
        //    	        {
        //
        //					LInt intValue = StringUtils::stringToInt(value.Mid(0, oneValue.GetLength()-2));
        //    	            KLOG("PADDING");
        //    	            KDESLOG(intValue);
        //					rule->addProperty(StyleTags::PADDING_TOP, intValue);
        //					rule->addProperty(StyleTags::PADDING_BOTTOM, intValue);
        //					rule->addProperty(StyleTags::PADDING_LEFT, intValue);
        //					rule->addProperty(StyleTags::PADDING_RIGHT, intValue);
        //    	        }
        //    	    }
        //
        //    	    delete values;
        //    	    values = NULL;
    } break;
    case StyleTags::BORDER_STYLE:
    case StyleTags::BORDER_TOP_STYLE:
    case StyleTags::BORDER_LEFT_STYLE:
    case StyleTags::BORDER_RIGHT_STYLE:
    case StyleTags::BORDER_BOTTOM_STYLE: {
        if (value.CompareNoCase(_CS("dotted"))) {
            rule->addProperty(cssTag, LGraphicsContext::kDotPen);
        } else {
            rule->addProperty(cssTag, LGraphicsContext::kSolidPen);
        }
    } break;
    case StyleTags::BORDER_BOTTOM_WIDTH:
    case StyleTags::BORDER_TOP_WIDTH:
    case StyleTags::BORDER_LEFT_WIDTH:
    case StyleTags::BORDER_RIGHT_WIDTH: {
        if (value.EndWithNoCase(_CS("px"))) {
            LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength() - 2));
            rule->addProperty(cssTag, intValue);
        }
    } break;
    case StyleTags::BACKGROUND: {
        KVector<String>* values = StringUtils::split(value, _CS(" "));
        LInt size = values->size();
        for (LInt i = 0; i < size; i++) {
            String oneValue = values->elementAt(i);
            if (oneValue.StartWith(_CS("#"))) {
                rule->addProperty(StyleTags::BACKGROUND_COLOR, getCssColor(oneValue));
            }

            if (oneValue.ToLower().StartWith(_CS("url"))) {
                LInt pos1 = oneValue.Find(_CS("\""));
                LInt pos2 = oneValue.ReverseFind(_CS("\""));
                String urlValue;
                if (!(pos1 >= 0 && pos2 >= 0)) {
                    pos1 = oneValue.Find(_CS("("));
                    pos2 = oneValue.ReverseFind(_CS(")"));
                }

                urlValue = oneValue.Mid(pos1 + 1, pos2 - pos1 - 1);
                rule->addProperty(cssTag, urlValue);
            }
        }

        delete values;
        values = kBoyiaNull;
    } break;
    case StyleTags::BACKGROUND_COLOR:
    case StyleTags::COLOR:
    case StyleTags::BORDER_COLOR:
    case StyleTags::BORDER_LEFT_COLOR:
    case StyleTags::BORDER_RIGHT_COLOR:
    case StyleTags::BORDER_BOTTOM_COLOR:
    case StyleTags::BORDER_TOP_COLOR: {
        LInt color = COLOR_WHITE;
        if (value.StartWith(_CS("#"))) {
            color = getCssColor(value);
        } else if (value.CompareNoCase(_CS("red"))) {
            color = COLOR_RED;
        } else if (value.CompareNoCase(_CS("blue"))) {
            color = COLOR_BLUE;
        } else if (value.CompareNoCase(_CS("green"))) {
            color = COLOR_GREEN;
        } else if (value.CompareNoCase(_CS("yellow"))) {
            color = COLOR_YELLOW;
        } else if (value.CompareNoCase(_CS("pink"))) {
            color = COLOR_PINK;
        }

        KFORMATLOG("color tag = %d and color = %x", cssTag, color);
        rule->addProperty(cssTag, color);
    } break;
    case StyleTags::FONT_SIZE: {
        if (value.CompareNoCase(_CS("large"))) {
            rule->addProperty(cssTag, 60);
        } else if (value.CompareNoCase(_CS("medium"))) {
            rule->addProperty(cssTag, 40);
        } else if (value.CompareNoCase(_CS("small"))) {
            rule->addProperty(cssTag, 24);
        } else if (value.EndWithNoCase(_CS("px"))) {
            LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength() - 2));
            rule->addProperty(cssTag, intValue);
        }
    } break;
    case StyleTags::TEXT_ALIGN: {
        if (value.CompareNoCase(_CS("left"))) {
            rule->addProperty(cssTag, LGraphicsContext::kTextLeft);
        } else if (value.CompareNoCase(_CS("center"))) {
            rule->addProperty(cssTag, LGraphicsContext::kTextCenter);
        } else if (value.CompareNoCase(_CS("right"))) {
            rule->addProperty(cssTag, LGraphicsContext::kTextRight);
        }
    } break;
    case StyleTags::BORDER_BOTTOM: {
        KVector<String>* values = StringUtils::split(value, _CS(" "));
        LInt size = values->size();
        for (LInt i = 0; i < size; ++i) {
            String oneValue = values->elementAt(i);
            if (oneValue.StartWith(_CS("#"))) {
                rule->addProperty(StyleTags::BORDER_BOTTOM_COLOR, getCssColor(oneValue));
            } else if (oneValue.StartWithNoCase(_CS("solid"))) {
                rule->addProperty(StyleTags::BORDER_BOTTOM_STYLE, LGraphicsContext::kSolidPen);
            } else if (oneValue.EndWithNoCase(_CS("px"))) {
                LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength() - 2));
                rule->addProperty(StyleTags::BORDER_BOTTOM_WIDTH, intValue);
            }
        }

        delete values;
        values = kBoyiaNull;
    } break;
    case StyleTags::BORDER_TOP: {
        KVector<String>* values = StringUtils::split(value, _CS(" "));
        LInt size = values->size();
        for (LInt i = 0; i < size; i++) {
            String oneValue = values->elementAt(i);
            if (oneValue.StartWith(_CS("#"))) {
                rule->addProperty(StyleTags::BORDER_TOP_COLOR, getCssColor(oneValue));
            } else if (oneValue.StartWithNoCase(_CS("solid"))) {
                rule->addProperty(StyleTags::BORDER_TOP_STYLE, LGraphicsContext::kSolidPen);
            } else if (oneValue.StartWithNoCase(_CS("px"))) {
                LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength() - 2));
                rule->addProperty(StyleTags::BORDER_TOP_WIDTH, intValue);
            }
        }

        delete values;
        values = kBoyiaNull;
    } break;
    case StyleTags::BORDER: {
        KVector<String>* values = StringUtils::split(value, _CS(" "));
        LInt size = values->size();
        for (LInt i = 0; i < size; i++) {
            String oneValue = values->elementAt(i);

            if (oneValue.StartWith(_CS("#"))) {
                rule->addProperty(StyleTags::BORDER_COLOR, getCssColor(oneValue));
            } else if (oneValue.StartWithNoCase(_CS("solid"))) {
                rule->addProperty(StyleTags::BORDER_STYLE, LGraphicsContext::kSolidPen);
            }
        }

        delete values;
        values = kBoyiaNull;
    } break;
    case StyleTags::DISPLAY: {
        if (value.CompareNoCase(_CS("none"))) {
            rule->addProperty(StyleTags::DISPLAY, Style::DISPLAY_NONE);
        } else if (value.CompareNoCase(_CS("block"))) {
            rule->addProperty(StyleTags::DISPLAY, Style::DISPLAY_BLOCK);
        } else if (value.CompareNoCase(_CS("inline"))) {
            rule->addProperty(StyleTags::DISPLAY, Style::DISPLAY_INLINE);
        }
    } break;
    case StyleTags::SCALE: {
        rule->addProperty(StyleTags::SCALE, StringUtils::stringToInt(value));
    } break;
    case StyleTags::Z_INDEX: {
        rule->addProperty(StyleTags::Z_INDEX, StringUtils::stringToInt(value));
    } break;
    case StyleTags::FOCUSABLE: {
        if (value.CompareNoCase(_CS("true"))) {
            rule->addProperty(cssTag, LTrue);
        } else {
            rule->addProperty(cssTag, LFalse);
        }
    } break;
    case StyleTags::FLEX_DIRECTION: {
        if (value.CompareNoCase(_CS("row"))) {
            rule->addProperty(StyleTags::FLEX_DIRECTION, Style::FLEX_ROW);
        } else if (value.CompareNoCase(_CS("column"))) {
            rule->addProperty(StyleTags::FLEX_DIRECTION, Style::FLEX_COLUMN);
        } else if (value.CompareNoCase(_CS("row-reverse"))) {
            rule->addProperty(StyleTags::FLEX_DIRECTION, Style::FLEX_ROW_REVERSE);
        } else if (value.CompareNoCase(_CS("column-reverse"))) {
            rule->addProperty(StyleTags::FLEX_DIRECTION, Style::FLEX_COLUMN_REVERSE);
        }
    } break;
    default: {
    } break;
    }
}

StyleManager* StyleParser::getStyleManager() const
{
    return m_styleManager;
}
}
