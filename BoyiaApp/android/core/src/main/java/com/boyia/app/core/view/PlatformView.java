package com.boyia.app.core.view;

import android.view.View;

/**
 * 处理android自定义view和boyia引擎同层渲染
 * @Author yanbo
 */
public interface PlatformView {
    View getView();
    String getName();
}
