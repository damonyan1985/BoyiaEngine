#include "LGdi.h"
#include "AppManager.h"
#include "PixelRatio.h"
#import <UIKit/UIKit.h>


@interface FontImplIOS : NSObject
@end

namespace util {
// TODO
class LineText {
public:
    LineText(OwnerPtr<String> ptr, LInt tw)
        : width(tw)
        , text(ptr)
    {
    }

    OwnerPtr<String> text;
    LInt width;
};

class FontIOS : public LFont {
public:
    FontIOS(const LFont& font);
    virtual ~FontIOS();

    virtual LInt getFontHeight() const;
    virtual LInt getFontWidth(LUint8 ch) const;
    virtual LInt getTextWidth(const String& text) const;
    virtual LInt calcTextLine(const String& text, LInt maxWidth) const;
    virtual LInt getLineSize() const;
    virtual LInt getLineWidth(LInt index) const;
    virtual LVoid getLineText(LInt index, String& text);
    
    virtual LInt getIndexByOffset(LInt line, LInt x);
    virtual LInt getOffsetByIndex(LInt line, LInt index);

private:
    UIFont* getUIFont() const;
    
    mutable KVector<OwnerPtr<LineText>> m_lines;
    mutable LInt m_height;
};

FontIOS::FontIOS(const LFont& font)
    : LFont(font)
    , m_height(0)
{
}

FontIOS::~FontIOS()
{
}

LInt FontIOS::getFontHeight() const
{
    //return (LInt)m_font.lineHeight;
    return m_height;
}

LInt FontIOS::getFontWidth(LUint8 ch) const
{
    return 0;
}

LInt FontIOS::getTextWidth(const String& text) const
{
    NSString* nsText = [[NSMutableString alloc] initWithUTF8String:GET_STR(text)];
    NSDictionary* dict = @{NSFontAttributeName:getUIFont()};
    //设置文本能占用的最大宽高
    CGSize maxSize = CGSizeMake(CGFLOAT_MAX, MAXFLOAT);
    
    CGRect rect =  [nsText boundingRectWithSize:maxSize
                                             options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                          attributes:dict context:nil];
    
    return rect.size.width;
}

LInt FontIOS::getLineSize() const
{
    return m_lines.size();
}

LInt FontIOS::getLineWidth(LInt index) const
{
    return m_lines.elementAt(index)->width;
}

LVoid FontIOS::getLineText(LInt index, String& text)
{
    text = *m_lines.elementAt(index)->text.get();
}

UIFont* FontIOS::getUIFont() const
{
    if (m_family.GetLength()) {
        return [UIFont fontWithName:[[NSString alloc] initWithUTF8String:GET_STR(m_family)] size:getFontSize()];
    }
    
    // 如果没有指定字体，则使用系统字体
    return [UIFont systemFontOfSize:getFontSize()];
}

LInt FontIOS::calcTextLine(const String& text, LInt maxWidth) const
{
//    if (m_family.GetLength()) {
//        font = [UIFont fontWithName:[[NSString alloc] initWithUTF8String:GET_STR(m_family)] size:getFontSize()];
//    } else {
//        // 如果没有指定字体，则使用系统字体
//        font = [UIFont systemFontOfSize:getFontSize()];
//    }
    
    //UIFont* font = [UIFont fontWithName:@"HoeflerText-Regular" size:getFontSize()];
//    float radio = yanbo::PixelRatio::ratio();
//    maxWidth = maxWidth * radio;
    // String换NSString
    NSString* nsText = [[NSMutableString alloc] initWithUTF8String:GET_STR(text)];
    
    NSDictionary* dict = @{NSFontAttributeName:getUIFont()};
    //设置文本能占用的最大宽高
    CGSize maxSize = CGSizeMake(CGFLOAT_MAX, MAXFLOAT);
    

    LInt currentLineWidth = 0;
    LInt maxLineWidth = 0;
    
    LInt start = 0;
    LInt end = 0;
    LInt i = 0;
    while (i < nsText.length) {
        end = i+1;
        NSString* rangeString = [nsText substringWithRange:NSMakeRange(start, end - start)];
        CGRect rect =  [rangeString boundingRectWithSize:maxSize
                                                 options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                              attributes:dict context:nil];
        
        if (rect.size.width <= maxWidth) {
            currentLineWidth = rect.size.width;
            i++;
        } else {
            maxLineWidth = maxLineWidth < currentLineWidth ?
                currentLineWidth : maxLineWidth;
            
            NSString* target = [nsText substringWithRange:NSMakeRange(start, end - start - 1)];
            //const char* buffer = [target UTF8String];
            NSData* nsData = [target dataUsingEncoding:NSUTF8StringEncoding];
            //OwnerPtr<String> lineText = new String((const LUint8*)buffer, (LInt)strlen(buffer));
            OwnerPtr<String> lineText = new String(_CS(nsData.bytes), (LInt)nsData.length);
            //yanbo::CharConvertor::WcharToChar(wstr.GetBuffer(), *lineText.get());
            m_lines.addElement(new LineText(lineText, currentLineWidth));
            currentLineWidth = 0;
            
            start = i;
        }

        m_height = m_height < rect.size.height ? rect.size.height : m_height;
    }

    if (currentLineWidth > 0) {
        maxLineWidth = maxLineWidth < currentLineWidth ?
            currentLineWidth : maxLineWidth;

        NSString* target = [nsText substringWithRange:NSMakeRange(start, end - start)];
        NSData* nsData = [target dataUsingEncoding:NSUTF8StringEncoding];
        
        OwnerPtr<String> lineText = new String((const LUint8*)nsData.bytes, (LInt)nsData.length);
        m_lines.addElement(new LineText(lineText, currentLineWidth));
    }

    return maxLineWidth;
}

// 根据位置定位所在字符串中的索引
// x为相对textview的位移
LInt FontIOS::getIndexByOffset(LInt line, LInt x)
{
    if (!m_lines.size()) {
        return 0;
    }
    
    NSString* nsText = [[NSMutableString alloc] initWithUTF8String:GET_STR((*m_lines[line]->text.get()))];
    CGSize maxSize = CGSizeMake(CGFLOAT_MAX, MAXFLOAT);
    
    NSDictionary* dict = @{NSFontAttributeName:getUIFont()};
    
    LInt i = 0;
    while (i < nsText.length) {
        NSString* rangeString = [nsText substringWithRange:NSMakeRange(0, i+1)];
        CGRect rect =  [rangeString boundingRectWithSize:maxSize
                                                 options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                              attributes:dict context:nil];
        
        if (x < rect.size.width) {
            return i;
        }
        
        i++;
    }
    
    return i;
}

LInt FontIOS::getOffsetByIndex(LInt line, LInt index)
{
    if (!m_lines.size()) {
        return 0;
    }
    
    NSString* nsText = [[NSMutableString alloc] initWithUTF8String:GET_STR((*m_lines[line]->text.get()))];
    CGSize maxSize = CGSizeMake(CGFLOAT_MAX, MAXFLOAT);
    
    NSDictionary* dict = @{NSFontAttributeName:getUIFont()};
    
    NSString* rangeString = [nsText substringWithRange:NSMakeRange(0, index)];
    CGRect rect =  [rangeString boundingRectWithSize:maxSize
                                             options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                          attributes:dict context:nil];
    
    return rect.size.width;
}

LFont* LFont::create(const LFont& font)
{
    return new FontIOS(font);
}
}

@implementation FontImplIOS


@end
