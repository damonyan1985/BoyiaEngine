package com.boyia.app.shell.update;

import com.boyia.app.common.db.BoyiaDAO;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import java.util.List;

public class DownloadDAO extends BoyiaDAO<DownloadData> {
    public DownloadDAO(SQLiteDatabase db) {
        super(db);
    }

    public DownloadData setDownloadInfo(Cursor cursor) {
        return super.setBeanData(cursor, DownloadData.class);
    }

    private ContentValues setContentValues(DownloadData info) {
        return super.setDbData(info, DownloadData.class);
    }

    public long insert(DownloadData info) {
        return super.insert(info, DownloadData.class);
    }

    public void update(DownloadData info) {
        super.update(info, info.getId(), DownloadData.class);
    }

    public void delete(int id) {
        super.delete(id);
    }

    public DownloadData queryById(int id) {
        return super.queryById(id, DownloadData.class);
    }

    public List<DownloadData> queryAll() {
        return super.queryAll(DownloadData.class);
    }

    public List<DownloadData> query(DownloadData info) {
        return super.queryByFilter(info, DownloadData.class);
    }

    @Override
    public String getTableName() {
        return super.getTableName();
    }
}

