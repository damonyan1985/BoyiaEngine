/*
 * ResourceLoader.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef ResourceLoader_h
#define ResourceLoader_h

#include "NetworkBase.h"
#include "HtmlRenderer.h"
#include "ResourceLoaderClient.h"

namespace yanbo
{
class UIView;
class ResourceLoader
{
public:
	enum LoadType
	{
	    HTMLDOC = 0,
	    CACHEJS,
	    CACHECSS,
	};
	
public:
	ResourceLoader(ResourceLoaderClient* client);
	virtual ~ResourceLoader();
	
public:
	void onDataReceived(const String& data);
	void onStatusCode(LInt statusCode);
	void onRedirectUrl(const String& redirectUrl);
	void onLoadError(LInt error);
	void onLoadFinished(const String& data, LInt resType);
	void onFileLen(LInt len);
	
public:
	void load(const String& url, LoadType type);
	void loadString(const String& src);
	void setView(UIView* view);

	void repaint(HtmlView* item = NULL);
	
	void executeDocument(const String& data);
	void executeCss(const String& data);
	void executeScript(const String& data);
	HtmlRenderer* render() const;
	UIView* view() const;

protected:
    HtmlRenderer*                     m_render;
    UIView*                           m_view;
    ResourceLoaderClient*             m_client;
    LInt                              m_cssSize;
};

}
#endif /* RESOURCELOADER_H_ */
