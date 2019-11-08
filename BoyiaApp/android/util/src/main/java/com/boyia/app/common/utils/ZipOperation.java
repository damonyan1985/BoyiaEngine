package com.boyia.app.common.utils;

import net.lingala.zip4j.core.ZipFile;
import net.lingala.zip4j.exception.ZipException;
import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.util.Zip4jConstants;

import java.io.File;

public class ZipOperation {
	public static final String ZIP_PASSWORD = "123456";

	public static boolean zip(String src, String dest) {
		File sourceFile = new File(src);
		File zipArchive = new File(dest);
		try {
			ZipFile zipFile = new ZipFile(zipArchive);
			zipFile.setFileNameCharset("GBK"); //设置编码格式（支持中文）

			ZipParameters zipParameters = new ZipParameters();
			zipParameters.setCompressionMethod(Zip4jConstants.COMP_DEFLATE); //压缩方式
			zipParameters.setCompressionLevel(Zip4jConstants.DEFLATE_LEVEL_NORMAL); // 压缩级别

			zipParameters.setEncryptFiles(true);
			zipParameters.setEncryptionMethod(Zip4jConstants.ENC_METHOD_STANDARD); // 加密方式
			zipParameters.setPassword(ZIP_PASSWORD.toCharArray());

			if (sourceFile.isDirectory()) {
				File[] listFile = sourceFile.listFiles();
				for (File file: listFile) {
					if (file.isDirectory()) {
						zipFile.addFolder(file, zipParameters);
					} else {
						zipFile.addFile(file, zipParameters);
					}
				}
			} else {
				zipFile.addFile(sourceFile, zipParameters);
			}
			System.out.println("ZipOperation zip: Succeed!");
			return true;
		} catch (ZipException e) {
			System.out.println("ZipOperation zip: Error：" + e);
			e.printStackTrace();
		}
		return false;
	}

	public static boolean unZipFile(String archive, String decompressDir) {
		File zipArchive = new File(archive);
		File sourceFile = new File(decompressDir);

		try {
			ZipFile zipFile = new ZipFile(zipArchive);
			zipFile.setFileNameCharset("GBK");  //设置编码格式（支持中文）
			if (!zipFile.isValidZipFile()){     //检查输入的zip文件是否是有效的zip文件
				System.out.println("ZipOperation Zip File is invalid");
				return false;
			}
			if (sourceFile.isDirectory() && !sourceFile.exists()) {
				sourceFile.mkdir();
			}
			if (zipFile.isEncrypted()) {
				System.out.println("ZipOperation unZipFile Zip File has password");
				zipFile.setPassword(ZIP_PASSWORD.toCharArray());
			}
			zipFile.extractAll(decompressDir); //解压
			System.out.println("ZipOperation unZipFile Succeed!");
			return true;
		} catch (ZipException e) {
			System.out.println("ZipOperation unZipFile Error!");
			e.printStackTrace();
		}

		return false;
	}
}
