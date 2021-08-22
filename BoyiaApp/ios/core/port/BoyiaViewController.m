//
//  BoyiaViewController.m
//  core
//
//  Created by yanbo on 2021/8/20.
//

@import MetalKit;
@import GLKit;

#import "BoyiaViewController.h"
#import "BoyiaView.h"
#import "ShaderType.h"

@interface BoyiaViewController ()

@property (nonatomic, strong) BoyiaView* boyiaView;
// Metal设备
@property (nonatomic, retain, nullable) id <MTLDevice> device;
// 渲染命令队列
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;

@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLDepthStencilState> depthStencilState;

@end

@implementation BoyiaViewController

- (instancetype)init {
  return [self initWithMetal:nil bundle:nil];
}

- (instancetype)initWithMetal:(NSString*)nibName
                         bundle:(NSBundle*)nibBundle {
  self = [super initWithNibName:nibName bundle:nibBundle];
//  if (self) {
//      [self metalInit];
//  }

  return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    NSLog(@"Hello BoyiaViewController");
    self.boyiaView = [[BoyiaView alloc] initWithFrame:self.view.bounds];
    
//    self.mtkView.device = MTLCreateSystemDefaultDevice();
//    self.mtkView.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
//    [self.view insertSubview:self.mtkView atIndex:0];
//    self.mtkView.delegate = self;
//    self.viewportSize = (vector_uint2){self.mtkView.drawableSize.width, self.mtkView.drawableSize.height};
//    
    [self metalInit];
}

// 初始化metal环境
- (void)metalInit {
    CAMetalLayer* layer = [self.boyiaView metalLayer];
    // 如果layer是空，则直接返回
    if (layer == nil) {
        return;
    }
    
    id device = MTLCreateSystemDefaultDevice();
    
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.device = device;

//    NSString* path = [NSBundle bundleWithIdentifier:@"com.boyia.bundle"].bundlePath;
//    NSLog(@"Result New data = %@",path);
//    NSError *error = nil;
//    id<MTLLibrary> defaultLibrary = [device newDefaultLibraryWithBundle:[NSBundle bundleWithPath:path] error:&error];
    id<MTLLibrary> defaultLibrary = [device newDefaultLibrary];
    // 获取shader入口
    id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexMain"];
    id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentMain"];
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.vertexFunction = vertexFunction;
    pipelineStateDescriptor.fragmentFunction = fragmentFunction;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipelineStateDescriptor.depthAttachmentPixelFormat =  MTLPixelFormatDepth32Float_Stencil8;
    pipelineStateDescriptor.stencilAttachmentPixelFormat =  MTLPixelFormatDepth32Float_Stencil8;
    self.pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                                         error:NULL];
    
    
    MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    self.depthStencilState = [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
    
    self.commandQueue = [device newCommandQueue];
}


@end
