#include "VideoView.h"

namespace yanbo {
VideoView::VideoView(
    const String& id,
    LBool selectable,
    const String& src)
    : BlockView(id, selectable)
    , m_src(src)
    , m_playing(false)
{
    m_mediaPlayer = LMediaPlayer::create(this);
}

VideoView::~VideoView()
{
    if (m_mediaPlayer) {
        delete m_mediaPlayer;
    }
}

LVoid VideoView::loadVideo()
{
    m_mediaPlayer->start(m_src);
}

LVoid VideoView::layout()
{
    //loadVideo();
    BlockView::layout();
}

LVoid VideoView::paint(LGraphicsContext& gc)
{
    BlockView::paint(gc);
    //gc.setHtmlView(this);
    //setClipRect(gc);
    gc.drawVideo(LRect(m_x, m_y, getWidth(), getHeight()), m_mediaPlayer);
    if (!m_playing) {
        loadVideo();
        m_playing = true;
    }
    //loadVideo();
}
}
