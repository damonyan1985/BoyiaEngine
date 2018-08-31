/*
 * FormView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef FormView_h
#define FormView_h

#include "InlineView.h"

namespace yanbo
{

class HtmlForm;
class FormView : public InlineView
{
public:
	FormView(const String& id,
			const String& name,
			const String& inValue,
			const String& inTitle);
	
	virtual ~FormView();
	
public:
	//virtual void layout(RenderContext& rc);
	void setValue(const String& inValue);
	const String& getValue() const;
	
	void setTitle(const String& title);
	const String& getTitle() const;
	void setParentForm(HtmlForm* form);
	const String& getMimeType() const;
	const String getEncodingParams() const;
	
	void setName(const String& name);
	const String& getName() const;

protected:
	String   m_name;
	String   m_value;
	String   m_title;
	String   m_mimeType;
	HtmlForm  *m_parentForm;
};
}
#endif /* FormView_H_ */
