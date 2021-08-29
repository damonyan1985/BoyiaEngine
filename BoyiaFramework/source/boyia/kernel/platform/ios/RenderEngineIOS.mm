//#include "IRenderEngine.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <Metal/Metal.h>

#import "IOSRenderer.h"
#include "RenderEngineIOS.h"
#include "ShaderType.h"
#include "PixelRatio.h"
#include "ShaderType.h"

// Need set xcode build settings Preprocessing
namespace yanbo {

#define METAL_COLOR_BIT(bit) ((float)bit/255)
#define METAL_COLOR(color) {\
    METAL_COLOR_BIT(color.m_red),\
    METAL_COLOR_BIT(color.m_green),\
    METAL_COLOR_BIT(color.m_blue),\
    METAL_COLOR_BIT(color.m_alpha)\
}

static LVoid screenToMetalPoint(
    int x,
    int y,
    float* metalX,
    float* metalY)
{
    *metalX = (1.0f * (x - PixelRatio::logicWidth() / 2)) / (PixelRatio::logicWidth() / 2);
    *metalY = ((1.0f * (PixelRatio::logicHeight() / 2 - y)) / (PixelRatio::logicHeight() / 2));
}

// TODO
RenderEngineIOS::RenderEngineIOS()
    : m_renderer(kBoyiaNull)
{
}

RenderEngineIOS::~RenderEngineIOS()
{
    
}
LVoid RenderEngineIOS::init()
{
}
LVoid RenderEngineIOS::reset()
{
    
}
LVoid RenderEngineIOS::render(RenderLayer* layer)
{
        
}

LVoid RenderEngineIOS::setContextIOS(IOSRenderer* renderer)
{
    m_renderer = renderer;
}

LVoid RenderEngineIOS::renderRect(RenderCommand* cmd)
{
    LRect& rect = cmd->rect;
    LColor& color = cmd->color;
    
    float x, y;
    VertexAttributes attr[6];
    
    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr[0].aPosition = {x, y, 0, 1};
    attr[0].aColor = METAL_COLOR(color);
    
    // left, bottom
    screenToMetalPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &x, &y);
    attr[1].aPosition = {x, y, 0, 1};
    attr[1].aColor = METAL_COLOR(color);
    
    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr[2].aPosition = {x, y, 0, 1};
    attr[2].aColor = METAL_COLOR(color);
    
    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr[3].aPosition = {x, y, 0, 1};
    attr[3].aColor = METAL_COLOR(color);
    
    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr[4].aPosition = {x, y, 0, 1};
    attr[4].aColor = METAL_COLOR(color);
    
    // right, top
    screenToMetalPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &x, &y);
    attr[5].aPosition = {x, y, 0, 1};
    attr[5].aColor = METAL_COLOR(color);
    
   
    [m_renderer setBuffer:&attr size:sizeof(attr)];
}

LVoid RenderEngineIOS::renderImage(RenderCommand* cmd)
{
    
}

LVoid RenderEngineIOS::renderText(RenderCommand* cmd)
{
    
}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineIOS();
}


}
