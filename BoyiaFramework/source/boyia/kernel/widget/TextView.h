/*
 * TextView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef TextView_h
#define TextView_h

#include "InlineView.h"

namespace yanbo {
class TextView : public InlineView {
public:
    TextView(const String& id, const String& text);
    ~TextView();

public:
    virtual void layout(RenderContext& rc);
    virtual LVoid paint(LGraphicsContext& gc);
    virtual LBool isText() const;

    void setAlignement(LGraphicsContext::TextAlign alignement);
    void setText(const String& text);
    const String& text() const { return m_text; }
    LInt calcTextLine(const String& text, LInt maxWidth);
    LBool isMultiLine();
    HtmlView* getLinkItem();
    LBool canDrawText() const;

private:
    class Line {
    public:
        Line(LInt length, const String& text);
        ~Line();
        LVoid paint(LGraphicsContext& gc, const LRect& rect);

    public:
        LInt m_lineLength;
        String m_text;
    };

protected:
    String m_text;
    KVector<Line*>* m_textLines;
    LFont* m_newFont;
    LInt m_maxWidth;

    LBool m_canDrawText;
};
}
#endif
