#ifndef MiniTextureCache_h
#define MiniTextureCache_h

#include "BoyiaPtr.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "HtmlView.h"
#include "KList.h"
#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>

namespace yanbo {
class Texture : public BoyiaRef {
public:
    Texture();
    LVoid initWithData(LVoid* data, LUint key, LInt width, LInt height);

    virtual ~Texture();

    GLuint texId;
    LUint texKey;
    LInt width;
    LInt height;
};

class TexturePair : public BoyiaRef {
public:
    TexturePair();

    ViewPainter* item;
    BoyiaPtr<Texture> tex;
};

typedef KList<BoyiaPtr<TexturePair>> TextureMap;
typedef HashMap<HashString, BoyiaPtr<Texture>> ImageTextureCache;
class TextureCache {
public:
    static TextureCache* getInst();
    LVoid clear();
    Texture* put(const LImage* image);
    Texture* find(LVoid* image);
    Texture* updateTexture(const LImage* image);
    LVoid updateTexture(Texture* tex, const LImage* image);
    // 查找文本产生的纹理
    Texture* findText(const ViewPainter* item);
    Texture* fetchTexture(ViewPainter* item, const LImage* image, LUint key);

    Texture* putImage(const LImage* image);
    Texture* findImage(const String& url);

private:
    Texture* find(HtmlView* item, LUint key);

    ImageTextureCache m_imageCache;
    TextureMap m_texMap;
};
}
#endif
