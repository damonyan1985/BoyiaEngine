package com.boyia.app.common.utils;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.math.BigInteger;
import java.net.HttpURLConnection;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.URL;
import java.net.URLConnection;
import java.security.MessageDigest;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.List;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.boyia.app.common.BoyiaApplication;

import com.boyia.app.common.http.HTTPResponse;
import com.boyia.app.common.http.HTTPUtil;
import android.graphics.BitmapFactory;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ActivityManager.RunningTaskInfo;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Bitmap.Config;
import android.graphics.Paint.FontMetrics;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Parcelable;
import android.text.TextUtils;
import android.view.View;
import android.widget.Toast;

public class BoyiaUtils {
	public static final String TAG = "BoyiaUtils";

	private static final String LIB_SO_PATH = "libs/libYanWeb.so";

	private static final String UNKNOWN_MAC_ADDRESS = "00:00:00:00";

	public static final String HOST = "http://172.16.13.201:9000";
	public static final String START_URL = HOST + "/test/test.json";

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
		BoyiaApplication.getInstance().getAppHandler().post(new Runnable() {
			@Override
			public void run() {
				Toast.makeText(BoyiaApplication.getInstance(), info,
							Toast.LENGTH_SHORT).show();
			}
		});

	}

	public static byte[] readStream(InputStream inStream) throws Exception {
		ByteArrayOutputStream outStream = new ByteArrayOutputStream();
		Bitmap b = Bitmap.createBitmap(100, 100, Config.ARGB_8888);

		byte[] buffer = new byte[1024];
		int len = 0;

		while ((len = inStream.read(buffer)) != -1) {
			outStream.write(buffer, 0, len);
		}

		outStream.close();
		inStream.close();
		b.compress(Bitmap.CompressFormat.PNG, 50, outStream);
		return outStream.toByteArray();
	}

	// 得到文件MD5值
	public static String getFileMD5(File file) {
		if (!file.isFile()) {
			return null;
		}

		MessageDigest digest = null;
		FileInputStream in = null;
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

	public static Rect getScreenRect(View v) {
		int[] loc = new int[2];
		v.getLocationOnScreen(loc);
		return new Rect(loc[0], loc[1], loc[0] + v.getMeasuredWidth(), loc[1]
				+ v.getMeasuredHeight());
	}

	// 判断权限
	public static boolean hasPemisson(Context context, String permission,
			String packageName) {
		PackageManager pkm = context.getPackageManager();
		return (PackageManager.PERMISSION_GRANTED == pkm.checkPermission(
				permission, packageName));
	}

	// 创建快捷方式
	public static void createShortCut(Activity act, int iconResId,
			int appnameResId) {
		Intent shortcutintent = new Intent(
				"com.android.launcher.action.INSTALL_SHORTCUT");
		shortcutintent.putExtra("duplicate", false);
		shortcutintent.putExtra(Intent.EXTRA_SHORTCUT_NAME,
				act.getString(appnameResId));
		Parcelable icon = Intent.ShortcutIconResource.fromContext(
				act.getApplicationContext(), iconResId);
		shortcutintent.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, icon);
		shortcutintent.putExtra(Intent.EXTRA_SHORTCUT_INTENT,
				new Intent(act.getApplicationContext(), act.getClass()));
		act.sendBroadcast(shortcutintent);
	}

	// 加载libs中的SO
	public static void loadLib() {
		System.loadLibrary("boyia");
	}

	// 从asset中加载SO
	public static void loadLibFromAssets(Context context) {
		try {
			InputStream in = context.getAssets().open(LIB_SO_PATH);
			File f = File.createTempFile("JNI-", "Temp");
			FileOutputStream out = new FileOutputStream(f);
			byte[] buf = new byte[1024];
			int len;
			while ((len = in.read(buf)) > 0) {
				out.write(buf, 0, len);
			}

			in.close();
			out.close();
			System.load(f.getAbsolutePath());
			// JNI Load Success /data/data/Mini.app/cache/JNI-501751177Temp
			BoyiaLog.d(BoyiaApplication.AUTHOR_NAME,
					"JNI Load Success " + f.getAbsolutePath());
			f.delete();
		} catch (Exception e) { // I am still lazy ~~~
			e.printStackTrace();
		}
	}

	public static Bitmap readCachedBitmap(String fileName, int width, int height) {
		Bitmap bitmap = null;
		if (BoyiaFileUtil.isFileExist(fileName)) {
			BitmapFactory.Options option = new BitmapFactory.Options();
			option.outHeight = height;
			option.outWidth = width;
			bitmap = BitmapFactory.decodeFile(fileName, option);
		}

		return bitmap;
	}

	public static SortedMap<String, String> sortMapByKey(Map<String, String> map) {
		if (map == null || map.isEmpty()) {
			return null;
		}

		SortedMap<String, String> sortMap = new TreeMap<String, String>(
				new MapKeyComparator());
		sortMap.putAll(map);
		return sortMap;
	}

	public static class MapKeyComparator implements Comparator<String> {
		public int compare(String str1, String str2) {
			return str1.compareTo(str2);
		}
	}

	public static boolean isActivityExist(Context context, String activityName) {
		ActivityManager manager = (ActivityManager) context
				.getSystemService(Context.ACTIVITY_SERVICE);
		String name = manager.getRunningTasks(1).get(0).topActivity
				.getClassName();
		if (name.equals(activityName)) {
			return true;
		}

		return false;
	}

	// 判断是否是以太网连接
	public static boolean checkEthernet(Context context) {
		ConnectivityManager conn = (ConnectivityManager) context
				.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo networkInfo = conn
				.getNetworkInfo(ConnectivityManager.TYPE_ETHERNET);
		return networkInfo.isConnected();
	}

	// 获得系统可用内存信息
	public static long getSystemAvaialbeMemorySize(Context context) {
		// 获得MemoryInfo对象
		MemoryInfo memoryInfo = new MemoryInfo();
		// 获得系统可用内存，保存在MemoryInfo对象上
		ActivityManager activityManager = (ActivityManager) context
				.getSystemService(Context.ACTIVITY_SERVICE);
		activityManager.getMemoryInfo(memoryInfo);
		long memSize = memoryInfo.availMem;
		return memSize;
	}

	// 获取系统总内存
	public static long getTotalMemory() {
		String str1 = "/proc/meminfo";// 系统内存信息文件
		String str2;
		String[] arrayOfString;
		long initial_memory = 0;

		try {
			FileReader localFileReader = new FileReader(str1);
			BufferedReader localBufferedReader = new BufferedReader(
					localFileReader, 8192);
			str2 = localBufferedReader.readLine();// 读取meminfo第一行，系统总内存大小

			arrayOfString = str2.split("\\s+");
			for (String num : arrayOfString) {
				BoyiaLog.i(str2, num + "\t");
			}

			initial_memory = Integer.valueOf(arrayOfString[1]).intValue() * 1024;// 获得系统总内存，单位是KB，乘以1024转换为Byte
			localBufferedReader.close();

		} catch (IOException e) {
			e.printStackTrace();
		}
		return initial_memory;
	}

	// 获取MAC地址
	public static String getMacAddress(Context context) {
		String mac = "10.0.0.1";

		WifiManager wifi = (WifiManager) context
				.getSystemService(Context.WIFI_SERVICE);

		if (wifi != null) {
			WifiInfo info = wifi.getConnectionInfo();
			if (info != null) {
				mac = info.getMacAddress();
				return mac;
			}
		}

		return mac;
	}

	/**
	 * 获取当前系统连接网络的网卡的mac地址
	 * 
	 * @return
	 */
	private static final String getCurrentSysMac() {
		String macAddress = null;
		byte[] mac = null;
		StringBuffer sb = new StringBuffer();
		try {
			Enumeration<NetworkInterface> netInterfaces = NetworkInterface
					.getNetworkInterfaces();
			while (netInterfaces.hasMoreElements()) {
				NetworkInterface ni = netInterfaces.nextElement();
				Enumeration<InetAddress> address = ni.getInetAddresses();
				while (address.hasMoreElements()) {
					InetAddress ip = address.nextElement();
					if (ip.isAnyLocalAddress() || !(ip instanceof Inet4Address)
							|| ip.isLoopbackAddress())
						continue;
					if (ip.isSiteLocalAddress())
						mac = ni.getHardwareAddress();
					else if (!ip.isLinkLocalAddress()) {
						mac = ni.getHardwareAddress();
						break;
					}
				}
			}
			if (mac != null) {
				for (int i = 0; i < mac.length; i++) {
					sb.append(parseByte(mac[i]));
				}
				macAddress = sb.substring(0, sb.length() - 1);
			}
		} catch (SocketException e) {
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}

		return macAddress;
	}

	private static String parseByte(byte b) {
		String s = "00" + Integer.toHexString(b) + ":";
		return s.substring(s.length() - 3);
	}

	public static void findAllSuperClass(Class<?> clazz, List<String> superList) {
		Class<?> superClass = clazz.getSuperclass();
		if (superClass != null) {
			superList.add(superClass.getName());
			findAllSuperClass(superClass, superList);
		}
	}

	public static String getAppVersionName(Context context) {
		String versionName = "1.0.0";
		if (TextUtils.isEmpty(versionName)) {
			try {
				// ---get the package info---
				PackageManager pm = context.getPackageManager();
				PackageInfo pi = pm.getPackageInfo(context.getPackageName(), 0);
				versionName = pi.versionName;
				if (versionName == null || versionName.length() <= 0) {
					return "";
				}
			} catch (Exception e) {
				BoyiaLog.e("VersionInfo", "Exception=" + e.toString());
			}
		}

		return versionName;
	}

	public static int getAppVersionCode(Context context) {
		int versionCode = 1;
		if (versionCode == -1) {
			try {
				// ---get the package info---
				PackageManager pm = context.getPackageManager();
				PackageInfo pi = pm.getPackageInfo(context.getPackageName(), 0);
				versionCode = pi.versionCode;
			} catch (Exception e) {
				BoyiaLog.e("VersionInfo", "Exception=" + e.toString());
			}
		}

		return versionCode;
	}

	public static boolean isPhoneNO(String value) {
		String regExp = "^[1]([3][0-9]{1}|59|58|88|89|[7][6-8]{1})[0-9]{8}$";
		Pattern p = Pattern.compile(regExp);
		Matcher m = p.matcher(value.trim());
		return m.find();
	}

	public static boolean isMobileNO(String mobiles) {
		Pattern p = Pattern
				.compile("^((1[3,5,8][0-9])|(14[5,7])|(17[0,6,7,8]))//d{8}$");
		Matcher m = p.matcher(mobiles);
		return m.matches();
	}

	public static Intent getExplicitIntent(Context context,
			Intent implicitIntent) {
		// Retrieve all services that can match the given intent
		PackageManager pm = context.getPackageManager();
		List<ResolveInfo> resolveInfo = pm.queryIntentServices(implicitIntent,
				0);
		// Make sure only one match was found
		if (resolveInfo == null || resolveInfo.size() != 1) {
			return null;
		}
		// Get component info and create ComponentName
		ResolveInfo serviceInfo = resolveInfo.get(0);
		String packageName = serviceInfo.serviceInfo.packageName;
		String className = serviceInfo.serviceInfo.name;
		ComponentName component = new ComponentName(packageName, className);
		// Create a new intent. Use the old one for extras and such reuse
		Intent explicitIntent = new Intent(implicitIntent);
		// Set the component to be explicit
		explicitIntent.setComponent(component);
		return explicitIntent;
	}

	public static boolean isAppOnForeground(Context context) {
		ActivityManager activityManager = (ActivityManager) context
				.getSystemService(Context.ACTIVITY_SERVICE);
		String packageName = context.getPackageName();

		Method method;
		if (Build.VERSION.SDK_INT < 21) {
			try {
				method = activityManager.getClass().getDeclaredMethod(
						"getRunningTasks", new Class[] { int.class });
				try {
					List<RunningTaskInfo> info = (List<RunningTaskInfo>) method
							.invoke(activityManager, 1);
				} catch (Exception e) {
					e.printStackTrace();
				}
			} catch (NoSuchMethodException e) {
				e.printStackTrace();
			}

		} else {
			RunningAppProcessInfo currentInfo = null;
			Field field = null;
			int START_TASK_TO_FRONT = 2;
			String pkgName = null;
			try {
				field = RunningAppProcessInfo.class
						.getDeclaredField("processState");
			} catch (Exception e) {
				return false;
			}
			List<RunningAppProcessInfo> appList = activityManager
					.getRunningAppProcesses();
			if (appList == null || appList.isEmpty()) {
				return false;
			}
			for (RunningAppProcessInfo app : appList) {
				if (app != null
						&& app.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND) {
					Integer state = null;
					try {
						state = field.getInt(app);
					} catch (Exception e) {
						return false;
					}
					if (state != null && state == START_TASK_TO_FRONT) {
						currentInfo = app;
						break;
					}
				}
			}
			if (currentInfo != null) {
				pkgName = currentInfo.processName;
			}
		}

		return false;
	}

	public static boolean isTextEmpty(String text) {
		if (text == null || text.length() == 0) {
			return true;
		} else {
			return false;
		}
	}

	public static boolean existSDCard() {
		if (android.os.Environment.getExternalStorageState().equals(
				android.os.Environment.MEDIA_MOUNTED)) {
			return true;
		} else {
			return false;
		}
	}

	public static boolean isNumeric(String str) {
		if (isTextEmpty(str)) {
			return false;
		}

		for (int i = str.length(); --i >= 0;) {
			if (!Character.isDigit(str.charAt(i))) {
				return false;
			}
		}
		return true;
	}

	public static void startIntent(Context context,
								   String action,
								   Map<String, String> params) {
		startIntent(context, action, params, -1);
	}

	public static void startIntent(
			Context context,
			String action,
			Map<String, String> params,
			int requestCode) {
		try {
			Intent intent = new Intent();
			intent.setAction(action);
			intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			
			if (params != null) {
				for (Map.Entry<String, String> entry : params.entrySet()) {
					intent.putExtra(entry.getKey(), entry.getValue());
				}
			}

			if (requestCode > 0) {
				((Activity) context).startActivityForResult(intent, requestCode);
			} else {
				context.startActivity(intent);
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public static String bytesToString(byte[] bytes) {
		String result = null;
		try {
			result = new String(bytes, HTTPUtil.HTTP_CHARSET_UTF8);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
			result = null;
		}
		
		return result;
	}

	/**
	 * 获取外网的IP(必须放到子线程里处理)
	 */
	public static String getNetIp() {
		String ip;
		InputStream inStream;
		try {
			URL infoUrl = new URL("http://1212.ip138.com/ic.asp");
			URLConnection connection = infoUrl.openConnection();
			HttpURLConnection httpConnection = (HttpURLConnection) connection;
			int responseCode = httpConnection.getResponseCode();
			if (responseCode == HttpURLConnection.HTTP_OK) {
				inStream = httpConnection.getInputStream();
				BufferedReader reader = new BufferedReader(new InputStreamReader(inStream, "gb2312"));
				StringBuilder builder = new StringBuilder();
				String line;
				while ((line = reader.readLine()) != null) {
					builder.append(line);
				}
				inStream.close();
				int start = builder.indexOf("[");
				int end = builder.indexOf("]");
				ip = builder.substring(start + 1, end);
				return ip;
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		return null;
	}
	
	public static String syncLoadResource(String url) {
		HTTPResponse data = HTTPUtil.getFileStream(url);
		InputStream in = data.getStream();
		ByteArrayOutputStream outStream = new ByteArrayOutputStream();
	
		byte[] buffer = new byte[1024];
		int len = 0;
        String result = "";
		try {
			while ((len = in.read(buffer)) != -1) {
				outStream.write(buffer, 0, len);
			}
			
			result = new String(outStream.toByteArray(), HTTPUtil.HTTP_CHARSET_UTF8);
			outStream.close();
			in.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return result;
	}
	
	public static int getFontWidth(Paint paint, char[] chars, int index) {
        float[] width = new float[1];
        paint.getTextWidths(chars, index, 1, width);
        return (int) Math.ceil(width[0]);
    }
	
	// 反射调用某个对象的某个函数
    private static Object callMethod(Object obj, String method, Object[] args) {
        try {
            Class clzz = obj.getClass();
            Method m = clzz.getDeclaredMethod(method);
            m.setAccessible(true);
            return m.invoke(obj, args);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return  null;
    }
    
    // 反射设置某个对象的属性成员
    private static void setProperty(Object obj, String PropertyName, Object value) {
        try {
            Class c = obj.getClass();
            Field f = c.getDeclaredField(PropertyName);
            f.setAccessible(true);
            f.set(obj, value);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
    
    // 处理patch
    public static native void nativeUpdatePatch(String oldPath, String newPath, String patchPath);
}