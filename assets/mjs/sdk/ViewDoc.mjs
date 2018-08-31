var root = null;
class ViewDoc extends View {
    fun rootView() {
        if (root == null) {
            root = new(ViewDoc);
            root.nativeView = JS_GetRootDocument();
        }
       
        return root;
    }
    
    fun createView(view, layout) {
        var v = new(view);
        v.createDoc(layout);
        v.setViewToNative();
        Util.log("begin addchild 5");
        return v;
    }
    
    fun createDoc(docText) {
        nativeView = JS_CreateDocument(docText);
    }
    
    fun setDoc(doc) {
        JS_SetDocument(this.nativeView, doc);
    }
    
    fun removeView(view) {
        JS_RemoveDocument(this.nativeView, view);
    }
   
    fun itemByID(view, id) {
        var v = new(view);
        if (Util.instanceof(v, ViewGroup)) {
            v.children = new(Array);
        }
        v.nativeView = JS_GetHtmlItem(this.nativeView, id);
        v.setViewToNative();
        return v;
    }
}
