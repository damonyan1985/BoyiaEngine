#ifndef HtmlView_h
#define HtmlView_h

// all render base class
#include "BoyiaPtr.h"
#include "HtmlTags.h"
#include "IViewListener.h"
#include "KList.h"
#include "LBaseView.h"
#include "LGdi.h"
#include "StyleManager.h"

namespace yanbo {

class HtmlView;
class RenderContext;
class HtmlDocument;
class BlockView;
typedef BoyiaList<BoyiaPtr<HtmlView>> HtmlViewList;
typedef LRect LayoutRect;
typedef LInt LayoutUnit;
typedef LPoint LayoutPoint;

class HtmlView : public LBaseView, public BoyiaRef {
public:
    HtmlView(const String& id, LBool selectable);

    // layout时计算相对坐标
    virtual LVoid layout(RenderContext& rc);
    virtual LVoid layout();
    LVoid relayout();

    // 绘制时根据元素的相对坐标计算绝对坐标
    virtual LVoid paint(LGraphicsContext& gc);
    virtual LVoid paintBorder(LGraphicsContext& gc, const util::Border& border, LayoutUnit x, LayoutUnit y);
    virtual LBool isText() const;
    virtual LBool isLink() const;
    virtual LBool isBlockView() const;
    virtual LBool isImage() const;
    virtual LBool isStyle() const;
    virtual LVoid execute();
    virtual LVoid setStyle(util::StyleManager* manager, util::StyleRule* parentRule);

    LVoid setId(const String& id);
    const String& getId() const;

    LVoid setClassName(const String& className);
    const String& getClassName() const;

    LVoid setTagType(HtmlTags::HtmlType tagType);
    HtmlTags::HtmlType getTagType() const;

    virtual LVoid setSelected(const LBool selected);
    LBool isSelected();
    LBool isSelectable();

    LVoid itemCenter(RenderContext& rc);
    virtual LVoid layoutInline(RenderContext& rc);

    HtmlView* getPreItem();
    HtmlView* getNextItem();
    BlockView* getContainingBlock() const;

    LBool isViewRoot() const;
    LVoid setIsViewRoot(LBool isViewRoot);

    LBool hasTransform() const;

    virtual LBool isReplaced() const;

    virtual LBool isInline() const;

    virtual LVoid addChild(HtmlView* child);
    LVoid removeChild(HtmlView* child);

    LVoid setParent(HtmlView* view);
    HtmlView* getParent() const;

    LBool isPositioned() const;
    LBool isRelPositioned() const;
    util::Style* getStyle() const;

    LayoutPoint getAbsoluteContainerTopLeft() const;
    LVoid setDocument(HtmlDocument* doc);
    HtmlDocument* getDocument() const;

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
    // avoid to use delete to free memory
    // must be use boyia ptr to do that.
    virtual ~HtmlView();
    LVoid handleXYPos(RenderContext& rc);

public:
    HtmlViewList m_children;

protected:
    /**
     * The listener who will receive event callbacks.
     */
    IViewListener* m_itemListener;
    String m_id;

    // html type
    HtmlTags::HtmlType m_type;
    /**  True if this item is selected */
    LBool m_selected;
    HtmlDocument* m_doc;

    String m_className;
    LBool m_isViewRoot;
    LBool m_clip;
    LayoutRect m_clipRect;
    LBool m_needLayout;
    HtmlViewList::Iterator m_iter; // 保留iter信息便于删除
    HtmlView* m_parent;
};
}

#endif
