package com.boyia.app.common.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.DateFormat;
import java.util.Date;

//import org.apache.http.util.EncodingUtils;

import com.boyia.app.common.BoyiaApplication;
import com.boyia.app.common.file.BoyiaFileInfo;

import android.content.Context;
import android.os.Environment;
import android.os.StatFs;
import android.view.View;
import android.widget.TextView;

public class BoyiaFileUtil {
	private static final String ANDROID_SECURE = "/mnt/sdcard/.android_secure";
	private static String sBoyiaRootPath = null;
	public static final String PRIVATE_FILE_PATH = BoyiaApplication.getCurrenContext().getFilesDir()
			.getAbsolutePath();
	public static final String PRIVATE_CACHE_PATH = BoyiaApplication.getCurrenContext().getCacheDir()
			.getAbsolutePath();

	public static String getFilePathRoot() {
		if (sBoyiaRootPath != null) {
			return sBoyiaRootPath;
		}
		
		if (BoyiaApplication.getCurrenContext() == null) {
			return null;
		}

		if (BoyiaUtils.existSDCard()) {
			File cacheDir = BoyiaApplication.getCurrenContext().getExternalCacheDir();
            if (cacheDir == null ||
                    !cacheDir.exists() ||
                    !cacheDir.isDirectory()) {
            	sBoyiaRootPath = Environment.getExternalStorageDirectory().getPath()
    					+ "/mini/";
				BoyiaFileUtil.createDirectory(sBoyiaRootPath);
            } else {
            	sBoyiaRootPath = cacheDir.getAbsolutePath() + "/";
            }
		} else {
			sBoyiaRootPath = PRIVATE_FILE_PATH + "/";
		}

		return sBoyiaRootPath;
	}

	public static FileOutputStream createPrivateFile(String fileName) {
		FileOutputStream output = null;
		if (BoyiaUtils.existSDCard()) {
			try {
				output = BoyiaApplication.getCurrenContext().openFileOutput(
						sBoyiaRootPath + fileName, Context.MODE_WORLD_READABLE);
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}
		}

		return output;
	}

	public static void removeBoyiaDir() {
	    FileUtil.deleteFolder(getFilePathRoot());
	}

	public static boolean setText(View view, int id, String text) {
		TextView textView = (TextView) view.findViewById(id);
		if (textView == null) {
			return false;
		}

		textView.setText(text);
		return true;
	}

	public static boolean setText(View view, int id, int text) {
		TextView textView = (TextView) view.findViewById(id);
		if (textView == null) {
			return false;
		}

		textView.setText(text);
		return true;
	}

	public static boolean setText(View view, String text) {
		TextView textView = (TextView) view;
		if (textView == null) {
			return false;
		}

		textView.setText(text);
		return true;
	}

	public static String formatDateString(Context context, long time) {
		DateFormat dateFormat = android.text.format.DateFormat
				.getDateFormat(context);
		DateFormat timeFormat = android.text.format.DateFormat
				.getTimeFormat(context);
		Date date = new Date(time);
		return dateFormat.format(date) + " " + timeFormat.format(date);
	}

	public static String convertStorage(long size) {
		long kb = 1024;
		long mb = kb * 1024;
		long gb = mb * 1024;

		if (size >= gb) {
			return String.format("%.1f GB", (float) size / gb);
		} else if (size >= mb) {
			float f = (float) size / mb;
			return String.format(f > 100 ? "%.0f MB" : "%.1f MB", f);
		} else if (size >= kb) {
			float f = (float) size / kb;
			return String.format(f > 100 ? "%.0f KB" : "%.1f KB", f);
		} else {
			return String.format("%d B", size);
		}
	}

	public static boolean isNormalFile(String fullName) {
		return !fullName.equals(ANDROID_SECURE);
	}

	public static String getNameFromFilepath(String filepath) {
		int pos = filepath.lastIndexOf('/');
		if (pos != -1) {
			return filepath.substring(pos + 1);
		}
		return "";
	}

	public static BoyiaFileInfo getFileInfo(File f) {
		BoyiaFileInfo lFileInfo = new BoyiaFileInfo();
		String filePath = f.getPath();
		File lFile = new File(filePath);
		lFileInfo.mFileName = f.getName();
		lFileInfo.mModifiedDate = lFile.lastModified();
		lFileInfo.mIsDir = lFile.isDirectory();
		lFileInfo.mFilePath = filePath;
		if (lFileInfo.mIsDir) {
			int lCount = 0;
			File[] files = lFile.listFiles();

			// null means we cannot access this dir
			if (files == null) {
				return null;
			}

			for (File child : files) {
				if (!child.isHidden()
						&& BoyiaFileUtil.isNormalFile(child.getAbsolutePath())) {
					lCount++;
				}
			}
			lFileInfo.mCount = lCount;
			lFileInfo.mFileSize = getFolderSize(lFile);
		} else {
			lFileInfo.mFileSize = lFile.length();
		}

		return lFileInfo;
	}

	public static long getFolderSize(File f) {
		long size = 0;
		File[] fileList = f.listFiles();
		for (int i = 0; i < fileList.length; i++) {
			if (fileList[i].isDirectory()) {
				size = size + getFolderSize(fileList[i]);
			} else {
				size = size + fileList[i].length();
			}
		}

		return size;
	}

	public static String getExtFromFilename(String filename) {
		int dotPosition = filename.lastIndexOf('.');
		if (dotPosition != -1) {
			return filename.substring(dotPosition + 1, filename.length());
		}
		return "";
	}

	public static String getPathFromFilepath(String filepath) {
		int pos = filepath.lastIndexOf('/');
		if (pos != -1) {
			return filepath.substring(0, pos);
		}
		return "";
	}

	public static String getNameFromFilename(String filename) {
		int dotPosition = filename.lastIndexOf('.');
		if (dotPosition != -1) {
			return filename.substring(0, dotPosition);
		}
		return "";
	}

	public static void writeFile2SdcardFile(String fileName, String write_str) {
		writeFile2SdcardFile(fileName, write_str.getBytes());
	}

	public static void writeFile2SdcardFile(String fileName, byte[] data) {
		FileOutputStream fout = null;
		try {
			fout = new FileOutputStream(fileName);
			try {
				fout.write(data);
				fout.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public String readFileSdcardFile(String fileName) {
		String res = "";
		try {
			FileInputStream fin = new FileInputStream(fileName);
			int length = fin.available();

			byte[] buffer = new byte[length];
			fin.read(buffer);
			//res = EncodingUtils.getString(buffer, "UTF-8");
			res = new String(buffer, "UTF-8");
			fin.close();
		} catch (Exception e) {
			e.printStackTrace();
		}

		return res;
	}

	public static boolean isFileExist(String filePath) {
		File file = new File(filePath);
		if (file.exists() && file.isFile()) {
			return true;
		}

		return false;
	}

	public static boolean isDir(String filePath) {
		File file = new File(filePath);
		if (file.exists() && file.isDirectory()) {
			return true;
		}

		return false;
	}

	public static File createDirectory(String filePath) {
		File file = new File(filePath);
		if (!file.exists() || !file.isDirectory()) {
			file.mkdirs();
		}

		return file;
	}

	public static String readByInputStream(InputStream is) {
		StringBuffer sb = null;
		InputStreamReader isr = null;
		BufferedReader br = null;
		try {
			isr = new InputStreamReader(is);
			br = new BufferedReader(isr);
			sb = new StringBuffer();
			String line = "";
			while (null != (line = br.readLine())) {
				sb.append(line);
			}
		} catch (IOException e) {
			sb = null;
			e.printStackTrace();
		} finally {
			if (null != is) {
				try {
					is.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (null != isr) {
				try {
					isr.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (null != br) {
				try {
					br.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}

		if (null != sb) {
			return sb.toString();
		}

		return null;
	}

	public static long getSdcardSize() {
		File path = Environment.getExternalStorageDirectory();
		StatFs statFs = new StatFs(path.getPath());
		long blocksize = statFs.getBlockSize();
		long totalblocks = statFs.getBlockCount();

		// 计算SD卡的空间大小
		return blocksize * totalblocks;
	}

	public static long getSdcardLeftSize() {
		File path = Environment.getExternalStorageDirectory();
		StatFs statFs = new StatFs(path.getPath());
		long blocksize = statFs.getBlockSize();
		long availableblocks = statFs.getAvailableBlocks();

		return availableblocks * blocksize;
	}
}
