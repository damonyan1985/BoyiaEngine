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

private:
    mutable KVector<OwnerPtr<LineText>> m_lines;
    mutable LInt m_height;
    UIFont* m_font;
};

FontIOS::FontIOS(const LFont& font)
    : LFont(font)
    , m_height(0)
{
    m_font = [UIFont fontWithName:@"HoeflerText-Regular" size:font.getFontSize()];
}

FontIOS::~FontIOS()
{
}

LInt FontIOS::getFontHeight() const
{
    return (LInt)m_font.lineHeight;
}

LInt FontIOS::getFontWidth(LUint8 ch) const
{
    return 0;
}

LInt FontIOS::getTextWidth(const String& text) const
{
    return 0;
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

LInt FontIOS::calcTextLine(const String& text, LInt maxWidth) const
{
    // TODO
    // String换NSString
    NSString* nsText = [[NSString alloc] initWithUTF8String:GET_STR(text)];
    
    NSDictionary *dict = @{NSFontAttributeName:m_font};

    //设置文本能占用的最大宽高
    CGSize maxSize = CGSizeMake(maxWidth, MAXFLOAT);
    CGRect rect =  [nsText boundingRectWithSize:maxSize options:NSStringDrawingUsesLineFragmentOrigin attributes:dict context:nil];

    return 0;
}

LFont* LFont::create(const LFont& font)
{
    return new FontIOS(font);
}
}

@implementation FontImplIOS


@end
