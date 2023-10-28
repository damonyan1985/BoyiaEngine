package com.boyia.app.bytrace

import com.android.build.api.variant.AndroidComponentsExtension
import org.gradle.api.Plugin
import org.gradle.api.Project

class BoyiaTracePlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val components = project.extensions.getByType(AndroidComponentsExtension::class.java)
        components.onVariants { variant ->
        }
    }
}