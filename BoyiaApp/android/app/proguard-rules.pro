# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

-dontwarn com.squareup.**
-dontwarn okio.**

# okhttp 3
-keepattributes Signature
-keepattributes *Annotation*
-keep class okhttp3.** { *; }
-keep interface okhttp3.** { *; }
-dontwarn okhttp3.**

# boyia util
-keep class com.boyia.app.core.** {*;}
-keep class com.boyia.app.common.utils.BoyiaUtils {*;}
-keep class com.boyia.app.common.utils.ZipOperation {*;}
-keep public class * extends android.app.Activity
-keep public class * extends android.app.Application
-keep public class * extends android.app.Service
-keep public class * extends android.content.BroadcastReceiver
-keep public class * extends android.content.ContentProvider
-keep public class * extends android.app.backup.BackupAgentHelper
-keep public class * extends android.database.sqlite.SQLiteOpenHelper{*;}

# DB Annotation
#-keep public class * extends java.lang.annotation.Annotation {*;}
#-keep public class com.boyia.app.common.db.BoyiaData {*;}
#-keep @com.boyia.app.common.db.DBAnnotation class * {*;}
-keep class com.boyia.app.common.db.DBAnnotation {*;}
-keep class * {
    @com.boyia.app.common.db.DBAnnotation$DBColumn <fields>;
}

#-keep public class com.boyia.app.update.DownloadData {*;}
-keep public class java.nio.* { *; }

# netty begin
-keepattributes Signature,InnerClasses
-keepclasseswithmembers class io.netty.** {
    *;
}
-dontwarn io.netty.**
-dontwarn sun.**
# netty end

# protobuf
-dontwarn com.google.**
-keep class com.google.protobuf.** {*;}
-keepattributes Annotation
-keepattributes Signature

# rxJava
-keep class rx.** {*;}

# glide
-dontwarn com.bumptech.glide.**
-keep class com.bumptech.glide.**{*;}
-keep public class * implements com.bumptech.glide.module.GlideModule
-keep public class * extends com.bumptech.glide.AppGlideModule
-keep public enum com.bumptech.glide.load.resource.bitmap.ImageHeaderParser$** {
  **[] $VALUES;
  public *;
}

# ttad
-keep class com.bytedance.sdk.openadsdk.** { *; }
-keep public interface com.bytedance.sdk.openadsdk.downloadnew.** {*;}
-keep class com.pgl.sys.ces.** {*;}
-keep class com.bytedance.embed_dr.** {*;}
-keep class com.bytedance.embedapplog.** {*;}

# umeng
-keep class com.umeng.** {*;}
-keepclassmembers class * {
   public <init> (org.json.JSONObject);
}
-keepclassmembers enum * {
    public static **[] values();
    public static ** valueOf(java.lang.String);
}
-keep public class com.boyia.app.R$*{
    public static final int *;
}