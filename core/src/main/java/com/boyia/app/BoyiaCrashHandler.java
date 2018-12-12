package com.boyia.app;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;

import com.boyia.app.utils.BoyiaFileUtil;
import com.boyia.app.utils.BoyiaLog;
import com.boyia.app.utils.BoyiaUtils;

import android.os.Looper;

public class BoyiaCrashHandler implements Thread.UncaughtExceptionHandler {
	private static final String CRASH_FILE_PATH = BoyiaFileUtil
			.getFilePathRoot() + "crash/";
    private static final String TAG = BoyiaCrashHandler.class.getSimpleName();
    private static final int CRASH_TIME_OUT = 3 * 1000;
	
	@Override
    public void uncaughtException(Thread thread, Throwable ex) {
        BoyiaLog.e(TAG, "Crash UncaughtException!!!!:\n");
        if (ex != null) {
            ex.printStackTrace();
        }

        // Write Crash Log
        writeCrashInfo(ex);
        showInfo();
        killProgress();
    }
	
	private void writeCrashInfo(Throwable ex) {
        BoyiaFileUtil.createDirectory(CRASH_FILE_PATH);
        BoyiaFileUtil.writeFile2SdcardFile(
        		CRASH_FILE_PATH + System.currentTimeMillis(),
        		catchCrashInfo(ex));
	}
	
	private void showInfo() {
        new Thread() {
            @Override
            public void run() {
                Looper.prepare();
                BoyiaUtils.showToast("很抱歉，Boyia引擎出现异常，3秒后将退出应用...");
                Looper.loop();
            }
        }.start();
	}
	
	private void killProgress() {
		new Thread() {
            public void run() {
                try {
                    Thread.sleep(CRASH_TIME_OUT);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                android.os.Process.killProcess(android.os.Process.myPid());
            }
        }.start();
	}

    private String catchCrashInfo(Throwable ex) {
        String log = null;
        ByteArrayOutputStream baos = null;
        PrintStream printStream = null;
        try {
            baos = new ByteArrayOutputStream();
            printStream = new PrintStream(baos);
            ex.printStackTrace(printStream);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (printStream != null) {
                    printStream.close();
                }
                if (baos != null) {
                    baos.close();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return log;
    }
}
