package com.boyia.app.common.json;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.json.JsonAnnotation.JsonKey;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
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
                // 如果jsonKey与key相等, 或者与field name一致
                if ((jsonKey != null && key.equals(jsonKey.name())) || key.equals(field.getName())) {
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
                if (field.getType().isArray()) {
                    BoyiaLog.d(TAG,
                            "specialValue array field type = "
                                    + field.getType() + " isList="
                                    + List.class.isAssignableFrom(field.getType())
                                    + " type=" + field.getType().getComponentType());

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
                } else if (List.class.isAssignableFrom(field.getType())) { // 是否是List的子类
                    Class<?> type = (Class<?>) ((ParameterizedType)field.getGenericType()).getActualTypeArguments()[0];
                    BoyiaLog.d(TAG,
                            "specialValue array field type = "
                                    + field.getType() + " isList="
                                    + List.class.isAssignableFrom(field.getType())
                                    + " type=" + type);
                    // 如果是list类型
                    ArrayList array = new ArrayList();
                    for (int i = 0; i < jarray.length(); i++) {
                        Object obj = toNormalValue(jarray.get(i), type);
                        // 如果不是普通类型
                        if (obj == null) {
                            obj = jsonParseImpl(jarray.getJSONObject(i), type);
                        }

                        array.add(obj);
                    }

                    field.set(newObject, array);
                }
            }
        } catch (Exception e) {
            BoyiaLog.e(TAG, "specialValue exec err=" + e.getMessage());
            e.printStackTrace();
        }
    }

    private static boolean isInt(Class<?> type) {
        return type == Integer.class || type == int.class;
    }

    private static boolean isFloat(Class<?> type) {
        return type == Float.class || type == float.class;
    }

    private static boolean isLong(Class<?> type) {
        return type == Long.class || type == long.class;
    }

    private static boolean isDouble(Class<?> type) {
        return type == Double.class || type == double.class;
    }

    private static Object toNormalValue(Object value, Class<?> type) {
        if (value == null) {
            return null;
        }

        if (isInt(type) || isFloat(type) || isDouble(type) || isLong(type) || type == String.class) {
            return value;
        }

        if (type == Date.class) {
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

                return value;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return null;
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
