#ifndef MiniTextureCache_h
#define MiniTextureCache_h

#include "BoyiaPtr.h"
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

    HtmlView* item;
    BoyiaPtr<Texture> tex;
};

typedef KList<BoyiaPtr<TexturePair>> TextureMap;
class TextureCache {
public:
    static TextureCache* getInst();
    LVoid clear();
    Texture* put(const LImage* image);
    Texture* find(LVoid* image);
    Texture* updateTexture(const LImage* image);
    LVoid updateTexture(Texture* tex, const LImage* image);

private:
    Texture* fetchTexture(HtmlView* item, const LRect& rect, const LImage* image, LUint key);
    Texture* find(HtmlView* item, LUint key);

    TextureMap m_texMap;
};
}
#endif
