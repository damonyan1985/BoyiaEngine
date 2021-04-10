#include "GLContext.h"
#include "JNIUtil.h"
#include "SalLog.h"
#include <EGL/egl.h>

namespace util {

class GLWindow {
public:
    GLWindow()
        : win(kBoyiaNull)
        , display(EGL_NO_DISPLAY)
        , surface(EGL_NO_SURFACE)
        , context(EGL_NO_CONTEXT)
        , width(0)
        , height(0)
    {
    }
    ANativeWindow* win;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
    int type;
    int width;
    int height;
};

GLWindow* GLContext::s_sharedWin = NULL;
GLContext::GLContext()
    : m_window(NULL)
{
}

// ANativeWindow's type is NativeWindowType for EGL,
// when call eglCreateWindowSurface
void GLContext::setWindow(void* win)
{
    if (!m_window) {
        //ANativeWindow_release(m_window->win);
        //delete m_window;
        //destroyGL();
        m_window = new GLWindow;
    }

    if (m_window->win) {
        ANativeWindow_release(m_window->win);
    }

    m_window->win = ANativeWindow_fromSurface(yanbo::JNIUtil::getEnv(), (jobject)win);
    KFORMATLOG("m_window win=%d", (LIntPtr)m_window->win);
}

void GLContext::initGL(EGLType type)
{
    if (m_window->surface != EGL_NO_SURFACE) {
        resetGL(type);
        return;
    }

    EGLint attribs[] = {
        EGL_SURFACE_TYPE, type == EWindow ? EGL_WINDOW_BIT : EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_NONE
    };

    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;

    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    const EGLint version[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    // 下一步，准备进行通过共享EGLContext，来完成
    // 普通纹理和视频纹理在多线程中传递过程
    if (EWindow == type) {
        context = eglCreateContext(display, config, NULL, version);
    } else {
        context = eglCreateContext(display, config,
            s_sharedWin ? s_sharedWin->context : NULL, version);
    }

    if (EBuffer == type) {
        m_window = new GLWindow;
    }

    m_window->display = display;
    m_window->context = context;
    m_window->config = config;
    m_window->type = type;

    resetGL(type);
}

void GLContext::resetGL(EGLType type)
{
    EGLSurface surface;
    if (EWindow == type) {
        surface = eglCreateWindowSurface(m_window->display, m_window->config, m_window->win, NULL);
    } else {
        const EGLint bufAttribs[] = {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE
        };
        surface = eglCreatePbufferSurface(m_window->display, m_window->config, bufAttribs);
    }

    if (eglMakeCurrent(m_window->display, surface, surface, m_window->context) == EGL_FALSE) {
        KLOG("Unable to eglMakeCurrent");
        return;
    }

    m_window->surface = surface;
    if (EWindow == type) {
        eglQuerySurface(m_window->display, surface, EGL_WIDTH, &m_window->width);
        eglQuerySurface(m_window->display, surface, EGL_HEIGHT, &m_window->height);
        s_sharedWin = m_window;
    }
}

void GLContext::postBuffer()
{
    eglSwapBuffers(m_window->display, m_window->surface);
}

void GLContext::destroyGL()
{
    if (m_window->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_window->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_window->context != EGL_NO_CONTEXT) {
            eglDestroyContext(m_window->display, m_window->context);
        }
        if (m_window->surface != EGL_NO_SURFACE) {
            eglDestroySurface(m_window->display, m_window->surface);
        }
        eglTerminate(m_window->display);
    }
    m_window->display = EGL_NO_DISPLAY;
    m_window->context = EGL_NO_CONTEXT;
    m_window->surface = EGL_NO_SURFACE;

    //ANativeWindow_release(m_window->win);
    if (EWindow == m_window->type) {
        ANativeWindow_release(m_window->win);
    }
    delete m_window;
}

int GLContext::viewWidth()
{
    return m_window ? m_window->width : 0;
}

int GLContext::viewHeight()
{
    return m_window ? m_window->height : 0;
}

bool GLContext::hasSharedContext()
{
    return s_sharedWin != NULL;
}
}
