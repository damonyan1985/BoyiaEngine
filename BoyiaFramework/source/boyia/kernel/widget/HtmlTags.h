#ifndef HtmlTags_h
#define HtmlTags_h

//#include "IdentityMap.h"
#include "HashMap.h"
#include "HashUtil.h"

namespace yanbo {
class HtmlTags {
public:
    typedef enum HtmlType {
        TAGNONE = 0,
        HTML,
        HEAD,
        META,
        BR,
        UL,
        DIV,
        TR,
        TD,
        LI,
        P,
        H1,
        H2,
        H3,
        H4,
        STRONG,
        B,
        A,
        PRE,
        CENTER,
        FONT,
        IMG,
        I,
        EM,
        STYLE,
        SCRIPT,
        FORM,
        INPUT,
        SELECT,
        BUTTON,
        TEXTAREA,
        BODY,
        VIEW,
        TABLE,
        TITLE,
        OBJECT,
        OPTION,
        SPAN,
        LINK,
        BASE,
        GAME,
        VIDEO,
        TAGEND
    } HtmlType;

public:
    static HtmlTags* getInstance();
    static LVoid destroyInstance();

    LVoid defineInitialSymbol(const String& htmlText, LInt htmlType);
    ~HtmlTags();

    LInt symbolAsInt(String& key);

private:
    HtmlTags();

private:
    static HtmlTags* m_htmlTags;
    //IdentityMap m_map;
    HashMap<HashString, LInt> m_map;
};
}
#endif
