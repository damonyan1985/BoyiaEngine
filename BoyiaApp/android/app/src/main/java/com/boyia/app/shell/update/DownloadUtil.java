package com.boyia.app.shell.update;

import com.boyia.app.shell.BoyiaAppDB;
import com.boyia.app.common.BaseApplication;

import android.database.sqlite.SQLiteDatabase;
import java.util.List;

public class DownloadUtil {
	protected static SQLiteDatabase getAppDB() {
        SQLiteDatabase db = null;
        try {
            BoyiaAppDB dbHelper = new BoyiaAppDB(BaseApplication.getInstance());
            db = dbHelper.getWritableDatabase();
        } catch (Exception e) {
            if (db != null) {
                db.close();
            }

            db = null;
        }

        return db;
    }
	
    public static void addDownloadInfo(DownloadData info) {
        SQLiteDatabase db = getAppDB();
        if (db != null) {
        	DownloadDAO infoDAO = new DownloadDAO(db);
        	info.setId((int)infoDAO.insert(info));
            db.close();
        }
    }
	
	public static List<DownloadData> getDownloadList() {
        SQLiteDatabase db = getAppDB();
        List<DownloadData> list = null;
        if (db != null) {
        	DownloadDAO infoDAO = new DownloadDAO(db);
            list = infoDAO.queryAll();
            db.close();
        }

        return list;
    }
	
    public static void updateDownloadInfo(DownloadData info) {
    	SQLiteDatabase db = getAppDB();
        if (db != null) {
        	DownloadDAO infoDAO = new DownloadDAO(db);
            infoDAO.update(info);
            db.close();
        }
    }

    public static String getDownloadTableName() {
	    String name = "";
        SQLiteDatabase db = getAppDB();
        if (db != null) {
            DownloadDAO infoDAO = new DownloadDAO(db);
            name = infoDAO.getTableName();
            db.close();
        }

        return name;
    }

    public static List<DownloadData> getDownloadList(DownloadData filter) {
        SQLiteDatabase db = getAppDB();
        List<DownloadData> list = null;
        if (db != null) {
            DownloadDAO infoDAO = new DownloadDAO(db);
            list = infoDAO.query(filter);
            db.close();
        }

        return list;
    }
}
