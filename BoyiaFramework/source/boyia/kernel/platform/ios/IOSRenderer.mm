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
#include "BoyiaSocket.h"
#include "Animation.h"

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

@interface IOSTexture()
@property (nonatomic, strong) id<MTLTexture> tex;
@end

@implementation IOSTexture {
    CVPixelBufferRef _pixelBuffer;
}

-(void)dealloc {
    if (_pixelBuffer) {
        CVPixelBufferRelease(_pixelBuffer);
        _pixelBuffer = NULL;
    }
}

-(instancetype)initTexture:(id<MTLTexture>)tex {
    self = [self init];
    if (self) {
        self.tex = tex;
        _pixelBuffer = NULL;
    }
    return self;
}
-(instancetype)initTexture:(id<MTLTexture>)tex andPixel:(CVPixelBufferRef)ref {
    self = [self initTexture:tex];
    if (self) {
        _pixelBuffer = ref;
    }
    
    return self;
}

@end



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

// url作为key进行纹理缓存，主要是处理重复下载问题用于Image
@property (nonatomic, strong) NSMutableDictionary* textureCache;


// 常量buffer判断图像类型
//@property (nonatomic, strong) id<MTLBuffer> uniformsBuffer;

@property (nonatomic, strong) NSMutableArray* cmdBuffer;

@property (nonatomic, assign) CGFloat statusBarHeight;
@property (nonatomic, assign) int keyboardHeight;

@property (nonatomic, strong) BoyiaTextInputView* textInputView;
@property (nonatomic, strong) BoyiaTextInputViewHider* textInputHider;

@end

@implementation IOSRenderer {
    yanbo::RenderEngineIOS* _engine;
    CVMetalTextureCacheRef _cvTextureCache;
}

+(CGFloat)getStatusBarHeight {
    NSSet *set = [[UIApplication sharedApplication] connectedScenes];
    UIWindowScene* windowScene = [set anyObject];
    UIStatusBarManager* statusBarManager =  windowScene.statusBarManager;
    return statusBarManager.statusBarFrame.size.height;
}

void testHashMap()
{
    HashMap<HashString, LInt> map;
    map.put(HashString(_CS("hello")), 1);
    map.put(HashString(_CS("hello1")), 2);
    map.put(HashString(_CS("hello2")), 3);
    map.put(HashString(_CS("hello3")), 4);
    
    map.map([](const HashString& k, const LInt& v) -> bool {
        BOYIA_LOG("hashmap map -> key: %s, value: %d", GET_STR(k), v);
        return false;
    });
}

+(IOSRenderer*)initRenderer:(CAMetalLayer*)layer {
    //CGFloat statusBarHeight = [[UIApplication sharedApplication] statusBarFrame].size.height;
//    UIStatusBarManager* statusBarManager = [UIApplication sharedApplication].windows.firstObject.windowScene.statusBarManager;
    //CGFloat statusBarHeight = statusBarManager.statusBarFrame.size.height;
    CGFloat statusBarHeight = [IOSRenderer getStatusBarHeight];
    CGFloat w = [UIScreen mainScreen].bounds.size.width;
    CGFloat h = [UIScreen mainScreen].bounds.size.height - statusBarHeight;
    
    NSLog(@"screen w=%f h=%f", w, h);
    
    // 720*1080
    CGFloat logicHeight = (1.0f * 720 / w) * h;
    yanbo::PixelRatio::setWindowSize(w, h);
    yanbo::PixelRatio::setLogicWindowSize(720, logicHeight);
    
    IOSRenderer* renderer = [[IOSRenderer alloc] initWithLayer:layer];
    renderer.statusBarHeight = statusBarHeight;
    
    yanbo::RenderEngineIOS* engine = static_cast<yanbo::RenderEngineIOS*>(yanbo::RenderThread::instance()->getRenderer());
    engine->setContextIOS(renderer);
    
    yanbo::AppManager::instance()->setViewport(LRect(0, 0, 720, logicHeight));
    yanbo::AppManager::instance()->start();
    //testHashMap();
    boyia::BoyiaSocket* socket = new boyia::BoyiaSocket(_CS("ws://127.0.0.1:8445/contacts?id=456"), kBoyiaNull);
    new yanbo::Timer(600, [socket]() -> LVoid {
        socket->send(_CS("hello world"));
    }, LTrue);
    
    return renderer;
}

+(IOSRenderer*)renderer {
    yanbo::RenderEngineIOS* engine = static_cast<yanbo::RenderEngineIOS*>(yanbo::RenderThread::instance()->getRenderer());
    if (!engine) {
        return nil;
    }
    
    return engine->iosRenderer();
}

+(void)runOnUiThread:(dispatch_block_t)block {
    dispatch_async(dispatch_get_main_queue(), block);
    
    // NSOperationQueue来切换回主线程
//    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
//    }];
}

-(CVMetalTextureCacheRef)cvTextureCache {
    return _cvTextureCache;
}

-(void)onFling:(CGPoint)speed pointStart:(CGPoint)start pointEnd:(CGPoint)end{
    LFlingEvent* evt = new LFlingEvent();
    evt->pt1.Set(yanbo::PixelRatio::viewX(start.x),
                 yanbo::PixelRatio::viewY(start.y - self.statusBarHeight));
    evt->pt2.Set(yanbo::PixelRatio::viewX(end.x),
                 yanbo::PixelRatio::viewY(end.y - self.statusBarHeight));

    // 速度是秒级别的，换算成毫秒级别
//    velocityX = velocityX / 1000;
//    velocityY = velocityY / 1000;
    evt->velocityX = yanbo::PixelRatio::viewX(speed.x);
    evt->velocityY = yanbo::PixelRatio::viewY(speed.y);

    yanbo::AppManager::instance()->uiThread()->handleFlingEvent(evt);
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
        [self initKeyBoardListeners];
    }
    
    return self;
}

-(void)initKeyBoardListeners {
    // 添加键盘弹出回调
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onKeyboardWillShow:)
                                                 name:UIKeyboardWillShowNotification
                                               object:nil];
    // 添加键盘隐藏回调
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(onKeyboardWillHide:)
                                                 name:UIKeyboardWillHideNotification
                                               object:nil];
}

-(void)onKeyboardWillShow:(NSNotification*)notification {
    NSDictionary* info = [notification userInfo];
    CGRect keyboardRect = [[info objectForKey:UIKeyboardFrameEndUserInfoKey]CGRectValue];
    self.keyboardHeight = keyboardRect.size.height;
    yanbo::AppManager::instance()->uiThread()->onKeyboardShow(0, yanbo::PixelRatio::viewY(self.keyboardHeight));
}

-(void)onKeyboardWillHide:(NSNotification*)notification {
    yanbo::AppManager::instance()->uiThread()->onKeyboardHide(0, yanbo::PixelRatio::viewY(self.keyboardHeight));
}

-(NSString*)localizedStringForKey:(NSString*)key {
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    return [bundle localizedStringForKey:key value:@"" table:nil];
}

-(void)initKeyboardHideButton {
    UIToolbar* backView = [[UIToolbar alloc]initWithFrame:CGRectMake(0, 0, 320, 30)];
    [backView setBarStyle:UIBarStyleDefault];
    
    UIBarButtonItem* btnSpace = [[UIBarButtonItem alloc]initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:self action:nil];
    
    UIButton* btn = [UIButton buttonWithType:UIButtonTypeSystem];
    
    [btn setTitle:[self localizedStringForKey:@"keyBoardHide"] forState:UIControlStateNormal];
    btn.frame = CGRectMake(2, 5, 70, 25);
    // hideKeyboard不加冒号表示没有参数，如果函数带参数则需要加冒号
    [btn addTarget:self action:@selector(hideKeyboard) forControlEvents:UIControlEventTouchUpInside];
    UIBarButtonItem* doneBtn = [[UIBarButtonItem alloc]initWithCustomView:btn];
    NSArray* buttonsArray = @[btnSpace,doneBtn];
    [backView setItems:buttonsArray];
    self.textInputView.inputAccessoryView = backView;
}

// 显示键盘
-(void)showKeyboard:(NSString*)text cursor:(int)cursor {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0)), dispatch_get_main_queue(), ^ {
        if (!self.textInputView) {
            self.textInputView = [[BoyiaTextInputView alloc] initWithRenderer:self];
            [self addToInputParentViewIfNeeded:self.textInputView];
            
            [self initKeyboardHideButton];
            //BOOL resign = [self canResignFirstResponder];
            BOOL can = [self.textInputView canBecomeFirstResponder];
            if (!can) {
                return;
            }
        }

        // 将目前控件中的文本设置到textInputView中
        [self.textInputView resetText:text cursor:cursor];
        // 要对模拟器进行设置才能弹出键盘，I/O -> keyboard -> toggle software keyboard
        if ([self.textInputView becomeFirstResponder]) {
            NSLog(@"result = true");
        }
    });
}

// 隐藏键盘
-(void)hideKeyboard {
    [IOSRenderer runOnUiThread:^{
        if (self.textInputView) {
            [self.textInputView resignFirstResponder];
        }
    }];
}

-(void)setInputText:(NSString*)text cursor:(NSUInteger)cursor {
    String strText((const LUint8*)[text UTF8String]);
    yanbo::AppManager::instance()->uiThread()->setInputText(strText, cursor);
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
    // 可以在此设置statusbar背景颜色
    self.renderPassDescriptor.colorAttachments[0].clearColor = METAL_COLOR(LColor(0xed, 0x40, 0x40, 0xFF));//MTLClearColorMake(1.0, 1.0, 0.0, 1.0);
    self.renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    self.renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    self.commandQueue = [device newCommandQueue];
    
    // 创建textureCache
    CVMetalTextureCacheCreate(NULL, NULL, device, NULL, &_cvTextureCache);
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

-(void)setTexture:(NSString*)key texture:(id<MTLTexture>)texture {
    [self.textureCache setObject:[[IOSTexture alloc]initTexture:texture] forKey:key];
}

-(void)setTexture:(NSString*)key texture:(id<MTLTexture>)texture pixel:(CVPixelBufferRef)ref {
    [self.textureCache setObject:[[IOSTexture alloc]initTexture:texture andPixel:ref] forKey:key];
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
        
    [self.textureCache setObject:[[IOSTexture alloc]initTexture:texture] forKey:key];
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
            IOSTexture* tex = self.textureCache[cmd.textureKey];
            if (tex) {
                [renderEncoder setFragmentTexture:tex.tex atIndex:0];
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

-(void)doesNotRecognizeSelector:(SEL)aSelector {
    NSLog(@"Cannot find method %@", NSStringFromSelector(aSelector));
}
@end
