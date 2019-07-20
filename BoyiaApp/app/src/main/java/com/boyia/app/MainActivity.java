package com.boyia.app;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.Window;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.proto.Server;

import io.reactivex.Observable;
import io.reactivex.ObservableEmitter;
import io.reactivex.ObservableOnSubscribe;
import io.reactivex.Observer;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.annotations.NonNull;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Consumer;
import io.reactivex.schedulers.Schedulers;

public class MainActivity extends BaseActivity {
    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        testRxJava();
        //this.getSystemService(Context.WINDOW_SERVICE);
        final Intent intent = new Intent(this, BoyiaActivity.class);
        startActivity(intent);
    }

    public void testRxJava() {
        Observer<String> observer = new Observer<String>() {
            @Override
            public void onSubscribe(Disposable d) {
            }

            @Override
            public void onNext(String s) {
                BoyiaLog.d("rxJavaTest", "onNext "+Thread.currentThread().getName());
            }

            @Override
            public void onError(Throwable e) {
            }

            @Override
            public void onComplete() {
            }
        };
        Observable observable = Observable.create(new ObservableOnSubscribe<String>() {
            @Override
            public void subscribe(@NonNull ObservableEmitter<String> subscriber) {
                BoyiaLog.d("rxJavaTest", "call "+Thread.currentThread().getName());
                // do something
                subscriber.onNext("");
            }
        })
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread());
        observable.subscribe(observer);
//        observable.subscribe(new Consumer<String>() {
//            @Override
//            public void accept(@NonNull String str) throws Exception {
//                BoyiaLog.d("rxJavaTest", "onNext "+Thread.currentThread().getName());
//            }
//        });
    }
}
