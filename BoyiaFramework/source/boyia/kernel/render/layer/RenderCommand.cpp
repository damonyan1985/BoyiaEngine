#include "RenderCommand.h"

namespace yanbo {
RenderCommand::RenderCommand(const LRect& commandRect, const LColor& commandColor)
    : rect(commandRect)
    , color(commandColor)
{
}

RenderCommand::~RenderCommand()
{
}

RenderRectCommand::RenderRectCommand(const LRect& rect, const LColor& color)
    : RenderCommand(rect, color)
{
}

LInt RenderRectCommand::type()
{
    return RenderCommand::kRenderRect;
}

RenderTextCommand::RenderTextCommand(
    const LRect& rect, const LColor& color,
    const LFont& commandFont, const String& commandText)
    : RenderCommand(rect, color)
    , font(commandFont)
    , text(commandText)
{
}

LInt RenderTextCommand::type()
{
    return RenderCommand::kRenderText;
}

RenderImageCommand::RenderImageCommand(const LRect& rect, const LColor& color, LVoid* commandImage)
    : RenderCommand(rect, color)
    , image(commandImage)
{
}

LInt RenderImageCommand::type()
{
    return RenderCommand::kRenderImage;
}

RenderVideoCommand::RenderVideoCommand(const LRect& rect, const LColor& color, LIntPtr id)
    : RenderCommand(rect, color)
    , playerId(id)
{
}

LInt RenderVideoCommand::type()
{
    return RenderCommand::kRenderVideo;
}

RenderPlatformCommand::RenderPlatformCommand(const LRect& rect, const LColor& color, LImage* commandImage)
    : RenderCommand(rect, color)
    , image(commandImage)
{
}

LInt RenderPlatformCommand::type()
{
    return RenderCommand::kRenderPlatform;
}

RenderRoundRectCommand::RenderRoundRectCommand(const LRect& rect, const LColor& color, LInt tlRadius, LInt trRadius, LInt brRadius, LInt blRadius)
    : RenderCommand(rect, color)
    , topLeftRadius(tlRadius)
    , topRightRadius(trRadius)
    , bottomRightRadius(brRadius)
    , bottomLeftRadius(blRadius)
{
}

LInt RenderRoundRectCommand::type()
{
    return kRenderRoundRect;
}
}
