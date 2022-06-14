package com.boyia.app.core.api;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
import android.provider.MediaStore;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.launch.BoyiaAppInfo;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.core.api.ApiConstants.ApiKeys;
import com.boyia.app.core.api.ApiConstants.ApiNames;
import com.boyia.app.core.api.ApiConstants.ApiRequestCode;
import com.boyia.app.core.api.ApiHandler.ApiHandlerCallback;

import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ApiImplementation {
    private static final String TAG = "ApiImplementation";
    private IBoyiaSender mSender;
    private WeakReference<Activity> mActivityRef;
    private Map<String, ApiCreator> mHandlerMap;
    // 处理activityresult
    private List<ApiHandler> mResultHandlers;

    public ApiImplementation(IBoyiaSender sender, Activity activity) {
        mSender = sender;
        mActivityRef = new WeakReference<>(activity);
        mHandlerMap = new HashMap<>();
        mResultHandlers = new ArrayList<>();
        initCommon();
    }

    private void initCommon() {
        registerHandler(ApiNames.NOTIFICATION_NAME, () -> (params, callback) -> {
            sendNotification(null, null);
            callback.callback(null);
        });

        registerHandler(ApiNames.LOCAL_SHARE_SET, () -> (params, callback) -> {
            try {
                setShare(params.getString(ApiKeys.IPC_SHARE_KEY), params.getString(ApiKeys.IPC_SHARE_VALUE));
            } catch (JSONException e) {
                e.printStackTrace();
                BoyiaLog.e(TAG, ApiNames.LOCAL_SHARE_SET + " error: ", e);
            }
        });

        registerHandler(ApiNames.PICK_IMAGE, () -> new ApiHandler() {
            private ApiHandlerCallback mCallback;
            @Override
            public void handle(JSONObject params, ApiHandlerCallback callback) {
                mCallback = callback;
                mResultHandlers.add(this);
                pickImage();
            }

            @Override
            public boolean onActivityResult(int requestCode, int resultCode, Intent data) {
                if (requestCode == ApiRequestCode.PHOTO_CODE) {
                    Uri uri = data.getData();
                    BoyiaLog.d(TAG, "image uri = " + uri);
                    mCallback.callback(null);
                    return true;
                }

                return false;
            }
        });
    }

    public void registerHandler(String method, ApiCreator creator) {
        mHandlerMap.put(method, creator);
    }

    public void sendNotification(String action, BoyiaAppInfo info) {
        Bundle bundle = new Bundle();
        bundle.putString(ApiKeys.NOTIFICATION_ACTION, action);
        bundle.putString(ApiKeys.NOTIFICATION_TITLE, info.mAppName);
        bundle.putString(ApiKeys.NOTIFICATION_ICON, info.mAppCover);
        BoyiaIpcData data =
                new BoyiaIpcData(ApiNames.NOTIFICATION_NAME, bundle);
        sendData(data);
    }

    /**
     * 避免多进程操作sharepreference
     * @param key
     * @param value
     */
    public void setShare(String key, String value) {
        Bundle bundle = new Bundle();
        bundle.putString(ApiKeys.IPC_SHARE_KEY, key);
        bundle.putString(ApiKeys.IPC_SHARE_VALUE, value);
        BoyiaIpcData data = new BoyiaIpcData(
                ApiNames.LOCAL_SHARE_SET,
                bundle
        );

        sendData(data);
    }

    /**
     * 打开相册
     */
    public void pickImage() {
        if (mActivityRef.get() == null) {
            return;
        }
        Intent pickIntent = new Intent(Intent.ACTION_PICK,
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        pickIntent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
        mActivityRef.get().startActivityForResult(pickIntent, ApiRequestCode.PHOTO_CODE);
    }

    private void sendData(BoyiaIpcData data) {
        try {
            mSender.sendMessageAsync(data, new IBoyiaIpcCallback() {
                @Override
                public void callback(BoyiaIpcData boyiaIpcData) {
                    BoyiaLog.d(TAG, "BoyiaApp boyiaIpcData = " + boyiaIpcData);
                }

                @Override
                public IpcScheduler scheduler() {
                    return runnable -> JobScheduler.jobScheduler().sendJob(runnable::run);
                }
            });
        } catch (RemoteException e) {
            BoyiaLog.e(TAG, String.format("sendData %s error", data.getMethod()), e);
        }
    }

    /**
     * 接受boyia参数，json格式
     * @param json
     */
    public void handleApi(String json, ApiHandlerCallback callback) {
        try {
            JSONObject args = new JSONObject(json);
            String method = args.getString(ApiConstants.API_METHOD_NAME);
            JSONObject params = args.getJSONObject(ApiConstants.API_METHOD_PARAMS);

            ApiCreator creator = mHandlerMap.get(method);
            if (creator == null) {
                return;
            }

            creator.create().handle(params, callback);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public boolean onActivityResult(int requestCode, int resultCode, Intent data) {
        for (ApiHandler handler : mResultHandlers) {
            if (handler.onActivityResult(requestCode, resultCode, data)) {
                // 完成之后从列表中移除
                mResultHandlers.remove(handler);
                return true;
            }
        }

        return false;
    }

    public interface ApiCreator {
        ApiHandler create();
    }
}
