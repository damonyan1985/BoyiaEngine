#ifndef HtmlView_h
#define HtmlView_h

// all render base class
#include "LGdi.h"
#include "KList.h"
#include "LBaseView.h"
#include "IViewListener.h"
#include "HtmlTags.h"
#include "CssManager.h"

namespace yanbo
{

class HtmlView;
class RenderContext;
class HtmlDocument;
class BlockView;
typedef KList<HtmlView*> HtmlViewList;
typedef LRect LayoutRect;
typedef LInt  LayoutUnit;
typedef LPoint LayoutPoint;

class HtmlView : public util::LBaseView
{
public:
	HtmlView(const String& id, LBool selectable);
	virtual ~HtmlView();
	
public:
	virtual void layout(RenderContext& rc);
	
	// prepare to reconstruct the layout
	virtual void layout();
	void relayout();

	virtual void paint(LGraphicsContext& gc);
	virtual void paintBorder(LGraphicsContext& gc, const util::Border& border, LayoutUnit x, LayoutUnit y);
	virtual LBool isText() const;
	virtual LBool isLink() const;
	virtual LBool isBlockView() const;
	virtual LBool isImage() const;
	virtual LBool isStyle() const;
	virtual LBool isJavaScript() const;
	virtual void execute();
	virtual void setStyle(util::CssManager* manager, util::CssRule* parentRule);
	
	void setId(const String& id);
	const String& getId() const;
	
	void setClassName(const String& className);
	const String& getClassName() const;
	
	void setTagType(HtmlTags::HtmlType tagType);
	HtmlTags::HtmlType getTagType() const;
	
	virtual void setSelected(const LBool selected);
	LBool isSelected();
	LBool isSelectable();
	
	void itemCenter(RenderContext& rc);
	virtual void layoutInline(RenderContext& rc);
	
	HtmlView* getPreItem();
	HtmlView* getNextItem();
	BlockView* getContainingBlock() const;

	LBool isViewRoot() const;
	void setIsViewRoot(LBool isViewRoot);

	LBool hasTransform() const;

	virtual LBool isReplaced() const;

	virtual LBool isInline() const;

	virtual void addChild(HtmlView* child);
	void removeChild(HtmlView* child);

	void setParent(HtmlView* view);
	HtmlView* getParent() const;

	LBool isPositioned() const;
	LBool isRelPositioned() const;
	util::Style* getStyle() const;

	LayoutPoint getAbsoluteContainerTopLeft() const;
	void setDocument(HtmlDocument* doc);
	HtmlDocument* getDocument() const;

	LVoid setPainter(LVoid* painter);
	LVoid* painter() const;

	LBool isClipItem() const;
	LVoid setClipRect(LGraphicsContext& gc);
	LayoutRect clipRect() const;

	LVoid relayoutZIndexChild();

	LVoid setListener(IViewListener* listener);
	IViewListener* getListener() const;

	LVoid setViewIter(const HtmlViewList::Iterator& iter);
	const HtmlViewList& getChildren() const;

private:
	HtmlView* getPreItem(HtmlView* currentItem);
	HtmlView* getNextItem(HtmlView* currentItem);
	
protected:
	void handleXYPos(RenderContext& rc);
	
public:	
	HtmlViewList     m_children;
	
protected:
    /**
     * The listener who will receive event callbacks.
     */
    IViewListener*            m_itemListener;
    String                    m_id;
    
    // html type
    HtmlTags::HtmlType        m_type;
    /**  True if this item is selected */
    LBool                     m_selected;
    HtmlDocument*             m_doc;
    
    String                    m_className;
	LBool                     m_isViewRoot;
	LVoid*                    m_painter;
	LBool                     m_clip;
	LayoutRect                m_clipRect;
	LBool                     m_needLayout;
	HtmlViewList::Iterator    m_iter; // 保留iter信息便于删除
	HtmlView*                 m_parent;
};

}

#endif
