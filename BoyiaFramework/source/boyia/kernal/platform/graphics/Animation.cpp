#include "Animation.h"
#include "AutoLock.h"
#include "LColor.h"
#include "SystemUtil.h"
#include "UIThread.h"
#include "UIView.h"
#include <functional>
#include <math.h>

namespace yanbo {

#define CONST_REFRESH_TIME 16 // ms

Animation::Animation(HtmlView* item)
    : m_item(item)
    , m_duration(0)
    , m_count(0)
{
}

Animation::~Animation()
{
}

LVoid Animation::setDuration(float duration)
{
    m_duration = duration;
    m_count = ceil(duration / CONST_REFRESH_TIME);
}

LBool Animation::isFinish()
{
    return m_count == 0 ? LTrue : LFalse;
}

LVoid Animation::step()
{
    --m_count;
}

LInt Animation::type()
{
    Animation::ENone;
}

ScaleAnimation::ScaleAnimation(HtmlView* item)
    : Animation(item)
    , m_deltaScale(0)
    , m_scale(0)
{
}

ScaleAnimation::~ScaleAnimation()
{
}

LVoid ScaleAnimation::setScale(float scale)
{
    m_scale = scale;
    m_deltaScale = (scale - m_item->getStyle()->scale) / (m_duration / CONST_REFRESH_TIME);
}

LVoid ScaleAnimation::setDeltaScale(HtmlView* item, float dt)
{
    HtmlViewList::Iterator iter = item->m_children.begin();
    HtmlViewList::Iterator iterEnd = item->m_children.end();
    for (; iter != iterEnd; ++iter) {
        (*iter)->getStyle()->scale += dt;
        setDeltaScale(*iter, dt);
    }
}

LVoid ScaleAnimation::step()
{
    //AutoLock lock(&util::UIThread::s_drawSyncMutex);

    m_item->getStyle()->scale += m_deltaScale;
    // for child
    setDeltaScale(m_item, m_deltaScale);
    // layout
    m_item->layout();
    // draw
    UIThread::instance()->drawOnly(m_item);

    Animation::step();
}

OpacityAnimation::OpacityAnimation(HtmlView* item)
    : Animation(item)
    , m_opacity(0)
{
}

LVoid OpacityAnimation::updateChildOpacity(HtmlView* item, LUint8 parentOpacity)
{
    HtmlViewList::Iterator iter = item->m_children.begin();
    HtmlViewList::Iterator iterEnd = item->m_children.end();
    for (; iter != iterEnd; ++iter) {
        LUint8 opacity = item->getStyle()->opacity;
        opacity = opacity * ((float)parentOpacity / 255.0);
        (*iter)->getStyle()->drawOpacity = opacity;
        updateChildOpacity(*iter, opacity);
    }
}

void OpacityAnimation::setOpacity(LInt opacity)
{
    m_opacity = opacity;
}

LVoid OpacityAnimation::step()
{
    if (m_item) {
        LInt fromOpacity = m_item->getStyle()->opacity;

        m_item->getStyle()->opacity = (LUint8)(fromOpacity + (m_opacity - fromOpacity) / m_count);
        m_item->getStyle()->drawOpacity = m_item->getStyle()->opacity;
        // for child
        updateChildOpacity(m_item, m_item->getStyle()->opacity);
        // draw
        UIThread::instance()->drawOnly(m_item);
    }

    Animation::step();
}

TranslateAnimation::TranslateAnimation(HtmlView* item)
    : Animation(item)
{
}

LVoid TranslateAnimation::step()
{
    // 设置相对位置
    m_item->setXpos(m_item->getXpos() + m_deltaPoint.iX);
    m_item->setYpos(m_item->getYpos() + m_deltaPoint.iY);

    UIThread::instance()->drawOnly(m_item);

    Animation::step();
}

LVoid TranslateAnimation::setPosition(const LPoint& point)
{
    m_point = point;
    m_deltaPoint.iX = (m_point.iX - m_item->getXpos()) / m_count;
    m_deltaPoint.iY = (m_point.iY - m_item->getYpos()) / m_count;
}

AnimationTask::~AnimationTask()
{
    m_animList.clear();
}

LVoid AnimationTask::addAnimation(Animation* anim)
{
    m_animList.push(anim);
}

LVoid AnimationTask::runTask()
{
    AnimList::Iterator iter = m_animList.begin();
    AnimList::Iterator iterEnd = m_animList.end();

    while (iter != iterEnd) {
        if ((*iter)->isFinish()) {
            AnimList::Iterator tmpIter = iter++;
            m_animList.erase(tmpIter);
        } else {
            (*iter)->step();
            ++iter;
        }
    }
}

LBool AnimationTask::isFinish()
{
    return m_animList.count() > 0 ? LFalse : LTrue;
}

Animator::Animator()
    : m_continue(LTrue)
{
}

Animator::~Animator()
{
    m_taskList.clear();
}

Animator* Animator::instance()
{
    static Animator sAnimator;
    return &sAnimator;
}

LVoid Animator::addTask(AnimationTask* task)
{
    AutoLock lock(&m_lock);
    m_taskList.push(task);
}

LVoid Animator::runTask(AnimationTask* task)
{
    addTask(task);
    static std::function<void(long, void*)> timeoutCallback = [this](long now, void* callback) -> LVoid {
        long delta = SystemUtil::getSystemTime() - now;
        if (delta && CONST_REFRESH_TIME - delta > 0) {
            this->waitTimeOut(CONST_REFRESH_TIME - delta);
        }

        UIThread::instance()->runAnimation(callback);
    };

    static std::function<void()> animCallback = [this]() -> LVoid {
        Message* msg = this->obtain();
        msg->type = ANIM_TIMEOUT;
        msg->obj = &timeoutCallback;
        msg->arg0 = (LIntPtr)&animCallback;
        msg->when = SystemUtil::getSystemTime();

        this->runTasks();
        this->postMessage(msg);
    };

    UIThread::instance()->runAnimation(&animCallback);
}

LBool Animator::hasAnimation()
{
    return m_taskList.count() > 0;
}

LVoid Animator::runTasks()
{
    AutoLock lock(&m_lock);
    AnimTaskList::Iterator iter = m_taskList.begin();
    AnimTaskList::Iterator iterEnd = m_taskList.end();

    while (iter != iterEnd) {
        (*iter)->runTask();
        if ((*iter)->isFinish()) {
            AnimTaskList::Iterator tmpIter = iter++;
            m_taskList.erase(tmpIter);
        } else {
            ++iter;
        }
    }

    //UIThread::instance()->submit();
}

LVoid Animator::handleMessage(Message* msg)
{
    switch (msg->type) {
    case Animator::ANIM_TIMEOUT: {
        std::function<void(long, void*)>* callback = (std::function<void(long, void*)>*)msg->obj;
        (*callback)(msg->when, (std::function<void()>*)msg->arg0);
    } break;
    }
}
}
