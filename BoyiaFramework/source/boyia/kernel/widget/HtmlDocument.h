/*
 * HtmlDocument.h
 *
 *  Created on: 2011-6-22
 *      Author: yanbo
 */

#ifndef HtmlDocument_h
#define HtmlDocument_h

#include "HtmlView.h"
//#include "IdentityMap.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "WeakPtr.h"

namespace yanbo {
typedef BoyiaList<WeakPtr<HtmlView>> WeakHtmlViewList;

class UIView;
class HtmlDocument {
public:
    HtmlDocument();
    ~HtmlDocument();

public:
    LVoid setRenderTreeRoot(HtmlView* root);
    HtmlView* getRenderTreeRoot() const;

    LVoid setPageTitle(const String& titleText);
    const String& getPageTitle() const;

    LVoid setPageUrl(const String& url);
    const String& getPageUrl() const;

    HtmlView* getPreItem();
    HtmlView* getNextItem();

    HtmlView* getCurrentItem();
    LVoid addHtmlView(HtmlView* item);

    LVoid resetHtmlFocus();
    LVoid clearHtmlList();

    const LayoutRect& getViewPort() const;

    LVoid putItemID(String& id, HtmlView* item);
    HtmlView* getItemByID(const String& id);

    LVoid setView(UIView* view);
    UIView* getView() const;
    
    LVoid setFocusView(HtmlView* view);

private:
    /**+
     * The URL of this page.
     */
    String m_url;

    /**
     *  The title text of this page.
     */
    String m_title;

    BoyiaPtr<HtmlView> m_root;

    WeakHtmlViewList m_itemList;
    WeakHtmlViewList::Iterator m_currentItemIter; // control up, down key event display
    LayoutRect m_viewRect;
    HashMap<HashString, WeakPtr<HtmlView>> m_idMap;
    UIView* m_view;
    
    WeakPtr<HtmlView> m_focus;
};
}
#endif /* HTMLDOCUMENT_H_ */
