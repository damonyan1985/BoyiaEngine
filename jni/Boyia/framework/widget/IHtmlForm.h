/*
 * IHtmlForm.h
 *
 *  Created on: 2011-7-8
 *      Author: yanbo
 */

#ifndef IHtmlForm_h
#define IHtmlForm_h

#include "UtilString.h"

namespace yanbo
{

class IHtmlForm
{
public:
	virtual String getAction() = 0;
	
	virtual String getMethod() = 0;
	
	virtual String encodeFormViews() = 0;
	
	virtual String getEncoding() = 0;
	
	virtual String getItemValue(const String& itemName) = 0;
	virtual String getItemMimeType(const String& itemName) = 0;
	virtual void setItemValue(const String& name, const String& value) = 0;
	virtual LBool hasPassword() = 0;
	virtual void postForm(String& outstream) = 0;
};

}
#endif /* IHtmlForm_h */
