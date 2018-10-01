/*
 * CssParser.cpp
 *
 *  Created on: 2011-6-23
 * Description: CSS Parser Engine
 *      Author: yanbo
 */

#include "CssParser.h"
#include "KVector.h"
#include "CssTags.h"
#include "StringUtils.h"
#include "Style.h"
#include "LGdi.h"
#include "LColor.h"
#include "SalLog.h"

namespace util
{

CssParser::CssParser()
    : m_cssManager(NULL)
{	
}

CssParser::~CssParser()
{	
	if (m_cssManager)
	{
	    delete m_cssManager;
	    m_cssManager = NULL;
	}
}

void CssParser::parseCss(InputStream& is)
{
	if (!m_cssManager)
	{
	    m_cssManager = new CssManager();
	}

	SelectorGroup* selectorGroup = new SelectorGroup(0, 20);
	Selector* selector = new Selector(0, 20);
	
	String selectText;
	
	LBool done = LFalse;
	LBool inComment = LFalse;
	LCharA prevChar = 0;
	
	LInt i = is.read();
	while (i != -1 && !done)
	{
	    if (i >= 32)
	    {
	        LCharA c = (LCharA)i;
	        if (inComment)
	        {
				if (c == '/' && prevChar == '*')
				{
					inComment = LFalse;
				}
	        }
	        else
	        {
	            switch (c)
				{
	            case '<': 
				    {
	            	    done = LTrue;
	            	    return;
	            	}
	            case '{': 
					{
	            	    
	            	    PropertyMap* declarations = parseDeclarations(is);
	            	    if (selectText.GetLength() > 0)
						{
							SimpleSelector* realSelecor = new SimpleSelector();
							realSelecor->setSelectorText(selectText);
	            	        selector->addElement(realSelecor);
	            	    }
	            	    selectorGroup->addElement(selector);
	            	    addSelectorGroup(selectorGroup, declarations);
	            	    
	            	    selector = new Selector();
	            	    selectText = _CS(NULL);
						selectorGroup = new SelectorGroup();

						if (declarations != NULL)
						{
							delete declarations;
							declarations = NULL;
						}

	            	}
					break;
	            case ',': 
					{
						SimpleSelector* realSelecor = new SimpleSelector();
						realSelecor->setSelectorText(selectText);
	            	    selector->addElement(realSelecor);
	            	    selectorGroup->addElement(selector);
	            	    selectText = _CS(NULL);
	            	    selector = new Selector();
	            	}
					break;
	            case ' ': 
				    {
	            	    if (selectText.GetLength() > 0)
	            	    {
							SimpleSelector* realSelecor = new SimpleSelector();
							realSelecor->setSelectorText(selectText);
	            	        selector->addElement(realSelecor);
	            	        selectText = _CS(NULL);
	            	    }
	            	}
				    break;
	            case '/': 
					{	            	
	            	}
					break;
	            case '*':
					{
						if (prevChar == '/')
					    {
							inComment = LTrue;
						}	            	
	            	}
					break;
                default: 
					{            
                        selectText += (LUint8)c;
                    }
					break;
	            }
	        }
	        prevChar = c;
	    }
	    
	    i = is.read();
	    if (is.isEnd())
	    {
	        done = LTrue;
	    }


	}
	
	// last judge the new selector
	if (selector->size() == 0)
	{
	    delete selector;
	    selector = NULL;
	}
	
	if (selectorGroup->size() == 0)
	{
		delete selectorGroup;
	    selectorGroup = NULL;
	}


	KFORMATLOG("ParseCSS = %s", "END");
}

PropertyMap* CssParser::parseDeclarations(InputStream& is)
{
	LBool done = LFalse;
	PropertyMap* declarations = new PropertyMap();
	LBool inPropertyField = LTrue;
	
	PropertyName      property;
	PropertyValue     value;
	
	do {
	    LInt i = is.read();
	    if (i < 0)
		{
	        done = LTrue;
	    }
	    else if (i >= 32)
		{
	        LCharA c = (LCharA)i;
	        switch (c)
			{
	        case '}': 
			    {
	        	    addDeclaration(declarations, property, value);
	        	    done = LTrue;
	        	}
				break;
	        case ':': 
				{
	        	    if(inPropertyField) 
					{
	        	        inPropertyField = !inPropertyField;
	        	    } 
					else 
					{
	        	        value += (LUint8)c;
	        	    }
	        	}
				break;
	        case ';': 
				{
	        	    addDeclaration(declarations, property, value);
	        	    property                = _CS(NULL);
	        	    value                   = _CS(NULL);
	        	    inPropertyField         = LTrue;
	        	}
				break;
	        case ' ': 
				{
	        	    if (!inPropertyField)
					{
	        	        value += (LUint8)c;
	        	    }
	        	}
				break;
	        default: 
				{
					if (inPropertyField)
					{
						property += (LUint8)c;
					}
					else 
					{
						value += (LUint8)c;
					}
	        	}
				break;
	        }
	    }
	} while (!done);

	return declarations;
}

void CssParser::addDeclaration(PropertyMap* properties, PropertyName& prop, PropertyValue& value)
{
	if (prop.GetLength() > 0 && value.GetLength() > 0)
	{
		// 除去空白
		PropertyName realProp = StringUtils::skipBlank(prop);
		// 计算处理了大小写的字符串的哈希值
	    properties->put(CssTags::getInstance()->genIdentify(realProp.ToLower()), value);
	}
}

void CssParser::addSelectorGroup(SelectorGroup* selectors, PropertyMap* declarations)
{
	KLOG("CssParser::addSelectorGroup");
	CssRule* rule = CssRule::New();
	CssTags* tags = CssTags::getInstance();
	
	PropertyMap::Iterator iter     = declarations->begin();
	PropertyMap::Iterator iterEnd = declarations->end();
	
	for (; iter != iterEnd; ++iter)
	{
		LUint property        = (*iter)->getKey();
		PropertyValue value   = (*iter)->getValue();

		value = StringUtils::skipBlank(value);
		KSTRLOG8(property);
		KSTRLOG8(value);
		addProperty(rule, property, value);
		KFORMATLOG("CssParser:: PropertyValue = %s", (const char*)value.GetBuffer());
	}
	
	if (selectors->size())
	{
		rule->setSelectorGroup(selectors);
		m_cssManager->addCssRule(rule);
	}

	KLOG("CssParser:: addSelectorGroup end");
}

LInt CssParser::getCssColor(const String& colorValue)
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
	}
	else
	{
		KFORMATLOG("getCssColor = %s", (const char*)colorValue.GetBuffer());
	    return LColor::parseRgbString(colorValue);
	}
}

void CssParser::addProperty(CssRule* rule, LUint property, PropertyValue& value)
{
	CssTags* tags = CssTags::getInstance();
    LInt cssTag = tags->symbolAsInt(property);
    switch (cssTag)
    {
    case CssTags::FONT_WEIGHT:
    	{
    		if (value.CompareNoCase(_CS("bold")))
    		{
    		    rule->addProperty(cssTag, LFont::FONT_STYLE_BOLD);
    		}
    	}
    	break;
    case CssTags::FONT_STYLE:
    	{
			if (value.CompareNoCase(_CS("italic")))
			{
				rule->addProperty(cssTag, LFont::FONT_STYLE_ITALIC);
			}
			else if (value.CompareNoCase(_CS("bold")))
			{
				rule->addProperty(cssTag, LFont::FONT_STYLE_BOLD);
			}
			else if (value.CompareNoCase(_CS("underline")))
			{
				rule->addProperty(cssTag, LFont::FONT_STYLE_UNDERLINE);
			}
    	}
    	break;
    case CssTags::WIDTH:
    case CssTags::HEIGHT:
    case CssTags::TOP:
    case CssTags::LEFT:
    case CssTags::MARGIN_TOP:
    case CssTags::MARGIN_LEFT:
    case CssTags::MARGIB_BOTTOM:
    case CssTags::MARGIN_RIGHT:
    case CssTags::PADDING_BOTTOM:
    case CssTags::PADDING_LEFT:
    case CssTags::PADDING_RIGHT:
    case CssTags::PADDING_TOP:
    	{   
    	    if (value.EndWithNoCase(_CS("px")))
    	    {
    	        LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength()-2));
    	        rule->addProperty(cssTag, intValue);
    	    }
    	}
    	break;
    case CssTags::POSITION:
    	{
    	    if (value.CompareNoCase(_CS("relative")))
    	    {
    	        rule->addProperty(cssTag, Style::RELATIVEPOSITION);
    	    }
    	    else if (value.CompareNoCase(_CS("absolute")))
    	    {
    	        rule->addProperty(cssTag, Style::ABSOLUTEPOSITION);
    	    }
    	    else if (value.CompareNoCase(_CS("fixed")))
    	    {
    	        rule->addProperty(cssTag, Style::FIXEDPOSITION);
    	    }
    	    else
    	    {
    	        rule->addProperty(cssTag, Style::STATICPOSITION);
    	    }
    	}
    	break;
    case CssTags::PADDING:
    	{
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
//					rule->addProperty(CssTags::PADDING_TOP, intValue);
//					rule->addProperty(CssTags::PADDING_BOTTOM, intValue);
//					rule->addProperty(CssTags::PADDING_LEFT, intValue);
//					rule->addProperty(CssTags::PADDING_RIGHT, intValue);
//    	        }
//    	    }
//    	    
//    	    delete values;
//    	    values = NULL;
    	}
    	break;
    case CssTags::BORDER_STYLE:
    case CssTags::BORDER_TOP_STYLE:
    case CssTags::BORDER_LEFT_STYLE:
    case CssTags::BORDER_RIGHT_STYLE:
    case CssTags::BORDER_BOTTOM_STYLE:	
    	{
    	    if (value.CompareNoCase(_CS("dotted")))
    	    {
    	        rule->addProperty(cssTag, LGraphicsContext::DotPen);
    	    }
    	    else
    	    {
    	        rule->addProperty(cssTag, LGraphicsContext::SolidPen);
    	    }
    	}
    	break;
    case CssTags::BACKGROUND:
    	{
    	    KVector<String>* values = StringUtils::split(value, _CS(" "));
    	    LInt size = values->size();
    	    for (LInt i=0; i<size; i++)
    	    {
    	        String oneValue = values->elementAt(i);
    	        if (oneValue.StartWith(_CS("#")))
    	        {
					rule->addProperty(CssTags::BACKGROUND_COLOR, getCssColor(oneValue));
    	        }
    	        
	            if (oneValue.ToLower().StartWith(_CS("url")))
	            {
	                LInt pos1 = oneValue.Find(_CS("\""));
	                LInt pos2 = oneValue.ReverseFind(_CS("\""));
	                String urlValue;
	                if (!(pos1 >= 0 && pos2 >= 0))
	                {
                        pos1 = oneValue.Find(_CS("("));
                        pos2 = oneValue.ReverseFind(_CS(")"));
	                }
	                
	                urlValue = oneValue.Mid(pos1+1, pos2-pos1-1);
	                rule->addProperty(cssTag, urlValue);
	            }
    	    }
    	    
    	    delete values;
    	    values = NULL;
    	}
    	break;
    case CssTags::BACKGROUND_COLOR:
    case CssTags::COLOR:
    case CssTags::BORDER_COLOR:
    case CssTags::BORDER_LEFT_COLOR:
    case CssTags::BORDER_RIGHT_COLOR:
    case CssTags::BORDER_BOTTOM_COLOR:
    case CssTags::BORDER_TOP_COLOR:	
    	{
    	    LInt color = COLOR_WHITE;
    	    if (value.StartWith(_CS("#")))
    	    {
    	        color = getCssColor(value);
    	    }
    	    else if (value.CompareNoCase(_CS("red")))
    	    {
    	        color = COLOR_RED;
    	    }
    	    else if (value.CompareNoCase(_CS("blue")))
    	    {
    	        color = COLOR_BLUE;
    	    }
    	    else if (value.CompareNoCase(_CS("green")))
    	    {
    	        color = COLOR_GREEN;
    	    }
    	    else if (value.CompareNoCase(_CS("yellow")))
    	    {
    	        color = COLOR_YELLOW;
    	    }
    	    else if (value.CompareNoCase(_CS("pink")))
    	    {
    	        color = COLOR_PINK;
    	    }

			KFORMATLOG("color tag = %d and color = %x", cssTag, color);
			rule->addProperty(cssTag, color);
    	}
    	break;
    case CssTags::FONT_SIZE:
    	{
    	    if (value.CompareNoCase(_CS("large")))
    	    {
    	        rule->addProperty(cssTag, 60);
    	    }
    	    else if (value.CompareNoCase(_CS("medium")))
    	    {
    	        rule->addProperty(cssTag, 40);
    	    }
    	    else if (value.CompareNoCase(_CS("small")))
    	    {
    	        rule->addProperty(cssTag, 24);
    	    }
			else if (value.EndWithNoCase(_CS("px")))
			{
				LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength()-2));
				rule->addProperty(cssTag, intValue);
			}
    	}
    	break;
    case CssTags::TEXT_ALIGN:
    	{
			if (value.CompareNoCase(_CS("left")))
			{
				rule->addProperty(cssTag, LGraphicsContext::TextLeft);
			}
			else if (value.CompareNoCase(_CS("center")))
			{
				rule->addProperty(cssTag, LGraphicsContext::TextCenter);
			}
			else if (value.CompareNoCase(_CS("right")))
			{
				rule->addProperty(cssTag, LGraphicsContext::TextRight);
			}
    	}
    	break;
    case CssTags::BORDER_BOTTOM:
    	{
   	        KVector<String>* values = StringUtils::split(value, _CS(" "));
    	    LInt size = values->size();
    	    for (LInt i = 0; i < size; ++i)
    	    {
    	        String oneValue = values->elementAt(i);
    	        if (oneValue.StartWith(_CS("#")))
    	        {
					rule->addProperty(CssTags::BORDER_BOTTOM_COLOR, getCssColor(oneValue));
    	        }
    	        else if (oneValue.StartWithNoCase(_CS("solid")))
    	        {
    	            rule->addProperty(CssTags::BORDER_BOTTOM_STYLE, LGraphicsContext::SolidPen);
    	        }
    	        else if (oneValue.EndWithNoCase(_CS("px")))
    	        {
					LInt intValue = StringUtils::stringToInt(value.Mid(0, value.GetLength()-2));
					rule->addProperty(CssTags::BORDER_BOTTOM_WIDTH, intValue);
    	        }
    	    }
    	    
    	    delete values;
    	    values = NULL;
    	}
    	break;
    case CssTags::BORDER_TOP:
    	{
   	        KVector<String>* values = StringUtils::split(value, _CS(" "));
    	    LInt size = values->size();
    	    for (LInt i = 0; i < size; i++)
    	    {
    	        String oneValue = values->elementAt(i);
    	        if (oneValue.StartWith(_CS("#")))
    	        {
					rule->addProperty(CssTags::BORDER_TOP_COLOR, getCssColor(oneValue));
    	        }
    	        else if (oneValue.StartWithNoCase(_CS("solid")))
    	        {
    	            rule->addProperty(CssTags::BORDER_TOP_STYLE, LGraphicsContext::SolidPen);
    	        }
    	    }
    	    
    	    delete values;
    	    values = NULL;
    	}
    	break;
    case CssTags::BORDER:
    	{
   	        KVector<String>* values = StringUtils::split(value, _CS(" "));
    	    LInt size = values->size();
    	    for (LInt i = 0; i < size; i++)
    	    {
    	        String oneValue = values->elementAt(i);

    	    	if (oneValue.StartWith(_CS("#")))
    	        {
					rule->addProperty(CssTags::BORDER_COLOR, getCssColor(oneValue));
    	        }
    	        else if (oneValue.StartWithNoCase(_CS("solid")))
    	        {
    	            rule->addProperty(CssTags::BORDER_STYLE, LGraphicsContext::SolidPen);
    	        }
    	    }
    	    
    	    delete values;
    	    values = NULL;
    	}
    	break;
    case CssTags::DISPLAY:
    	{
    	    if (value.CompareNoCase(_CS("none")))
    	    {
    	        rule->addProperty(CssTags::DISPLAY, Style::DISPLAY_NONE);
    	    }
    	    else if (value.CompareNoCase(_CS("block")))
    	    {
    	        rule->addProperty(CssTags::DISPLAY, Style::DISPLAY_BLOCK);
    	    }
    	    else if (value.CompareNoCase(_CS("inline")))
    	    {
    	        rule->addProperty(CssTags::DISPLAY, Style::DISPLAY_INLINE);
    	    }
    	}
    	break;
    case CssTags::SCALE:
        {
        	rule->addProperty(CssTags::SCALE, StringUtils::stringToInt(value));
        }
        break;
    case CssTags::Z_INDEX:
        {
        	rule->addProperty(CssTags::Z_INDEX, StringUtils::stringToInt(value));
        }
        break;
    case CssTags::FOCUSABLE:
        {
        	if (value.CompareNoCase(_CS("true")))
        	{
        		rule->addProperty(cssTag, LTrue);
        	}
        	else
        	{
        		rule->addProperty(cssTag, LFalse);
        	}
        }
        break;
    case CssTags::FLEX_DIRECTION:
        {
            if (value.CompareNoCase(_CS("row")))
        	{
                rule->addProperty(CssTags::FLEX_DIRECTION, Style::FLEX_ROW);
            }
            else if (value.CompareNoCase(_CS("column")))
            {
                rule->addProperty(CssTags::FLEX_DIRECTION, Style::FLEX_COLUMN);
            }
            else if (value.CompareNoCase(_CS("row-reverse")))
            {
                rule->addProperty(CssTags::FLEX_DIRECTION, Style::FLEX_ROW_REVERSE);
            }
            else if (value.CompareNoCase(_CS("column-reverse")))
            {
                rule->addProperty(CssTags::FLEX_DIRECTION, Style::FLEX_COLUMN_REVERSE);
            }
        }
        break;
    default:
    	{        	
    	}
    	break;
    }
}

CssManager* CssParser::getCssManager() const
{
    return m_cssManager;
}

}
