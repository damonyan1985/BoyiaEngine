package com.boyia.app.bytrace

import com.android.build.api.instrumentation.*
import org.objectweb.asm.ClassVisitor

/**
 * 以前是Transform，现在直接换成AsmClassVisitorFactory
 */
abstract class BoyiaTraceTransform : AsmClassVisitorFactory<InstrumentationParameters.None> {
    override fun createClassVisitor(
            classContext: ClassContext,
            nextClassVisitor: ClassVisitor
    ): ClassVisitor {
        return BoyiaTraceClassVisitor(nextClassVisitor, classContext.currentClassData.className)
    }

    override fun isInstrumentable(classData: ClassData): Boolean {
        return (classData.className.startsWith("com.boyia.app.shell.login"))
    }
}