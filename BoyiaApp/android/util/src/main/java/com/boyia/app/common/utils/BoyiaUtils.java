package com.boyia.app.common.utils;

import java.io.File;
import java.io.FileInputStream;
import java.math.BigInteger;
import java.security.MessageDigest;
import com.boyia.app.common.BaseApplication;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Paint.FontMetrics;
import android.widget.Toast;

public class BoyiaUtils {
	public static final String TAG = BoyiaUtils.class.getSimpleName();

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

	// Sub Thread exec Toast needs Looper
	public static void showToast(final String info) {
		BoyiaLog.d("engine", "toast="+info);
		BaseApplication.getInstance().getAppHandler().post(new Runnable() {
			@Override
			public void run() {
				Toast.makeText(BaseApplication.getInstance(), info,
							Toast.LENGTH_SHORT).show();
			}
		});

	}

	// 得到文件MD5值
	public static String getFileMD5(File file) {
		if (!file.isFile()) {
			return null;
		}

		MessageDigest digest;
		FileInputStream in;
		byte buffer[] = new byte[1024];
		int len;
		try {
			digest = MessageDigest.getInstance("MD5");
			in = new FileInputStream(file);
			while ((len = in.read(buffer, 0, 1024)) != -1) {
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
		System.loadLibrary("boyia");
	}

	public static boolean isTextEmpty(String text) {
		return text == null || text.length() == 0;
	}

	public static boolean existSDCard() {
		return android.os.Environment.getExternalStorageState().equals(
				android.os.Environment.MEDIA_MOUNTED);
	}
	
	public static int getFontWidth(Paint paint, char[] chars, int index) {
        float[] width = new float[1];
        paint.getTextWidths(chars, index, 1, width);
        return (int) Math.ceil(width[0]);
    }
    
    // 处理patch
    public static native void nativeUpdatePatch(String oldPath, String newPath, String patchPath);
}