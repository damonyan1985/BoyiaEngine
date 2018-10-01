#! /bin/bash
cd jni
ndk-build
cd ..
rm -rf bin
rm -rf gen
mkdir -p gen/com/boyia/app
BUILD_TOOL_VERSION=28.0.3
SDK_VERSION=android-28
SDKLIB=sdklib.jar
AAPT_PATH=$ANDROID_SDK_HOME/build-tools/$BUILD_TOOL_VERSION/aapt
echo $AAPT_PATH
$ANDROID_SDK_HOME/build-tools/$BUILD_TOOL_VERSION/aapt package -f -m -J gen -S res -I $ANDROID_SDK_HOME/platforms/$SDK_VERSION/android.jar -M AndroidManifest.xml
## Make bin directory
mkdir bin
javac -target 1.8 -bootclasspath $ANDROID_SDK_HOME/platforms/$SDK_VERSION/android.jar -d bin gen/com/boyia/app/*.java src/com/boyia/app/*.java src/com/boyia/app/ui/*.java  src/com/boyia/app/utils/*.java src/com/boyia/app/file/*.java src/com/boyia/app/base/*.java src/com/boyia/app/http/*.java src/com/boyia/app/input/*.java src/com/boyia/app/loader/*.java src/com/boyia/app/job/*.java src/com/boyia/app/core/*.java src/com/boyia/app/upgrade/*.java
$ANDROID_SDK_HOME/build-tools/$BUILD_TOOL_VERSION/dx --dex --output=bin/classes.dex bin
$ANDROID_SDK_HOME/build-tools/$BUILD_TOOL_VERSION/aapt package -f -A assets -S res -I $ANDROID_SDK_HOME/platforms/$SDK_VERSION/android.jar -M AndroidManifest.xml -F bin/resources.ap_
java -cp $ANDROID_SDK_HOME/tools/lib/$SDKLIB com.android.sdklib.build.ApkBuilderMain bin/BoyiaApp_unsigned.apk -v -u -z bin/resources.ap_ -f bin/classes.dex -rf src -nf libs
keytool -list -v -keystore ~/.android/debug.keystore -alias androiddebugkey -storepass android -keypass android
jarsigner -verbose -keystore ~/.android/debug.keystore -signedjar bin/BoyiaApp_signed.apk bin/BoyiaApp_unsigned.apk  androiddebugkey -storepass android
adb install -r bin/BoyiaApp_signed.apk




