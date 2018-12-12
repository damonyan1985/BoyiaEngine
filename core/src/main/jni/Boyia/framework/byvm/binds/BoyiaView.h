/*
============================================================================
 Name        : BoyiaView.h
 Author      : yanbo
 Version     : BoyiaView v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#ifndef BoyiaView_h
#define BoyiaView_h

#include "HtmlView.h"
#include "BoyiaBase.h"
#include "KVector.h"

namespace boyia
{
class BoyiaView : public BoyiaBase
{
public:
	BoyiaView();
	virtual ~BoyiaView();
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

    virtual void addListener(LInt type, BoyiaValue* callback);

protected:
	yanbo::HtmlView* m_item;
};
}

#endif
