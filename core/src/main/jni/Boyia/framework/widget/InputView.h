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

namespace yanbo
{

class InputView : public FormView
{
public:
    enum InputType
    {
    	NOTSUPPORTED,
    	TEXT,
    	PASSWORD,
    	CHECKBOX,
    	RADIO,
    	SUBMIT,
    	RESET,
    	FILE,
    	HIDDEN,
    	IMAGE,
    	BUTTON
    };
    
private:
    LInt                 m_type;
    LBool                m_checked;
    LBool                m_activated;
    LFont*               m_newFont;
    
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
	virtual void layout(RenderContext& rc);
	virtual void paint(LGraphicsContext& dc);
	virtual void execute();
	
	int getInputType();
	virtual void setSelected(const LBool selected);
	void setInputValue(const String& text);

private:
	void paintTextBox(LGraphicsContext& gc, LayoutUnit x, LayoutUnit y);
	void paintButton(LGraphicsContext& gc, LayoutUnit x, LayoutUnit);
	void paintRadioButton(LGraphicsContext& gc, LayoutUnit x, LayoutUnit);
	void paintCheckBox(LGraphicsContext& gc, LayoutUnit x, LayoutUnit);
};

}
#endif /* InputView_H_ */
