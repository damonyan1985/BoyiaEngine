/*
 * ResourceLoader.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef ResourceLoader_h
#define ResourceLoader_h

#include "HtmlRenderer.h"
#include "NetworkBase.h"

namespace yanbo {

class UIView;
class ResourceLoader {
public:
    enum LoadType {
        kHtmlDoc = 0,
        kCacheStyleSheet,
        kCacheScript
    };

public:
    ResourceLoader(UIView* view);
    virtual ~ResourceLoader();

public:
    LVoid onLoadError(LInt error);
    LVoid onLoadFinished(const String& data, LInt resType);
    LVoid onFileLen(LInt len);

public:
    LVoid load(const String& url, LoadType type);
    LVoid loadString(const String& src);

    LVoid repaint(HtmlView* item = NULL);

    LVoid executeDocument(const String& data);
    LVoid executeStyleSheet(const String& data);
    LVoid executeScript(const String& data);
    LVoid startupPage();
    HtmlRenderer* render() const;
    UIView* view() const;

protected:
    OwnerPtr<HtmlRenderer> m_render;
    UIView* m_view;
    LInt m_cssSize;
};
}
#endif /* RESOURCELOADER_H_ */
