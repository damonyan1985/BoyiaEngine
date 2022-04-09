package com.boyia.app.shell;

import java.io.IOException;
import java.io.InputStream;

import com.boyia.app.common.db.BoyiaDB;
import com.boyia.app.common.utils.BoyiaFileUtil;

import android.content.Context;

public class BoyiaAppDB extends BoyiaDB {
	private static final String APP_DB_NAME = "boyia_app_db";
	private static final String DB_FILE = "sql/boyia_app.sql";

	public BoyiaAppDB(Context context) {
		super(context, APP_DB_NAME);
	}

	@Override
	protected String getSqlString() {
		String sqls;
        try {
            InputStream is = getContext().getAssets().open(DB_FILE);
            sqls = BoyiaFileUtil.readByInputStream(is);
        } catch (IOException e) {
            sqls = null;
            e.printStackTrace();
        }
        
        return sqls;
	}
}
