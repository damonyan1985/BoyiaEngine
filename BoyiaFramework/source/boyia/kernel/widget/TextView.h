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
class TextLine;
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

    LInt lineSize() const;
    ViewPainter* linePainter(LInt i) const;

protected:
    String m_text;
    KVector<TextLine*>* m_textLines;
    LFont* m_newFont;
    LInt m_maxWidth;
};
}
#endif
