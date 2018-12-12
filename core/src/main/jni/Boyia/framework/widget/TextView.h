/*
 * TextView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef TextView_h
#define TextView_h

#include "InlineView.h"

namespace yanbo
{

class TextView : public InlineView
{
public:
	TextView(
			const String& id,
			const String& text,
			LBool selectable);
	
	~TextView();
	
public:
	virtual void layout(RenderContext& rc);
	virtual void paint(LGraphicsContext& dc);
	virtual LBool isText() const;
	
	void setAlignement(LGraphicsContext::TextAlign alignement);
	void setText(const String& text);
	const String& text() const {return m_text;}
	LInt calcTextLine(const String& text, LInt maxWidth);
	LBool isMultiLine();
	HtmlView* getLinkItem();
	
private:
	class Line
	{
	public:
		Line(LInt length, const String& text);
		~Line();
		
	public:
		LInt m_lineLength;
		String m_text;
	};
	
protected:
	String m_text;
	KVector<Line*> *m_textLines;
	LFont* m_newFont;
	LInt m_maxWidth;
};

}
#endif /* TextView_H_ */
