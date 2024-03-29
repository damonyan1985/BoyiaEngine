#include "TextureCache.h"
#include "AutoObject.h"
#include "ImageView.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "JNIUtil.h"

namespace yanbo {
Texture::Texture()
    : texId(0)
    , width(0)
    , height(0)
    , attached(LFalse)
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &texId);
}

LVoid Texture::attach(const HashPtr& ptr)
{
    if (attached) {
        return;
    }

    BOYIA_LOG("Texture attach id=%ld and tid=%d", (jlong)ptr.value(), (jint)texId);
    JNIUtil::callStaticVoidMethod(
        "com/boyia/app/core/BoyiaBridge",
        "attachTexture",
        "(JI)V", (jlong)ptr.value(), (jint)texId);

    attached = LTrue;
}

LVoid Texture::initExternal(LInt width, LInt height)
{
    this->width = width;
    this->height = height;
    glGenTextures(1, &texId);

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);

    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Must call this function to generate texture
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA, // RGB 3, RGBA 4
        this->width,
        this->height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        kBoyiaNull);
}

LVoid Texture::initWithData(const LVoid* data, LInt width, LInt height)
{
    this->width = width;
    this->height = height;

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
        this->width,
        this->height,
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

// 查找文字纹理
Texture* TextureCache::findText(const ViewPainter* item)
{
    TextTextureCache::Iterator iter = m_textCache.begin();
    TextTextureCache::Iterator iterEnd = m_textCache.end();

    for (; iter != iterEnd; ++iter) {
        if (item == (*iter)->item) {
            return (*iter)->tex.get();
        }
    }

    return kBoyiaNull;
}

// 创建文字纹理
Texture* TextureCache::createText(ViewPainter* item, const LImage* image)
{
    TexturePair* pair = new TexturePair;
    pair->item = item;

    pair->tex = new Texture();
    pair->tex->initWithData(image->pixels(), image->width(), image->height());
    m_textCache.push(pair);

    return pair->tex.get();
}

// 更新文字纹理
LVoid TextureCache::updateText(Texture* tex, const LImage* image)
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

LVoid TextureCache::clear()
{
    m_imageCache.clear();
    m_textCache.clear();
}

Texture* TextureCache::findImage(const String& url)
{
    return m_imageCache.get(HashString(url, LFalse)).get();
}

Texture* TextureCache::putImage(const LImage* image)
{
    Texture* tex = findImage(image->url());
    if (!tex) {
        tex = new Texture();
        tex->initWithData(image->pixels(), image->width(), image->height());

        m_imageCache.put(HashString(image->url()), tex);
    }

    return tex;
}

Texture* TextureCache::putImage(const String& url, const LVoid* pixels, LInt width, LInt height)
{
    Texture* tex = findImage(url);
    if (!tex) {
        tex = new Texture();
        tex->initWithData(pixels, width, height);

        m_imageCache.put(HashString(url), tex);
    }

    return tex;
}

Texture* TextureCache::createExternal(const HashPtr& ptr, LInt width, LInt height)
{
    Texture* tex = new yanbo::Texture();
    tex->initExternal(width, width);
    m_externalCache.put(ptr, tex);
    return tex;
}

Texture* TextureCache::findExternal(const HashPtr& ptr)
{
    return m_externalCache.get(ptr).get();
}
} // namespace yanbo
