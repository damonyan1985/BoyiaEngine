#ifndef RenderContext_h
#define RenderContext_h

#include "PlatformLib.h"
#include "Style.h"
#include "HtmlView.h"

namespace yanbo
{

// render context
class RenderContext
{
public:
    RenderContext();
    ~RenderContext();
    
    void newLine();
    void newLine(HtmlView* item);
    
    LInt getMaxWidth();
    void setMaxWidth(LInt maxWidth);
    
    LInt getMaxHeight();
    void setMaxHeight(LInt maxHeight);
    
    LInt getX();
    void setX(LInt x);
    
    void addX(LInt x);
    void addY(LInt y);
    
    LInt getY();
    void setY(LInt y);
    
    LInt getLineNumber();
    void setLineNumber(LInt lineNumber);
    
    void incLineNumber();
    
//    const util::Style& getStyle() const;
//    void setStyle(const util::Style& style);
    
    LBool getCenter() const;
    void setCenter(LBool center);
    
    void setNextLineHeight(LInt h);
    LInt getNextLineHeight();
    void addItemInterval();
    
    void addLineItemCount();
    void resetLineItemCount();
    LInt getLineItemCount();
    
    void setNewLineXStart(LInt x);
    LInt getNewLineXStart();

    void addLineItem(HtmlView* item);
    void relayoutLineItems();
   
private:
    LInt               m_maxWidth;       // page width
    LInt               m_maxHeight;      // page height
    LInt               m_x;              // current x
    LInt               m_y;              // current y
    LInt               m_lineNumber;     // current line number
    LInt               m_nextLineHeight; // current HtmlView height
    LBool              m_center;
    LInt               m_itemCountOfline;
    LInt               m_newLineXStart;

    HtmlViewList       m_lineItems;
};

}
#endif
