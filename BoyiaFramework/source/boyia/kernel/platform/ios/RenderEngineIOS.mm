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

static LVoid createVertexAttr(const LRect& rect, const LColor& color, KVector<VertexAttributes>& vertexs)
{
    VertexAttributes attr;
    float x, y;
    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 1, 1 };
    vertexs.addElement(attr);

    // left, bottom
    screenToMetalPoint(rect.iTopLeft.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 0, 1 };
    vertexs.addElement(attr);

    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 0, 0 };
    vertexs.addElement(attr);

    // right, bottom
    screenToMetalPoint(rect.iBottomRight.iX, rect.iBottomRight.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 1, 1 };
    vertexs.addElement(attr);

    // left, top
    screenToMetalPoint(rect.iTopLeft.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 0, 0 };
    vertexs.addElement(attr);

    // right, top
    screenToMetalPoint(rect.iBottomRight.iX, rect.iTopLeft.iY, &x, &y);
    attr.aPosition = {x, y, 0, 1};
    attr.aColor = METAL_COLOR(color);
    attr.aTexCoord = { 1, 0 };
    vertexs.addElement(attr);
}

// TODO
RenderEngineIOS::RenderEngineIOS()
    : m_renderer(kBoyiaNull)
    , m_vertexs(0, 256)
{
    init();
}

RenderEngineIOS::~RenderEngineIOS()
{
    
}
LVoid RenderEngineIOS::init()
{
    // 初始化渲染函数
    m_functions[RenderCommand::kRenderRect] = (RenderFunction)&RenderEngineIOS::renderRect;
    m_functions[RenderCommand::kRenderText] = (RenderFunction)&RenderEngineIOS::renderText;
    m_functions[RenderCommand::kRenderImage] = (RenderFunction)&RenderEngineIOS::renderImage;
}

LVoid RenderEngineIOS::reset()
{
    
}

LVoid RenderEngineIOS::render(RenderLayer* layer)
{
    // 清空缓冲区
    m_vertexs.clear();
    // 开始渲染
//    renderImpl(layer);
//    // 刷新缓冲区
//    setBuffer();
//
//    [m_renderer render];
}

LVoid RenderEngineIOS::renderImpl(RenderLayer* layer)
{
    if (!layer) {
        return;
    }

    // 先渲染当前layer
    if (layer->m_buffer) {
        LInt commandSize = layer->m_buffer->size();
        for (LInt i = 0; i < commandSize; i++) {
            RenderCommand* cmd = layer->m_buffer->elementAt(i).get();
            (this->*(m_functions[cmd->type()]))(cmd);
        }
    }
    
    // 再渲染子layer
    for (LInt i = 0; i < layer->m_children.size(); i++) {
        renderImpl(layer->m_children[i]);
    }
}

LVoid RenderEngineIOS::setContextIOS(IOSRenderer* renderer)
{
    m_renderer = renderer;
}

LVoid RenderEngineIOS::renderRect(RenderCommand* cmd)
{
    LRect& rect = cmd->rect;
    LColor& color = cmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    createVertexAttr(rect, color, m_vertexs);
    [m_renderer appendBatchCommand:BatchCommandNormal size:6 key:nil];
    
    
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
    RenderTextCommand* textCmd = static_cast<RenderTextCommand*>(cmd);
    LRect& rect = textCmd->rect;
    LColor& color = textCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    createVertexAttr(rect, color, m_vertexs);
    
    // 从普通指针转为OC ARC指针，使用完之后释放
    UIImage* image = (__bridge_transfer UIImage*)imageCmd->image;
    CGImageRef imageRef = image.CGImage;
    
    // 读取图片的宽高
    size_t width = CGImageGetWidth(imageRef);
    size_t height = CGImageGetHeight(imageRef);
    
    Byte* data = (Byte*) calloc(width * height * 4, sizeof(Byte)); //rgba共4个byte
    
    CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width*4,
                                                       CGImageGetColorSpace(imageRef), kCGImageAlphaPremultipliedLast);
    
    // 3在CGContextRef上绘图
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    
    CGContextRelease(context);
    
    NSString* key = STR_TO_OCSTR(imageCmd->url);
    [m_renderer setTextureData:key data:data width:width height:height];
    
    [m_renderer appendBatchCommand:BatchCommandTexture size:6 key:key];
    
    // 释放data
    if (data) {
        free(data);
    }
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
    
    createVertexAttr(rect, color, m_vertexs);
    
    //NSString* nsText = [[NSString alloc] initWithUTF8String: GET_STR(textCmd->text)];
    //UIFont* font = [UIFont fontWithName:nsText size:textCmd->font.getFontSize()];
    int scale = 1;
    UIFont* font = [UIFont systemFontOfSize:textCmd->font.getFontSize()*scale];
    int width = textCmd->rect.GetWidth() * scale;
    int height = textCmd->rect.GetHeight() * scale;
    Byte* data = (Byte*)malloc(width * height * 4); // rgba共4个byte
    memset(data, 0, width * height * 4);
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
    //[textAttributes setValue:[UIColor redColor] forKey:NSForegroundColorAttributeName];
    [textAttributes setValue:uiColor forKey:NSForegroundColorAttributeName];
    //[textAttributes setValue:@3 forKey:NSStrokeWidthAttributeName];
    
    // 开始绘制文字
//    [text drawInRect:CGRectMake(0, 0, width * 2, height)
//      withAttributes:textAttributes];
    [text drawAtPoint:CGPointMake(0, 0) withAttributes:textAttributes];

    UIGraphicsPopContext();
    
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    [m_renderer setTextureData:text data:data width:width height:height];
    [m_renderer appendBatchCommand:BatchCommandTexture size:6 key:text];
    
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
