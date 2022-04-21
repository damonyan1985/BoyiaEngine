package com.boyia.app.core;

import android.widget.Toast;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.BaseApplication;
import com.boyia.app.core.launch.BoyiaAppInfo;
import com.boyia.app.loader.mue.MainScheduler;

import java.io.File;

public class BoyiaBridge {
    private static final String TAG = "BoyiaBridge";

    public static int getTextSize(String text) {
        return text.length();
    }

    public static String getAppRoot() {
        return BoyiaFileUtil.getAppRoot();
    }

    // dpi为每英寸所占的像素
    public static float getDisplayDensity() {
        // 每英寸占用的像素除以160
        float density = BaseApplication.getInstance().
                getResources().getDisplayMetrics().density;
        BoyiaLog.i(TAG, "BoyiaBridge.getDisplayDensity() = " + density);
        return density;
    }

    public static void showToast(final String info) {
        BoyiaLog.d(TAG, "toast=" + info);
        MainScheduler.mainScheduler().sendJob(() -> {
            Toast.makeText(BaseApplication.getInstance(), info,
                    Toast.LENGTH_SHORT).show();
        });
    }
}