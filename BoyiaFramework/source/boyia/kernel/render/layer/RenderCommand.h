#ifndef RenderCommand_h
#define RenderCommand_h

#include "LGdi.h"
#include "UtilString.h"

namespace yanbo {
class RenderCommand : public BoyiaRef {
public:
    enum CommandType {
        kRenderRect,
        kRenderText,
        kRenderImage,
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
};

class RenderRectCommand : public RenderCommand {
public:
    RenderRectCommand(const LRect& rect, const LColor& color);

    virtual LInt type();
};

class RenderImageCommand : public RenderCommand {
public:
    RenderImageCommand(const LRect& rect, const LColor& color, LImage* image);
    virtual LInt type();

    LImage* image;
};

class RenderVideoCommand : public RenderCommand {
public:
    RenderVideoCommand(const LRect& rect, const LColor& color, LMediaPlayer* player);
    virtual LInt type();

    LMediaPlayer* mediaPlayer;
};

class RenderPlatformCommand : public RenderCommand {
public:
    RenderPlatformCommand(const LRect& rect, const LColor& color, LImage* image);
    virtual LInt type();

    // 加载外界纹理
    LImage* image;
};
}
#endif
