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
-keep public class * extends java.lang.annotation.Annotation {*;}
-keep public class com.boyia.app.common.db.BoyiaData {*;}
-keep public class com.boyia.app.update.DownloadData {*;}
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
