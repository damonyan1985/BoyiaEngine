package com.boyia.app.common.utils;

import net.lingala.zip4j.core.ZipFile;
import net.lingala.zip4j.exception.ZipException;
import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.util.Zip4jConstants;

import java.io.File;

public class ZipOperation {
    private static final String TAG = "ZipOperation";
    public static final String ZIP_PASSWORD = "123456";

    public static boolean zip(String src, String dest) {
        File sourceFile = new File(src);
        File zipArchive = new File(dest);
        try {
            ZipFile zipFile = new ZipFile(zipArchive);
            // 设置编码格式（支持中文）
            zipFile.setFileNameCharset("GBK");

            ZipParameters zipParameters = new ZipParameters();
            // 设置压缩方式
            zipParameters.setCompressionMethod(Zip4jConstants.COMP_DEFLATE);
            // 设置压缩级别
            zipParameters.setCompressionLevel(Zip4jConstants.DEFLATE_LEVEL_NORMAL);

            zipParameters.setEncryptFiles(true);
            // 设置加密方式
            zipParameters.setEncryptionMethod(Zip4jConstants.ENC_METHOD_STANDARD);
            zipParameters.setPassword(ZIP_PASSWORD.toCharArray());

            if (sourceFile.isDirectory()) {
                File[] listFile = sourceFile.listFiles();
                for (File file : listFile) {
                    if (file.isDirectory()) {
                        zipFile.addFolder(file, zipParameters);
                    } else {
                        zipFile.addFile(file, zipParameters);
                    }
                }
            } else {
                zipFile.addFile(sourceFile, zipParameters);
            }
            BoyiaLog.d(TAG, "ZipOperation zip: Succeed!");
            return true;
        } catch (ZipException e) {
            BoyiaLog.e(TAG, "ZipOperation zip error", e);
            e.printStackTrace();
        }
        return false;
    }

    public static boolean unZipFile(String archive, String decompressDir) {
        File zipArchive = new File(archive);
        File sourceFile = new File(decompressDir);

        try {
            ZipFile zipFile = new ZipFile(zipArchive);
            // 设置编码格式（支持中文）
            zipFile.setFileNameCharset("GBK");
            // 检查输入的zip文件是否是有效的zip文件
            if (!zipFile.isValidZipFile()) {
                BoyiaLog.d(TAG, "ZipOperation Zip File is invalid");
                return false;
            }
            if (sourceFile.isDirectory() && !sourceFile.exists()) {
                sourceFile.mkdir();
            }
            if (zipFile.isEncrypted()) {
                BoyiaLog.d(TAG, "ZipOperation unZipFile Zip File has password");
                zipFile.setPassword(ZIP_PASSWORD.toCharArray());
            }

            // 解压zip包
            zipFile.extractAll(decompressDir);
            BoyiaLog.d(TAG, "ZipOperation unZipFile Succeed!");
            return true;
        } catch (ZipException e) {
            BoyiaLog.e(TAG, "ZipOperation unZipFile Error!", e);
            e.printStackTrace();
        }

        return false;
    }
}
