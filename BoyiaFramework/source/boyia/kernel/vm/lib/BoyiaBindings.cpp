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
static NativeFunction* sFunTable = kBoyiaNull;
typedef LInt(*NativePtr)(LVoid* vm);
static LVoid AppendNative(LUintPtr id, NativePtr ptr)
{
    static LInt index = 0;
    sFunTable[index++] = { id, ptr };
}

static LVoid InitLib()
{
    sFunTable = new NativeFunction[MAX_NATIVE_FUN_SIZE];
    AppendNative(GenIdentByStr("new", 3), CreateObject);
    AppendNative(GenIdentByStr("BY_Content", 10), getFileContent);
    // Array Api Begin
    AppendNative(GenIdentByStr("BY_GetFromArray", 15), getElementFromVector);
    AppendNative(GenIdentByStr("BY_AddInArray", 13), addElementToVector);
    AppendNative(GenIdentByStr("BY_GetArraySize", 15), getVectorSize);
    AppendNative(GenIdentByStr("BY_ClearArray", 13), clearVector);
    AppendNative(GenIdentByStr("BY_RemoveWidthIndex", 19), removeElementWidthIndex);
    AppendNative(GenIdentByStr("BY_RemoveFromArray", 18), removeElementFromVector);
    // Array Api End
    AppendNative(GenIdentByStr("BY_Log", 6), logPrint);
    AppendNative(GenIdentByStr("BY_Json", 7), jsonParseWithCJSON);
    AppendNative(GenIdentByStr("BY_CreateDocument", 17), createJSDocument);
    AppendNative(GenIdentByStr("BY_AppendView", 13), appendView);
    AppendNative(GenIdentByStr("BY_GetRootDocument", 18), getRootDocument);
    AppendNative(GenIdentByStr("BY_SetDocument", 14), setDocument);
    AppendNative(GenIdentByStr("BY_RemoveDocument", 17), removeDocument);
    AppendNative(GenIdentByStr("BY_SetXpos", 10), setViewXpos);
    AppendNative(GenIdentByStr("BY_SetYpos", 10), setViewYpos);
    AppendNative(GenIdentByStr("BY_DrawView", 11), drawView);
    AppendNative(GenIdentByStr("BY_GetViewXpos", 14), getViewXpos);
    AppendNative(GenIdentByStr("BY_GetViewYpos", 14), getViewYpos);
    AppendNative(GenIdentByStr("BY_GetViewWidth", 15), getViewWidth);
    AppendNative(GenIdentByStr("BY_GetViewHeight", 16), getViewHeight);
    AppendNative(GenIdentByStr("BY_SetViewStyle", 15), setViewStyle);
    AppendNative(GenIdentByStr("BY_LoadData", 11), loadDataFromNative);
    AppendNative(GenIdentByStr("BY_StartScale", 13), startScale);
    AppendNative(GenIdentByStr("BY_StartOpacity", 15), startOpacity);
    AppendNative(GenIdentByStr("BY_CallStaticMethod", 19), callStaticMethod);
    AppendNative(GenIdentByStr("BY_StartTranslate", 17), startTranslate);
    AppendNative(GenIdentByStr("BY_GetHtmlItem", 14), getHtmlItem);
    AppendNative(GenIdentByStr("BY_LoadImage", 12), loadImageByUrl);
    AppendNative(GenIdentByStr("BY_SetViewText", 14), setViewGroupText);
    AppendNative(GenIdentByStr("BY_SetInputViewText", 19), setInputViewText);
    AppendNative(GenIdentByStr("BY_AddEventListener", 19), addEventListener);
    AppendNative(GenIdentByStr("BY_SetToNativeView", 18), setToNativeView); 
    AppendNative(GenIdentByStr("BY_InstanceOfClass", 18), instanceOfClass);
    AppendNative(GenIdentByStr("BY_CreateViewGroup", 18), createViewGroup);
    AppendNative(GenIdentByStr("BY_SetImageUrl", 14), setImageUrl);
    AppendNative(GenIdentByStr("BY_ViewCommit", 13), viewCommit);
    // End
    AppendNative(0, kBoyiaNull);

    InitNativeFun(sFunTable);
}

extern LVoid CompileScript(char* code, LVoid* vm)
{
    if (!sFunTable) {
        InitLib();
    }

    CompileCode(code, vm);
}
