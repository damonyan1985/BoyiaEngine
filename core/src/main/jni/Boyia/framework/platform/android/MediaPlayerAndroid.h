#ifndef MediaPlayerAndroid_h
#define MediaPlayerAndroid_h

#include "LGdi.h"

namespace util
{
struct JMediaPlayer;
class MediaPlayerAndroid : public LMediaPlayer
{
public:
	MediaPlayerAndroid(LVoid* view);
	~MediaPlayerAndroid();

public:
	virtual void start(const String& url);
	virtual void pause();
	virtual void stop();
	virtual void seek(int progress);
	virtual void updateTexture(float* matrix);

	bool canDraw();
	int texId();

private:
	void createTextureId();

	struct JMediaPlayer* m_player;
	LUint32 m_texID;
	LVoid*  m_view;
};
}

#endif
