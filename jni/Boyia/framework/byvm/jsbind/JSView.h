/*
============================================================================
 Name        : JSView.h
 Author      : yanbo
 Version     : JSView v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#ifndef JSView_h
#define JSView_h

#include "HtmlView.h"
#include "JSBase.h"
#include "KVector.h"

namespace mjs
{

class JSView : public JSBase
{
public:
	JSView();
	virtual ~JSView();
	void setX(LInt x);
	void setY(LInt y);

	LInt left() const;
	LInt top() const;

	LInt width() const;
	LInt height() const;

	void setStyle(String& cls);

	yanbo::HtmlView* item() const;

	void drawView();
	void commit();
	// 动画
	void startScale(LInt scale, LInt duration);
	void startOpacity(LInt opacity, LInt duration);
    void startTranslate(const LPoint& point, LInt duration);

    virtual void addListener(LInt type, MiniValue* callback);

protected:
	yanbo::HtmlView* m_item;
};
}

#endif
