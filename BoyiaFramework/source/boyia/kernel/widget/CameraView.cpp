#include "CameraView.h"

namespace yanbo {
CameraView::CameraView(
    const String& id,
    LBool selectable,
    const String& src)
    : BlockView(id, selectable)
{
    m_camera = LCamera::create(this);
}

CameraView::~CameraView()
{
    if (m_camera) {
        delete m_camera;
    }
}

LVoid CameraView::CameraView()
{
    m_camera->start();
}

LVoid CameraView::layout()
{
    BlockView::layout();
}

LVoid CameraView::paint(LGraphicsContext& gc)
{
    BlockView::paint(gc);
}
}