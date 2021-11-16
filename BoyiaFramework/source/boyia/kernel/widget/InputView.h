/*
 * InputView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef InputView_h
#define InputView_h

#include "FormView.h"
#include "LGdi.h"
#include "TextView.h"

namespace yanbo {

class InputView : public FormView {
public:
    enum InputType {
        kInputNone,
        kInputText,
        kInputPassword,
        kInputCheckbox,
        kInputRadio,
        kInputSubmit,
        kInputReset,
        kInputFile,
        kInputHidden,
        kInputImage,
        kInputButton
    };

public:
    InputView(
        const String& id,
        const String& name,
        const String& value,
        const String& title,
        const String& imageUrl);

    static InputView* create(
        const String& id,
        const String& name,
        const String& value,
        const String& title,
        const String& inputType,
        const String& imageUrl);

public:
    virtual LVoid execute();

    virtual LInt getInputType();
    virtual LVoid setInputValue(const String& text);

protected:
    virtual ~InputView();

    LVoid initView();
    LVoid layoutBegin(RenderContext& rc);
    LVoid layoutEnd(RenderContext& rc);
    
    LVoid paintBegin(LGraphicsContext& gc, LayoutPoint& point);
    
    LVoid layoutText();
    
    //virtual LBool isReplaced() const;

    LInt m_type;
    LBool m_checked;
    LBool m_activated;
    LFont* m_newFont;
    // inputview包含一个textview
    TextView* m_text;
};
}
#endif /* InputView_h */
