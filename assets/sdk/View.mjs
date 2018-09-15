class View {
    prop nativeView = null;
    
    fun addEventListener(type, callback) {
        JS_AddEventListener(this.nativeView, type, callback);
    }
    
    fun setViewToNative() {
        JS_SetJSViewToNativeView(this.nativeView, this);
    }
    
    fun setNativeView(view) {
        this.nativeView = view;
    }
    
    fun setLeft(x) {
        JS_SetXpos(this.nativeView, x);
    }
       
    fun setTop(y) {
        JS_SetYpos(this.nativeView, y);
    }
        
    fun left() {
        return JS_GetViewXpos(this.nativeView);
    }
    
    fun top() {
        return JS_GetViewYpos(this.nativeView);
    }
    
    fun width() {
        return JS_GetViewWidth(this.nativeView);
    }
    
    fun height() {
        return JS_GetViewHeight(this.nativeView);
    }
    
    fun setStyle(style) {
        JS_SetViewStyle(this.nativeView, style);
    }
    
    fun startScale(scale, duration) {
        JS_StartScale(this.nativeView, scale, duration);
    }
    
    fun startOpacity(opacity, duration) {
        JS_StartOpacity(this.nativeView, opacity, duration);
    }
    
    fun draw() {
        JS_DrawView(this.nativeView);
    }
    
    fun startTranslate(x, y, duration) {
        JS_StartTranslate(this.nativeView, x, y, duration);
    }
    
    fun commit() {
        JS_ViewCommit(this.nativeView);
    }
}

class ViewGroup extends View {
    prop children = null;
    fun createViewGroup(nv) {
        var vg = new(ViewGroup);
        vg.nativeView = nv;
        vg.children = new(Array);
        return vg;
    }
    
    fun createVgWithId(id, sel) {
        var vg = new(ViewGroup);
        vg.nativeView = JS_CreateViewGroup(id, sel);
        vg.children = new(Array);
        return vg;
    }
 
    fun setText(text) {
        JS_SetViewText(this.nativeView, text);
    }
    
    fun appendView(view) {
        Util.log("ViewGroup appendView");
        //if (this.children) {
            this.children.put(view);
        //}
        JS_AppendView(this.nativeView, view.nativeView);
    }
}