package com.boyia.app.common.utils;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.content.Context;

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
