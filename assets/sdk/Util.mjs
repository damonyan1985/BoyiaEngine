var true = 1, false = 0, null = 0;
//var touchDown = 1, touchUp = 2, touchMove = 4;
class Util {
    fun log(info) {
        JS_Log(info);
    }
    
    fun json(info) {
        JS_Json(info);
    }
    
    fun instanceof(obj, cls) {
        JS_InstanceOfClass(obj, cls);
    }
    
    fun showToast(info) {
        JS_CallStaticMethod("com/boyia/app/utils/BoyiaUtils", "showToast", "(Ljava/lang/String;)V", info);
    }
}