package com.boyia.app.upgrade;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import android.content.res.AssetManager;

import com.boyia.app.common.BoyiaApplication;
import com.boyia.app.common.job.JobBase;
import com.boyia.app.common.job.JobScheduler;
import com.boyia.app.common.loader.BoyiaLoader;
import com.boyia.app.common.loader.ILoaderCallback;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.FileUtil;
import com.boyia.app.common.utils.ZipOperation;

public class UpgradeUtil {
	private static final String ZIP_NAME = "contacts.zip";
	private static final String APP_PATH = "apps/contacts";
	
	private static final String APP_ASSETS_ZIP_PATH = "apps/contacts.zip";
	private static final String UNZIP_PATH = "/assets/apps/contacts";
	private static final String ZIP_PATH = "/assets/apps/" + ZIP_NAME;
	public static final String  ZIP_URL = "https://raw.githubusercontent.com/damonyan1985/BoyiaApp/master/apps/contacts.zip";
    
	// 如果没有更新则将asset中的zip包放入/data/data/files下
	public static void upgradeAsset(final UpgradeListener listener) {
		JobScheduler.getInstance().sendJob(new JobBase() {
			@Override
			public void exec() {
				copyAsset();
				unzipApp();
				
				if (listener != null) {
					listener.onUpgradeCompleted();
				}
			}
		});
	}
	
	public static void upgradeAppFromUrl(final String url, final UpgradeListener listener) {
		new BoyiaLoader(new ILoaderCallback() {
			private FileOutputStream mOutPut = null;
			private File mFile = null;
			private long mSize = 0;
			private int mCurrentSize = 0;

			@Override
			public void onLoaderStart() {
				try {
					File appDir = BoyiaApplication.getCurrenContext().getFilesDir();
					mFile = new File(appDir, ZIP_NAME);
					if (mFile.exists()) {
						mFile.delete();
					}
					mOutPut = new FileOutputStream(mFile);
				} catch (IOException e) {
					e.printStackTrace();
				}
			}

			@Override
			public void onLoaderDataSize(long size) {
				mSize = size;
			}

			@Override
			public void onLoaderRedirectUrl(String redirectUrl) {
				try {
					mOutPut.close();
					mFile.delete();
				} catch (IOException e) {
					e.printStackTrace();
				}

				upgradeAppFromUrl(redirectUrl, listener);
			}

			@Override
			public void onLoaderDataReceive(byte[] data, int length, Object msg) {
				try {
					mOutPut.write(data, 0, length);
				} catch (IOException e) {
					e.printStackTrace();
				}
				
				mCurrentSize += length;
				if (listener != null) {
					float progress = ((float)mCurrentSize / mSize) * 100;
					listener.onUpgradeProgress((int) progress);
				}
			}

			@Override
			public void onLoaderFinished(Object msg) {
				try {
					mOutPut.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
				
				// download completed, and now unzip file
				unzipApp();
				if (listener != null) {
					listener.onUpgradeCompleted();
				}
			}

			@Override
			public void onLoaderError(String error, Object msg) {
				try {
					mOutPut.close();
					mFile.delete();
				} catch (IOException e) {
					e.printStackTrace();
				}
				
				// retry download
				upgradeAppFromUrl(url, listener);
			}
		}, true).load(url);

	}
	
	private static void copyAsset() {
		File appDir = BoyiaApplication.getCurrenContext().getFilesDir();
		File appFile = new File(appDir, ZIP_NAME);
		if (appFile.exists()) {
			appFile.delete();
		}
		try {
			//获取资产目录管理器
			AssetManager assetManager = BoyiaApplication.getCurrenContext().getAssets();
			InputStream is = assetManager.open(APP_ASSETS_ZIP_PATH);//输入流
			FileOutputStream fos = new FileOutputStream(appFile);//输出流
			byte[] buffer = new byte[1024];
			int len = 0;
			while((len=is.read(buffer))!=-1){
				fos.write(buffer,0,len);
			}
			fos.close();
			is.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private static void unzipApp() {
		File appDir = BoyiaApplication.getCurrenContext().getFilesDir();
		File appZip = new File(appDir, ZIP_NAME);
		if (!appZip.exists()) {
			return;
		}
		File appFile = new File(appDir, APP_PATH);
		if (appFile.exists()) {
			FileUtil.deleteFolder(appFile.getPath());
		}
		
		appFile.mkdirs();
		BoyiaLog.d("UpgradeUtil", "AppZip path="+appZip.getPath());

		ZipOperation.unZipFile(appZip.getPath(), appFile.getPath());
		
		if (appFile.exists() && appFile.isDirectory()) {
			BoyiaLog.d("UpgradeUtil", "appFile path="+appFile.getPath());
			File[] list = appFile.listFiles();
			if (list.length > 0) {
				for (int i = 0; i < list.length; ++i) {
					BoyiaLog.d("UpgradeUtil", "appFile files="+list[i].getPath());
				}
			}
		} else {
			BoyiaLog.d("UpgradeUtil", "AppFile Create Error");
		}
	}
	
	public static void main(String[] args) {
		String sourceRoot = System.getProperty("user.dir");
		String zipPath = sourceRoot + ZIP_PATH;
		String unzipPath = sourceRoot + UNZIP_PATH;
		FileUtil.deleteFolder(unzipPath);
		
		ZipOperation.unZipFile(zipPath, unzipPath);
	}
	
	public interface UpgradeListener {
		void onUpgradeCompleted();
		void onUpgradeProgress(int progress);
	}
}
