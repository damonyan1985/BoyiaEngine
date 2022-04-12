package com.boyia.app.core.launch;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.ProcessUtil;
import com.boyia.app.common.utils.ProcessUtil.ProcessInfo;

import java.util.List;

public class BoyiaAppLauncher {
    private static final String TAG = "BoyiaAppLauncher";
    public static final String BOYIA_APP_PROCESS_PREFIX = "boyia_app_";
    private static final String BOYIA_APP_ACTION_FORMAT = "com.boyia.app.sub%s.action";

    private static final String[] BOYIA_APP_PROCESS_ENDS = {"a", "b", "c", "d", "e", "f"};

    public static void launch(BoyiaAppInfo info) {
        List<ProcessInfo> list = getRunningBoyiaProcessList();

        for (int i = 0; i < BOYIA_APP_PROCESS_ENDS.length; i++) {
            if (canUse(list, BOYIA_APP_PROCESS_ENDS[i])) {
                launchApp(BOYIA_APP_PROCESS_ENDS[i]);
                return;
            }
        }
    }

    public static boolean canUse(List<ProcessInfo> list, String processEnd) {
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).mName.endsWith(processEnd)) {
                return false;
            }
        }

        return true;
    }

    public static void launchApp(String processEnd) {
        String actionName = String.format(BOYIA_APP_ACTION_FORMAT, processEnd);
        BoyiaLog.d(TAG,"launchApp action = " + processEnd);
        Intent intent = new Intent();
        intent.setAction(actionName);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        BaseApplication.getInstance().startActivity(intent);
    }

    /**
     * 获取正在运行的BoyiaApp进程
     */
    public static List<ProcessInfo> getRunningBoyiaProcessList() {
        List<ProcessInfo> list = ProcessUtil.getProcessList();

        StringBuilder debugBuilder = new StringBuilder();
        debugBuilder.append("ProcessList:");
        ActivityManager am = (ActivityManager) BaseApplication.getInstance().getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningAppProcessInfo info : am.getRunningAppProcesses()) {
            if (info.processName.startsWith(BOYIA_APP_PROCESS_PREFIX)) {
                list.add(new ProcessInfo(info.pid, info.processName));
                debugBuilder.append("-" + info.processName);
            }
        }

        BoyiaLog.d(TAG, debugBuilder.toString());
        return list;
    }
}
