#include "InlineView.h"

namespace yanbo
{

InlineView::InlineView(const String& id, LBool selectable)
    : HtmlView(id, selectable)
{
}

InlineView::~InlineView()
{	
}

void InlineView::addChild(HtmlView* child)
{
	addChildIgnoringContinuation(child);
}

void InlineView::addChildToContinuation(HtmlView* child)
{
}

void InlineView::addChildIgnoringContinuation(HtmlView* child)
{
	HtmlView::addChild(child);
}

LBool InlineView::isInline() const
{
	return LTrue;
}

}
