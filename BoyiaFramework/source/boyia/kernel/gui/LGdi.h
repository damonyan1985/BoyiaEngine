/*
============================================================================
 Name        : lgdi.h
 Author      : yanbo
 Version     :
 Copyright   : Your copyright notice
 Description : lgdi.h - header file
============================================================================
*/

#ifndef LGdi_h
#define LGdi_h

//#include "BoyiaPtr.h"
#include "WeakPtr.h"
#include "LGraphic.h"
#include "UtilString.h"

// software abstraction layer API
// graphic abstract class
namespace util {

class LImage;
class LFont;
class LColor;
class LMediaPlayer;
class ViewPainter {
public:
    ViewPainter()
        : m_painter(kBoyiaNull)
    {
    }

    virtual ~ViewPainter()
    {
    }
    
    virtual LBool canDraw() const
    {
        return LTrue;
    }

    LVoid setPainter(BoyiaRef* painter)
    {
        m_painter = painter;
    }

    LVoid* painter() const
    {
        return m_painter.get();
    }

protected:
    BoyiaPtr<BoyiaRef> m_painter;
};

class LGraphicsContext {
public:
    enum BrushStyle {
        kNullBrush = 1,
        kSolidBrush
    };

    enum PenStyle {
        kNullPen = 1,
        kSolidPen,
        kDotPen
    };

    enum TextAlign {
        kTextLeft = 1,
        kTextCenter,
        kTextRight
    };

public:
#if ENABLE(BOYIA_KERNEL)
    static LGraphicsContext* create()
    {
        return kBoyiaNull;
    }
#else
    static LGraphicsContext* create();
#endif
    virtual ~LGraphicsContext()
    {
    }

public:
    virtual LVoid drawLine(const LPoint& p1, const LPoint& p2) = 0;
    virtual LVoid drawLine(LInt x0, LInt y0, LInt x1, LInt y1) = 0;
    virtual LVoid drawRect(const LRect& rect) = 0;
    virtual LVoid drawRect(LInt x, LInt y, LInt w, LInt h) = 0;

    virtual LVoid drawEllipse(const LRect& rect) = 0;

    // 弧形角的方框
    virtual LVoid drawRoundRect(const LRect& rect, const LSize& cornerSize) = 0;
    
    virtual LVoid drawRoundRect(const LRect& aRect, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius) {}

    virtual LVoid drawText(const String& text, const LRect& rect) = 0;
    virtual LVoid drawText(const String& text, const LRect& rect, TextAlign align) = 0;
    virtual LVoid drawText(const String& text, const LPoint& p) = 0;

    virtual LVoid drawImage(const LPoint& tl, const LImage* image) = 0;
    virtual LVoid drawImage(const LImage* image) = 0;
    virtual LVoid drawImage(const LRect& rect, const LImage* image, const LRect& sRect) = 0;
    virtual LVoid drawRoundImage(const LImage* image, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius) {}

    virtual LVoid drawVideo(const LRect& rect, const LMediaPlayer* mp) = 0;

#if ENABLE(BOYIA_PLATFORM_VIEW)
    virtual LVoid drawPlatform(const LRect& rect, LVoid* platformView) = 0;
#endif

    virtual LVoid setBrushStyle(BrushStyle style) = 0;
    virtual LVoid setPenStyle(PenStyle style) = 0;

    virtual LVoid setBrushColor(const LColor& color) = 0;
    virtual LVoid setPenColor(const LColor& color) = 0;
    virtual LVoid setFont(const LFont& font) = 0;

    virtual LVoid submit() = 0;
    virtual LVoid reset() = 0;
    virtual LVoid setHtmlView(ViewPainter* item) = 0;
    virtual LVoid save() = 0;
    virtual LVoid clipRect(const LRect& rect) = 0;
    virtual LVoid restore() = 0;
};

class LColor {
public:
    LColor()
        : m_red(0)
        , m_green(0)
        , m_blue(0)
        , m_alpha(255)
    {
    }

    LColor(LUint8 aRed, LUint8 aGreen, LUint8 aBlue)
        : m_red(aRed)
        , m_green(aGreen)
        , m_blue(aBlue)
        , m_alpha(255)
    {
    }

    LColor(LUint8 aRed, LUint8 aGreen, LUint8 aBlue, LUint8 alpha)
        : m_red(aRed)
        , m_green(aGreen)
        , m_blue(aBlue)
        , m_alpha(alpha)
    {
    }

    LColor(const LColor& rgb)
    {
        set(rgb);
    }

    ~LColor() { }

public:
    LVoid set(LUint8 aRed, LUint8 aGreen, LUint8 aBlue, LUint8 alpha)
    {
        m_red = aRed;
        m_green = aGreen;
        m_blue = aBlue;
        m_alpha = alpha;
    }

    LColor& operator=(const LColor& rgb)
    {
        set(rgb);
        return *this;
    }

    LVoid set(const LColor& rgb)
    {
        m_red = rgb.m_red;
        m_green = rgb.m_green;
        m_blue = rgb.m_blue;
        m_alpha = rgb.m_alpha;
    }

public:
    LUint8 m_red;
    LUint8 m_green;
    LUint8 m_blue;
    LUint8 m_alpha;
};

class LImage : public BoyiaRef {
public:
    LImage()
        : m_loaded(LFalse)
        , m_width(0)
        , m_height(0)
    {
    }
    virtual ~LImage() { }

#if ENABLE(BOYIA_KERNEL)
    static LImage* create(LVoid* item)
    {
        return kBoyiaNull;
    }
#else
    static LImage* create(LVoid* item);
#endif

public:
    virtual LVoid load(const String& path, LVoid* image) { }
    virtual LVoid create(const String& aFileName) { }

    virtual LVoid setLoaded(LBool loaded) { m_loaded = loaded; }
    LBool isLoaded() { return m_loaded; }

    void setRect(const LRect& rect) { m_rect = rect; }
    const LRect& rect() const { return m_rect; }

    virtual LVoid* item() const = 0;
    virtual LVoid* pixels() const = 0;
    virtual LInt width() const { return m_width; }
    virtual LInt height() const { return m_height; }
    virtual const String& url() const = 0;

protected:
    LBool m_loaded;
    // 要绘制的尺寸和坐标
    LRect m_rect;
    // 图片的实际尺寸
    LInt m_width;
    LInt m_height;
};

class LFont : public BoyiaRef {
public:
    typedef enum FontStyle {
        FONT_STYLE_NORMAL = 0,
        FONT_STYLE_ITALIC,
        FONT_STYLE_BOLD,
        FONT_STYLE_UNDERLINE,
    } FontStyle;

public:
#if ENABLE(BOYIA_KERNEL)
    static LFont* create(const LFont& font)
    {
        return kBoyiaNull;
    }
#else
    static LFont* create(const LFont& font); // instance a platform font
#endif
    LFont()
        : m_fontStyle(FONT_STYLE_NORMAL)
        , m_size(60)
    {
    }
    LFont(const LFont& font)
        : m_fontStyle(font.m_fontStyle)
        , m_size(font.m_size)
        , m_family(font.m_family)
    {
    }
    virtual ~LFont() { }

public:
    virtual LInt getFontHeight() const
    {
        return m_size > 0 ? m_size : 12;
    }

    virtual LInt getFontWidth(LUint16 ch) const
    {
        return 12;
    }

    virtual LInt getTextWidth(const String& text) const
    {
        return text.GetLength() * getFontAdapter();
    }

    virtual void setFontSize(LInt size)
    {
        m_size = size;
    }

    LInt getFontSize() const
    {
        return m_size;
    }

    virtual void setFontStyle(FontStyle style)
    {
        m_fontStyle = style;
    }
    
    virtual void setFamily(const String& family)
    {
        m_family = family;
    }
    
    const String& getFamily() const
    {
        return m_family;
    }

    FontStyle getFontStyle() const
    {
        return m_fontStyle;
    }

    virtual LInt getLineSize() const
    {
        return 0;
    }

    virtual LInt getLineWidth(LInt index) const
    {
        return 0;
    }

    virtual LVoid getLineText(LInt index, String& text)
    {
    }

    virtual LInt calcTextLine(const String& text, LInt maxWidth) const
    {
        return 0;
    }
    
    virtual LInt getIndexByOffset(LInt line, LInt x)
    {
        return 0;
    }

    virtual LInt getOffsetByIndex(LInt line, LInt x)
    {
        return 0;
    }
    
private:
    // 主要用来做字体宽高适配
    LInt getFontAdapter() const
    {
        return (m_size * 2 / 3);
    }

protected:
    String m_family;
    FontStyle m_fontStyle;
    LInt m_size;
};

class LMediaPlayer {
public:
#if ENABLE(BOYIA_KERNEL)
    static LMediaPlayer* create(LVoid* view)
    {
        return kBoyiaNull;
    }
#else
    static LMediaPlayer* create(LVoid* view); // instance a platform mediaplayer
#endif
    virtual ~LMediaPlayer()
    {
    }
    virtual LVoid start(const String& url) = 0;
    virtual LVoid seek(LInt progress) = 0;
    virtual LVoid pause() = 0;
    virtual LVoid stop() = 0;
    
    virtual LVoid setPlayerId(LIntPtr id)
    {
        m_id = id;
    }
    
    virtual LIntPtr playerId() const
    {
        return m_id;
    }
    
private:
    LIntPtr m_id = 0;
};

class Editor {
public:
#if ENABLE(BOYIA_KERNEL)
    static Editor* get()
    {
        return kBoyiaNull;
    }
#else
    static Editor* get(); // instance a platform Editor
#endif
    Editor()
        : m_view(kBoyiaNull)
    {
    }

    virtual ~Editor()
    {
    }
    
    virtual LVoid showKeyboard(const String& text, LInt cursor) = 0;

    Editor* setView(BoyiaRef* view)
    {
        m_view = view;
        return this;
    }

    BoyiaRef* view() const
    {
        return m_view.get();
    }

    LVoid removeView(BoyiaRef* view)
    {
        if (m_view.get() == view) {
            m_view = kBoyiaNull;
        }
    }
//    Editor* setView(LVoid* view)
//    {
//        m_view = view;
//        return this;
//    }
//
//    LVoid* view() const
//    {
//        return m_view;
//    }
//
//    LVoid removeView(LVoid* view)
//    {
//        if (m_view == view) {
//            m_view = kBoyiaNull;
//        }
//    }

private:
    WeakPtr<BoyiaRef> m_view;
    //LVoid* m_view;
};
}

using util::Editor;
using util::LColor;
using util::LFont;
using util::LGraphicsContext;
using util::LImage;
using util::LMediaPlayer;
using util::ViewPainter;

#endif // LGDI_H

// End of File
