package com.boyia.app.bytrace

import org.objectweb.asm.AnnotationVisitor
import org.objectweb.asm.MethodVisitor
import org.objectweb.asm.Opcodes

class BoyiaTraceMethodVisitor(methodVisitor: MethodVisitor, private val className: String, private val methodName: String?) : MethodVisitor(Opcodes.ASM9, methodVisitor) {
    override fun visitAnnotation(descriptor: String?, visible: Boolean): AnnotationVisitor {
        return super.visitAnnotation(descriptor, visible)
    }

    override fun visitCode() {
        super.visitCode()
        mv.visitLdcInsn("TAG")
        mv.visitLdcInsn("$className------->$methodName")
        mv.visitMethodInsn(Opcodes.INVOKESTATIC, "android/util/Log", "d", "(Ljava/lang/String;Ljava/lang/String;)I", false)
        mv.visitInsn(Opcodes.POP)
    }

    override fun visitEnd() {
        super.visitEnd()
    }
}