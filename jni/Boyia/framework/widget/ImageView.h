/*
 * ImageView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef ImageView_h
#define ImageView_h

#include "InlineView.h"

namespace yanbo
{
class ImageView : public InlineView
{
public:
	ImageView(
			const String& id,
			LBool selectable,
			const String& src);
	
	~ImageView();
	
public:
	virtual LBool isImage() const;
	const String& url() const;
	void setUrl(const String& url);
	void loadImage(const String& url);
	
	virtual void paint(LGraphicsContext& gc);
	virtual void layoutInline(RenderContext& rc);
	
private:
	LImage* m_image;
	String m_src;
};

}

#endif /* ImageView_H_ */
