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
#import "IOSRenderer.h"

@interface BoyiaViewController ()

@property (nonatomic, strong) BoyiaView* boyiaView;

@property (nonatomic, strong) CADisplayLink* displayLink;

@property (nonatomic, strong) IOSRenderer* renderer;

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
    
    self.view = self.boyiaView;
    
    CAMetalLayer* layer = [self.boyiaView metalLayer];
    self.renderer = [[IOSRenderer alloc] initWithLayer:layer];
        
    self.displayLink = [CADisplayLink displayLinkWithTarget:self
                                                   selector:@selector(render:)];
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
}

// 收到vsync信号之后进行渲染
-(void)render:(CADisplayLink*)link {

    [self.renderer render];
    // 避免一直渲染造成性能损耗
    self.displayLink.paused = YES;
}

-(void)await {
    if (self.displayLink != nil) {
        self.displayLink.paused = NO;
    }
}


@end
