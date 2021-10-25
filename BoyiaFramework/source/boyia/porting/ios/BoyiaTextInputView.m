//
//  BoyiaTextInputView.m
//  core
//
//  Created by yanbo on 2021/10/25.
//

#import "BoyiaTextInputView.h"

const CGRect kInvalidFirstRect = {{-1, -1}, {9999, 9999}};

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
@end

@implementation BoyiaTextInputView

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
    
    return self;
}

-(BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)deleteBackward {
}

- (void)insertText:(nonnull NSString *)text {
    
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

