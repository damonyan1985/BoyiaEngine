package com.boyia.app.common.utils;

import java.io.File;
import java.io.FileInputStream;
import java.lang.reflect.Method;
import java.math.BigInteger;
import java.security.MessageDigest;

import com.boyia.app.common.BaseApplication;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.Paint.FontMetrics;
import android.os.Build;
import android.util.Size;
import android.view.Display;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Toast;

public class BoyiaUtils {
    public static final String TAG = "BoyiaUtils";
    public static final String LIBRARY_NAME = "boyia";
    public static final int LOAD_FILE_SIZE = 1024;
    private static final int SCREEN_DP_WITH = 720;

    // 根据align对字符串进行居左，居右，或是居中对齐
    // 整个字符串会被框进传入的rect范围内
    public static void drawText(String s, Rect r, int align, Canvas canvas,
                                Paint paint) {

        BoyiaLog.d(TAG, "drawText align=" + align);
        FontMetrics fontMetrics = paint.getFontMetrics();
        float baseLine = 0;
        baseLine -= fontMetrics.ascent;
        float fontHeight = fontMetrics.bottom - fontMetrics.ascent;
        baseLine += (r.height() - fontHeight) / 2;
        baseLine += r.top;

        int left = r.left;
        int textLen = (int) paint.measureText(s);
        if (align == GraphicsConst.TEXT_ALIGN_CENTER) {
            BoyiaLog.d(TAG, "drawText TEXT_ALIGN_CENTER");
            left += (r.width() - textLen) / 2;
        } else if (align == GraphicsConst.TEXT_ALIGN_RIGHT) {
            left += r.width() - textLen;
        }

        if (canvas != null) {
            canvas.drawText(s, left, baseLine, paint);
        }
    }

    // Activity中启动必须搭配taskAffinity才能开启一个新的任务栈
    // Service中启动无需taskAffinity
    public static void startApp(final String info) {
//        Intent intent = new Intent();
//        intent.setAction("com.boyia.app.sub.action");
//        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
//        BaseApplication.getInstance().startActivity(intent);
    }

    // 获取字符串的md5值
    public static String getStringMD5(String str) {
        if (isTextEmpty(str)) {
            return null;
        }

        try {
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            byte[] bytes = md5.digest(str.getBytes());
            StringBuilder builder = new StringBuilder();
            for (byte b : bytes) {
                String s = Integer.toHexString(b & 0xFF);
                if (s.length() == 1) {
                    builder.append("0");
                }

                builder.append(s);
            }

            return builder.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    // 得到文件MD5值
    public static String getFileMD5(File file) {
        if (!file.isFile()) {
            return null;
        }

        MessageDigest digest;
        FileInputStream in;
        byte buffer[] = new byte[LOAD_FILE_SIZE];
        int len;
        try {
            digest = MessageDigest.getInstance("MD5");
            in = new FileInputStream(file);
            while ((len = in.read(buffer, 0, LOAD_FILE_SIZE)) != -1) {
                digest.update(buffer, 0, len);
            }
            in.close();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        BigInteger bigInt = new BigInteger(1, digest.digest());
        return bigInt.toString(16);
    }

    // 加载libs中的SO
    public static void loadLib() {
        System.loadLibrary(LIBRARY_NAME);
    }

    public static boolean isTextEmpty(String text) {
        return text == null || text.length() == 0;
    }

    // 判断是否存在SD卡
    public static boolean existSDCard() {
        return android.os.Environment.getExternalStorageState().equals(
                android.os.Environment.MEDIA_MOUNTED);
    }

    public static int getFontWidth(Paint paint, char[] chars, int index) {
        float[] width = new float[1];
        paint.getTextWidths(chars, index, 1, width);
        return (int) Math.ceil(width[0]);
    }

    public static Display getDisplay() {
        WindowManager wm = (WindowManager) BaseApplication.getInstance().getSystemService(Context.WINDOW_SERVICE);
        return wm.getDefaultDisplay();
    }

    // 获取应用显示宽高，不包含系统装饰，如底部导航之类
    public static Point getScreenSize() {
        Point size = new Point();
        getDisplay().getSize(size);
        return size;
    }

    // 获取屏幕真实宽高
    public static Point getRealScreenSize() {
        Point size = new Point();
        getDisplay().getRealSize(size);
        return size;
    }

    // 适配屏幕宽高, 以宽度为基准，自定义虚拟dp
    public static int dp(int dpValue) {
        // 四舍五入
        return (int) Math.round(dpValue * radio());
    }

    // 相对宽度，虚拟dp值
    public static double width() {
        return SCREEN_DP_WITH;
    }

    // 相对高度，虚拟dp值
    public static double height() {
        Point point = getScreenSize();
        return SCREEN_DP_WITH * (((double) point.y) / point.x);
    }

    // 以720为基准
    private static double radio() {
        Point point = getScreenSize();
        if (point.x > point.y) {
            return ((double) point.y) / SCREEN_DP_WITH;
        }

        return ((double) point.x) / SCREEN_DP_WITH;
    }

    // 系统dp值与px进行转换
    // dp为dip/160, dip是屏幕像素除以屏幕尺寸（英寸）
    public static int dp2px(int dp) {
        return Math.round(dp * BaseApplication.getInstance().getResources().getDisplayMetrics().density);
    }

    public static int px2dp(int px) {
        return Math.round(px / BaseApplication.getInstance().getResources().getDisplayMetrics().density);
    }

    public static void setStatusbarTransparent(Activity activity) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return;
        }
        transparentStatusBar(activity);
        //setRootView(activity);
    }

    private static void transparentStatusBar(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
            activity.getWindow().setStatusBarColor(Color.TRANSPARENT);
        } else {
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        }
    }

    private static void setRootView(Activity activity) {
        ViewGroup parent = (ViewGroup) activity.findViewById(android.R.id.content);
        for (int i = 0, count = parent.getChildCount(); i < count; i++) {
            View childView = parent.getChildAt(i);
            if (childView instanceof ViewGroup) {
                childView.setFitsSystemWindows(true);
                ((ViewGroup) childView).setClipToPadding(true);
            }
        }
    }

    // 获取status bar高度
    public static int getStatusBarHeight(Activity activity) {
        int resourceId = activity.getResources().getIdentifier("status_bar_height", "dimen", "android");
        if (resourceId > 0) {
            return activity.getResources().getDimensionPixelSize(resourceId);
        }
        return 0;
    }

    // 获取虚拟按键的高度
    public static int getNavigationBarHeight(Activity context) {
        int result = 0;
        if (checkNavigationBarShow(context)) {
            Resources res = context.getResources();
            int resourceId = res.getIdentifier("navigation_bar_height", "dimen", "android");
            if (resourceId > 0) {
                result = res.getDimensionPixelSize(resourceId);
            }
        }
        return result;
    }

    // 检查虚拟导航键是否显示
    public static boolean checkNavigationBarShow(Activity context) {
        boolean show;
        Display display = context.getWindow().getWindowManager().getDefaultDisplay();
        Point point = new Point();
        display.getRealSize(point);

        View decorView = context.getWindow().getDecorView();
        Configuration conf = context.getResources().getConfiguration();
        if (Configuration.ORIENTATION_LANDSCAPE == conf.orientation) {
            View contentView = decorView.findViewById(android.R.id.content);
            show = (point.x != contentView.getWidth());
        } else {
            Rect rect = new Rect();
            decorView.getWindowVisibleDisplayFrame(rect);
            show = (rect.bottom != point.y);
        }
        return show;
    }

    public static boolean isAppInstalled(Context context, String packageName) {
        if (isTextEmpty(packageName)) {
            return false;
        }

        try {
            context.getPackageManager().getPackageInfo(packageName, 0);
        } catch (NameNotFoundException e) {
            BoyiaLog.e(TAG, "invoke isAppInstalled error", e);
            return false;
        }

        return true;
    }

    public static int px2sp(float pxValue) {
        final float fontScale = BaseApplication.getInstance().getResources().getDisplayMetrics().scaledDensity;
        return (int) (pxValue / fontScale + 0.5f);
    }

    public static int sp2px(float spValue) {
        final float fontScale = BaseApplication.getInstance().getResources().getDisplayMetrics().scaledDensity;
        return (int) (spValue * fontScale + 0.5f);
    }
}