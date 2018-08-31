#ifndef JSImageView_h
#define JSImageView_h

#include "JSView.h"
namespace mjs
{
class JSImageView : public JSView
{
public:
	JSImageView(yanbo::HtmlView* item);
	void setImageUrl(const String& url);
	void loadImage(const String& url);
};
}

#endif
