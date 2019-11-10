package com.boyia.app.common.db;

;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import com.boyia.app.common.utils.BoyiaLog;

public class BoyiaDB extends SQLiteOpenHelper {
    private static final String TAG = BoyiaDB.class.getSimpleName();
    private static final int DEFAULT_DB_VERSION = 81;
    private static final String DEFAULT_DB_NAME = "BoyiaDB";
    private Context mContext;
    private String mDbName = null;

    public BoyiaDB(Context context, String dbName) {
        super(context, dbName, null, DEFAULT_DB_VERSION);
        mContext = context;
    }

    protected Context getContext() {
        return mContext;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        try {
            initDatabase(db);
        } catch (Exception e) {
            mContext.deleteDatabase(getDBName());
            initDatabase(db);
        }
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        BoyiaLog.i(TAG, "onUpgrade oldVersion = " + oldVersion + "; newVersion = "
                + newVersion);

        try {
            // delete exist database before done upgrade.
            mContext.deleteDatabase(mDbName);
            initDatabase(db);
        } catch (Exception e) {
            mContext.deleteDatabase(mDbName);
            initDatabase(db);
        }
    }

    private void initDatabase(SQLiteDatabase db) {
        String sqls = getSqlString();
        if (null != sqls && sqls.length() > 0) {
            String[] sqlArray = sqls.split(";");
            for (String sql : sqlArray) {
                db.execSQL(sql);
            }
        }
    }

    protected String getSqlString() {
        return null;
    }

    protected String getDBName() {
        return DEFAULT_DB_NAME;
    }
}
