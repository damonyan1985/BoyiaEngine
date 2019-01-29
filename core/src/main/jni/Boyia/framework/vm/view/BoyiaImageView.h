#ifndef BoyiaImageView_h
#define BoyiaImageView_h

#include "BoyiaView.h"
namespace boyia
{
class BoyiaImageView : public BoyiaView
{
public:
	BoyiaImageView(yanbo::HtmlView* item);
	void setImageUrl(const String& url);
	void loadImage(const String& url);
};
}

#endif
