package com.boyia.app.common.db;

import com.boyia.app.common.db.DBAnnotation.DBColumn;
import com.boyia.app.common.db.DBAnnotation.DBKey;

public class BoyiaData {
    @DBKey
    @DBColumn(name = "id")
    private Integer id;
    public Integer getId() {
        return id;
    }

    public void setId(Integer id) {
        this.id = id;
    }
}

