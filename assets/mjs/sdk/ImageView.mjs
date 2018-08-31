class ImageView extends View {
    fun loadImage(url) {
        Util.log(url);
        JS_LoadImage(this.nativeView, url);
    }
    
    fun setImageUrl(url) {
        JS_SetImageUrl(this.nativeView, url);
    }
}