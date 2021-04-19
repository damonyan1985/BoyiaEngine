#ifndef MiniTextureCache_h
#define MiniTextureCache_h

#include "BoyiaPtr.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "HtmlView.h"
#include "KList.h"
#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>
#define GL_TEXTURE_EXTERNAL_OES 0x8D65

namespace yanbo {
class Texture : public BoyiaRef {
public:
    Texture();

    // For external texture
    LVoid initWithData(LInt width, LInt height);

    // For image texture
    LVoid initWithData(LVoid* data, LInt width, LInt height);

    virtual ~Texture();

    GLuint texId;
    LInt width;
    LInt height;
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
class TextureCache {
public:
    static TextureCache* getInst();
    LVoid clear();

    LVoid updateText(Texture* tex, const LImage* image);
    // 查找文本产生的纹理
    Texture* findText(const ViewPainter* item);
    Texture* createText(ViewPainter* item, const LImage* image);

    Texture* putImage(const LImage* image);
    Texture* findImage(const String& url);

private:
    ImageTextureCache m_imageCache;
    TextTextureCache m_textCache;
};
}
#endif
