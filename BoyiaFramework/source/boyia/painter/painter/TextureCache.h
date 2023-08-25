#ifndef MiniTextureCache_h
#define MiniTextureCache_h

#include "BoyiaPtr.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "HtmlView.h"
#include "KList.h"

#ifdef OPENGLES_3
#include <GLES3/gl3.h>
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

namespace yanbo {
class Texture : public BoyiaRef {
public:
    Texture();

    // For external texture
    LVoid initExternal(LInt width, LInt height);

    // 外接纹理需要attach
    LVoid attach(const HashPtr& ptr);

    // For image texture
    LVoid initWithData(const LVoid* data, LInt width, LInt height);

protected:
    // avoid to use delete to free memory
    // must be use boyia ptr to do that.
    ~Texture();

public:
    GLuint texId;
    LInt width;
    LInt height;
    LBool attached;
};

class TexturePair : public BoyiaRef {
public:
    TexturePair();

    ViewPainter* item;
    BoyiaPtr<Texture> tex;
};

// 文本产生的纹理
typedef KList<BoyiaPtr<TexturePair>> TextTextureCache;
// 图片产生的纹理
typedef HashMap<HashString, BoyiaPtr<Texture>> ImageTextureCache;
// 外接纹理
typedef HashMap<HashPtr, BoyiaPtr<Texture>> ExternalTextureCache;

class TextureCache {
public:
    static TextureCache* getInst();
    LVoid clear();

    LVoid updateText(Texture* tex, const LImage* image);
    // 查找文本产生的纹理
    Texture* findText(const ViewPainter* item);
    Texture* createText(ViewPainter* item, const LImage* image);

    Texture* putImage(const LImage* image);
    Texture* putImage(const String& url, const LVoid* pixels, LInt width, LInt height);
    Texture* findImage(const String& url);

    Texture* createExternal(const HashPtr& ptr, LInt width, LInt height);
    Texture* findExternal(const HashPtr& ptr);

private:
    ImageTextureCache m_imageCache;
    TextTextureCache m_textCache;
    ExternalTextureCache m_externalCache;
};
}
#endif
