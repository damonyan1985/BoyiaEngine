/*
 * IHtmlForm.h
 *
 *  Created on: 2011-7-8
 *      Author: Administrator
 */

#ifndef IHtmlForm_h
#define IHtmlForm_h

#include "UtilString.h"

namespace yanbo
{

/**
 * Objects of this class can be sent to the server as
 * a form that has been filled out in the browser. One subclass
 * implements a true Html Form while another is just some stored data
 * that is sent to the server pretending to be a filled in form.
 */
class IHtmlForm
{
public:
	/** Return url to post to. */
	virtual String getAction() = 0;
	
	/** Return GET or POST. */
	virtual String getMethod() = 0;
	
	/** Return URL encoded parameter string. */
	virtual String encodeFormViews() = 0;
	
	/** Return encoding for From. */
	virtual String getEncoding() = 0;
	
	virtual String getItemValue(const String& itemName) = 0;
	virtual String getItemMimeType(const String& itemName) = 0;
	virtual void setItemValue(const String& name, const String& value) = 0;
	virtual LBool hasPassword() = 0;
	
	/** Write the form data to the output stream. */
	virtual void postForm(String& outstream) = 0;
};

}
#endif /* IHTMLFORM_H_ */
