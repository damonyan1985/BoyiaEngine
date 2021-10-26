//
//  IOSRender.m
//  core
//
//  Created by yanbo on 2021/8/28.
//
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <Metal/Metal.h>
#import <CoreText/CoreText.h>

#import "IOSRenderer.h"
#import "BoyiaTextInputView.h"

//#import "ImageLoaderIOS.h"

#include "RenderEngineIOS.h"
#include "RenderCommand.h"
#include "PixelRatio.h"
#include "NetworkBase.h"
#include "StringBuilder.h"
#include "ShaderType.h"
#include "PlatformBridge.h"
#include "FileUtil.h"
#include "RenderThread.h"
#include "AppManager.h"

@interface BoyiaTextInputViewHider : UIView
@end

@implementation BoyiaTextInputViewHider {
}

@end

class Client : public yanbo::NetworkClient {
public:
    virtual LVoid onDataReceived(const LByte* data, LInt size)
    {
        LByte* destData = new LByte[size];
        util::LMemcpy(destData, data, size);
        m_builder.append(destData, 0, size, LFalse);
    }
    
    virtual LVoid onStatusCode(LInt statusCode)
    {
        
    }
    
    virtual LVoid onFileLen(LInt len)
    {
        
    }
    
    virtual LVoid onRedirectUrl(const String& redirectUrl)
    {
        
    }
    
    virtual LVoid onLoadError(LInt error)
    {
        
    }
    
    virtual LVoid onLoadFinished()
    {
        OwnerPtr<String> buffer = m_builder.toString();
        NSString* text = [[NSString alloc] initWithUTF8String: (const char*)buffer->GetBuffer()];
        NSLog(@"onLoadFinished data = %@",text);
    }
    
private:
    yanbo::StringBuilder m_builder;
};


@interface BatchCommand : NSObject

@property (nonatomic, assign) NSInteger cmdType;
@property (nonatomic, assign) NSInteger size;
@property (nonatomic, strong) NSString* textureKey;

@end

@implementation BatchCommand

-(instancetype)initWithParams:(NSInteger)cmdType size:(NSInteger)size key:(NSString*)key {
    self = [super init];
    if (self != nil) {
        self.cmdType = cmdType;
        self.size = size;
        self.textureKey = key;
    }
    
    return self;
}

@end

// 对于熟悉IOS中nonatomic与atomic互斥
// strong,copy,weak,assign互斥
@interface IOSRenderer()

@property (nonatomic, strong) CAMetalLayer* metalLayer;
// 渲染指令队列
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;

// 管道状态
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLDepthStencilState> depthStencilState;
@property (nonatomic, strong) MTLRenderPassDescriptor* renderPassDescriptor;

// 顶点缓冲
@property (nonatomic, strong) id<MTLBuffer> verticeBuffer;

// 纹理缓存
@property (nonatomic, strong) NSMutableDictionary* textureCache;

// 常量buffer判断图像类型
//@property (nonatomic, strong) id<MTLBuffer> uniformsBuffer;

@property (nonatomic, strong) NSMutableArray* cmdBuffer;

@property (nonatomic, assign) CGFloat statusBarHeight;

@property (nonatomic, strong) BoyiaTextInputView* textInputView;
@property (nonatomic, strong) BoyiaTextInputViewHider* textInputHider;

@end

@implementation IOSRenderer {
    yanbo::RenderEngineIOS* _engine;
}

+(IOSRenderer*)initRenderer:(CAMetalLayer*)layer {
    //CGFloat statusBarHeight = [[UIApplication sharedApplication] statusBarFrame].size.height;
    UIStatusBarManager* statusBarManager = [UIApplication sharedApplication].windows.firstObject.windowScene.statusBarManager;
    CGFloat statusBarHeight = statusBarManager.statusBarFrame.size.height;
    int w = [UIScreen mainScreen].bounds.size.width;
    int h = [UIScreen mainScreen].bounds.size.height - statusBarHeight;
    
    // 720*1080
    LInt logicHeight = (1.0f * 720 / w) * h;
    yanbo::PixelRatio::setWindowSize(w, h);
    yanbo::PixelRatio::setLogicWindowSize(720, logicHeight);
    
    IOSRenderer* renderer = [[IOSRenderer alloc] initWithLayer:layer];
    renderer.statusBarHeight = statusBarHeight;
    yanbo::RenderEngineIOS* engine = static_cast<yanbo::RenderEngineIOS*>(yanbo::RenderThread::instance()->getRenderer());
    engine->setContextIOS(renderer);
    
    yanbo::AppManager::instance()->setViewport(LRect(0, 0, 720, logicHeight));
    yanbo::AppManager::instance()->start();
    
    return renderer;
}

+(IOSRenderer*)renderer {
    yanbo::RenderEngineIOS* engine = static_cast<yanbo::RenderEngineIOS*>(yanbo::RenderThread::instance()->getRenderer());
    if (!engine) {
        return nil;
    }
    
    return engine->iosRenderer();
}

+(void)runOnUiThead:(dispatch_block_t)block {
    dispatch_async(dispatch_get_main_queue(), block);
    
    // NSOperationQueue来切换回主线程
//    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
//    }];
}

-(void)onFling:(CGPoint)point{
//    LFlingEvent* evt = new LFlingEvent();
//    evt->pt1.Set(yanbo::PixelRatio::viewX(x1), yanbo::PixelRatio::viewY(y1));
//    evt->pt2.Set(yanbo::PixelRatio::viewX(x2), yanbo::PixelRatio::viewY(y2));
//
//    // 速度是秒级别的，换算成毫秒级别
//    //velocityX = velocityX / 1000;
//    //velocityY = velocityY / 1000;
//    evt->velocityX = yanbo::PixelRatio::viewX(velocityX);
//    evt->velocityY = yanbo::PixelRatio::viewY(velocityY);
//
//    yanbo::AppManager::instance()->uiThread()->handleFlingEvent(evt);
}

-(void)handleTouchEvent:(int)type x:(int)x y:(int)y {
    y -= self.statusBarHeight;
    yanbo::AppManager::instance()->handleTouchEvent(type, x, y);
}

-(instancetype)initWithLayer:(CAMetalLayer*)layer {
    self = [super init];
    if (self != nil) {
        self.metalLayer = layer;
        // TODO 后期需要移到engine中
        [self initMetal];
        
        self.cmdBuffer = [[NSMutableArray alloc] initWithCapacity:1024];
        self.textureCache = [NSMutableDictionary new];
        self.textInputHider = [[BoyiaTextInputViewHider alloc] init];
        
//        _engine = new yanbo::RenderEngineIOS();
//        _engine->setContextIOS(self);
        
        _engine = static_cast<yanbo::RenderEngineIOS*>(yanbo::RenderThread::instance()->getRenderer());
        
//        HttpEngineIOS* engine = [HttpEngineIOS alloc];
//        String url = _CS("https://www.baidu.com");
//        [engine loadUrlWithData:kHttpGet url:GET_STR(url) callback:[HttpClient new]];
        
        
        
//        yanbo::NetworkBase* loader = yanbo::NetworkBase::create();
//        
//        loader->loadUrl(_CS("https://www.baidu.com"), new Client());
        
        
        
        String content;
        //String appPath = _DSTR(_CS(yanbo::PlatformBridge::getAppPath())) + _CS("contacts/app.json");
        String appPath = _CS(yanbo::PlatformBridge::getBoyiaJsonPath());
        FileUtil::readFile(appPath, content);
        printf("boyia json=%s\n", GET_STR(content));
        
        printf("boyia dir=%s\n", GET_STR(appPath));
        
        printf("intptr size=%ld\n", sizeof(LIntPtr));
        
        [self initFonts];
    }
    
    return self;
}

-(void)showKeyboard:(NSString*)text {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0)), dispatch_get_main_queue(), ^ {
        if (!self.textInputView) {
            self.textInputView = [[BoyiaTextInputView alloc] initWithRenderer:self];
            [self addToInputParentViewIfNeeded:self.textInputView];
            //BOOL resign = [self canResignFirstResponder];
            BOOL can = [self.textInputView canBecomeFirstResponder];
            if (!can) {
                return;
            }
        }

        // 将目前控件中的文本设置到textInputView中
        [self.textInputView insertText:text];
        // 要对模拟器进行设置才能弹出键盘，I/O -> keyboard -> toggle software keyboard
        if ([self.textInputView becomeFirstResponder]) {
            NSLog(@"result = true");
        }
    });
}

-(void)setInputText:(NSString*)text {
    String strText((const LUint8*)[text UTF8String]);
    yanbo::AppManager::instance()->uiThread()->setInputText(strText, 0);
    strText.ReleaseBuffer();
}

-(void)addToInputParentViewIfNeeded:(BoyiaTextInputView*)inputView {
    if (![inputView isDescendantOfView:self.textInputHider]) {
        [self.textInputHider addSubview:inputView];
    }
    UIView* parentView = [IOSRenderer getKeyWindow]; //self.boyiaView;
    if (self.textInputHider.superview != parentView) {
        [parentView addSubview:self.textInputHider];
    }
}

+(UIWindow*)getKeyWindow {
    UIApplication* application = [UIApplication sharedApplication];
    if (@available(iOS 13.0, *)) {
        for (UIWindowScene* windowScene in application.connectedScenes) {
            if (windowScene.activationState == UISceneActivationStateForegroundActive) {
                for (UIWindow* window in windowScene.windows) {
                    if (window.isKeyWindow) {
                        return window;
                    }
                }
            }
        }
    } else {
        return [application keyWindow];
    }
    
    return nil;
}

// 初始化metal环境
-(void)initMetal {
    
    id device = MTLCreateSystemDefaultDevice();
    
    self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self.metalLayer.device = device;
    
    // 从当前framework bundle中获取metal
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    NSError* error = nil;
    id<MTLLibrary> defaultLibrary = [device newDefaultLibraryWithBundle:bundle error:&error];
    
    id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexMain"];
    id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentMain"];
    
    // 创建Render Pipeline
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    
    // 透明必须设置以下
    pipelineStateDescriptor.colorAttachments[0].blendingEnabled = YES;
    pipelineStateDescriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pipelineStateDescriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    pipelineStateDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pipelineStateDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    // 透明设置
    
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    //pipelineStateDescriptor.depthAttachmentPixelFormat =  MTLPixelFormatDepth32Float_Stencil8;
    //pipelineStateDescriptor.stencilAttachmentPixelFormat =  MTLPixelFormatDepth32Float_Stencil8;
    self.pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                                         error:NULL];
    
    
    MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    self.depthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    
    self.renderPassDescriptor = [MTLRenderPassDescriptor new];
    self.renderPassDescriptor.colorAttachments[0].clearColor = METAL_COLOR(LColor(0xed, 0x40, 0x40, 0xFF));//MTLClearColorMake(1.0, 1.0, 0.0, 1.0);
    self.renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    self.renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    self.commandQueue = [device newCommandQueue];
}

-(void)clearBatchCommandBuffer {
    [self.cmdBuffer removeAllObjects];
}

-(BOOL)appendBatchCommand:(BatchCommandType)cmdType size:(NSInteger)size key:(NSString*)key {
    // 属性一致，无需新增cmd
    // 都是普通类型，属性一致
    // 都是纹理类型，且使用相同纹理，属性一致
    if (self.cmdBuffer.count > 0) {
        // 如果都是普通类型，即非纹理
        BatchCommand* lastCmd = [self.cmdBuffer lastObject];
        if (lastCmd.cmdType == cmdType && cmdType == BatchCommandNormal) {
            lastCmd.size += size;
            return NO;
        }

        // 如果都是纹理类型，且使用的纹理都是一样的
        if (lastCmd.cmdType == cmdType
            && cmdType == BatchCommandTexture
            && key != nil
            && key.length > 0
            && [key isEqualToString:lastCmd.textureKey]) {
            lastCmd.size += size;
            return NO;
        }
    }
    
    // 属性不一致，新增cmd
    id cmd = [[BatchCommand alloc]initWithParams:cmdType size:size key:key];
    [self.cmdBuffer addObject:cmd];
    
    return YES;
}

-(void)setVerticeBuffer:(const void*)buffer size:(NSUInteger)size {
    self.verticeBuffer = [self.metalLayer.device newBufferWithBytes:buffer length:size options:MTLResourceStorageModeShared];
}

//-(void)setUniformBuffer:(const void*)buffer size:(NSUInteger)size {
////    self.uniformsBuffer = [self.metalLayer.device newBufferWithLength:size options:MTLResourceOptionCPUCacheModeDefault];
////    memcpy([self.uniformsBuffer contents], buffer, size);
//
//    self.uniformsBuffer = [self.metalLayer.device newBufferWithBytes:buffer length:size options:MTLResourceStorageModeShared];
//}

-(id<MTLTexture>)getTexture:(NSString*)key {
    return self.textureCache[key];
}

-(void)setTextureData:(NSString*)key data:(Byte*)data width:(NSUInteger)width height:(NSUInteger)height {
    if (!data) {
        return;
    }
    
    MTLTextureDescriptor *textureDescriptor = [[MTLTextureDescriptor alloc] init];
    textureDescriptor.pixelFormat = MTLPixelFormatRGBA8Unorm;
    textureDescriptor.width = width;
    textureDescriptor.height = height;
    
    id texture = [self.metalLayer.device newTextureWithDescriptor:textureDescriptor]; // 创建纹理
    if (texture == nil) {
        return;
    }
    [self.textureCache setObject:texture forKey:key];
    MTLRegion region = {{ 0, 0, 0 }, {width, height, 1}}; // 纹理上传的范围
    // UIImage的数据需要转成二进制才能上传，且不用jpg、png的NSData
    [texture replaceRegion:region
                mipmapLevel:0
                  withBytes:data
                bytesPerRow:4 * width];
}

-(CAMetalLayer*)layer {
    return self.metalLayer;
}

-(float)getRenderStatusBarHight {
    return self.statusBarHeight;
}

-(void)render {
//    CGFloat statusBar = [[UIApplication sharedApplication] statusBarFrame].size.height;
//    LRect rect1(100, 100 + statusBar, 100, 100);
//    yanbo::RenderRectCommand* cmd1 = new yanbo::RenderRectCommand(rect1, LColor(0, 255, 0, 255));
//    _engine->renderRect(cmd1);
//
//    LRect rect2(100, 600 + statusBar, 100, 100);
//    yanbo::RenderRectCommand* cmd2 = new yanbo::RenderRectCommand(rect2, LColor(0, 0, 255, 255));
//    _engine->renderRect(cmd2);
//
//    LRect rect3(100, 300 + statusBar, 100, 100);
//    yanbo::RenderRectCommand* cmd3 = new yanbo::RenderRectCommand(rect3, LColor(0, 0, 255, 255));
//    _engine->renderRect(cmd3);
//
//
//
//
//
//    String text = _CS("只是测试而已哈哈哈哈哈");
//    LFont* font = LFont::create(LFont());
//    font->setFontSize(24);
//    font->calcTextLine(text, 200);
//
//    LRect rect4(260, 260, font->getLineWidth(0), font->getFontHeight());
//
//    String key;
//    font->getLineText(0, key);
//
//    yanbo::RenderTextCommand* cmd4 = new yanbo::RenderTextCommand(rect4, LColor(0, 255, 0, 255), *font, key);
//    _engine->renderText(cmd4);
//
//    _engine->setBuffer();
    
    if (self.renderPassDescriptor == nil) {
        return;
    }
    
    if (self.cmdBuffer.count <= 0) {
        return;
    }
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    
    CAMetalLayer* layer = self.metalLayer;
    id drawable = layer.nextDrawable;
    
    // 必须设置texture, 应该所有绘制操作都是作用在texture上
    self.renderPassDescriptor.colorAttachments[0].texture = [drawable texture];
    
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:self.renderPassDescriptor];
    // 设置显示区域大小
    NSLog(@"width=%f height=%f", layer.drawableSize.width, layer.drawableSize.height);
    [renderEncoder setViewport:(MTLViewport){0.0, self.statusBarHeight, layer.drawableSize.width, layer.drawableSize.height - self.statusBarHeight, -1.0, 1.0 }];
    [renderEncoder setRenderPipelineState:self.pipelineState];
    
    // 设置缓冲区
    [renderEncoder setVertexBuffer:self.verticeBuffer
                            offset:0
                           atIndex:0];
    
    NSUInteger index = 0;
    LInt uniformIndex = 0;
    for (NSUInteger i = 0; i < self.cmdBuffer.count; i++) {
        BatchCommand* cmd = [self.cmdBuffer objectAtIndex:i];
        
        //Uniforms uniforms = _engine->uniforms()[uniformIndex++];
        // 设置uniforms，告知shader所需要渲染的图形的类型
        id<MTLBuffer> uniformsBuffer = [self.metalLayer.device newBufferWithLength:sizeof(Uniforms) options:MTLResourceOptionCPUCacheModeDefault];
        memcpy([uniformsBuffer contents], &_engine->uniforms()[uniformIndex++], sizeof(Uniforms));
        
        [renderEncoder setFragmentBuffer:uniformsBuffer offset:0 atIndex:0];
        
        if (cmd.cmdType == BatchCommandTexture) {
            id tex = self.textureCache[cmd.textureKey];
            if (tex) {
                [renderEncoder setFragmentTexture:tex atIndex:0];
            } else {
                // Error
            }
        }
        
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:index
                          vertexCount:cmd.size];
        
        index += cmd.size;
    }
    
#if 0
    {
        LRect rect1(100, 400, 200, 200);
        yanbo::RenderRoundRectCommand* cmd1 = new yanbo::RenderRoundRectCommand(rect1, LColor(0, 255, 0, 120), 40, 40, 40, 40);
        _engine->renderRoundRectEx(cmd1);

       

        // 设置缓冲区
        [renderEncoder setVertexBuffer:self.verticeBuffer
                                offset:0
                               atIndex:0];
    
        
//        id<MTLBuffer> uniformsBuffer = [self.metalLayer.device newBufferWithLength:sizeof(Uniforms) options:MTLResourceOptionCPUCacheModeDefault];
//        memcpy([uniformsBuffer contents], _engine->uniforms().getBuffer(), sizeof(Uniforms));

        [renderEncoder setFragmentBuffer:self.uniformsBuffer offset:0 atIndex:0];

        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                          vertexStart:0
                          vertexCount:self.verticeBuffer.length];
    }
#endif
    
    
    
    [renderEncoder endEncoding];
    
    [commandBuffer presentDrawable:drawable];
    
    [commandBuffer commit];
}

// 获取图标字体
-(void)initFonts {
    // Icon font 'BoyiaFont'
    NSBundle* coreBundle = [NSBundle bundleWithIdentifier:@"com.boyia.core"];
    NSString* appDir = [coreBundle pathForResource:@"metal" ofType:@"bundle"];
    NSBundle* appBundle = [NSBundle bundleWithPath:appDir];
    NSString* fontPath = [appBundle.bundlePath stringByAppendingString:@"/apps/font/icon.ttf"];
    
    NSURL* fontUrl = [NSURL fileURLWithPath:fontPath];
    CGDataProviderRef fontDataProvider = CGDataProviderCreateWithURL((__bridge CFURLRef)fontUrl);
    CGFontRef fontRef = CGFontCreateWithDataProvider(fontDataProvider);
    CGDataProviderRelease(fontDataProvider);
    
    CTFontManagerRegisterGraphicsFont(fontRef, NULL);
    NSString* fontName = CFBridgingRelease(CGFontCopyPostScriptName(fontRef));
    NSLog(@"fontName=%@", fontName);
}
@end
