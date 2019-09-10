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
import com.boyia.app.common.BaseApplication;
import android.content.Context;
import android.os.Environment;
import android.os.StatFs;
import android.view.View;
import android.widget.TextView;

public class BoyiaFileUtil {
	private static final String ANDROID_SECURE = "/mnt/sdcard/.android_secure";
	private static String sBoyiaRootPath = null;
	public static final String PRIVATE_FILE_PATH = BaseApplication.getInstance().getFilesDir()
			.getAbsolutePath();
	public static final String PRIVATE_CACHE_PATH = BaseApplication.getInstance().getCacheDir()
			.getAbsolutePath();

	public static String getFilePathRoot() {
		if (sBoyiaRootPath != null) {
			return sBoyiaRootPath;
		}
		
		if (BaseApplication.getInstance() == null) {
			return null;
		}

		if (BoyiaUtils.existSDCard()) {
			File cacheDir = BaseApplication.getInstance().getExternalCacheDir();
            if (cacheDir == null ||
                    !cacheDir.exists() ||
                    !cacheDir.isDirectory()) {
            	sBoyiaRootPath = Environment.getExternalStorageDirectory().getPath()
    					+ "/boyia/";
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
				output = BaseApplication.getInstance().openFileOutput(
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
		StringBuffer sb;
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
}
