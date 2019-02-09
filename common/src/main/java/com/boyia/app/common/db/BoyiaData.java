package com.boyia.app.common.db;

import com.boyia.app.common.db.DBAnnotation.DBColumn;
public class BoyiaData {
    @DBColumn(name = "id")
    private int id;
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }
}

