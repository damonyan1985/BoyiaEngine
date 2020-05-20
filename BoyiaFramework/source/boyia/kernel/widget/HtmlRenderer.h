#ifndef HtmlRenderer_h
#define HtmlRenderer_h

#include "HtmlDocument.h"
#include "StyleParser.h"

namespace yanbo {
class ResourceLoader;
class HtmlRenderer {
public:
    HtmlRenderer();
    ~HtmlRenderer();

public:
    // layout all html items
    void renderHTML(HtmlDocument* doc,
        const String& buffer,
        ResourceLoader* loader);

    void layout();
    util::StyleManager* getStyleManager() const;
    void paint(HtmlView* item);
    util::StyleParser* getStyleParser() const;

private:
    util::StyleParser* m_styleParser;
    HtmlDocument* m_htmlDoc;
    LRect m_rect;
    ResourceLoader* m_loader;
};
}
#endif
