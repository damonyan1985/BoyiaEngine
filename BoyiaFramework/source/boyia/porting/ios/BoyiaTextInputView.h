//
//  BoyiaInputTextView.h
//  core
//
//  Created by yanbo on 2021/10/25.
//

#ifndef BoyiaInputTextView_h
#define BoyiaInputTextView_h

#import <UIKit/UIKit.h>

@class IOSRenderer;

// BoyiaTextInputView
@interface BoyiaTextInputView : UIView<UITextInput>

-(instancetype)initWithRenderer:(IOSRenderer*)renderer;
-(void)resetText:(nonnull NSString *)text cursor:(NSUInteger)cursor;

// UITextInput
@property(nonatomic, readonly) NSMutableString* text;
@property(nonatomic, readonly) NSMutableString* markedText;
@property(nonatomic, readwrite, copy) UITextRange* selectedTextRange;
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

// 修改inputAccessoryView相关属性
@property (nullable, readwrite, strong) UIView *inputAccessoryView;

@end


#endif /* BoyiaInputTextView_h */
