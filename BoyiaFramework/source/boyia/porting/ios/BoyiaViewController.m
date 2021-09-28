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

#pragma mark - BoyiaTextPosition

@interface BoyiaTextPosition : UITextPosition

@property(nonatomic, readonly) NSUInteger index;

+ (instancetype)positionWithIndex:(NSUInteger)index;
- (instancetype)initWithIndex:(NSUInteger)index;

@end

@implementation BoyiaTextPosition

+ (instancetype)positionWithIndex:(NSUInteger)index {
  return [[BoyiaTextPosition alloc] initWithIndex:index];
}

- (instancetype)initWithIndex:(NSUInteger)index {
  self = [super init];
  if (self) {
    _index = index;
  }
  return self;
}

@end

#pragma mark - BoyiaTextRange

@interface BoyiaTextRange : UITextRange <NSCopying>

@property(nonatomic, readonly) NSRange range;

+ (instancetype)rangeWithNSRange:(NSRange)range;

@end

@implementation BoyiaTextRange

+ (instancetype)rangeWithNSRange:(NSRange)range {
  return [[BoyiaTextRange alloc] initWithNSRange:range];
}

- (instancetype)initWithNSRange:(NSRange)range {
  self = [super init];
  if (self) {
    _range = range;
  }
  return self;
}

- (UITextPosition*)start {
  return [BoyiaTextPosition positionWithIndex:self.range.location];
}

- (UITextPosition*)end {
  return [BoyiaTextPosition positionWithIndex:self.range.location + self.range.length];
}

- (BOOL)isEmpty {
  return self.range.length == 0;
}

- (id)copyWithZone:(NSZone*)zone {
  return [[BoyiaTextRange allocWithZone:zone] initWithNSRange:self.range];
}

- (BOOL)isEqualTo:(BoyiaTextRange*)other {
  return NSEqualRanges(self.range, other.range);
}
@end

#pragma mark - BoyiaTextInputView
// 软键盘控制
@interface BoyiaTextInputView : UIView<UITextInput>

// UITextInput
@property(nonatomic, readonly) NSMutableString* text;
@property(nonatomic, readonly) NSMutableString* markedText;
@property(readwrite, copy) UITextRange* selectedTextRange;
@property(nonatomic, strong) UITextRange* markedTextRange;
@property(nonatomic, copy) NSDictionary* markedTextStyle;
@property(nonatomic, assign) id<UITextInputDelegate> inputDelegate;

// UITextInputTraits
@property(nonatomic) UITextAutocapitalizationType autocapitalizationType;
@property(nonatomic) UITextAutocorrectionType autocorrectionType;
@property(nonatomic) UITextSpellCheckingType spellCheckingType;
@property(nonatomic) BOOL enablesReturnKeyAutomatically;
@property(nonatomic) UIKeyboardAppearance keyboardAppearance;
@property(nonatomic) UIKeyboardType keyboardType;
@property(nonatomic) UIReturnKeyType returnKeyType;
@property(nonatomic, getter=isSecureTextEntry) BOOL secureTextEntry;
@property(nonatomic) UITextSmartQuotesType smartQuotesType API_AVAILABLE(ios(11.0));
@property(nonatomic) UITextSmartDashesType smartDashesType API_AVAILABLE(ios(11.0));
@property(nonatomic, copy) UITextContentType textContentType API_AVAILABLE(ios(10.0));

@property(nonatomic, assign) UIAccessibilityElement* backingTextInputAccessibilityObject;

@end

@interface BoyiaTextInputView()
@end

@implementation BoyiaTextInputView {
}

@synthesize tokenizer = _tokenizer;

- (BOOL)hasText {
  return self.text.length > 0;
}

- (UITextPosition*)beginningOfDocument {
  return [BoyiaTextPosition positionWithIndex:0];
}

- (UITextPosition*)endOfDocument {
  return [BoyiaTextPosition positionWithIndex:self.text.length];
}

-(instancetype)init {
    self = [super init];
    if (self != nil) {
        // UITextInput
        _text = [[NSMutableString alloc] init];
        _markedText = [[NSMutableString alloc] init];
        _selectedTextRange = [[BoyiaTextRange alloc] initWithNSRange:NSMakeRange(0, 0)];
        //_markedRect = kInvalidFirstRect;
        //_cachedFirstRect = kInvalidFirstRect;
        
        // UITextInputTraits
        _autocapitalizationType = UITextAutocapitalizationTypeSentences;
        _autocorrectionType = UITextAutocorrectionTypeDefault;
        _spellCheckingType = UITextSpellCheckingTypeDefault;
        _enablesReturnKeyAutomatically = NO;
        _keyboardAppearance = UIKeyboardAppearanceDefault;
        _keyboardType = UIKeyboardTypeDefault;
        _returnKeyType = UIReturnKeyDone;
        _secureTextEntry = NO;
        
//        _accessibilityEnabled = NO;
//        _decommissioned = NO;
        if (@available(iOS 13.0, *)) {
            UITextInteraction* interaction =
              [UITextInteraction textInteractionForMode:UITextInteractionModeEditable];
            interaction.textInput = self;
            [self addInteraction:interaction];
        }
    }
    
    return nil;
}

#pragma mark - UIResponder Overrides
-(BOOL)canBecomeFirstResponder {
    return YES;
}

@end

@interface BoyiaTextInputViewHider : UIView
@end

@implementation BoyiaTextInputViewHider {
}

@end

#pragma mark - BoyiaGestureRecognizer
@interface BoyiaGestureRecognizer : UIPanGestureRecognizer
-(instancetype)initWithTarget:(id)target action:(SEL)action;

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
@property (nonatomic, strong) BoyiaTextInputView* textInputView;
@property (nonatomic, strong) BoyiaTextInputViewHider* textInputHider;

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
        
    self.displayLink = [CADisplayLink displayLinkWithTarget:self
                                                   selector:@selector(render:)];
    [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
    //self.inputView = [[BoyiaTextInputView alloc] init];
    self.textInputHider = [[BoyiaTextInputViewHider alloc] init];
    
    BoyiaTextInputView* textInputView = [[BoyiaTextInputView alloc] init];
    BOOL result = [textInputView canBecomeFirstResponder];
    [self addToInputParentViewIfNeeded:textInputView];
    
    BoyiaGestureRecognizer* gesture = [[BoyiaGestureRecognizer alloc]initWithTargetRenderer:self action:@selector(handlePanGesture:) renderer:self.renderer];
    [self.boyiaView addGestureRecognizer:gesture];
}

-(void)handlePanGesture:(UIPanGestureRecognizer*)gesture {
    CGPoint point = [gesture translationInView:gesture.view];
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
//    BOOL result1 = [self canBecomeFirstResponder];
//    BOOL result2 = [self becomeFirstResponder];
//    BOOL result2 = [self.textInputView becomeFirstResponder];
//
//    BOOL result3 = [self.boyiaView canBecomeFirstResponder];
//    BOOL result4 = [self.boyiaView becomeFirstResponder];
//
//    BOOL result5 = [self.textInputHider canBecomeFirstResponder];
//    BOOL result6 = [self.textInputHider becomeFirstResponder];
    
}

// 收到vsync信号之后进行渲染
-(void)render:(CADisplayLink*)link {

    //[self.renderer render];
    // 避免一直渲染造成性能损耗
    self.displayLink.paused = YES;
}

-(void)await {
    if (self.displayLink != nil) {
        self.displayLink.paused = NO;
    }
}

- (void)addToInputParentViewIfNeeded:(BoyiaTextInputView*)inputView {
  if (![inputView isDescendantOfView:self.textInputHider]) {
    [self.textInputHider addSubview:inputView];
  }
  UIView* parentView = self.boyiaView;
  if (self.textInputHider.superview != parentView) {
    [parentView addSubview:self.textInputHider];
  }
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

-(UIWindow*)getKeyWindow {
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

@end
