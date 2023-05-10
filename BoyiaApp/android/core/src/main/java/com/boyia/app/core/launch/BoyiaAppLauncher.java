package com.boyia.app.core.launch;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.SparseArray;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaShare;
import com.boyia.app.common.utils.FileUtil;
import com.boyia.app.common.utils.ProcessUtil;
import com.boyia.app.common.utils.ProcessUtil.ProcessInfo;
import com.boyia.app.common.utils.ZipOperation;
import com.boyia.app.core.BoyiaBridge;

import java.io.File;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class BoyiaAppLauncher {
    private static final String TAG = "BoyiaAppLauncher";
    public static final String BOYIA_APP_PROCESS_PREFIX = "boyia_app_";
    public static final String BOYIA_APP_INFO_KEY = "boyia_app";

    private static final String BOYIA_APP_ACTION_FORMAT = "com.boyia.app.sub%s.action";

    private static final String[] BOYIA_APP_PROCESS_ENDS = {"a", "b", "c", "d", "e", "f"};
    private static final String BOYIA_APP_SDK_UNZIP_KEY = "boyia_app_sdk_unzip";

    private HandlerThread mLaunchThread;
    private SparseArray<String> mActions = null;

    private Handler mHandler = null;

    private LinkedHashMap<String, BoyiaAppInfo> mManager;

    private static class BoyiaAppLauncherHolder {
        private static final BoyiaAppLauncher APP_LAUNCHER = new BoyiaAppLauncher();
    }

    public static BoyiaAppLauncher launcher() {
        return BoyiaAppLauncherHolder.APP_LAUNCHER;
    }

    private BoyiaAppLauncher() {
        mActions = new SparseArray<>();
        mLaunchThread = new HandlerThread(TAG);
        mLaunchThread.start();
        mHandler = new Handler(mLaunchThread.getLooper());
        mHandler.post(BoyiaAppLauncher::initSdk);
    }

    public void launch(BoyiaAppInfo info) {
        mHandler.post(() -> {
            if (!BoyiaShare.getImpl(BOYIA_APP_SDK_UNZIP_KEY, false)) {
                BoyiaLog.d(TAG, "sdk is not init");
                return;
            }

            String end = mActions.get(info.mAppId);
            if (end != null) {
                launchApp(end, info);
                return;
            }

            initApp(info);
            launchImpl(info);
        });
    }

    /**
     * 当进程被杀死时，从缓存中移除
     */
    public void notifyAppExit(int appId) {
        mHandler.post(() -> {
            mActions.remove(appId);
        });
    }

    /**
     * 实际上就是判断应用的zip包是否被解压了
     * 如果没有解压就进行解压
     * @param info
     */
    private void initApp(BoyiaAppInfo info) {
        String appPath = BoyiaBridge.getAppRoot() + "apps" + File.separator + info.mAppName;
        File file = new File(appPath);
        if (file.exists()) {
            return;
        }

        BoyiaFileUtil.createDirectory(appPath);
        ZipOperation.unZipFile(info.mAppPath, appPath);
    }

    /**
     * 解压sdk.zip
     */
    private static void initSdk() {
        String sdkPath = BoyiaBridge.getAppRoot() + "apps" + File.separator + "sdk";
        File file = new File(sdkPath);
        if (file.exists()) {
            if (BoyiaShare.getImpl(BOYIA_APP_SDK_UNZIP_KEY, false)) {
                return;
            }

            FileUtil.deleteFolder(sdkPath);
        }

        String assetsFile = "out/sdk.zip";
        BoyiaFileUtil.createDirectory(sdkPath);

        String tmpDir = BoyiaBridge.getAppRoot() + "tmp";
        BoyiaFileUtil.createDirectory(tmpDir);

        String sdkFile = tmpDir + File.separator + "sdk_tmp1.zip";
        if (!BoyiaFileUtil.copyFromAssets(assetsFile, sdkFile)) {
            return;
        }

        if (ZipOperation.unZipFile(sdkFile, sdkPath)) {
            BoyiaShare.putImpl(BOYIA_APP_SDK_UNZIP_KEY, true);
        }
    }

    /**
     * 只允许主进程进行调用，其他进程可以通过IPC来调用此方法
     * @param info
     */
    private void launchImpl(BoyiaAppInfo info) {
        List<ProcessInfo> list = getRunningBoyiaProcessList();
        for (int i = 0; i < BOYIA_APP_PROCESS_ENDS.length; i++) {
            if (canUse(list, BOYIA_APP_PROCESS_ENDS[i])) {
                launchApp(BOYIA_APP_PROCESS_ENDS[i], info);
                return;
            }
        }

        // 没有空进程可以使用了，随机选一个最先使用的进程进行复用
        int random = new Random().nextInt(BOYIA_APP_PROCESS_ENDS.length);
        launchApp(BOYIA_APP_PROCESS_ENDS[random], info);
    }

    private boolean canUse(List<ProcessInfo> list, String processEnd) {
        for (int i = 0; i < list.size(); i++) {
            if (list.get(i).mName.endsWith(processEnd)) {
                return false;
            }
        }

        return true;
    }

    private void launchApp(String processEnd, BoyiaAppInfo info) {
        String actionName = String.format(BOYIA_APP_ACTION_FORMAT, processEnd);
        BoyiaLog.d(TAG,"launchApp action = " + processEnd);
        Intent intent = new Intent();
        intent.setAction(actionName);
        // 使用application的context来启动activity时必须加入FLAG_ACTIVITY_NEW_TASK
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        // 设置应用信息
        if (info != null) {
            Bundle bundle = new Bundle();
            bundle.putParcelable(BOYIA_APP_INFO_KEY, info);
            intent.putExtras(bundle);
        }

        BaseApplication.getInstance().startActivity(intent);
        mActions.put(info.mAppId, processEnd);
    }

    /**
     * 获取正在运行的BoyiaApp进程
     */
    private static List<ProcessInfo> getRunningBoyiaProcessList() {
        List<ProcessInfo> list = ProcessUtil.getProcessList();

        StringBuilder debugBuilder = new StringBuilder();
        debugBuilder.append("ProcessList:");
        ActivityManager am = (ActivityManager) BaseApplication.getInstance().getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningAppProcessInfo info : am.getRunningAppProcesses()) {
            if (info.processName.contains(BOYIA_APP_PROCESS_PREFIX)) {
                list.add(new ProcessInfo(info.pid, info.processName));
                debugBuilder.append("-" + info.processName);
            }
        }

        BoyiaLog.d(TAG, debugBuilder.toString());
        return list;
    }
}
