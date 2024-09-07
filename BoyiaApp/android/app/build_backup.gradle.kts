plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
    id("com.google.protobuf")
    id("com.boyia.app.bytrace")
}

apply(from = "$rootDir/scripts/config.gradle.kts")
android {
    namespace =  "com.boyia.app.shell"
    compileSdk = 33

    signingConfigs {
        create("release") {
            keyAlias = "${extra["APP_ALIAS"]}"
            storeFile = File("$rootDir/app/${extra["APP_KEY_STORE"]}")
            keyPassword = "${extra["APP_KEY_PWD"]}"
            storePassword = "${extra["APP_STORE_PWD"]}"
        }
    }

    //compileSdkVersion gradle.constants.sdkVersion
    defaultConfig {
        applicationId = "com.boyia.app.shell"
        minSdk = 24
        targetSdk = 33
        versionCode = 1
        versionName = "1.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        vectorDrawables {
            useSupportLibrary = true
        }
    }

    buildTypes {
        release {
            isDebuggable = true
            isMinifyEnabled = false
            signingConfig = signingConfigs.getByName("release")
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }

        getByName("debug") {
            isMinifyEnabled = false
            isDebuggable = true
            applicationIdSuffix = ".dev"
            versionNameSuffix = "-DEV"
            signingConfig = signingConfigs.getByName("release")
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }

    productFlavors {
        create("boyia") {
            applicationId = "com.boyia.app.shell"
            versionName = "1.0"
            versionCode = 1
            manifestPlaceholders["boyiaChannel"] = "BoyiaApp"
        }

        create("boyiaPlugin") {
            applicationId = "com.boyia.app.plugin"
            versionName = "1.0"
            versionCode = 1
            manifestPlaceholders["boyiaChannel"] = "BoyiaPluginApp"
        }
    }

    sourceSets {
        getByName("main") {
            java.srcDirs(
                "src/main/java",
                "src/main/aidl",
                "src/main/kotlin")
        }

        getByName("boyiaPlugin") {
            java.srcDirs("src/main/plugin")
            //aidl.srcDirs = []
            //manifest.srcFile ''
            //assets.srcDirs = []
            //res.srcDirs = []
        }
    }

    applicationVariants.all {
        val buildType = buildType.name
        outputs.all {
            if (this is com.android.build.gradle.internal.api.ApkVariantOutputImpl) {
                this.outputFileName?.let {
                    val name = this.outputFileName.substring(0, this.outputFileName.indexOf("-"))
                    this.outputFileName = "${name}-${versionCode}-${flavorName}-${buildType}.apk"
                }
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }
    buildFeatures {
        compose = true
    }

    ndkVersion = "26.1.10909125"
}

protobuf {
    protoc {
        artifact = "com.google.protobuf:protoc:3.21.11"
    }
    generateProtoTasks {
        all().forEach {
            it.builtins {
                create("java")
            }
        }
    }
}

dependencies {
    implementation(fileTree(mapOf("dir" to "libs", "dir" to "$rootDir/library", "include" to listOf("*.jar", "*.aar"))))
    //debugImplementation project(path: ':core', configuration: 'debug')
    //releaseImplementation project(path: ':core', configuration: 'release')
    //debugImplementation project(':core')
    //releaseImplementation project(':core')
    //implementation gradle.library.boyia_core
    //api project(':util')
    //api project(':loader')

    api(libs.boyia.util)
    api(libs.boyia.loader)
    api(libs.okio)
    api(libs.okhttp)
    implementation(libs.boyia.core)
    implementation(libs.boyia.debug)
    implementation(libs.netty)

    // use protobuf begin
    implementation(libs.protobuf)
    // use protobuf end

    //implementation project(':advert')
    implementation(libs.multidex)
    implementation(libs.androidx)

    implementation(libs.umeng.common)
    implementation(libs.umeng.sdk)
    implementation(libs.recycler.view)
    implementation(libs.coordinator)
    implementation(libs.multidex)


    //implementation gradle.library.constraintlayout
//    implementation gradle.library.icon_core
//
//    implementation gradle.library.icon_google_lib
//    implementation gradle.library.icon_google_extend_lib
//    implementation gradle.library.icon_material_lib

    // compose lib
    implementation(libs.compose.ui)
    implementation(libs.compose.foundation)
    implementation(libs.compose.material)
    implementation(libs.compose.livedata)
    implementation(libs.compose.uitool)
    implementation(libs.compose.rxjava)
    implementation(libs.compose.activity)
    implementation(libs.multidex)
    implementation(libs.multidex)
    // compose coil
    implementation(libs.coil)
    implementation(libs.compose.coil)

    // 协程
    implementation(libs.coroutines.core)
    implementation(libs.coroutines.android)
}