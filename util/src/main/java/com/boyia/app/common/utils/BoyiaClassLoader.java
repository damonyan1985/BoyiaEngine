package com.boyia.app.common.utils;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;

import dalvik.system.DexClassLoader;
// 用于对java dex进行隔离
public class BoyiaClassLoader extends DexClassLoader {
    public static final String TAG = BoyiaClassLoader.class.getSimpleName();
    private ClassLoader mParent;
    public BoyiaClassLoader(String dexPath, String optimizedDirectory, String libraryPath, ClassLoader parent) {
        super(dexPath, optimizedDirectory, libraryPath, new ClassLoader() {
            @Override
            public URL getResource(String resName) {
                return null;
            }

            @Override
            public Enumeration<URL> getResources(String resName) throws IOException {
                return null;
            }

            @Override
            public Class<?> loadClass(String className) throws ClassNotFoundException {
                return null;
            }
        });
        mParent = parent;
    }

    @Override
    public Class<?> loadClass(String className) throws ClassNotFoundException {
        Class<?> clzz = null;
        try {
            clzz = super.loadClass(className);
        } catch (Exception ex) {
            BoyiaLog.d(TAG, "Cannot load class from subloader");
        }

        if (clzz != null) {
            return clzz;
        }

        return mParent.loadClass(className);
    }

    @Override
    public URL getResource(String resName) {
        URL url = null;
        try {
            url = super.getResource(resName);
        } catch (Exception ex) {
            BoyiaLog.d(TAG, "Cannot load res from subloader");
        }

        if (url != null) {
            return url;
        }

        return mParent.getResource(resName);
    }

    @Override
    public Enumeration<URL> getResources(String resName) throws IOException {
        Enumeration<URL> enumeration = null;
        try {
            enumeration = super.getResources(resName);
        } catch (Exception ex) {
            BoyiaLog.d(TAG, "Cannot load Resources from subloader");
        }

        if (enumeration != null) {
            return enumeration;
        }

        return mParent.getResources(resName);
    }
}
