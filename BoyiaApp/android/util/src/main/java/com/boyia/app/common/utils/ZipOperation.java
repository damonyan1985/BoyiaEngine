package com.boyia.app.common.utils;

//import java.io.BufferedInputStream;
//import java.io.BufferedOutputStream;
//import java.io.File;
//import java.io.FileInputStream;
//import java.io.FileOutputStream;
//import java.io.IOException;
//import java.util.Enumeration;
//import java.util.zip.ZipEntry;
//import java.util.zip.ZipFile;
//import java.util.zip.ZipOutputStream;
import net.lingala.zip4j.core.ZipFile;
import net.lingala.zip4j.exception.ZipException;
import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.util.Zip4jConstants;

import java.io.File;

public class ZipOperation {
	public static final String ZIP_PASSWORD = "123456";
//	public static boolean zip(String src, String dest) {
//		// 提供了一个数据项压缩成一个ZIP归档输出流
//		ZipOutputStream out = null;
//		try {
//
//			File outFile = new File(dest);// 源文件或者目录
//			if (outFile.exists()) {
//				outFile.delete();
//			}
//
//			if (!outFile.getParentFile().exists()) {
//				outFile.getParentFile().mkdirs();
//			}
//			File fileOrDirectory = new File(src);// 压缩文件路径
//			out = new ZipOutputStream(new FileOutputStream(outFile));
//			// 如果此文件是一个文件，否则为false。
//			if (fileOrDirectory.isFile()) {
//				zipFileOrDirectory(out, fileOrDirectory, "");
//			} else {
//				// 返回一个文件或空阵列。
//				File[] entries = fileOrDirectory.listFiles();
//				for (int i = 0; i < entries.length; i++) {
//					// 递归压缩，更新curPaths
//					zipFileOrDirectory(out, entries[i], "");
//				}
//			}
//		} catch (IOException ex) {
//			ex.printStackTrace();
//			closeZipStream(out);
//			return false;
//		}
//
//		return true;
//	}
//
//	private static void closeZipStream(ZipOutputStream out) {
//		if (out != null) {
//			try {
//				out.close();
//			} catch (IOException ex) {
//				ex.printStackTrace();
//			}
//		}
//	}
//
//	private static void zipFileOrDirectory(ZipOutputStream out,
//			File fileOrDirectory, String curPath) throws IOException {
//		// 从文件中读取字节的输入流
//		FileInputStream in = null;
//		try {
//			// 如果此文件是一个目录，否则返回false。
//			if (!fileOrDirectory.isDirectory()) {
//				// 压缩文件
//				String fileName = curPath
//						+ fileOrDirectory.getName();
//				byte[] buffer = new byte[4096];
//				int bytes_read;
//				in = new FileInputStream(fileOrDirectory);
//				// 实例代表一个条目内的ZIP归档
//				ZipEntry entry = new ZipEntry(fileName);
//				// 条目的信息写入底层流
//				out.putNextEntry(entry);
//				while ((bytes_read = in.read(buffer)) != -1) {
//					out.write(buffer, 0, bytes_read);
//				}
//				out.closeEntry();
//				System.out.println("compress:"+fileName);
//			} else {
//				// 压缩目录
//				File[] entries = fileOrDirectory.listFiles();
//				for (int i = 0; i < entries.length; i++) {
//					// 递归压缩，更新curPaths
//					zipFileOrDirectory(out, entries[i], curPath
//							+ fileOrDirectory.getName() + "/");
//				}
//			}
//		} catch (IOException ex) {
//			ex.printStackTrace();
//		} finally {
//			if (in != null) {
//				try {
//					in.close();
//				} catch (IOException ex) {
//					ex.printStackTrace();
//				}
//			}
//		}
//	}
//
//	public static boolean unZipFile(String archive, String decompressDir) {
//		try {
//			BufferedInputStream bi;
//			ZipFile zf = new ZipFile(archive);
//			Enumeration<?> e = zf.entries();
//			while (e.hasMoreElements()) {
//				ZipEntry ze2 = (ZipEntry) e.nextElement();
//				String entryName = ze2.getName();
//				String path = decompressDir + "/" + entryName;
//				if (ze2.isDirectory()) {
//					System.out.println("正在创建解压目录 - " + entryName);
//					File decompressDirFile = new File(path);
//					if (!decompressDirFile.exists()) {
//						decompressDirFile.mkdirs();
//					}
//				} else {
//					System.out.println("正在创建解压文件 - " + entryName);
//					String fileDir = path.substring(0, path.lastIndexOf("/"));
//					File fileDirFile = new File(fileDir);
//					if (!fileDirFile.exists()) {
//						fileDirFile.mkdirs();
//					}
//					BufferedOutputStream bos = new BufferedOutputStream(
//							new FileOutputStream(decompressDir + "/" + entryName));
//					bi = new BufferedInputStream(zf.getInputStream(ze2));
//					byte[] readContent = new byte[1024];
//					int readCount = bi.read(readContent);
//					while (readCount != -1) {
//						bos.write(readContent, 0, readCount);
//						readCount = bi.read(readContent);
//					}
//					bos.close();
//				}
//			}
//			zf.close();
//		} catch (Exception ex) {
//			ex.printStackTrace();
//			return false;
//		}
//
//		return true;
//	}

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
