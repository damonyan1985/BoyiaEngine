#include "TextureCache.h"
#include "AutoObject.h"
#include "ImageView.h"
#include "SalLog.h"
#include "StringUtils.h"

namespace yanbo {
Texture::Texture()
    : texId(0)
    , width(0)
    , height(0)
    , texKey(0)
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &texId);
}

LVoid Texture::initWithData(LVoid* data, LUint key, LInt width, LInt height)
{
    texKey = key;
    glGenTextures(1, &texId);

    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA, // RGB 3, RGBA 4
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);
}

TexturePair::TexturePair()
    : item(kBoyiaNull)
{
}

TextureCache* TextureCache::getInst()
{
    static TextureCache sCache;
    return &sCache;
}

Texture* TextureCache::find(LVoid* image)
{
    LUint key = 0;
    HtmlView* item = (HtmlView*)image;
    if (item->isImage()) {
        ImageView* imgItem = (ImageView*)item;
        key = StringUtils::hashCode(imgItem->url());
    }

    return find(item, key);
}

Texture* TextureCache::put(const LImage* image)
{
    LUint key = 0;
    HtmlView* item = (HtmlView*)image->item();
    KFORMATLOG("MiniTextureCache::put isImage=%d", item->isImage());
    if (item->isImage()) {
        ImageView* imgItem = (ImageView*)item;
        key = StringUtils::hashCode(imgItem->url());
        KFORMATLOG("MiniTextureCache::put tex=%x", key);
    }

    Texture* tex = find(item, key);

    if (!tex) {
        tex = fetchTexture(item, image, key);
    }

    return tex;
}

Texture* TextureCache::find(HtmlView* item, LUint key)
{
    TextureMap::Iterator iter = m_texMap.begin();
    TextureMap::Iterator iterEnd = m_texMap.end();

    for (; iter != iterEnd; ++iter) {
        // 0表示是文本产生的纹理
        if (key == 0) {
            if (item == (*iter)->item) {
                return (*iter)->tex.get();
            }
        } else {
            if ((*iter)->tex->texKey == key) {
                return (*iter)->tex.get();
            }
        }
    }

    return kBoyiaNull;
}

Texture* TextureCache::findText(const ViewPainter* item)
{
    TextureMap::Iterator iter = m_texMap.begin();
    TextureMap::Iterator iterEnd = m_texMap.end();

    for (; iter != iterEnd; ++iter) {
        if (item == (*iter)->item) {
            return (*iter)->tex.get();
        }
    }

    return kBoyiaNull;
}

Texture* TextureCache::fetchTexture(ViewPainter* item, const LImage* image, LUint key)
{
    TexturePair* pair = new TexturePair;
    pair->item = item;

    pair->tex = new Texture();
    pair->tex->initWithData(image->pixels(), key, image->width(), image->height());
    m_texMap.push(pair);

    return pair->tex.get();
}

LVoid TextureCache::updateTexture(Texture* tex, const LImage* image)
{
    if (tex) {
        glBindTexture(GL_TEXTURE_2D, tex->texId);

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0, // level
            0, // xoffset
            0, // yoffset
            image->width(), // width
            image->height(), // height
            GL_RGBA, // format
            GL_UNSIGNED_BYTE, // type
            image->pixels());
    }
}

Texture* TextureCache::updateTexture(const LImage* image)
{
    Texture* tex = find(image->item());
    updateTexture(tex, image);
    return tex;
}

LVoid TextureCache::clear()
{
    m_texMap.clear();
}
} // namespace yanbo
