package com.boyia.app;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.Window;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.proto.Server;

public class MainActivity extends BaseActivity {
    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        //this.getSystemService(Context.WINDOW_SERVICE);
        final Intent intent = new Intent(this, BoyiaActivity.class);
        startActivity(intent);
    }
}
