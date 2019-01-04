package com.boyia.app.update;

import com.boyia.app.common.db.BoyiaData;

public class DownloadData extends BoyiaData {
    public static final int PAUSE = 0x1;
    // DOWNLOADING是一种特殊状态，此过程不用存储在数据库中
    public static final int DOWNLOADING = 0x2;
    public static final int FINISHED = 0x3;
    public static final int ERROR = 0x4;

    private String file_url;
    private String file_path;
    private String file_name;
    private int current_size = 0;
    private long max_len = 0;
    private int status = PAUSE;

    public String getFile_url() {
        return file_url;
    }

    public void setFile_url(String file_url) {
        this.file_url = file_url;
    }

    public String getFile_path() {
        return file_path;
    }

    public void setFile_path(String file_path) {
        this.file_path = file_path;
    }

    public String getFile_name() {
        return file_name;
    }

    public void setFile_name(String file_name) {
        this.file_name = file_name;
    }

    public int getCurrent_size() {
        return current_size;
    }

    public void setCurrent_size(int current_size) {
        this.current_size = current_size;
    }

    public long getMax_len() {
        return max_len;
    }

    public void setMax_len(long max_len) {
        this.max_len = max_len;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }
}
