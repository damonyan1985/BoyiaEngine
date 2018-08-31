#ifndef Animation_h
#define Animation_h

#include "HtmlView.h"
#include "MiniThread.h"
#include "KList.h"
#include "KRefPtr.h"
#include "LGraphic.h"

namespace yanbo
{
class Animation : public KRef
{
public:
	enum AnimType
	{
		ENone,
		EScale,
        EOpacity,
        ETranslate,
	};
	Animation(HtmlView* item);
	virtual ~Animation();

	LVoid setDuration(float duration);
	virtual LBool isFinish();
	virtual LVoid     step();
	virtual LInt      type();

protected:
	HtmlView* m_item;
	LInt   m_count;
	float  m_duration;
};

class ScaleAnimation : public Animation
{
public:
	ScaleAnimation(HtmlView* item);
    virtual ~ScaleAnimation();
    LVoid setScale(float scale);
	virtual LVoid step();

private:
	static LVoid setDeltaScale(HtmlView* item, float dt);

	float m_deltaScale;
	float m_scale;
};

class OpacityAnimation : public Animation
{
public:
	OpacityAnimation(HtmlView* item);
	virtual LVoid step();
	void setOpacity(LInt opacity);

private:
	static LVoid updateChildOpacity(HtmlView* item, LUint8 parentOpacity);
	LUint8 m_opacity;
};

class TranslateAnimation : public Animation
{
public:
	TranslateAnimation(HtmlView* item);
	virtual LVoid step();
	void setPosition(const LPoint& point);

private:
	LPoint m_point;
	LPoint m_deltaPoint;
};


typedef KList<KRefPtr<Animation> > AnimList;
class AnimationTask : public KRef
{
public:
    virtual ~AnimationTask();
    LVoid addAnimation(Animation* anim);
    LVoid runTask();
    LBool isFinish();

private:
    AnimList m_animList;
};

typedef KList<KRefPtr<AnimationTask> > AnimTaskList;
class AnimationThread : public MiniThread
{
public:
	static AnimationThread* instance();
	virtual ~AnimationThread();

	LVoid runTask(AnimationTask* task);
	virtual LVoid run();
	LVoid runTasks();

private:
	AnimationThread();
	LVoid addTask(AnimationTask* task);

	AnimTaskList         m_taskList;
    LBool                m_continue;
    static AnimationThread* s_inst;
};
}

#endif
