package com.boyia.app.core.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.view.View;

public class TestPlatformViewFactory implements PlatformViewFactory {
    @Override
    public PlatformView createView(Context context, String viewId) {
        return new TestPlatformView(context);
    }

    public static class TestPlatformView implements PlatformView {
        private View mView;
        public TestPlatformView(Context context) {
            mView = new TestPlatformViewImpl(context);
        }

        @Override
        public View getView() {
            return mView;
        }
    }

    public static class TestPlatformViewImpl extends View {
        public TestPlatformViewImpl(Context context) {
            super(context);
        }

        @Override
        public void onDraw(Canvas canvas) {
            super.onDraw(canvas);

            //canvas.drawColor(Color.BLUE);
//            Paint paint1 = new Paint();
//            paint1.setColor(Color.BLUE);
//            canvas.drawRect(
//                    new Rect(0, 0, 300, 300),
//                    paint1);
//            Paint paint = new Paint();
//            paint.setColor(Color.YELLOW);
//            paint.setTextSize(36);
//            canvas.drawText("TestPlatformView",
//                    0, 0, paint);
        }
    }
}
