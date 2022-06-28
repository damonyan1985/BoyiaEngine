#include "Animation.h"
#include "AppManager.h"
#include "AutoLock.h"
#include "BlockView.h"
#include "LColorUtil.h"
#include "OwnerPtr.h"
#include "PlatformBridge.h"
#include "SystemUtil.h"
#include "UIThread.h"
#include "UIView.h"
#include <math.h>

namespace yanbo {

#define CONST_REFRESH_TIME 10 // ms

Animation::Animation(HtmlView* item)
    : m_item(item)
    , m_duration(0)
    , m_count(0)
{
}

Animation::~Animation()
{
}

// 毫秒计算
LVoid Animation::setDuration(LReal duration)
{
    m_duration = duration;
    m_count = ceil(duration / CONST_REFRESH_TIME);
}

LBool Animation::isFinish()
{
    return m_count == 0;
}

LVoid Animation::stop()
{
    m_count = 0;
}

LVoid Animation::step()
{
    m_count = m_count > 0 ? --m_count : 0;
}

LInt Animation::type()
{
    return Animation::ENone;
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
    , m_point(0, 0)
    , m_deltaPoint(0, 0)
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

LReal* VelocityAnimation::SPLINE_POSITION = kBoyiaNull;
const LReal VelocityAnimation::INFLEXION = 0.35f;
const LReal VelocityAnimation::DECELERATION_RATE = (LReal)(log(0.78) / log(0.9));
const LReal VelocityAnimation::GRAVITY_EARTH = 9.80665f;

VelocityAnimation::VelocityAnimation(HtmlView* item)
    : Animation(item)
    , m_friction(0.015f)
    , m_finalX(0)
    , m_finalY(0)
    , m_total(0)
    , m_velocity(0)
{
    m_physicalCoeff = GRAVITY_EARTH
        * 39.37f
        * m_friction
        * 1;
    //* PlatformBridge::getDisplayDensity() * 160.0f;
    if (!SPLINE_POSITION) {
        init();
    }
}

LVoid VelocityAnimation::init()
{
    SPLINE_POSITION = new LReal[NB_SAMPLES + 1];

    float START_TENSION = 0.5f;
    float END_TENSION = 1.0f;

    float P1 = START_TENSION * INFLEXION;
    float P2 = 1.0f - END_TENSION * (1.0f - INFLEXION);

    float x_min = 0.0f;
    float y_min = 0.0f;
    for (int i = 0; i < NB_SAMPLES; i++) {
        float alpha = (float)i / NB_SAMPLES;

        float x_max = 1.0f;
        float x, tx, coef;
        while (true) {
            x = x_min + (x_max - x_min) / 2.0f;
            coef = 3.0f * x * (1.0f - x);
            tx = coef * ((1.0f - x) * P1 + x * P2) + x * x * x;
            if (abs(tx - alpha) < 1E-5)
                break;
            if (tx > alpha)
                x_max = x;
            else
                x_min = x;
        }
        SPLINE_POSITION[i] = coef * ((1.0f - x) * START_TENSION + x) + x * x * x;
    }
    SPLINE_POSITION[NB_SAMPLES] = 1.0f;
}

LVoid VelocityAnimation::setFriction(LReal friction)
{
    m_friction = friction;
}

LReal VelocityAnimation::getSplineDeceleration(LReal velocity)
{
    return log(INFLEXION * abs(m_velocity) / (m_friction * m_physicalCoeff));
}

LInt VelocityAnimation::getSplineFlingDuration(LReal velocity)
{
    LReal l = getSplineDeceleration(velocity);
    LReal decelMinusOne = DECELERATION_RATE - 1.0;
    return (LInt)(1000.0 * exp(l / decelMinusOne));
}

LReal VelocityAnimation::getSplineFlingDistance(LReal velocity)
{
    LReal l = getSplineDeceleration(velocity);
    LReal decelMinusOne = DECELERATION_RATE - 1.0;
    return m_friction * m_physicalCoeff * exp(DECELERATION_RATE / decelMinusOne * l);
}

LVoid VelocityAnimation::setVelocity(LReal velocityX, LReal velocityY)
{
    if (!m_item) {
        return;
    }
    
    m_velocity = hypot(velocityX, velocityY);
    setDuration(getSplineFlingDuration(m_velocity) / 1000);

    LReal coeffX = m_velocity == 0 ? 1.0f : velocityX / m_velocity;
    LReal coeffY = m_velocity == 0 ? 1.0f : velocityY / m_velocity;

    LReal totalDistance = getSplineFlingDistance(m_velocity) / 1000;

    BlockView* view = static_cast<BlockView*>(m_item.get());
    LInt startX = view->getScrollXPos();
    LInt startY = view->getScrollYPos();

    LInt maxX = view->getWidth();
    LInt maxY = view->getHeight();

    m_finalX = startX + (LInt)round(totalDistance * coeffX);

    m_finalX = LMin(m_finalX, 0);
    m_finalX = LMax(m_finalX, 0);

    m_finalY = startY + (LInt)round(totalDistance * coeffY);

    m_finalY = LMin(m_finalY, 0);
    m_finalY = LMax(m_finalY, -1 * view->scrollHeight());

    BOYIA_LOG("VelocityAnimation.setVelocity, maxY=%d totalDistance=%f m_duration=%f m_count=%d", maxY, totalDistance, m_duration, m_count);
}

LVoid VelocityAnimation::setDuration(float duration)
{
    Animation::setDuration(duration);
    m_total = m_count;
}

LVoid VelocityAnimation::step()
{
    if (!m_item) {
        return;
    }

    LReal t = ((LReal)(m_total - m_count)) / m_total;
    LInt index = (LInt)(NB_SAMPLES * t);

    LReal distanceCoef = 1.f;
    LReal velocityCoef = 0.f;

    if (index < NB_SAMPLES) {
        LReal t_inf = (LReal)index / NB_SAMPLES;
        LReal t_sup = (LReal)(index + 1) / NB_SAMPLES;
        LReal d_inf = SPLINE_POSITION[index];
        LReal d_sup = SPLINE_POSITION[index + 1];
        velocityCoef = (d_sup - d_inf) / (t_sup - t_inf);
        distanceCoef = d_inf + (t - t_inf) * velocityCoef;
    }

    BlockView* view = static_cast<BlockView*>(m_item.get());
    LInt startX = view->getScrollXPos();
    LInt startY = view->getScrollYPos();

    //LInt maxX = view->getWidth();
    //LInt maxY = view->getHeight();

    LInt currX = startX + round(distanceCoef * (m_finalX - startX));

    currX = LMin(currX, 0);
    currX = LMax(currX, 0);

    LInt currY = startY + round(distanceCoef * (m_finalY - startY));

    currY = LMin(currY, 0);
    currY = LMax(currY, -1 * view->scrollHeight());

    view->setScrollPos(0, currY);

    //UIThread::instance()->drawOnly(view);
    LGraphicsContext* gc = AppManager::instance()->uiThread()->graphics();
    view->paint(*gc);

    if (currX == m_finalX && currY == m_finalY) {
        m_count = 0;
    }

    Animation::step();
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
    return m_animList.count() == 0;
}

class ClosureTask {
public:
    ClosureTask(const closure& func)
        : m_func(func)
    {
    }
    
    ~ClosureTask()
    {
        
    }

    closure m_func;
};

Animator::Animator()
{
    start();
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
    AutoLock lock(&m_animLock);
    m_taskList.push(task);
}

LVoid Animator::postTimeout()
{
    long now = SystemUtil::getSystemTime();
    postTask([self = this, now]() -> void {
        long delta = SystemUtil::getSystemTime() - now;
        if (delta >= 0 && CONST_REFRESH_TIME - delta > 0) {
            self->waitTimeOut(CONST_REFRESH_TIME - delta);
        }
    });
}

LVoid Animator::postTask(const closure& func)
{
    Message* msg = this->obtain();
    msg->type = ANIM_TIMEOUT;
    msg->obj = new ClosureTask(func);
    msg->when = SystemUtil::getSystemTime();
    postMessage(msg);
}

LVoid Animator::runTask(AnimationTask* task)
{
    addTask(task);
    postTimeout();
}

LBool Animator::hasAnimation()
{
    return m_taskList.count() > 0;
}

LVoid Animator::startAnimation(Animation* anim)
{
    AnimationTask* task = new AnimationTask();
    task->addAnimation(anim);
    runTask(task);
}

LVoid Animator::runTasks()
{
    AutoLock lock(&m_animLock);
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

    // 提交进行渲染
    UIThread::instance()->submit();
}

LVoid Animator::handleMessage(Message* msg)
{
    switch (msg->type) {
    case Animator::ANIM_TIMEOUT: {
        OwnerPtr<ClosureTask> task = static_cast<ClosureTask*>(msg->obj);
        task->m_func();

        UIThread::instance()->postClosureTask([self = this]() -> void {
            // UIThread中执行动画
            self->runTasks();
            // 如果还有动画存在
            KLOG("VelocityAnimation.handleMessage, run tasks");
            if (self->hasAnimation()) {
                KLOG("VelocityAnimation.hasAnimation");
                self->postTimeout();
            }
        });
    } break;
    }
}

Timer::Timer(LInt milliseconds, const closure& func, LBool loop)
    : m_loop(loop)
{
    start();
    
    postTask([self = this, milliseconds, func]() -> void {
        if (milliseconds > 0) {
            self->waitTimeOut(milliseconds);
        }
        
        UIThread::instance()->postClosureTask([func]() -> void {
            func();
        });
    });
}

LVoid Timer::postTask(const closure& func)
{
    Message* msg = this->obtain();
    msg->type = kTimeOut;
    msg->obj = new ClosureTask(func);
    msg->when = SystemUtil::getSystemTime();
    postMessage(msg);
}

LVoid Timer::handleMessage(Message* msg)
{
    switch (msg->type) {
    case kTimeOut: {
        OwnerPtr<ClosureTask> task = static_cast<ClosureTask*>(msg->obj);
        task->m_func();
        
        if (m_loop) {
            postTask(task->m_func);
        } else {
            quit();
        }
    } break;
    }
}
}
