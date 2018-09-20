/*
 * HtmlForm.h
 *
 *  Created on: 2011-7-1
 *      Author: yanbo
 */

#ifndef HtmlForm_h
#define HtmlForm_h


#include "BlockView.h"
#include "KList.h"
#include "FormView.h"
#include "IHtmlForm.h"

namespace yanbo
{

typedef KList<FormView*> FormViewList;
class HtmlForm : public BlockView,
        public IHtmlForm
{
public:
	HtmlForm(const String& id,
			const String& action,
			const String& method,
			const String& encoding,
			LBool selectable);
	
	virtual ~HtmlForm();
	
public:
	virtual void layout(RenderContext& rc);
	void addFormView(FormView* aFormView);
	const FormViewList& getFormViews() const;
	void reset();
	FormView* getFormView(const String& itemName);
	void UnCheckRadioButtons(const String& name);
	LInt getLastSubmitTime();
	
public:
	/** Return url to post to. */
	virtual String getAction();
	
	/** Return GET or POST. */
	virtual String getMethod();
	
	/** Return URL encoded parameter string. */
	virtual String encodeFormViews();
	
	/** Return encoding for From. */
	virtual String getEncoding();
	
	virtual String getItemValue(const String& itemName);
	virtual String getItemMimeType(const String& itemName);
	virtual void setItemValue(const String& name, const String& value);
	virtual LBool hasPassword();
	
	/** Write the form data to the output stream. */
	virtual void postForm(String& outstream);
	
private:
	FormViewList               m_formList;
	String                    m_action;
	String                    m_method;
	String                    m_encoding;
	String                    m_boundaryString;
	LInt                       m_lastSubmitTime;
};

}
#endif /* HTMLFORM_H_ */
