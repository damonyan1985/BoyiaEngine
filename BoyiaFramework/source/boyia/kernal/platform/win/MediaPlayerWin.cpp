#ifndef MediaPlayerWin_h
#define MediaPlayerWin_h

#include "LGdi.h"

namespace util {
class MediaPlayerWin : public LMediaPlayer {
public:
    MediaPlayerWin(LVoid* view);
    ~MediaPlayerWin();

public:
    virtual void start(const String& url);
    virtual void pause();
    virtual void stop();
    virtual void seek(int progress);
    virtual void updateTexture(float* matrix);

private:
    LVoid* m_view;
};

MediaPlayerWin::MediaPlayerWin(LVoid* view)
    : m_view(view)
{
}

MediaPlayerWin::~MediaPlayerWin()
{
}

void MediaPlayerWin::start(const String& url)
{
}

void MediaPlayerWin::updateTexture(float* matrix)
{
}

void MediaPlayerWin::seek(int progress)
{
}

void MediaPlayerWin::pause()
{
}

void MediaPlayerWin::stop()
{
}

LMediaPlayer* LMediaPlayer::create(LVoid* view)
{
    return new MediaPlayerWin(view);
}
}

#endif