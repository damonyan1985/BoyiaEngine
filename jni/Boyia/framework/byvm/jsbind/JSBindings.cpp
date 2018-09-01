/*
============================================================================
 Name        : JSBindings.cpp
 Author      : yanbo
 Version     : Bindings v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#include "BoyiaLib.h"

static NativeFunMap* sFunMap = NULL;
static LVoid initLib() {
	NativeFunction funs[] = {
		{ GenIdentByStr("JS_Content", 10) , getFileContent },
		{ GenIdentByStr("JS_GetFromArray", 15),   getElementFromVector },
		{ GenIdentByStr("JS_AddInArray", 13),  addElementToVector },
		{ GenIdentByStr("JS_GetArraySize", 15), getVectorSize },
		{ GenIdentByStr("JS_ClearArray", 13), clearVector },
		{ GenIdentByStr("JS_Log", 6),  logPrint },
		{ GenIdentByStr("JS_Json", 7),  jsonParseWithCJSON },
		{ GenIdentByStr("JS_CreateDocument", 17), createJSDocument },
		{ GenIdentByStr("JS_AppendView", 13), appendView },
		{ GenIdentByStr("JS_GetRootDocument", 18), getRootDocument },
		{ GenIdentByStr("JS_SetDocument", 14), setDocument },
		{ GenIdentByStr("JS_RemoveDocument", 17), removeDocument },
		{ GenIdentByStr("JS_SetXpos", 10), setViewXpos},
		{ GenIdentByStr("JS_SetYpos", 10), setViewYpos},
		{ GenIdentByStr("JS_DrawView", 11), drawView},
		{ GenIdentByStr("JS_RemoveFromArray", 18), removeElementFromVector },
		{ GenIdentByStr("JS_GetViewXpos", 14), getViewXpos },
		{ GenIdentByStr("JS_GetViewYpos", 14), getViewYpos },
		{ GenIdentByStr("JS_GetViewWidth", 15), getViewWidth },
		{ GenIdentByStr("JS_GetViewHeight", 16), getViewHeight },
		{ GenIdentByStr("JS_SetViewStyle", 15), setViewStyle },
		{ GenIdentByStr("JS_LoadData", 11), loadDataFromNative },
		{ GenIdentByStr("JS_SetTouchCallback", 19), setJSTouchCallback },
		{ GenIdentByStr("JS_StartScale", 13), startScale },
		{ GenIdentByStr("JS_StartOpacity", 15), startOpacity },
		{ GenIdentByStr("JS_CallStaticMethod", 19), callStaticMethod },
		{ GenIdentByStr("JS_StartTranslate", 17), startTranslate },
		{ GenIdentByStr("JS_GetHtmlItem", 14), getHtmlItem },
		{ GenIdentByStr("JS_LoadImage", 12), loadImageByUrl },
		{ GenIdentByStr("JS_SetViewText", 14), setViewGroupText },
		{ GenIdentByStr("JS_SetInputViewText", 19), setInputViewText },
		{ GenIdentByStr("JS_AddEventListener", 19), addEventListener },
		{ GenIdentByStr("JS_SetJSViewToNativeView", 24), setJSViewToNativeView },
		{ GenIdentByStr("JS_InstanceOfClass", 18), instanceOfClass },
		{ GenIdentByStr("JS_CreateViewGroup", 18), createViewGroup},
		{ GenIdentByStr("JS_SetImageUrl", 14), setImageUrl},
		{ GenIdentByStr("JS_ViewCommit", 13), viewCommit}
	};

	LInt size = sizeof(funs)/sizeof(NativeFunction);
	sFunMap = new NativeFunMap;
	sFunMap->mFun = new NativeFunction[size];
	sFunMap->mSize = size;
	while (size)
	{
		sFunMap->mFun[--size].mNameKey = funs[size].mNameKey;
		sFunMap->mFun[size].mAddr = funs[size].mAddr;
	}
}

extern void compileJs(char* code)
{
    if (NULL == sFunMap)
    {
    	initLib();
    }

    CompileCode(code, sFunMap);
}
