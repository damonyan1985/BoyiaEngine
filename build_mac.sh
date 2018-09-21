#! /bin/bash
cd jni
ndk-build
cd ..
rm -r bin
rm -r gen
mkdir -p gen/com/boyia/app
$ANDROID_SDK_HOME/build-tools/27.0.3/aapt package -f -m -J gen -S res -I $ANDROID_SDK_HOME/platforms/android-27/android.jar -M AndroidManifest.xml
## Make bin directory
mkdir bin
javac -target 1.8 -bootclasspath $ANDROID_SDK_HOME/platforms/android-27/android.jar -d bin gen/com/boyia/app/*.java src/com/boyia/app/*.java src/com/boyia/app/utils/*.java src/com/boyia/app/file/*.java src/com/boyia/app/base/*.java src/com/boyia/app/http/*.java src/com/boyia/app/input/*.java src/com/boyia/app/loader/*.java src/com/boyia/app/job/*.java src/com/boyia/app/core/*.java src/com/boyia/app/upgrade/*.java
$ANDROID_SDK_HOME/build-tools/27.0.3/dx --dex --output=bin/classes.dex bin
$ANDROID_SDK_HOME/build-tools/27.0.3/aapt package -f -A assets -S res -I $ANDROID_SDK_HOME/platforms/android-27/android.jar -M AndroidManifest.xml -F bin/resources.ap_
java -cp $ANDROID_SDK_HOME/tools/lib/sdklib-26.0.0-dev.jar com.android.sdklib.build.ApkBuilderMain bin/BoyiaApp_unsigned.apk -v -u -z bin/resources.ap_ -f bin/classes.dex -rf src -nf libs
keytool -list -v -keystore ~/.android/debug.keystore -alias androiddebugkey -storepass android -keypass android
jarsigner -verbose -keystore ~/.android/debug.keystore -signedjar bin/BoyiaApp_signed.apk bin/BoyiaApp_unsigned.apk  androiddebugkey -storepass android
adb install -r bin/BoyiaApp_signed.apk




