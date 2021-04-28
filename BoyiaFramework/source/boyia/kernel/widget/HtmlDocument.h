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

namespace yanbo {

class UIView;
class HtmlDocument {
public:
    HtmlDocument();
    ~HtmlDocument();

public:
    void setRenderTreeRoot(HtmlView* root);
    HtmlView* getRenderTreeRoot() const;

    void setPageTitle(const String& titleText);
    const String& getPageTitle() const;

    void setPageUrl(const String& url);
    const String& getPageUrl() const;

    HtmlView* getPreItem();
    HtmlView* getNextItem();

    HtmlView* getCurrentItem();
    void addHtmlView(HtmlView* item);

    void resetHtmlFocus();
    void clearHtmlList();

    const LayoutRect& getViewPort() const;

    void putItemID(String& id, HtmlView* item);
    HtmlView* getItemByID(const String& id);

    void setView(UIView* loader);
    UIView* getView() const;

private:
    /**+
     * The URL of this page.
     */
    String m_url;

    /**
     *  The title text of this page.
     */
    String m_title;

    HtmlView* m_root;

    HtmlViewList m_itemList;
    HtmlViewList::Iterator m_currentItemIter; // control up, down key event display
    LayoutRect m_viewRect;
    HashMap<HashString, HtmlView*> m_idMap;
    UIView* m_view;
};
}
#endif /* HTMLDOCUMENT_H_ */
