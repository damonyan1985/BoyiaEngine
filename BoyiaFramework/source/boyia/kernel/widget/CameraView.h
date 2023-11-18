#ifndef CameraView_h
#define CameraView_h

#include "BlockView.h"

// 摄像头处理
namespace yanbo {
class CameraView : public BlockView {
public:
    CameraView(
        const String& id,
        LBool selectable,
        const String& src);

    ~CameraView();

    virtual LVoid layout();
    virtual LVoid paint(LGraphicsContext& gc);

    // 启动摄像头
    LVoid start();
    // 开始录制视频
    LVoid startRecording();
    // 停止录制视频
    LVoid stopRecording();

private:
    LCamera* m_camera;    
};    
}

#endif