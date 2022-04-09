package com.boyia.app.common.utils;

import java.io.File;
import java.io.FileInputStream;
import java.math.BigInteger;
import java.security.MessageDigest;

import com.boyia.app.common.BaseApplication;

import android.content.Context;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.Paint.FontMetrics;
import android.util.Size;
import android.view.Display;
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

    // Sub Thread exec Toast needs Looper
    public static void showToast(final String info) {
        BoyiaLog.d("engine", "toast=" + info);
        BaseApplication.getInstance().getAppHandler().post(()-> {
                Toast.makeText(BaseApplication.getInstance(), info,
                        Toast.LENGTH_SHORT).show();
            }
        );
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
        WindowManager wm =  (WindowManager) BaseApplication.getInstance().getSystemService(Context.WINDOW_SERVICE);
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
}