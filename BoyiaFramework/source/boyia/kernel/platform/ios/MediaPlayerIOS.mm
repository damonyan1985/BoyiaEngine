#ifndef MediaPlayerIOS_h
#define MediaPlayerIOS_h

#include "LGdi.h"

namespace util {
// TODO
class MediaPlayerIOS : public LMediaPlayer {
public:
    MediaPlayerIOS(LVoid* view);
    ~MediaPlayerIOS();

public:
    virtual void start(const String& url);
    virtual void pause();
    virtual void stop();
    virtual void seek(int progress);
    virtual void updateTexture(float* matrix);

private:
    LVoid* m_view;
};

MediaPlayerIOS::MediaPlayerIOS(LVoid* view)
    : m_view(view)
{
}

MediaPlayerIOS::~MediaPlayerIOS()
{
}

void MediaPlayerIOS::start(const String& url)
{
}

void MediaPlayerIOS::updateTexture(float* matrix)
{
}

void MediaPlayerIOS::seek(int progress)
{
}

void MediaPlayerIOS::pause()
{
}

void MediaPlayerIOS::stop()
{
}

LMediaPlayer* LMediaPlayer::create(LVoid* view)
{
    return new MediaPlayerIOS(view);
}
}

#endif
