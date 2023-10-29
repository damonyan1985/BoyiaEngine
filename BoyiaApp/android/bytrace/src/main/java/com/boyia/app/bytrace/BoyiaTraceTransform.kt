package com.boyia.app.bytrace

import com.android.build.api.instrumentation.AsmClassVisitorFactory
import com.android.build.api.instrumentation.ClassContext
import com.android.build.api.instrumentation.InstrumentationParameters
import org.objectweb.asm.ClassVisitor

abstract class BoyiaTraceTransform : AsmClassVisitorFactory<InstrumentationParameters.None> {
    override fun createClassVisitor(
            classContext: ClassContext,
            nextClassVisitor: ClassVisitor
    ): ClassVisitor {
        return BoyiaTraceClassVisitor(nextClassVisitor, classContext.currentClassData.className)
    }
}