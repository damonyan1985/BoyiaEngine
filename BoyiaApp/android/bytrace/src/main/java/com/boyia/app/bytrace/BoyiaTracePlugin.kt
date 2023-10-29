package com.boyia.app.bytrace

import com.android.build.api.instrumentation.FramesComputationMode
import com.android.build.api.instrumentation.InstrumentationScope
import com.android.build.api.variant.AndroidComponentsExtension
import com.android.build.api.variant.ApplicationVariant
import org.gradle.api.Plugin
import org.gradle.api.Project

class BoyiaTracePlugin : Plugin<Project> {
    override fun apply(project: Project) {
        val components = project.extensions.getByType(AndroidComponentsExtension::class.java)
        components.onVariants { variant ->
            if (variant is ApplicationVariant) {
                variant.instrumentation.transformClassesWith(BoyiaTraceTransform::class.java, InstrumentationScope.ALL) {}
                variant.instrumentation.setAsmFramesComputationMode(
                        FramesComputationMode.COMPUTE_FRAMES_FOR_INSTRUMENTED_METHODS
                )
            }
        }
    }
}