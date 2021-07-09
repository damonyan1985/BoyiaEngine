package com.boyia.app.loader.mue;

import com.boyia.app.loader.job.JobScheduler;

public class TestObservable {
    public static void test() {
        MueTask.create((Subscriber<String> subscriber) -> {
            subscriber.onNext("");
            subscriber.onComplete();
        }).subscribeOn(JobScheduler.jobScheduler())
        .observeOn(MainScheduler.mainScheduler())
        .subscribe(new Subscriber<String>() {
            @Override
            public void onNext(String s) {
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
