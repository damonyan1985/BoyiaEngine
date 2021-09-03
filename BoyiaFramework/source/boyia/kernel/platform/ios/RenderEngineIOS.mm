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
    RenderImageCommand* imageCmd = static_cast<RenderImageCommand*>(cmd);
    // 从普通指针转为OC ARC指针，使用完之后释放
    UIImage* image = (__bridge_transfer UIImage*)imageCmd->image;
    
}

LVoid RenderEngineIOS::renderText(RenderCommand* cmd)
{
    //m_vertexs.clear();
    RenderTextCommand* textCmd = static_cast<RenderTextCommand*>(cmd);
    LRect& rect = textCmd->rect;
    LColor& color = textCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    float x, y;
    
    
    VertexAttributes attr;
    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 1, 1 };
    m_vertexs.addElement(attr);

    // left, bottom
    screenToMetalPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 0, 1 };
    m_vertexs.addElement(attr);

    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 0, 0 };
    m_vertexs.addElement(attr);

    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 1, 1 };
    m_vertexs.addElement(attr);

    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 0, 0 };
    m_vertexs.addElement(attr);

    // right, top
    screenToMetalPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 1, 0 };
    m_vertexs.addElement(attr);
    
    
    
    //NSString* nsText = [[NSString alloc] initWithUTF8String: GET_STR(textCmd->text)];
    //UIFont* font = [UIFont fontWithName:nsText size:textCmd->font.getFontSize()];
    UIFont* font = [UIFont systemFontOfSize:textCmd->font.getFontSize()];
    int width = textCmd->rect.GetWidth();
    int height = textCmd->rect.GetHeight();
    Byte* data = (Byte*)malloc(width * height * 4); // rgba共4个byte
    memset(data, 255, width * height * 4);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width*4,
                                                       colorSpace,
                                                       kCGImageAlphaPremultipliedLast);
    // push context之后才能绘制文字
    UIGraphicsPushContext(context);
    
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1.0, -1.0);
    CGContextSetShouldAntialias(context, YES);
    
    // 转成OC字符串
    NSString* text = [[NSString alloc] initWithUTF8String: GET_STR(textCmd->text)];
    // 文字颜色
    UIColor* uiColor = [UIColor colorWithRed:METAL_COLOR_BIT(textCmd->color.m_red)
                                    green:METAL_COLOR_BIT(textCmd->color.m_green)
                                     blue:METAL_COLOR_BIT(textCmd->color.m_blue)
                                    alpha:METAL_COLOR_BIT(textCmd->color.m_alpha)];
    // 设置文字绘制属性
    NSMutableDictionary* textAttributes = [NSMutableDictionary new];
    [textAttributes setValue:font forKey:NSFontAttributeName];
    [textAttributes setValue:[UIColor blackColor] forKey:NSForegroundColorAttributeName];
    
    // 开始绘制文字
    [text drawInRect:CGRectMake(0, 0, width, height)
      withAttributes:textAttributes];

    UIGraphicsPopContext();
    
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    [m_renderer setTextureData:text data:data width:width height:height];
    
    // 释放data
    if (data) {
        free(data);
    }
}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineIOS();
}


}
