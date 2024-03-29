#ifndef RenderCommand_h
#define RenderCommand_h

#include "LGdi.h"
#include "UtilString.h"

namespace yanbo {
class RenderCommand : public BoyiaRef {
public:
    enum CommandType {
        kRenderRect = 0,
        kRenderRoundRect,
        kRenderText,
        kRenderImage,
        kRenderRoundImage,
        kRenderVideo,
        kRenderPlatform
    };

    RenderCommand(const LRect& rect, const LColor& color);
    virtual ~RenderCommand();

    virtual LInt type() = 0;

    LRect rect;
    LColor color;
};

class RenderTextCommand : public RenderCommand {
public:
    RenderTextCommand(const LRect& rect, const LColor& color, const LFont& font, const String& text);
    virtual LInt type();

    LFont font;
    String text;
    // 裁剪范围
    LRect clipRect;
};

class RenderRectCommand : public RenderCommand {
public:
    RenderRectCommand(const LRect& rect, const LColor& color);

    virtual LInt type();
};

class RenderImageCommand : public RenderCommand {
public:
    RenderImageCommand(const LRect& rect, const LColor& color, LVoid* image);
    virtual LInt type();

    String url;
    LVoid* image;
    // 裁剪范围
    LRect clipRect;
};

class RenderVideoCommand : public RenderCommand {
public:
    RenderVideoCommand(const LRect& rect, const LColor& color, LIntPtr playerId);
    virtual LInt type();

    LIntPtr playerId;
};

class RenderPlatformCommand : public RenderCommand {
public:
    RenderPlatformCommand(const LRect& rect, const LColor& color, LImage* image);
    virtual LInt type();

    // 加载外界纹理
    LImage* image;
    // 裁剪范围
    LRect clipRect;
};

class RenderRoundRectCommand : public RenderCommand {
public:
    RenderRoundRectCommand(const LRect& rect, const LColor& color, LInt topLeftRadius, LInt topRightRadius, LInt bottomRightRadius, LInt bottomLeftRadius);
    virtual LInt type();
    
    LInt topLeftRadius;
    LInt topRightRadius;
    LInt bottomRightRadius;
    LInt bottomLeftRadius;
};

class RenderRoundImageCommand : public RenderRoundRectCommand {
public:
    RenderRoundImageCommand(
        const LRect& rect,
        const LColor& color,
        LVoid* image,
        LInt topLeftRadius,
        LInt topRightRadius,
        LInt bottomRightRadius,
        LInt bottomLeftRadius);
    
    virtual LInt type();
    
    String url;
    
    LVoid* image;
    // 裁剪范围
    LRect clipRect;
};
}
#endif
