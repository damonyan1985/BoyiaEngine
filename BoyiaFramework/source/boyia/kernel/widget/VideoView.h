#ifndef VideoView_h
#define VideoView_h

#include "BlockView.h"

namespace yanbo {
class VideoView : public BlockView {
public:
    VideoView(
        const String& id,
        LBool selectable,
        const String& src);

    ~VideoView();

    virtual LVoid layout();
    virtual LVoid paint(LGraphicsContext& gc);

private:
    LVoid loadVideo();

private:
    String m_src;
    LMediaPlayer* m_mediaPlayer;
    bool m_playing;
};
}
#endif
