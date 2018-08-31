class InputView extends View {

    fun initView() {
        this.addEventListener(1, this.touchDownCallback);
    }
    
    fun touchDownCallback() {
        //JS_CallStaticMethod("com/boyia/app/core/MiniUIView", "showKeyboard", "(I)V", this.nativeView);
    }
}