#ifndef JSViewGroup_h
#define JSViewGroup_h

#include "JSView.h"
namespace mjs
{
class JSViewGroup : public JSView
{
public:
	JSViewGroup(yanbo::HtmlView* item);
	JSViewGroup(const String& id, LBool selectable);
	LVoid setText(const String& text);
	void appendView(JSView* view);
};
}

#endif
