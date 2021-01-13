package com.boyia.app.common.json;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.json.JsonAnnotation.JsonKey;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * author yanbo
 * 全新小型Json解析库，可用于混淆的代码，使用org.json进行二次开发
 * 注意，本解析库只对字段注解进行判断，不判断注解名
 */
public class BoyiaJson {
    private static final String TAG = "BoyiaJson";
    public static final SimpleDateFormat DATE_FULL = new SimpleDateFormat(
            "yyyy-MM-dd HH:mm:ss", Locale.getDefault());

    public static final SimpleDateFormat DATE_SIMPLE = new SimpleDateFormat(
            "yyyy-MM-dd", Locale.getDefault());

    private static void test() {
        String testJson = "{ 'pw' : [{'data':1}, {'data':2}] }";
        JSONObject jsonObject = null;
        try {
            jsonObject = new JSONObject(testJson);
            jsonObject.optJSONObject("pw");
        } catch (JSONException e) {
            e.printStackTrace();
            BoyiaLog.d("yanbo", "ETST" + e.toString());
        }
    }

    public static <T> T jsonParse(String jsonStr, Class<T> cls) {
        T object = null;
        try {
            JSONObject jsonObject = new JSONObject(jsonStr);
            try {
                object = jsonParseImpl(jsonObject, cls);
            } catch (Exception e) {
                e.printStackTrace();
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        return object;
    }

    private static <T> T jsonParseImpl(JSONObject jsonObject,
                                            Class<T> cls) throws Exception {
        if (null == jsonObject || null == cls) {
            return null;
        }

        // 获取类的字段
        Field[] fields = cls.getDeclaredFields();
        // 初始化一个实体类
        T newObject = cls.newInstance();
        for (int i = 0; i < jsonObject.names().length(); i++) {
            String key = String.valueOf(jsonObject.names().get(i));
            Object value = jsonObject.opt(key);
            for (Field field : fields) {
                JsonKey jsonKey = field.getAnnotation(JsonKey.class);
                // 如果jsonKey与key相等
                if (key.equals(jsonKey.name())) {
                    field.setAccessible(true);
                    if (!normalValue(value, field, newObject)) {
                        // 是一个类的对象
                        specialValue(value, field, newObject);
                    }
                    break;
                }

            }
        }

        return newObject;
    }

    private static <T> void specialValue(Object value, Field field, T newObject) {
        try {
            if (value instanceof JSONObject) {
                // 如果是类对象
                field.set(newObject, jsonParseImpl((JSONObject) value, field.getType()));
            } else if (value instanceof JSONArray) {
                // 如果是数组
                JSONArray jarray = (JSONArray) value;
                Class<?> type = field.getType().getComponentType();
                Object[] array = (Object[]) Array.newInstance(type, jarray.length());

                for (int i = 0; i < jarray.length(); i++) {
                    array[i] = toNormalValue(jarray.get(i), type);
                    // 如果不是普通类型
                    if (array[i] == null) {
                        array[i] = jsonParseImpl(jarray.getJSONObject(i), type);
                    }
                }
                // 奇葩！数组竟然代表了多个参数，
                // 例如数组长度为1代表一个参数，
                // 数组长度为2代表两个参数。。。
                //method.invoke(newObject, new Object[] {array});
                field.set(newObject, array);
            }
        } catch (Exception e) {
            BoyiaLog.e(TAG, "specialValue exec err=" + e.getMessage());
            e.printStackTrace();
        }
    }

    private static Object toNormalValue(Object value, Class<?> type) {
        if (!(value instanceof String)) {
            return null;
        }

        if (type == int.class) {
            try {
                value = Integer.valueOf(String.valueOf(value));
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else if (type == float.class) {
            try {
                value = Float.valueOf(String.valueOf(value));
            } catch (Exception e) {
                value = 0.0f;
            }
        } else if (type == Date.class) {
            try {
                if ("".equals(value)) {
                    value = new Date(0);
                } else if (value.equals(DATE_FULL.format(DATE_FULL
                        .parse((String) value)))) {
                    value = DATE_FULL.parse((String) value);
                } else if (value.equals(DATE_SIMPLE
                        .format(DATE_SIMPLE.parse((String) value)))) {
                    value = DATE_SIMPLE.parse((String) value);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        if (type != String.class) {
            return null;
        }

        // 默认string类型
        return value;
    }

    private static <T> boolean normalValue(Object value, Field field, T object) {
        value = toNormalValue(value, field.getType());
        if (value == null) {
            return false;
        }

        try {
            field.set(object, value);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }
}
