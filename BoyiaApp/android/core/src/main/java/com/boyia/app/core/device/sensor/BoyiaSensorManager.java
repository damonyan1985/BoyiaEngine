package com.boyia.app.core.device.sensor;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.util.SparseArray;

import com.boyia.app.common.BaseApplication;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * 传感器管理
 */
public class BoyiaSensorManager {
    public static final int ACCELER_TYPE = 1;
    public static final int MAGNETIC_TYPE = 2;

    /**
     * 用于生成唯一id
     */
    private AtomicInteger mIdBase;
    private SparseArray<SensorItem> mSensorItems;

    public static class SensorManagerHolder {
        private static BoyiaSensorManager INSTANCE = new BoyiaSensorManager();
    }

    public static BoyiaSensorManager getInstance() {
        return SensorManagerHolder.INSTANCE;
    }

    private BoyiaSensorManager() {
        mIdBase = new AtomicInteger(0);
        mSensorItems = new SparseArray<>();
    }

    private int genSensorItem(int type) {
        Sensor sensor = getSensor(type);
        if (sensor == null) {
            return 0;
        }

        int id = mIdBase.incrementAndGet();
        mSensorItems.put(id, new SensorItem(id, sensor));
        return id;
    }

    private Sensor getSensor(int type) {
        SensorManager manager = (SensorManager) BaseApplication.getInstance().getSystemService(Context.SENSOR_SERVICE);
        switch (type) {
            case ACCELER_TYPE:
                return manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
            case MAGNETIC_TYPE:
                return manager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        }

        return null;
    }

    /**
     * SensorItem provide id for boyia app
     */
    private static class SensorItem {
        public int mId;
        public Sensor mSensor;

        public SensorItem(int id, Sensor sensor) {
            mSensor = sensor;
            mId = id;
        }
    }
}
