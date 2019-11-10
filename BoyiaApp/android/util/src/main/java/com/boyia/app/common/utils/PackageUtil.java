package com.boyia.app.common.utils;

// Boyia App打包工具
public class PackageUtil {
    // 开发模式下使用asset做测试
    public static final String APP_PATH = "/apps/contacts";
    public static final String ZIP_PATH = "/apps/out/apps/contacts.zip";

    public static void main(String[] args) {
        String prjRoot = System.getProperty("user.dir");
        String srcPath = prjRoot + APP_PATH;
        String zipPath = prjRoot + ZIP_PATH;

        ZipOperation.zip(srcPath, zipPath);
    }
}
