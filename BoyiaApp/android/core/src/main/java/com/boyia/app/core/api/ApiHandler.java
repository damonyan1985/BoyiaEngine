package com.boyia.app.core.api;

import android.content.Intent;

import org.json.JSONObject;

public interface ApiHandler {
    void handle(JSONObject params, ApiHandlerCallback callback);
    default boolean onActivityResult(int requestCode, int resultCode, Intent data) {
        return false;
    }

    interface ApiHandlerCallback {
        void callback(String json);
    }
}
