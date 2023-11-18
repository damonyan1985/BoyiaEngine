#include "LGdi.h"
#include "UIThread.h"
#include "CameraView.h"

namespace util {

class CameraWin : public LCamera {
public:
    // 启动摄像头
    virtual LVoid start() 
    {

    }
    // 开始录制视频
    virtual LVoid startRecording()
    {

    }
    // 停止录制视频
    virtual LVoid stopRecording()
    {

    }
};

LCamera* Camera::create(LVoid* view)
{
    return CameraWin();
}
}