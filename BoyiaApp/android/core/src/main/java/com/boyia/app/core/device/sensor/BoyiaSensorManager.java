package com.boyia.app.core.device.sensor;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
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

    private Sensor getSensor(SensorManager manager, int type) {
        switch (type) {
            case ACCELER_TYPE:
                return manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
            case MAGNETIC_TYPE:
                return manager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD); // 磁场变化
            default:
                return null;
        }
    }

    public int addListener(int type) {
        SensorManager manager = (SensorManager) BaseApplication.getInstance().getSystemService(Context.SENSOR_SERVICE);
        Sensor sensor = getSensor(manager, type);

        if (sensor == null) {
            return 0;
        }

        SensorEventListener listener = new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent event) {
                // TODO 调用native方法发送
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
                // TODO
            }
        };

        int id = mIdBase.incrementAndGet();
        mSensorItems.put(id, new SensorItem(id, listener));

        manager.registerListener(listener, sensor, SensorManager.SENSOR_DELAY_GAME);
        // id需要回传给app，可以使用id来接受信息
        return id;
    }

    public void removeListener(int type, int id) {
        SensorItem item = mSensorItems.get(id);

        mSensorItems.remove(id);

        SensorManager manager = (SensorManager) BaseApplication.getInstance().getSystemService(Context.SENSOR_SERVICE);
        Sensor sensor = getSensor(manager, type);

        if (sensor == null) {
            return;
        }

        manager.unregisterListener(item.mListener, sensor);
    }

    /**
     * SensorItem provide id for boyia app
     */
    private static class SensorItem {
        public int mId;
        public SensorEventListener mListener;

        public SensorItem(int id, SensorEventListener listener) {
            mId = id;
            mListener = listener;
        }
    }
}
