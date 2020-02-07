package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.JobScheduler;

public class TestObservable {
    public static void test() {
        Observable.create((Subscriber<String> subscriber) -> {
            subscriber.onNext("");
            subscriber.onComplete();
        }).subscribeOn(JobScheduler.getInstance())
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
