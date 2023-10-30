package com.boyia.app.bytrace

import org.objectweb.asm.ClassVisitor
import org.objectweb.asm.MethodVisitor
import org.objectweb.asm.Opcodes

class BoyiaTraceClassVisitor(nextVisitor: ClassVisitor, private val className: String) :
        ClassVisitor(Opcodes.ASM9, nextVisitor) {
    override fun visit(version: Int, access: Int, className: String?, signature: String?, superName: String?, interfaces: Array<out String>?) {
        super.visit(version, access, className, signature, superName, interfaces)
    }

    override fun visitMethod(access: Int, methodName: String?, desc: String?, signature: String?, exceptions: Array<out String>?): MethodVisitor {
        //return super.visitMethod(access, methodName, descriptor, signature, exceptions)
        val methodVisitor = cv.visitMethod(access, methodName, desc, signature, exceptions)
        return BoyiaTraceMethodVisitor(methodVisitor, className, methodName, access, desc)
    }

    override fun visitEnd() {
        super.visitEnd()
    }
}