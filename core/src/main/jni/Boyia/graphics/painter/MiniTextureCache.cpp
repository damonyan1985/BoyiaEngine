#include "MiniTextureCache.h"
#include "AutoObject.h"
#include "StringUtils.h"
#include "SalLog.h"
#include "ImageView.h"

namespace yanbo
{
MiniTextureCache* MiniTextureCache::s_instance = NULL;

MiniTexture::MiniTexture()
    : texId(0)
    , width(0)
    , height(0)
    , texKey(0)
{
}

MiniTexture::~MiniTexture()
{
	glDeleteTextures(1, &texId);
}

LVoid MiniTexture::initWithData(LVoid* data, LUint key, LInt width, LInt height)
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
    : item(NULL)
{
}

MiniTextureCache* MiniTextureCache::getInst()
{
	if (!s_instance)
	{
		s_instance = new MiniTextureCache();
	}

	return s_instance;
}

MiniTexture* MiniTextureCache::find(LVoid* image)
{
	LUint key = 0;
	HtmlView* item = (HtmlView*)image;
	if (item->isImage())
	{
		ImageView* imgItem = (ImageView*)item;
		key = StringUtils::hashCode(imgItem->url());
	}

	return find(item, key);
}

MiniTexture* MiniTextureCache::put(const LImage* image)
{
	LUint key = 0;
	HtmlView* item = (HtmlView*) image->item();
	KFORMATLOG("MiniTextureCache::put isImage=%d", item->isImage());
	if (item->isImage())
	{
		ImageView* imgItem = (ImageView*)item;
		key = StringUtils::hashCode(imgItem->url());
		KFORMATLOG("MiniTextureCache::put tex=%x", key);
	}

	MiniTexture* tex = find(item, key);

	if (tex == NULL)
	{
		tex = fetchTexture(item, image->rect(), image, key);
	}

	return tex;
}

MiniTexture* MiniTextureCache::find(HtmlView* item, LUint key)
{
	TextureMap::Iterator iter = m_texMap.begin();
	TextureMap::Iterator iterEnd = m_texMap.end();

	for (; iter != iterEnd; ++iter)
	{
		// 0表示是文本产生的纹理
		if (key == 0)
		{
            if (item == (*iter)->item)
            {
            	return (*iter)->tex.get();
            }
		}
		else
		{
            if ((*iter)->tex->texKey == key)
            {
            	return (*iter)->tex.get();
            }
		}
	}

	return NULL;
}

MiniTexture* MiniTextureCache::fetchTexture(HtmlView* item, const LRect& rect, const LImage* image, LUint key)
{
//	GLuint textID;
//	glGenTextures(1, &textID);

	TexturePair* pair = new TexturePair;
	pair->item = item;
//	pair->tex = new MiniTexture(textID);
//	pair->tex->texKey = key;
//	pair->tex->width = image->width();
//	pair->tex->height = image->height();
	pair->tex = new MiniTexture();
	pair->tex->initWithData(image->pixels(), key, image->width(), image->height());
	m_texMap.push(pair);

//	glBindTexture(GL_TEXTURE_2D, textID);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	glTexImage2D(
//	    GL_TEXTURE_2D,
//	    0,
//	    GL_RGBA, // RGB 3, RGBA 4
//	    image->width(),
//	    image->height(),
//	    0,
//	    GL_RGBA,
//	    GL_UNSIGNED_BYTE,
//	    image->pixels());

	return pair->tex.get();
}

MiniTexture* MiniTextureCache::updateTexture(const LImage* image)
{
	MiniTexture* tex = find(image->item());

	if (tex)
	{
		glBindTexture(GL_TEXTURE_2D, tex->texId);

		glTexImage2D(
			    GL_TEXTURE_2D,
			    0,
			    GL_RGBA,
			    image->width(),
			    image->height(),
			    0,
			    GL_RGBA,
			    GL_UNSIGNED_BYTE,
			    image->pixels());
	}

	return tex;
}

LVoid MiniTextureCache::clear()
{
	m_texMap.clear();
}
}
