package com.boyia.app.core;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;

import java.io.File;

public class BoyiaBridge {
    private static final String TAG = "BoyiaBridge";

    public static int getTextSize(String text) {
        return text.length();
    }

    public static String getAppRoot() {
        return BoyiaFileUtil.getAppRoot();
    }
}