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
        [self.renderer handleTouchEvent:type x:touchPoint.x y:touchPoint.y];
    }
    NSLog(@"event: x=%f and y=%f", touchPoint.x, touchPoint.y);
}

// override
-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    //[[self getKeyWindow] endEditing:NO];
    //[self.boyiaView becomeFirstResponder];
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
    
    CAMetalLayer* layer = [self.boyiaView metalLayer];
    //self.renderer = [[IOSRenderer alloc] initWithLayer:layer];
    
    self.renderer = [IOSRenderer initRenderer:layer];
        
//    self.displayLink = [CADisplayLink displayLinkWithTarget:self
//                                                   selector:@selector(render:)];
    //[self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
    //self.inputView = [[BoyiaTextInputView alloc] init];
    
    
    BoyiaGestureRecognizer* gesture = [[BoyiaGestureRecognizer alloc]initWithTargetRenderer:self action:@selector(handlePanGesture:) renderer:self.renderer];
    [self.boyiaView addGestureRecognizer:gesture];
}

-(void)handlePanGesture:(UIPanGestureRecognizer*)gesture {
    //CGPoint point = [gesture translationInView:gesture.view];
    if (gesture.state == UIGestureRecognizerStateEnded) {
        CGPoint speed = [gesture velocityInView:gesture.view];
        NSLog(@"speed = %@", NSStringFromCGPoint(speed));
        
        if (self.renderer) {
            [self.renderer onFling:speed];
        }
    }
}

//-(BOOL)canBecomeFirstResponder {
//    return YES;
//}

-(void)viewDidAppear:(BOOL)animated;  {
    [super viewDidAppear:animated];
    
//    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(NSEC_PER_SEC*3)), dispatch_get_main_queue(), ^ {
//        self.textInputView = [[BoyiaTextInputView alloc] init];
//        [self addToInputParentViewIfNeeded:self.textInputView];
//        //BOOL resign = [self canResignFirstResponder];
//        BOOL can = [self.textInputView canBecomeFirstResponder];
//        if (!can) {
//            return;
//        }
//
//        // 此处返回YES也弹不出键盘。。。
//        if ([self.textInputView becomeFirstResponder]) {
//            NSLog(@"result = true");
//        }
//    });
}

//-(void)touchEvent:(int)type withEvent:(UIEvent*)event  {
//    NSSet* allTouches = [event allTouches];
//    UITouch* touch = [allTouches anyObject];
//    CGPoint touchPoint = [touch preciseLocationInView:[touch view]];
//    if (self.renderer) {
//        [self.renderer handleTouchEvent:type x:touchPoint.x y:touchPoint.y];
//    }
//    NSLog(@"event: x=%f and y=%f", touchPoint.x, touchPoint.y);
//}
//
//// override
//-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
//    //[[self getKeyWindow] endEditing:NO];
//    //[self.boyiaView becomeFirstResponder];
//
//
//
//
//    [self touchEvent:0 withEvent:event];
//}
//
//// override
//-(void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
//    [self touchEvent:1 withEvent:event];
//}
//
//// override
//-(void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
//    [self touchEvent:2 withEvent:event];
//}

@end
