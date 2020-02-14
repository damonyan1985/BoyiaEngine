package com.boyia.app.leaker;

import android.os.Debug;
import java.text.SimpleDateFormat;

public class HeapDumper {
    private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("yyyyMMddhhmmss");
    public void dumpHeap() {
        try {
            Debug.dumpHprofData(DATE_FORMAT.format(System.currentTimeMillis()));
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
