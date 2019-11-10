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

private:
    LInt m_type;
    LBool m_checked;
    LBool m_activated;
    LFont* m_newFont;

public:
    InputView(
        const String& id,
        const String& name,
        const String& value,
        const String& title,
        const String& inputType,
        const String& imageUrl);

    ~InputView();

public:
    virtual LVoid layout(RenderContext& rc);
    virtual LVoid paint(LGraphicsContext& dc);
    virtual LVoid execute();

    LInt getInputType();
    virtual LVoid setSelected(const LBool selected);
    LVoid setInputValue(const String& text);

private:
    LVoid initView();
    LVoid paintTextBox(LGraphicsContext& gc, LayoutUnit x, LayoutUnit y);
    LVoid paintButton(LGraphicsContext& gc, LayoutUnit x, LayoutUnit);
    LVoid paintRadioButton(LGraphicsContext& gc, LayoutUnit x, LayoutUnit);
    LVoid paintCheckBox(LGraphicsContext& gc, LayoutUnit x, LayoutUnit);
};
}
#endif /* InputView_h */
