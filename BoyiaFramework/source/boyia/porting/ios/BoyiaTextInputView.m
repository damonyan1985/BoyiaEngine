//
//  BoyiaTextInputView.m
//  core
//
//  Created by yanbo on 2021/10/25.
//

#import "BoyiaTextInputView.h"
#import "IOSRenderer.h"

const CGRect kInvalidFirstRect = {{-1, -1}, {9999, 9999}};

NSRange RangeForCharactersInRange(NSString* text, NSRange range) {
  if (text == nil || range.location + range.length > text.length) {
    return NSMakeRange(NSNotFound, 0);
  }
  NSRange sanitizedRange = [text rangeOfComposedCharacterSequencesForRange:range];
  // We don't want to override the length, we just want to make sure we don't
  // select into the middle of a multi-byte character. Taking the
  // `sanitizedRange`'s length will end up altering the actual selection.
  return NSMakeRange(sanitizedRange.location, range.length);
}

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

@interface BoyiaTextRange : UITextRange<NSCopying>

@property(nonatomic, readonly) NSRange range;

+(instancetype)rangeWithNSRange:(NSRange)range;

@end

@implementation BoyiaTextRange

+(instancetype)rangeWithNSRange:(NSRange)range {
  return [[BoyiaTextRange alloc] initWithNSRange:range];
}

-(instancetype)initWithNSRange:(NSRange)range {
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
@interface BoyiaTextInputView()

@property (nonatomic, weak) IOSRenderer* renderer;

@end

@implementation BoyiaTextInputView {
    BoyiaTextRange* _selectedTextRange;
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

-(instancetype)initWithRenderer:(IOSRenderer*)renderer {
    self = [self init];
    if (self != nil) {
        self.renderer = renderer;
    }
    
    return self;
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
    
    return self;
}

-(BOOL)canBecomeFirstResponder {
    return YES;
}

// 删除键
-(void)deleteBackward {
    if (self.text.length == 0) {
        return;
    }
    
    if (_selectedTextRange.isEmpty && [self hasText]) {
        UITextRange* oldSelectedRange = _selectedTextRange;
        NSRange oldRange = ((BoyiaTextRange*)oldSelectedRange).range;
        if (oldRange.location > 0) {
            NSRange newRange = NSMakeRange(oldRange.location - 1, 1);
            _selectedTextRange = [[BoyiaTextRange rangeWithNSRange:newRange] copy];
        }
    }

    if (!_selectedTextRange.isEmpty) {
        [self replaceRange:_selectedTextRange withText:@""];
    }

    [self.renderer setInputText:self.text];
}

-(UITextRange*)selectedTextRange {
    return [_selectedTextRange copy];
}

-(void)resetText:(nonnull NSString *)text {
    //[self replaceRange:_selectedTextRange withText:@""];
    [self replaceRangeLocal:NSMakeRange(0, self.text.length) withText:@""];
    [self replaceRange:_selectedTextRange withText:text];
}

// 键盘输入时会调用
-(void)insertText:(nonnull NSString *)text {
    [self replaceRange:_selectedTextRange withText:text];
    [self.renderer setInputText:self.text];
}

- (NSWritingDirection)baseWritingDirectionForPosition:(nonnull UITextPosition *)position inDirection:(UITextStorageDirection)direction {
    return NSWritingDirectionNatural;
}

- (CGRect)caretRectForPosition:(nonnull UITextPosition *)position {
    return CGRectZero;
}

- (nullable UITextRange *)characterRangeAtPoint:(CGPoint)point {
    return nil;
}

- (nullable UITextRange *)characterRangeByExtendingPosition:(nonnull UITextPosition *)position inDirection:(UITextLayoutDirection)direction {
    return nil;
}

- (nullable UITextPosition *)closestPositionToPoint:(CGPoint)point {
    return nil;
}

- (nullable UITextPosition *)closestPositionToPoint:(CGPoint)point withinRange:(nonnull UITextRange *)range {
    return range.start;
}

- (NSComparisonResult)comparePosition:(nonnull UITextPosition *)position toPosition:(nonnull UITextPosition *)other {
    return NSOrderedSame;
}

- (CGRect)firstRectForRange:(nonnull UITextRange *)range {
    return kInvalidFirstRect;
}

- (NSInteger)offsetFromPosition:(nonnull UITextPosition *)from toPosition:(nonnull UITextPosition *)toPosition {
    return ((BoyiaTextPosition*)toPosition).index - ((BoyiaTextPosition*)from).index;
}

- (nullable UITextPosition *)positionFromPosition:(nonnull UITextPosition *)position inDirection:(UITextLayoutDirection)direction offset:(NSInteger)offset {
    return nil;
}

- (nullable UITextPosition *)positionFromPosition:(nonnull UITextPosition *)position offset:(NSInteger)offset {
    return nil;
}

- (nullable UITextPosition *)positionWithinRange:(nonnull UITextRange *)range farthestInDirection:(UITextLayoutDirection)direction {
    return nil;
}

- (void)replaceRange:(nonnull UITextRange *)range withText:(nonnull NSString *)text {
    NSRange replaceRange = ((BoyiaTextRange*)range).range;
    [self replaceRangeLocal:replaceRange withText:text];
}

-(void)replaceRangeLocal:(NSRange)range withText:(NSString*)text {
    NSRange selectedRange = _selectedTextRange.range;
    NSRange intersectionRange = NSIntersectionRange(range, selectedRange);
    if (range.location <= selectedRange.location)
        selectedRange.location += text.length - range.length;
    if (intersectionRange.location != NSNotFound) {
        selectedRange.location += intersectionRange.length;
        selectedRange.length -= intersectionRange.length;
    }

    [self.text replaceCharactersInRange:[self clampSelection:range forText:self.text]
                           withString:text];

    [self setSelectedTextRangeLocal:[BoyiaTextRange
                                          rangeWithNSRange:[self clampSelection:selectedRange
                                                                        forText:self.text]]];
    
    NSLog(@"BoyiaTextInputView text=%@", self.text);
}

- (void)setSelectedTextRangeLocal:(UITextRange*)selectedTextRange {
  if (_selectedTextRange != selectedTextRange) {
    UITextRange* oldSelectedRange = _selectedTextRange;
    if (self.hasText) {
        BoyiaTextRange* textRange = (BoyiaTextRange*)selectedTextRange;
      _selectedTextRange = [[BoyiaTextRange
          rangeWithNSRange:RangeForCharactersInRange(self.text, textRange.range)] copy];
    } else {
      _selectedTextRange = [selectedTextRange copy];
    }
  }
}

-(NSRange)clampSelection:(NSRange)range forText:(NSString*)text {
  int start = MIN(MAX(range.location, 0), text.length);
  int length = MIN(range.length, text.length - start);
  return NSMakeRange(start, length);
}

- (nonnull NSArray<UITextSelectionRect *> *)selectionRectsForRange:(nonnull UITextRange *)range {
    return @[];
}

- (void)setBaseWritingDirection:(NSWritingDirection)writingDirection forRange:(nonnull UITextRange *)range {
}

- (void)setMarkedText:(nullable NSString *)markedText selectedRange:(NSRange)selectedRange {
}

- (nullable NSString *)textInRange:(nonnull UITextRange *)range {
    return nil;
}

- (nullable UITextRange *)textRangeFromPosition:(nonnull UITextPosition *)fromPosition toPosition:(nonnull UITextPosition *)toPosition {
    return nil;
}

- (void)unmarkText {
}

- (CGRect)bounds {
  return super.bounds;
}

@end
/// BoyiaTextInputView结束

