//
//  IOSRender.m
//  core
//
//  Created by yanbo on 2021/8/28.
//
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include <Metal/Metal.h>

#import "IOSRenderer.h"
//#import "ImageLoaderIOS.h"

#include "RenderEngineIOS.h"
#include "RenderCommand.h"
#include "PixelRatio.h"
#include "NetworkBase.h"
#include "StringBuilder.h"

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

@end

@implementation IOSRenderer {
    yanbo::RenderEngineIOS* _engine;
}

-(instancetype)initWithLayer:(CAMetalLayer*)layer {
    self = [super init];
    if (self != nil) {
        
        [self initMetal:layer];
        
        int w = [UIScreen mainScreen].bounds.size.width;
        int h = [UIScreen mainScreen].bounds.size.height;
        
        // 720*1080
        LInt logicHeight = (1.0f * 720 / w) * h;
        yanbo::PixelRatio::setWindowSize(w, h);
        yanbo::PixelRatio::setLogicWindowSize(720, logicHeight);
        _engine = new yanbo::RenderEngineIOS();
        _engine->setContextIOS(self);
        
//        HttpEngineIOS* engine = [HttpEngineIOS alloc];
//        String url = _CS("https://www.baidu.com");
//        [engine loadUrlWithData:kHttpGet url:GET_STR(url) callback:[HttpClient new]];
        yanbo::NetworkBase* loader = yanbo::NetworkBase::create();
        
        loader->loadUrl(_CS("https://www.baidu.com"), new Client());
    }
    
    return self;
}

// 初始化metal环境
-(void)initMetal:(CAMetalLayer*)layer {
    self.metalLayer = layer;
    id device = MTLCreateSystemDefaultDevice();
    
    self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self.metalLayer.device = device;
    
    // 从当前framework bundle中获取dmetal
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    NSError *error = nil;
    id<MTLLibrary> defaultLibrary = [device newDefaultLibraryWithBundle:bundle error:&error];
    
    id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexMain"];
    id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentMain"];
    
    // 创建Render Pipeline
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    
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
    self.renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0, 1.0, 0.0, 1.0);
    self.renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    self.renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    self.commandQueue = [device newCommandQueue];
}

-(void)setBuffer:(const void*)buffer size:(NSUInteger)size {
    self.verticeBuffer = [self.metalLayer.device newBufferWithBytes:buffer length:size options:MTLResourceStorageModeShared];
}

-(CAMetalLayer*)layer {
    return self.metalLayer;
}

-(void)render {
    CGFloat statusBar = [[UIApplication sharedApplication] statusBarFrame].size.height;
    LRect rect1(100, 100 + statusBar, 300, 300);
    yanbo::RenderRectCommand* cmd1 = new yanbo::RenderRectCommand(rect1, LColor(0, 255, 0, 255));
    _engine->renderRect(cmd1);
    
    LRect rect2(100, 600 + statusBar, 300, 300);
    yanbo::RenderRectCommand* cmd2 = new yanbo::RenderRectCommand(rect2, LColor(0, 0, 255, 255));
    _engine->renderRect(cmd2);
    
    _engine->setBuffer();
    
    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
    
    if (self.renderPassDescriptor == nil) {
        return;
    }
    
    CAMetalLayer* layer = self.metalLayer;
    id drawable = layer.nextDrawable;
    
    // 必须设置texture, 应该所有绘制操作都是作用在texture上
    self.renderPassDescriptor.colorAttachments[0].texture = [drawable texture];
    
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:self.renderPassDescriptor];
    // 设置显示区域大小
    NSLog(@"width=%f height=%f", layer.drawableSize.width, layer.drawableSize.height);
    [renderEncoder setViewport:(MTLViewport){0.0, 0.0, layer.drawableSize.width, layer.drawableSize.height, -1.0, 1.0 }];
    [renderEncoder setRenderPipelineState:self.pipelineState];
    
    [renderEncoder setVertexBuffer:self.verticeBuffer
                            offset:0
                           atIndex:0];
    
//    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
//                      vertexStart:0
//                      vertexCount:6];
    
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                      vertexStart:0
                      vertexCount:(self.verticeBuffer.length/sizeof(VertexAttributes))];

    
// 如果使用索引
//    id<MTLBuffer> indexBuffer = [self.metalLayer.device newBufferWithBytes:index length:sizeof(index) options:MTLResourceStorageModeShared];
//    UInt16 index[12] = {
//        0, 1, 2, 2, 3, 0,
//        0 + 4, 1 + 4, 2 + 4, 2 + 4, 3 + 4, 0 + 4,
//        //6, 7, 8, 9, 10, 11
//    };
//
//    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
//                              indexCount:indexBuffer.length/sizeof(UInt16)
//                               indexType:MTLIndexTypeUInt16
//                             indexBuffer:indexBuffer
//                       indexBufferOffset:0];
    
    [renderEncoder endEncoding];
    
    [commandBuffer presentDrawable:drawable];
    
    [commandBuffer commit];
}
@end
