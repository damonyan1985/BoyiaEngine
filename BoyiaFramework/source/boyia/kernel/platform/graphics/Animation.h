#ifndef Animation_h
#define Animation_h

#include "BoyiaPtr.h"
#include "HtmlView.h"
#include "KList.h"
#include "LGraphic.h"
#include "MessageThread.h"
#include "WeakPtr.h"

namespace yanbo {

typedef LVoid (*AnimationCallback)();

class Animation;
class AnimationTask;

using AnimList = KList<BoyiaPtr<Animation>>;
using AnimTaskList = KList<BoyiaPtr<AnimationTask>>;

class Animation : public BoyiaRef {
public:
    enum AnimType {
        ENone,
        EScale,
        EOpacity,
        ETranslate,
        EVelocity
    };
    Animation(HtmlView* item);
    virtual ~Animation();

    LVoid setDuration(float duration);
    virtual LBool isFinish();
    virtual LVoid step();
    virtual LInt type();

protected:
    WeakPtr<HtmlView> m_item;
    LInt m_count;
    float m_duration;
};

class ScaleAnimation : public Animation {
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

class OpacityAnimation : public Animation {
public:
    OpacityAnimation(HtmlView* item);
    virtual LVoid step();
    void setOpacity(LInt opacity);

private:
    static LVoid updateChildOpacity(HtmlView* item, LUint8 parentOpacity);
    LUint8 m_opacity;
};

class TranslateAnimation : public Animation {
public:
    TranslateAnimation(HtmlView* item);
    virtual LVoid step();
    LVoid setPosition(const LPoint& point);

private:
    LPoint m_point;
    LPoint m_deltaPoint;
};

class VelocityAnimation : public Animation {
public:
    VelocityAnimation(HtmlView* item);
    LVoid setFriction(LReal friction);
    LVoid setVelocity(LReal velocityX, LReal velocityY);
    virtual LVoid step();
    virtual LBool isFinish();

private:
    LReal m_velocityX;
    LReal m_velocityY;
    LReal m_friction;
};

class AnimationTask : public BoyiaRef {
public:
    virtual ~AnimationTask();
    LVoid addAnimation(Animation* anim);
    LVoid runTask();
    LBool isFinish();

private:
    AnimList m_animList;
};

class Animator : public MessageThread {
public:
    enum {
        ANIM_TIMEOUT
    };
    static Animator* instance();
    virtual ~Animator();

    LBool hasAnimation();
    LVoid startAnimation(Animation* anim);

    LVoid runTask(AnimationTask* task);
    LVoid runTasks();

    virtual LVoid handleMessage(Message* msg);

private:
    Animator();
    LVoid addTask(AnimationTask* task);

    AnimTaskList m_taskList;
    LBool m_continue;
    // 保护动画任务的添加
    Mutex m_lock;
};
}

#endif
