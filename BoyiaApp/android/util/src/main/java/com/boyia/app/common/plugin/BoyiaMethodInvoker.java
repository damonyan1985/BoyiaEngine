package com.boyia.app.common.plugin;

import com.boyia.app.common.utils.BoyiaLog;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

public class BoyiaMethodInvoker {
    private static final String TAG = "BoyiaMethodInvoker";
    private static Map<String, Class> sClassMap = new HashMap<>();

    /**
     * 查找插件中的类
     * @param clazzName
     * @return
     * @throws ClassNotFoundException
     */
    public static Class forName(String clazzName) throws ClassNotFoundException {
        Class clazz = sClassMap.get(clazzName);
        if (clazz == null) {
            clazz = Class.forName(clazzName);
            sClassMap.put(clazzName, clazz);
        }

        return clazz;
    }

    /**
     * 调用插件类方法
     * @param target
     * @param className
     * @param methodName
     * @param paramTypes
     * @param paramValues
     * @return
     */
    public static Object invokeMethod(Object target, String className, String methodName, Class[] paramTypes,
                                      Object[] paramValues) {
        try {
            Class clazz = forName(className);
            return invokeMethod(target, clazz, methodName, paramTypes, paramValues);
        }catch (ClassNotFoundException e) {
            BoyiaLog.e(TAG,"ClassNotFoundException", e);
        }
        return null;
    }

    public static Object invokeMethod(Object target, Class clazz, String methodName, Class[] paramTypes,
                                      Object[] paramValues) {
        try {
            Method method = clazz.getDeclaredMethod(methodName, paramTypes);
            if (!method.isAccessible()) {
                method.setAccessible(true);
            }
            return method.invoke(target, paramValues);
        } catch (SecurityException e) {
            BoyiaLog.e(TAG, "SecurityException", e);
        } catch (IllegalArgumentException e) {
            BoyiaLog.e(TAG,"IllegalArgumentException", e);
        } catch (IllegalAccessException e) {
            BoyiaLog.e(TAG,"IllegalAccessException", e);
        } catch (NoSuchMethodException e) {
            BoyiaLog.e(TAG,"NoSuchMethodException", e);
        } catch (InvocationTargetException e) {
            BoyiaLog.e(TAG,"InvocationTargetException", e);
        }
        return null;
    }
}
