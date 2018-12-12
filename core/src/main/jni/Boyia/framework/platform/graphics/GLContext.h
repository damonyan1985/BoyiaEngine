#ifndef GLContext_h
#define GLContext_h

#include <android/native_window_jni.h>

namespace util
{
struct GLWindow;
class GLContext
{
public:
	enum EGLType {
        EBuffer = 0,
        EWindow,
	};

	GLContext();
//	static void initGLContext(EGLType type);
//	static void destroyGLContext();
//	static void postToScreen();

//	static GLContext* instance();
	void setWindow(void* win);

	void initGL(EGLType type);
	void postBuffer();
	void destroyGL();

	int viewWidth();
	int viewHeight();

	bool hasSharedContext();
private:
	GLWindow* m_window;

	static GLWindow* s_sharedWin;
	//static GLContext* s_instance;
	//static EGLContext s_sharedContext;
};
}

#endif
