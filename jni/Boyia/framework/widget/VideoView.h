#ifndef VideoView_h
#define VideoView_h

#include "BlockView.h"

namespace yanbo
{
class VideoView: public BlockView
{
public:
	VideoView(
			const String& id,
			LBool selectable,
			const String& src);

	~VideoView();

	virtual void layout();
	virtual void paint(LGraphicsContext& gc);

private:
	void loadVideo();

private:
	String m_src;
	LMediaPlayer* m_mediaPlayer;
	bool m_playing;
};
}
#endif
