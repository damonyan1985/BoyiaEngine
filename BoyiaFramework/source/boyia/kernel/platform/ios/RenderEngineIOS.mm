//#include "IRenderEngine.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <Metal/Metal.h>

#import "IOSRenderer.h"
#include "RenderEngineIOS.h"
#include "ShaderType.h"
#include "PixelRatio.h"
#include "AppManager.h"


// Need set xcode build settings Preprocessing
namespace yanbo {

using LRectF = util::LRect_t<float>;
using LPointF = util::LPoint_t<float>;
using LSizeF = util::LSize_t<float>;

LVoid screenToMetalPoint(
    float x,
    float y,
    float* metalX,
    float* metalY)
{
    *metalX = (1.0f * (x - PixelRatio::logicWidth() / 2.0f)) / (PixelRatio::logicWidth() / 2.0f);
    *metalY = ((1.0f * (PixelRatio::logicHeight() / 2.0f - y)) / (PixelRatio::logicHeight() / 2.0f));
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


static LVoid createVertexAttrEx(const LRectF& rect, const LColor& color, KVector<VertexAttributes>& vertexs)
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

inline static float realLength(LInt length)
{
    return yanbo::PixelRatio::ratio() * length;
}

inline static float coreLength(float length)
{
    return length / yanbo::PixelRatio::ratio();
}

LVoid fixIosPlatformRect(const LRect& src, LRectF& dest)
{
    float scale = [[UIScreen mainScreen]scale];
    // 一个dp占多少像素
    float left = ceil(realLength(src.iTopLeft.iX) * scale);
    float top = ceil(realLength(src.iTopLeft.iY) * scale);
    
    float width = ceil(realLength(src.GetWidth()) * scale);
    float height = ceil(realLength(src.GetHeight()) * scale);
    
    dest.Set(LPointF(coreLength(left)/scale, coreLength(top)/scale), LSizeF(coreLength(width)/scale, coreLength(height)/scale));
}

// TODO
RenderEngineIOS::RenderEngineIOS()
    : m_renderer(kBoyiaNull)
    , m_vertexs(0, 256)
    , m_uniforms(0, 256)
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
    m_functions[RenderCommand::kRenderRoundRect] = (RenderFunction)&RenderEngineIOS::renderRoundRect;
    m_functions[RenderCommand::kRenderText] = (RenderFunction)&RenderEngineIOS::renderText;
    m_functions[RenderCommand::kRenderImage] = (RenderFunction)&RenderEngineIOS::renderImage;
}

LVoid RenderEngineIOS::reset()
{
}

LVoid RenderEngineIOS::render(RenderLayer* layer)
{
    [m_renderer clearBatchCommandBuffer];
    // 清空缓冲区
    m_vertexs.clear();
    m_uniforms.clear();
    // 开始渲染
    renderImpl(layer);
    // 刷新缓冲区
    setBuffer();

    [m_renderer render];
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
    LRect& srcRect = cmd->rect;
    LColor& color = cmd->color;
    
    if (!srcRect.GetWidth() || !srcRect.GetHeight()) {
        return;
    }
    
    LRectF rect;
//    rect.Set(LPointF(srcRect.iTopLeft.iX, srcRect.iTopLeft.iY), LSizeF(srcRect.GetWidth(), srcRect.GetHeight()));
    fixIosPlatformRect(srcRect, rect);
    
    float h = rect.GetHeight() * PixelRatio::ratio();
    
    float scale = [[UIScreen mainScreen]scale];
    
    NSLog(@"renderRect height=%d h=%f scale=%f pixel=%f", rect.GetHeight(), h, scale, scale * h);
    
    createVertexAttrEx(rect, color, m_vertexs);
    //createVertexAttr(srcRect, color, m_vertexs);
    
    //[m_renderer appendBatchCommand:BatchCommandNormal size:6 key:nil];
    
    if ([m_renderer appendBatchCommand:BatchCommandNormal size:6 key:nil]) {
        Uniforms uniforms;
        uniforms.uType = 0;
        m_uniforms.addElement(uniforms);
    }
}

LVoid RenderEngineIOS::setBuffer()
{
    NSLog(@"setBuffer() size=%d", m_vertexs.size());
    if (m_vertexs.size()) {
        [m_renderer setVerticeBuffer:m_vertexs.getBuffer() size:(m_vertexs.size() * sizeof(VertexAttributes))];
    }
}

LVoid RenderEngineIOS::renderImage(RenderCommand* cmd)
{
    RenderImageCommand* imageCmd = static_cast<RenderImageCommand*>(cmd);
    LRect& rect = imageCmd->rect;
    LColor& color = imageCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    NSString* key = STR_TO_OCSTR(imageCmd->url);
    id tex = [m_renderer getTexture:key];
    if (!tex) {
        if (!imageCmd->image) {
            return;
        }
        
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
        
        
        [m_renderer setTextureData:key data:data width:width height:height];
        
        // 释放data
        if (data) {
            free(data);
        }
    }
    
    createVertexAttr(rect, color, m_vertexs);
    
    if ([m_renderer appendBatchCommand:BatchCommandTexture size:6 key:key]) {
        Uniforms uniforms;
        uniforms.uType = 1;
        m_uniforms.addElement(uniforms);
    }
}

LVoid RenderEngineIOS::renderText(RenderCommand* cmd)
{
    RenderTextCommand* textCmd = static_cast<RenderTextCommand*>(cmd);
    LRect& rect = textCmd->rect;
    LColor& color = textCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    //LRectF rect;
    //fixIosPlatformRect(srcRect, rect);
    
    createVertexAttr(rect, color, m_vertexs);
    
    int scale = 1;
    UIFont* font = [UIFont systemFontOfSize:textCmd->font.getFontSize()*scale];
    //UIFont* font = [UIFont fontWithName:@"BoyiaFont" size:textCmd->font.getFontSize()*scale];
    
    int width = rect.GetWidth() * scale;
    int height = rect.GetHeight() * scale;
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
    //NSString* text = @"\U0000e800";
    // 文字颜色
    UIColor* uiColor = [UIColor colorWithRed:METAL_COLOR_BIT(textCmd->color.m_red)
                                    green:METAL_COLOR_BIT(textCmd->color.m_green)
                                     blue:METAL_COLOR_BIT(textCmd->color.m_blue)
                                    alpha:METAL_COLOR_BIT(textCmd->color.m_alpha)];
    
    // 设置文字绘制属性
    NSMutableDictionary* textAttributes = [NSMutableDictionary new];
    [textAttributes setValue:font forKey:NSFontAttributeName];
    [textAttributes setValue:uiColor forKey:NSForegroundColorAttributeName];
    //[textAttributes setValue:UIColor.blueColor forKey:NSForegroundColorAttributeName];
    
    // 开始绘制文字
//    [text drawInRect:CGRectMake(0, 0, width * 2, height)
//      withAttributes:textAttributes];
    [text drawAtPoint:CGPointMake(0, 0) withAttributes:textAttributes];

    UIGraphicsPopContext();
    
    CGContextRelease(context);
    CGColorSpaceRelease(colorSpace);
    
    [m_renderer setTextureData:text data:data width:width height:height];
    //[m_renderer appendBatchCommand:BatchCommandTexture size:6 key:text];
    if ([m_renderer appendBatchCommand:BatchCommandTexture size:6 key:text]) {
        Uniforms uniforms;
        uniforms.uType = 1;
        m_uniforms.addElement(uniforms);
    }
    
    // 释放data
    if (data) {
        free(data);
    }
}

LVoid RenderEngineIOS::renderRoundRect(RenderCommand* cmd)
{
    RenderRoundRectCommand* roundCmd = static_cast<RenderRoundRectCommand*>(cmd);
    
    LRect& rect = roundCmd->rect;
    LColor& color = roundCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    createVertexAttr(rect, color, m_vertexs);
    
    if ([m_renderer appendBatchCommand:BatchCommandRound size:6 key:nil]) {
        Uniforms uniforms;
        uniforms.uType = 4;
        
        uniforms.uRadius.topLeftRadius = realLength(roundCmd->topRightRadius);
        uniforms.uRadius.topRightRadius = realLength(roundCmd->topRightRadius);
        uniforms.uRadius.bottomRightRadius = realLength(roundCmd->bottomRightRadius);
        uniforms.uRadius.bottomLeftRadius = realLength(roundCmd->bottomLeftRadius);
        
        uniforms.uRadius.topLeft = {
            realLength(rect.iTopLeft.iX + roundCmd->topLeftRadius),
            realLength(rect.iTopLeft.iY + roundCmd->topLeftRadius) + [m_renderer getRenderStatusBarHight]
        };
        
        uniforms.uRadius.topRight = {
            realLength(rect.iBottomRight.iX - roundCmd->topRightRadius),
            realLength(rect.iTopLeft.iY + roundCmd->topRightRadius) + [m_renderer getRenderStatusBarHight]
        };
        
        uniforms.uRadius.bottomLeft = {
            realLength(rect.iTopLeft.iX + roundCmd->bottomLeftRadius),
            realLength(rect.iBottomRight.iY - roundCmd->bottomLeftRadius) + [m_renderer getRenderStatusBarHight]
        };
        
        uniforms.uRadius.bottomRight = {
            realLength(rect.iBottomRight.iX - roundCmd->bottomRightRadius),
            realLength(rect.iBottomRight.iY - roundCmd->bottomRightRadius) + [m_renderer getRenderStatusBarHight]
        };
        
        m_uniforms.addElement(uniforms);
    }
}

LVoid RenderEngineIOS::appendUniforms(LInt type)
{
    Uniforms uniforms;
    uniforms.uType = type;
    m_uniforms.addElement(uniforms);
}

// Test Api
LVoid RenderEngineIOS::renderRoundRectEx(RenderCommand* cmd)
{
    // later remove
    m_vertexs.clear();
    m_uniforms.clear();
    
    RenderRoundRectCommand* roundCmd = static_cast<RenderRoundRectCommand*>(cmd);
    
    LRect& rect = roundCmd->rect;
    LColor& color = roundCmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    createVertexAttr(rect, color, m_vertexs);
    
    Uniforms uniforms;
    uniforms.uType = 4;
    
    uniforms.uRadius.topLeftRadius = realLength(roundCmd->topRightRadius);
    uniforms.uRadius.topRightRadius = realLength(roundCmd->topRightRadius);
    uniforms.uRadius.bottomRightRadius = realLength(roundCmd->bottomRightRadius);
    uniforms.uRadius.bottomLeftRadius = realLength(roundCmd->bottomLeftRadius);
    
    uniforms.uRadius.topLeft = {
        realLength(rect.iTopLeft.iX + roundCmd->topLeftRadius),
        realLength(rect.iTopLeft.iY + roundCmd->topLeftRadius) + [m_renderer getRenderStatusBarHight]
    };
    
    uniforms.uRadius.topRight = {
        realLength(rect.iBottomRight.iX - roundCmd->topRightRadius),
        realLength(rect.iTopLeft.iY + roundCmd->topRightRadius) + [m_renderer getRenderStatusBarHight]
    };
    
    uniforms.uRadius.bottomLeft = {
        realLength(rect.iTopLeft.iX + roundCmd->bottomLeftRadius),
        realLength(rect.iBottomRight.iY - roundCmd->bottomLeftRadius) + [m_renderer getRenderStatusBarHight]
    };
    
    uniforms.uRadius.bottomRight = {
        realLength(rect.iBottomRight.iX - roundCmd->bottomRightRadius),
        realLength(rect.iBottomRight.iY - roundCmd->bottomRightRadius) + [m_renderer getRenderStatusBarHight]
    };
    
    m_uniforms.addElement(uniforms);
    
    // later remove
    setBuffer();
}

const KVector<Uniforms>& RenderEngineIOS::uniforms() const
{
    return m_uniforms;
}

IOSRenderer* RenderEngineIOS::iosRenderer() const
{
    return m_renderer;
}

// Test Api
LVoid RenderEngineIOS::renderRectEx(RenderCommand* cmd)
{
    m_vertexs.clear();
    
    LRect& rect = cmd->rect;
    LColor& color = cmd->color;
    
    if (!rect.GetWidth() || !rect.GetHeight()) {
        return;
    }
    
    createVertexAttr(rect, color, m_vertexs);
    setBuffer();
}

IRenderEngine* IRenderEngine::create()
{
    return new RenderEngineIOS();
}


}
