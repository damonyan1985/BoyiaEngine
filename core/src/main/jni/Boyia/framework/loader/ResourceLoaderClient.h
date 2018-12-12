/*
 * ResourceLoaderClient.h
 *
 *  Created on: 2011-7-19
 *      Author: Administrator
 */

#ifndef RESOURCELOADERCLIENT_H_
#define RESOURCELOADERCLIENT_H_
namespace yanbo
{
class ResourceLoaderClient
{
public:
	virtual void onHtmlRenderFinished() = 0;
	virtual void onViewNeedToDraw() = 0;
};
}
#endif /* RESOURCELOADERCLIENT_H_ */
