package com.boyia.app.common.utils;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class JSONUtil {
	public static final SimpleDateFormat DATE_FULL = new SimpleDateFormat(
			"yyyy-MM-dd HH:mm:ss", Locale.getDefault());

	public static final SimpleDateFormat DATE_SIMPLE = new SimpleDateFormat(
			"yyyy-MM-dd", Locale.getDefault());
	
	private static void test() {
		String testJson =  "{ 'pw' : [{'data':1}, {'data':2}] }";
		JSONObject jsonObject = null;
		try {
			jsonObject = new JSONObject(testJson);
			jsonObject.optJSONObject("pw");
		} catch (JSONException e) {
			e.printStackTrace();
			BoyiaLog.d("yanbo", "ETST" + e.toString());
		}
	}
	
	public static <T> T parseJSON(String jsonStr, Class<T> cls) {
		T object = null;
		try {
			JSONObject jsonObject = new JSONObject(jsonStr);
			try {
				object = jsonObjectToObject(jsonObject, cls);
			} catch (Exception e) {
				e.printStackTrace();
			}
		} catch (JSONException e) {
			e.printStackTrace();
		}

		return object;
	}

	private static <T> T jsonObjectToObject(JSONObject jsonObject,
			Class<T> cls) throws Exception {
		if (null == jsonObject || null == cls) {
			return null;
		}

		Field[] fields = cls.getDeclaredFields();
		T newObject = cls.newInstance();
		for (int i = 0; i < jsonObject.names().length(); i++) {
			String jsonName = String.valueOf(jsonObject.names().get(i));
			Object value = jsonObject.opt(jsonName);
			for (Field field : fields) {
				BoyiaLog.d("hello=", "jsonName="+jsonName.toUpperCase(Locale.getDefault())
				    + " fieldName="+field.getName().toUpperCase(Locale.getDefault()));
				if (jsonName.toUpperCase(Locale.getDefault()).equals(
						field.getName().toUpperCase(Locale.getDefault()))) {
					field.setAccessible(true);
					if (!normalValue(value, field, newObject)) {
						// 是一个类的对象
						specialValue(jsonObject, jsonName, field, newObject);
					}
					break;
				}

			}
		}

		return newObject;
	}
	
	private static <T> void specialValue(
			JSONObject jsonObject, 
			String jsonName,
			Field field,
			T newObject) {
		try {
		    JSONObject jobject = jsonObject.optJSONObject(jsonName);
		    if (jobject != null) {
				field.set(newObject, jsonObjectToObject(jobject, field.getType()));
		    } else if (field.getType().isArray()) {
		    	JSONArray jarray = jsonObject.optJSONArray(jsonName);
		    	if (jarray == null) {
		    		return;
		    	}
		    	
		    	Class<?> type = field.getType().getComponentType();
		    	Object[] array = (Object[]) Array.newInstance(type, 2);
		    	BoyiaLog.d("yanbo", "getComponentType = " + array.getClass().getSimpleName());
		    	for (int i = 0; i < jarray.length(); i++) {
		    		array[i] = jsonObjectToObject(jarray.getJSONObject(i), type);
		    	}
		    	
		    	//BoyiaLog.d("yanbo", "getComponentType = " + method.toString());
		    	// 奇葩！数组竟然代表了多个参数，
		    	// 例如数组长度为1代表一个参数，
		    	// 数组长度为2代表两个参数。。。
		    	//method.invoke(newObject, new Object[] {array});
				field.set(newObject, array);
		    }
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	private static <T> boolean normalValue(
			Object value, 
			Field field, 
            T object) {
		if (value instanceof String && field.getType() == int.class) {
			try {
				value = Integer.valueOf(String.valueOf(value));

			} catch (Exception e) {
				//value = 0;
				e.printStackTrace();
			}
		} else if (value instanceof String
				&& field.getType() == float.class) {
			try {
				value = Float.valueOf(String.valueOf(value));
			} catch (Exception e) {
				value = 0.0f;
			}
		} else if (value instanceof String
				&& field.getType() == Date.class) {
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
		} else if (!(field.getType() == int.class
				|| field.getType() == String.class)) {
			return false;
		}
		
		try {
			field.set(object, value);
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		return true;
	}
}
