package com.boyia.app.common.utils;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.Application;
import android.content.Context;
import android.os.Build;
import android.os.Process;

import com.boyia.app.common.BaseApplication;

import java.util.ArrayList;
import java.util.List;

/**
 * 进程信息获取类
 */
public class ProcessUtil {
    /**
     * 获取当前正在运行的进程名列表
     * @return
     */
    public static List<ProcessInfo> getProcessList() {
        List<ProcessInfo> list = new ArrayList<>();

        ActivityManager am = (ActivityManager) BaseApplication.getInstance().getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningAppProcessInfo info : am.getRunningAppProcesses()) {
            list.add(new ProcessInfo(info.pid, info.processName));
        }

        return list;
    }

    // 是否是主进程
    public static boolean isMainProcess() {
        //String packageNmae = BaseApplication.getInstance().getApplicationInfo().packageName;
        String mainProgressName = BaseApplication.getInstance().getApplicationInfo().processName;
        String currentProcessName = getCurrentProcessName();
        if (currentProcessName == null || mainProgressName == null) {
            return false;
        }

        return mainProgressName.equals(currentProcessName);
    }

    /**
     * 是否是boyia app进程
     * @return boolean
     */
    public static boolean isBoyiaAppProcess() {
        String prefix = BaseApplication.getInstance().getApplicationInfo().packageName + ":boyia_app_";
        String currentProcessName = getCurrentProcessName();
        if (currentProcessName == null) {
            return false;
        }

        return currentProcessName.startsWith(prefix);
    }

    /**
     * 获取当前进程
     * @return String
     */
    public static String getCurrentProcessName() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            return Application.getProcessName();
        }

        int currentPid = Process.myPid();
        List<ProcessInfo> infoList = getProcessList();
        for (ProcessInfo info : infoList) {
            if (info.mPid == currentPid) {
                return info.mName;
            }
        }

        return null;
    }

    /**
     * 进程信息
     */
    public static class ProcessInfo {
        public int mPid;
        public String mName;

        public ProcessInfo(int pid, String name) {
            mPid = pid;
            mName = name;
        }
    }
}
