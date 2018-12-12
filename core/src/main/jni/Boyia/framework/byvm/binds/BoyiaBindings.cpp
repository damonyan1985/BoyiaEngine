/*
============================================================================
 Name        : BoyiaBindings.cpp
 Author      : yanbo
 Version     : Bindings v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#include "BoyiaLib.h"

#define MAX_NATIVE_FUN_SIZE 40
static NativeFunction* sFunTable = NULL;
static LVoid InitLib() {
    sFunTable = new NativeFunction[MAX_NATIVE_FUN_SIZE] {
        { GenIdentByStr("new", 3) , CreateObject },
        { GenIdentByStr("BY_Content", 10) , getFileContent },
        // Array Api Begin
        { GenIdentByStr("BY_GetFromArray", 15),   getElementFromVector },
        { GenIdentByStr("BY_AddInArray", 13),  addElementToVector },
        { GenIdentByStr("BY_GetArraySize", 15), getVectorSize },
        { GenIdentByStr("BY_ClearArray", 13), clearVector },
        { GenIdentByStr("BY_RemoveWidthIndex", 19), removeElementWidthIndex },
        { GenIdentByStr("BY_RemoveFromArray", 18), removeElementFromVector },
        // Array Api End
        { GenIdentByStr("BY_Log", 6),  logPrint },
        { GenIdentByStr("BY_Json", 7),  jsonParseWithCJSON },
        { GenIdentByStr("BY_CreateDocument", 17), createJSDocument },
        { GenIdentByStr("BY_AppendView", 13), appendView },
        { GenIdentByStr("BY_GetRootDocument", 18), getRootDocument },
        { GenIdentByStr("BY_SetDocument", 14), setDocument },
        { GenIdentByStr("BY_RemoveDocument", 17), removeDocument },
        { GenIdentByStr("BY_SetXpos", 10), setViewXpos},
        { GenIdentByStr("BY_SetYpos", 10), setViewYpos},
        { GenIdentByStr("BY_DrawView", 11), drawView},
        { GenIdentByStr("BY_GetViewXpos", 14), getViewXpos },
        { GenIdentByStr("BY_GetViewYpos", 14), getViewYpos },
        { GenIdentByStr("BY_GetViewWidth", 15), getViewWidth },
        { GenIdentByStr("BY_GetViewHeight", 16), getViewHeight },
        { GenIdentByStr("BY_SetViewStyle", 15), setViewStyle },
        { GenIdentByStr("BY_LoadData", 11), loadDataFromNative },
        { GenIdentByStr("BY_SetTouchCallback", 19), setJSTouchCallback },
        { GenIdentByStr("BY_StartScale", 13), startScale },
        { GenIdentByStr("BY_StartOpacity", 15), startOpacity },
        { GenIdentByStr("BY_CallStaticMethod", 19), callStaticMethod },
        { GenIdentByStr("BY_StartTranslate", 17), startTranslate },
        { GenIdentByStr("BY_GetHtmlItem", 14), getHtmlItem },
        { GenIdentByStr("BY_LoadImage", 12), loadImageByUrl },
        { GenIdentByStr("BY_SetViewText", 14), setViewGroupText },
        { GenIdentByStr("BY_SetInputViewText", 19), setInputViewText },
        { GenIdentByStr("BY_AddEventListener", 19), addEventListener },
        { GenIdentByStr("BY_SetToNativeView", 18), setToNativeView },
        { GenIdentByStr("BY_InstanceOfClass", 18), instanceOfClass },
        { GenIdentByStr("BY_CreateViewGroup", 18), createViewGroup},
        { GenIdentByStr("BY_SetImageUrl", 14), setImageUrl},
        { GenIdentByStr("BY_ViewCommit", 13), viewCommit},
        // End
        { 0,  NULL }
    };

    InitNativeFun(sFunTable);
}

extern void CompileScript(char* code) {
    if (!sFunTable) {
        InitLib();
    }

    CompileCode(code);
}
