package com.boyia.app.loader.mue;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.job.JobScheduler;

public class TestObservable {
    private static final String TAG = "TestObservable";
    public static void test() {
        MueTask.create((Subscriber<String> subscriber) -> {
            // 对应subscribe中的onNext
            subscriber.onNext("123");
            // 对应subscribe中的onComplete
            subscriber.onComplete();
        })
        .map(s -> (Integer.valueOf(s) + 2))
        .subscribeOn(JobScheduler.jobScheduler())
        .observeOn(MainScheduler.mainScheduler())
        .subscribe(new Subscriber<Integer>() {
            @Override
            public void onNext(Integer s) {
                BoyiaLog.d(TAG, "TestObservable result = " + s);
            }

            @Override
            public void onError(Throwable e) {
            }

            @Override
            public void onComplete() {
            }
        });
    }
}
