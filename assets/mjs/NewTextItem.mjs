class NewTextItem extends ViewDoc {
    prop title = null;
    prop info = null;
    prop image = null;
    prop titleText = null;
    
    fun initView() {
        if (this.image == null) {
            this.image = this.itemByID(ImageView, "hot");
        }
        if (this.title == null) {
            this.title = this.itemByID(ViewGroup, "title");
        }
        if (this.info == null) {
            this.info = this.itemByID(ViewGroup, "info");
        }
        
        this.addEventListener(2, this.touchDownCallback);
    }
    
    fun touchDownCallback() {
        JS_CallStaticMethod("com/boyia/app/utils/BoyiaUtils", "showToast", "(Ljava/lang/String;)V", this.titleText);
    }
    
    fun setTitle(t) {
        this.titleText = t;
        this.title.setText(t);
    }
    
    fun loadImage(url) {
        this.image.loadImage(url);
    }
    
    fun setImage(url) {
        this.image.setImageUrl(url);
    }
    
    fun setInfo(i) {
        this.info.setText(i);
    }
}