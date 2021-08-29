//#include "IRenderEngine.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <Metal/Metal.h>

#import "IOSRenderer.h"
#include "RenderEngineIOS.h"
#include "ShaderType.h"
#include "PixelRatio.h"


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
    , m_vertexs(0, 256)
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
    
    
    VertexAttributes attr;
    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);

    // left, bottom
    screenToMetalPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);

    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);

    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);

    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);

    // right, top
    screenToMetalPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);
    
    
#if 0
    // left, bottom
    screenToMetalPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);

    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);
    
    // right, top
    screenToMetalPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);
    
    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    m_vertexs.addElement(attr);
#endif
}

LVoid RenderEngineIOS::setBuffer()
{
    NSLog(@"setBuffer() size=%d", m_vertexs.size());
    [m_renderer setBuffer:m_vertexs.getBuffer() size:(m_vertexs.size() * sizeof(VertexAttributes))];
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
