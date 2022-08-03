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

#pragma mark - BoyiaGestureRecognizer
// 使用GesturerRecognizer替代controller自身的touch事件接口
@interface BoyiaGestureRecognizer : UIPanGestureRecognizer
-(instancetype)initWithTargetRenderer:(id)target action:(SEL)action renderer:(IOSRenderer*)renderer;

@property (nonatomic, weak) IOSRenderer* renderer;
@end

@implementation BoyiaGestureRecognizer

-(instancetype)initWithTargetRenderer:(id)target action:(SEL)action renderer:(IOSRenderer*)renderer {
    self = [super initWithTarget:target action:action];
    if (self) {
        self.renderer = renderer;
    }
    
    return self;
}

-(void)touchEvent:(int)type withEvent:(UIEvent*)event  {
    NSSet* allTouches = [event allTouches];
    UITouch* touch = [allTouches anyObject];
    CGPoint touchPoint = [touch preciseLocationInView:[touch view]];
    if (self.renderer) {
        [self.renderer handleTouchEvent:type point:touchPoint];
    }
    NSLog(@"event: x=%f and y=%f", touchPoint.x, touchPoint.y);
}

// override
-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesBegan:touches withEvent:event];
    
    [self touchEvent:0 withEvent:event];
}

// override
-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesEnded:touches withEvent:event];
    [self touchEvent:1 withEvent:event];
}

// override
-(void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesMoved:touches withEvent:event];
    [self touchEvent:2 withEvent:event];
}

@end


#pragma mark - BoyiaViewController

@interface BoyiaViewController ()

@property (nonatomic, strong) BoyiaView* boyiaView;
@property (nonatomic, strong) CADisplayLink* displayLink;
@property (nonatomic, strong) IOSRenderer* renderer;

@end

@implementation BoyiaViewController

-(instancetype)init {
  return [self initWithMetal:nil bundle:nil];
}

-(instancetype)initWithMetal:(NSString*)nibName
                         bundle:(NSBundle*)nibBundle {
  self = [super initWithNibName:nibName bundle:nibBundle];
//  if (self) {
//      [self metalInit];
//  }

  return self;
}

-(void)viewDidLoad {
    [super viewDidLoad];
    NSLog(@"Hello BoyiaViewController");
    
    self.boyiaView = [[BoyiaView alloc] initWithFrame:self.view.bounds];
    
    self.view = self.boyiaView;
    // 使用metallayer
    CAMetalLayer* layer = [self.boyiaView metalLayer];
    // 创建IOS Renderer
    self.renderer = [IOSRenderer initRenderer:layer];
    
    // 添加手势监听
    BoyiaGestureRecognizer* gesture = [[BoyiaGestureRecognizer alloc]initWithTargetRenderer:self action:@selector(handlePanGesture:) renderer:self.renderer];
    gesture.delegate = self;
    [self.boyiaView addGestureRecognizer:gesture];
}

// 防止侧滑事件被阻止
-(BOOL)gestureRecognizer:(UIGestureRecognizer*)gestureRecognizer
    shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer*)otherGestureRecognizer
    API_AVAILABLE(ios(13.4)) {
  return YES;
}

-(void)handlePanGesture:(UIPanGestureRecognizer*)gesture {
    //CGPoint point = [gesture translationInView:gesture.view];
    if (gesture.state == UIGestureRecognizerStateEnded) {
        CGPoint point = [gesture locationInView:gesture.view];
        CGPoint speed = [gesture velocityInView:gesture.view];
        NSLog(@"speed = %@", NSStringFromCGPoint(speed));

        if (self.renderer) {
            [self.renderer onFling:speed pointStart:point pointEnd:point];
        }
    }
}

//-(BOOL)canBecomeFirstResponder {
//    return YES;
//}

-(void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidAppear:animated];
    //[self.renderer cacheCode];
}

@end
