/*
 * CssRule.h
 *
 * Created on: 2011-6-23
 * Author    : yanbo
 */

#ifndef CssRule_h
#define CssRule_h

#include "UtilString.h"
#include "KListMap.h"
#include "KVector.h"
#include "Style.h"
#include "KRef.h"

namespace util
{
class SimpleSelector 
{
public:
    enum Match 
	{
	    None = 0,
	    Id,
	    Class,
	    PseudoClass,
	    Tag
	};

	enum Relation 
	{
	    Descendant = 0, // div a
        Child,          // div > a
		DirectAdjacent, // div + p
		IndirectAdjacent, // div ~ p
		SubSelector,
		ShadowDescendant
	};

	enum PseudoType
	{
		PseudoNotParsed = 0,
        PseudoLink,
	    PseudoVisited,
		PseudoAny,
		PseudoAnyLink,
		PseudoHover,
		PseudoDrag,
		PseudoFocus,
        PseudoActive,
	};

public:
	SimpleSelector();
	~SimpleSelector();

public:
	void setRelation(LInt8 relation);
	LInt8 getRelation() const;

	void setSelectorText(const String& selectorText);
	const String& getSelectorText() const;

	void setPseudoType(LInt8 pseudoType);
	LInt8 getPseudoType() const;

	void setMatch(LInt8 match);
	LInt8 getMatch() const;

private:
	String m_selectorText;
	LInt8 m_relation;
	LInt8 m_pseudoType;
	LInt8 m_match;
};

typedef String                                PropertyName;
typedef String                                PropertyValue;
typedef KVector<SimpleSelector*>               Selector;
typedef KVector<Selector*>                     SelectorGroup;

class CssPropertyValue
{
public:
	struct PropertySpecificity
	{
	public:
		PropertySpecificity()
			: m_important(0)
			, m_id(0)
			, m_classOrPseudo(0)
			, m_tag(0)
		{
		}

		LUint8 m_important;
		LUint8 m_id;
		LUint8 m_classOrPseudo;
		LUint8 m_tag;
	};

public:
   CssPropertyValue();
   ~CssPropertyValue();
   
   CssPropertyValue(const CssPropertyValue& proValue);
   const CssPropertyValue& operator = (const CssPropertyValue& proValue);

   void computeSpecificity(const Selector* selector);
   void resetSpecificity();
   void setSpecificity(const PropertySpecificity& specificity);
   const PropertySpecificity& getSpecificity() const;

   static void computeSpecificity(const Selector* selector, PropertySpecificity& specificity);
   static LBool compareSpecificity(const PropertySpecificity& specificity1,  const PropertySpecificity& specificity2);

public:
   LInt                    intVal;
   String                 strVal;
   PropertySpecificity     m_specificity;
};

typedef KListMap<LUint, PropertyValue>             PropertyMap;
typedef KListMap<LInt, CssPropertyValue>           AttributeMap;

class CssRule : public KRef
{
public:
	static CssRule* New();
	static CssRule* New(CssRule& rule);
	~CssRule();
	
public:
	void addProperty(LInt property, LInt value);
	void addProperty(LInt property, const String& value);
	
	CssPropertyValue& getPropertyValue(LInt property);
	
	void copyPropertiesFrom(const CssRule* rule);
	void createStyle(Style& style);
	
	const AttributeMap& getProperties() const;
	const AttributeMap* getPropertiesPtr() const;

	void setSelectorGroup(SelectorGroup* group);
	
	SelectorGroup* getSelectorGroup() const;
	
	LBool isPropertyEmpty();
	
private:
	CssRule();
	void construct();
	void construct(CssRule& rule);
	void setStyleProperties(Style& style, LInt property,
			const CssPropertyValue& value);

private:
	AttributeMap                   m_properties; // properties
	SelectorGroup                  *m_selectorGroup; // selector
};

}
#endif /* CSSRULE_H_ */
