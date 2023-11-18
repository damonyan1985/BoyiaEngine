package com.boyia.app.bytrace

import org.objectweb.asm.*
import org.objectweb.asm.commons.AdviceAdapter
import java.util.Stack
import java.util.concurrent.atomic.AtomicInteger

class BoyiaTraceMethodVisitor(methodVisitor: MethodVisitor,
                              private val className: String,
                              private val methodName: String?,
                              access: Int, desc: String?)
    : AdviceAdapter(Opcodes.ASM9, methodVisitor, access, className, desc) {
    companion object {
        const val TRACE_CLASS = "com/boyia/app/shell/util/BoyiaTraceTimeLog"
        const val TRACE_CLASS_METHOD_ENTER = "enter";
        const val TRACE_CLASS_METHOD_EXIT = "exit";
        var traceId = AtomicInteger(0)
        val traceIdQueue = Stack<Int>()
    }

    private fun isNeedVisitMethod(): Boolean {
        if (className.contains("BoyiaTraceTimeLog")) {
            return false
        }
        return methodName != "<clinit>" && methodName != "<init>"
    }

    override fun visitAnnotation(descriptor: String?, visible: Boolean): AnnotationVisitor {
        return super.visitAnnotation(descriptor, visible)
    }

    override fun onMethodEnter() {
        println("BoyiaTraceMethodVisitor $TRACE_CLASS_METHOD_ENTER className:$className, method: $methodName")
        if (isNeedVisitMethod()) {
            traceId.incrementAndGet()
            traceIdQueue.push(traceId.get())
            try {
                mv.visitLdcInsn(traceId.get())
                // 反射调用壳工程中的TRACE_CLASS enter方法
                mv.visitMethodInsn(INVOKESTATIC, TRACE_CLASS, TRACE_CLASS_METHOD_ENTER, "(I)V", false)
            } catch (e: Throwable) {
                e.printStackTrace()
            }
        }
        super.onMethodEnter()
    }

    /**
     * 此方法可能会调用多次，比如有卫语句，return之类时就调用
     */
    override fun onMethodExit(opcode: Int) {
        if (isNeedVisitMethod()) {
            try {
                pushArgs()
                mv.visitMethodInsn(INVOKESTATIC,
                        TRACE_CLASS,
                        TRACE_CLASS_METHOD_EXIT,
                        "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", false)
            } catch (e: Throwable) {
                e.printStackTrace()
            }
        }
        println("BoyiaTraceMethodVisitor $TRACE_CLASS_METHOD_EXIT className:$className, method: $methodName")
        super.onMethodExit(opcode)
    }

    private fun getArgsType() : String {
        val argsTypes = Type.getArgumentTypes(methodDesc) ?: return "null"

        val max = argsTypes.size - 1
        if (max == -1)
            return "[]"

        val b = StringBuilder()
        b.append('[')
        for (i in 0..max) {
            b.append(argsTypes[i].className)
            if (i == max)
                b.append(']').toString()
            b.append(", ")
        }

        return b.toString()
    }

    private fun pushArgs() {
        val id = traceIdQueue.peek()
        mv.visitLdcInsn(id)
        mv.visitLdcInsn(className) // className
        mv.visitLdcInsn(methodName) // methodbName
        mv.visitLdcInsn(getArgsType()) // argsTypes
        mv.visitLdcInsn(returnType.className)
    }

    /**
     * 函数访问结尾，删除traceId
     */
    override fun visitEnd() {
        super.visitEnd()
        try {
            if (isNeedVisitMethod()) {
                traceIdQueue.pop()
            }
        } catch (e: Throwable) {
            e.printStackTrace()
        }
    }
}