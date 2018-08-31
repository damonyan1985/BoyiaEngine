#include "ArmFunction.h"
#include "SalLog.h"
#include <android/log.h>

// r            使用任何可用的通用寄存器
// m            使用变量的内存地址
//--------------------------------------
// +            可读可写
// =            只写
// &            该输出操作数不能使用输入部分使用过的寄存器，只能 +& 或 =& 方式使用
LVoid ArmMemeset(LVoid* dest, LInt val, LInt len)
{
//	__asm__ __volatile__(
//	    "ldr r0,%0 \t\n"
//	    "ldr r1,%1 \t\n"
//		"ldr r2,%2 \t\n"
//		"armmemset: strb r1, [r0], #1 \t\n"
//		"subs r2, r2, #1 \t\n"
//		"bne armmemset \t\n"
//	    :"+m"(dest), "+m"(val), "+m"(len));
}

LVoid ArmMemcpy(LVoid* dest, const LVoid* src, LInt len)
{
	//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "ArmMemcpy r1=%x len=%d", (int)src, len);
	// ldrb r3, [r1], #1 => [r1]->r3, r1+=1
	// strb r3, [r0], #1 => r3->[r0], r0+=1
//	__asm__ __volatile__(
//		"ldr r2,%2 \t\n"
//		"cmp r2, #0 \t\n"
//		"ble end\t\n"
//		"ldr r0,%0 \t\n"
//		"ldr r1,%1 \t\n"
//		"armmemcpy: ldrb r3, [r1], #1\t\n"
//		"strb r3, [r0], #1 \t\n"
//		"subs r2, r2, #1 \t\n"
//		"bne armmemcpy \t\n"
//		"end: \t\n"
//		:"+m"(dest), "+m"(src), "+m"(len));
}
