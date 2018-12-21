package com.boyia.app;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import com.boyia.app.base.BaseActivity;
import com.boyia.app.utils.LoadUtil;

public class MainActivity extends BaseActivity {
    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        final Intent intent = new Intent(this, BoyiaActivity.class);
        startActivity(intent);
    }
}
