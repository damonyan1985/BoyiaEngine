package com.boyia.app.common.utils;

import android.app.ActivityManager;
import android.content.Context;

import com.boyia.app.common.BoyiaApplication;

public class ProcessUtil {
    public static String getCurrentProcessName(Context context) {
        int pid = android.os.Process.myPid();
        String processName = null;
        ActivityManager manager = (ActivityManager) context.getSystemService
                (Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningAppProcessInfo process : manager.getRunningAppProcesses()) {
            if (process.pid == pid) {
                processName = process.processName;
            }
        }
        return processName;
    }
}
