package com.boyia.app.common.db;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.db.DBAnnotation.DBKey;
import com.boyia.app.common.db.DBAnnotation.DBColumn;
import com.boyia.app.common.db.DBAnnotation.DBTable;
import com.boyia.app.common.utils.BoyiaUtils;

// sqlite orm映射工具
public class BoyiaDAO<T extends BoyiaData> {
    private static final String TAG = "BoyiaDAO";
    private SQLiteDatabase mDb;

    public BoyiaDAO(SQLiteDatabase db) {
        this.mDb = db;
    }

    public T setBeanData(Cursor cursor, Class<T> cls) {
        if (cursor == null) {
            return null;
        }

        T bean = null;
        try {
            bean = cls.newInstance();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        setBeanField(cls, bean, cursor);
        return bean;
    }

    private void setBeanField(Class<?> cls, T bean, Cursor cursor) {
        Field[] fields = cls.getDeclaredFields();
        for (Field field : fields) {
            field.setAccessible(true);
            // 判断是否为静态变量或者常量
            if (Modifier.isStatic(field.getModifiers())
                    || Modifier.isFinal(field.getModifiers())) {
                continue;
            }
            setFieldValue(bean, field, cursor);
        }

        // 设置父类元素
        if (!cls.getSuperclass().equals(Object.class)) {
            BoyiaLog.d("yanbo", "setBeanField classname=" + cls.getSimpleName()
                    + "!=" + Object.class.getSimpleName());
            setBeanField(cls.getSuperclass(), bean, cursor);
        }
    }

    private String getColumnName(Field field) {
        DBColumn dbField = field.getAnnotation(DBColumn.class);
        return (dbField == null || BoyiaUtils.isTextEmpty(dbField.name())) ?
                field.getName() : dbField.name();
    }

    private void setFieldValue(T bean, Field field, Cursor cursor) {
        try {
            if (bean != null) {
                String columnName = getColumnName(field);
                if (field.getType() == Integer.class) {
                    field.set(bean, cursor.getInt(cursor
                            .getColumnIndex(columnName)));
                } else if (field.getType() == String.class) {
                    field.set(bean, cursor.getString(cursor
                            .getColumnIndex(columnName)));
                } else if (field.getType() == Float.class) {
                    field.set(bean, cursor.getFloat(cursor
                            .getColumnIndex(columnName)));
                } else if (field.getType() == Long.class) {
                    field.set(bean, cursor.getLong(cursor
                            .getColumnIndex(columnName)));
                }
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private boolean isTableKey(Field field) {
        return field.getAnnotation(DBKey.class) != null;
    }

    public ContentValues setDbData(T bean, Class<T> cls) {
        ContentValues cv = new ContentValues();

        Field[] fields = cls.getDeclaredFields();
        for (Field field : fields) {
            field.setAccessible(true);
            // 判断是否为静态变量或者常量
            if (Modifier.isStatic(field.getModifiers())
                    || Modifier.isFinal(field.getModifiers())) {
                continue;
            }

            if (isTableKey(field)) {
                continue;
            }

            try {
                String columnName = getColumnName(field);
                if (field.getType() == int.class) {
                    cv.put(columnName, (Integer) field.get(bean));
                } else if (field.getType() == String.class) {
                    cv.put(columnName, (String) field.get(bean));
                } else if (field.getType() == float.class) {
                    cv.put(columnName, field.getFloat(bean));
                } else if (field.getType() == long.class) {
                    long value = field.getLong(bean); // Field.getLong()传入的参数必须是包含基础类型字段的对象
                    BoyiaLog.d(TAG, "setDbData long value = " + value);
                    cv.put(columnName, value);
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }

        BoyiaLog.d(TAG, "setDbData cv = " + cv.toString());
        return cv;
    }

    // 需要被重载
    public String getTableName() {
        Type type = ((ParameterizedType) getClass().getGenericSuperclass()).getActualTypeArguments()[0];
        Class<?> clzz = (Class<?>) type;
        BoyiaLog.d(TAG, "getTableName=" + clzz.getAnnotation(DBTable.class).name());
        return clzz.getAnnotation(DBTable.class).name();
    }

    public long insert(T bean, Class<T> cls) {
        ContentValues cv = setDbData(bean, cls);
        return mDb.insert(getTableName(), null, cv);
    }

    public void update(T bean, int id, Class<T> cls) {
        ContentValues cv = setDbData(bean, cls);
        mDb.update(getTableName(), cv, "id = ?",
                new String[]{String.valueOf(id)});
    }

    public void delete(int id) {
        mDb.delete(getTableName(), "id = ?",
                new String[]{String.valueOf(id)});
    }

    public T queryById(int id, Class<T> cls) {
        T appInfo = null;
        Cursor cursor = mDb.rawQuery("select * from " + getTableName()
                + " where id = ?", new String[]{String.valueOf(id)});
        if (cursor.moveToFirst()) {
            appInfo = setBeanData(cursor, cls);
        }
        cursor.close();
        return appInfo;
    }

    public List<T> queryAll(Class<T> cls) {
        List<T> Infos = null;
        Cursor cursor = mDb.rawQuery("select * from " + getTableName(),
                new String[]{});
        while (cursor.moveToNext()) {
            if (null == Infos) {
                Infos = new ArrayList<T>();
            }
            T appInfo = setBeanData(cursor, cls);
            Infos.add(appInfo);
        }
        cursor.close();
        return Infos;
    }
}

