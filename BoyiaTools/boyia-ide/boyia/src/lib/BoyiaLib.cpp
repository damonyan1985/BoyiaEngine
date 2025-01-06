#include "BoyiaLib.h"
#include "BoyiaCore.h"
//#include "BoyiaImageView.h"
//#include "BoyiaInputView.h"
#include "BoyiaMemory.h"
#include "BoyiaNetwork.h"
#include "BoyiaSocket.h"
//#include "BoyiaViewDoc.h"
//#include "BoyiaViewGroup.h"
#include "PlatformBridge.h"
#include "BoyiaError.h"
#if ENABLE(BOYIA_ANDROID)
#include "JNIUtil.h"
#endif
#include "SalLog.h"
#include "StringUtils.h"
//#include "UIView.h"
#include "cJSON.h"
//#include <android/log.h>
#include <stdio.h>

//extern void BoyiaLog(const char* format, ...);
extern LVoid GCAppendRef(LVoid* address, LUint8 type, LVoid* vm);
extern LVoid GetIdentName(LUintPtr key, BoyiaStr* str, LVoid* vm);

char* convertMStr2Str(BoyiaStr* str)
{
    char* newStr = NEW_BUFFER(char, str->mLen + 1);
    LMemset(newStr, 0, str->mLen + 1);

    LMemcpy(newStr, str->mPtr, str->mLen);
    return newStr;
}

LVoid boyiaStrToNativeStr(BoyiaValue* strVal, String& str)
{
    BoyiaStr* bstr = GetStringBuffer(strVal);
    str.Copy(_CS(convertMStr2Str(bstr)), LTrue, bstr->mLen);
}

// 处理布尔类型返回
static LInt resultInt(LInt result, LVoid* vm)
{
    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = result;
    SetNativeResult(&value, vm);
    return kOpResultSuccess;
}

LInt getFileContent(LVoid* vm)
{
    // 0 索引第一个参数
    BoyiaValue* value = (BoyiaValue*)GetLocalValue(0, vm);
    if (!value) {
        return kOpResultEnd;
    }

    //char* fileName = convertMStr2Str(&value->mValue.mStrVal);
    char* fileName = convertMStr2Str(GetStringBuffer(value));
    FILE* file = fopen(fileName, "r");
    FREE_BUFFER(fileName);

    fseek(file, 0, SEEK_END);
    int len = ftell(file); //获取文件长度
    LInt8* buf = NEW_ARRAY(LInt8, (len + 1), vm);

    LMemset(buf, 0, len + 1);
    rewind(file);
    fread(buf, sizeof(char), len, file);
    fclose(file);

    SetStringResult(buf, len, vm);
    printf("getFileContent data=%s\n", buf);
    return kOpResultSuccess;
}

LInt addElementToVector(LVoid* vm)
{
    BOYIA_LOG("addElementToVector %d", 1);
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* element = (BoyiaValue*)GetLocalValue(1, vm);

    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
    if (fun->mParamSize >= GET_FUNCTION_COUNT(fun)) {
        BoyiaValue* value = fun->mParams;
        LInt count = GET_FUNCTION_COUNT(fun);
        fun->mParams = NEW_ARRAY(BoyiaValue, (count + 10), vm);
        fun->mParamCount = count + 10;
        LMemcpy(fun->mParams, value, count * sizeof(BoyiaValue));
        VM_DELETE(value, vm);
    }
    BOYIA_LOG("addElementToVector %d", 2);
    //fun->mParams[fun->mParamSize++] = *element;
    ValueCopy(fun->mParams + fun->mParamSize++, element);

    return kOpResultSuccess;
}

LInt getElementFromVector(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* index = (BoyiaValue*)GetLocalValue(1, vm);
    //BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(2);

    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
    BoyiaValue* result = &fun->mParams[index->mValue.mIntVal];
    SetNativeResult(result, vm);
    return kOpResultSuccess;
}

static LBool compareValue(BoyiaValue* src, BoyiaValue* dest)
{
    if (src->mValueType != dest->mValueType) {
        return LFalse;
    }

    switch (src->mValueType) {
    case BY_CHAR:
    case BY_INT:
    case BY_NAVCLASS:
        return src->mValue.mIntVal == dest->mValue.mIntVal ? LTrue : LFalse;
    case BY_CLASS:
    case BY_FUNC:
        return src->mValue.mObj.mPtr == dest->mValue.mObj.mPtr ? LTrue : LFalse;
    case BY_STRING:
        return MStrcmp(&src->mValue.mStrVal, &dest->mValue.mStrVal);
    default:
        break;
    }

    return LFalse;
}

LInt removeElementWidthIndex(LVoid* vm)
{
    BoyiaValue* array = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* idxVal = (BoyiaValue*)GetLocalValue(1, vm);

    LInt idx = idxVal->mValue.mIntVal;
    BoyiaFunction* fun = (BoyiaFunction*)array->mValue.mObj.mPtr;
    for (LInt i = idx; i < fun->mParamSize - 1; ++i) {
        ValueCopy(fun->mParams + i, fun->mParams + i + 1);
    }

    --fun->mParamSize;
    return kOpResultSuccess;
}

LInt removeElementFromVector(LVoid* vm)
{
    BoyiaValue* array = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(1, vm);

    BoyiaFunction* fun = (BoyiaFunction*)array->mValue.mObj.mPtr;
    //LInt size = fun->mParamSize - deltaIndex->mValue.mIntVal;
    LInt idx = fun->mParamSize - 1;
    while (idx >= 0) {
        BoyiaValue* elem = fun->mParams + idx;
        if (compareValue(elem, val)) {
            for (LInt i = idx; i < fun->mParamSize - 1; ++i) {
                ValueCopy(fun->mParams + i, fun->mParams + i + 1);
            }

            --fun->mParamSize;
            break;
        }

        --idx;
    }

    return kOpResultSuccess;
}

LInt getVectorSize(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;

    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = fun->mParamSize;

    SetNativeResult(&value, vm);

    return kOpResultSuccess;
}

LInt clearVector(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
    fun->mParamSize = deltaIndex->mValue.mIntVal;

    return kOpResultSuccess;
}

LInt logPrint(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    if (val->mValueType == BY_INT) {
        BOYIA_LOG("Boyia [info]: %lld", val->mValue.mIntVal);
    } else if (val->mValueType == BY_CLASS && GetBoyiaClassId(val) == kBoyiaString) {
        //char* log = convertMStr2Str(&val->mValue.mStrVal);
        char* log = convertMStr2Str(GetStringBuffer(val));
        BOYIA_LOG("Boyia [info]: %s", (const char*)log);
        FREE_BUFFER(log);
    }

    return kOpResultSuccess;
}

static LUint getJsonObjHash(cJSON* json, LVoid* vm)
{
    return GenIdentByStr(json->string, LStrlen(_CS(json->string)), vm);
}

static BoyiaFunction* getObjFun(cJSON* json, LVoid* vm)
{
    BoyiaFunction* fun = NEW(BoyiaFunction, vm);
    LInt size = cJSON_GetArraySize(json);
    fun->mParams = NEW_ARRAY(BoyiaValue, size, vm);
    fun->mParamSize = size;
    fun->mFuncBody = kBoyiaNull;
    return fun;
}

LVoid jsonParse(cJSON* json, BoyiaValue* value, LVoid* vm)
{
    if (json->valuestring) {
        value->mNameKey = getJsonObjHash(json, vm);
    }

    if (json->type == cJSON_Object) {
        cJSON* child = json->child;
        LInt index = 0;

        BoyiaFunction* fun = getObjFun(json, vm);
        value->mValueType = BY_CLASS;
        value->mValue.mObj.mPtr = (LIntPtr)fun;
        value->mValue.mObj.mSuper = 0;
        GCAppendRef(fun, BY_CLASS, vm);

        while (child) {
            fun->mParams[index].mNameKey = getJsonObjHash(child, vm);
            jsonParse(child, fun->mParams + index++, vm);
            child = child->next;
        }
    } else if (json->type == cJSON_Array) {
        LInt size = cJSON_GetArraySize(json);
        BoyiaFunction* fun = (BoyiaFunction*)CopyObject(kBoyiaArray, size, vm);
        value->mValueType = BY_CLASS;
        value->mValue.mObj.mPtr = (LIntPtr)fun;
        value->mValue.mObj.mSuper = 0;

        cJSON* child = json->child;
        while (child) {
            fun->mParams[fun->mParamSize].mNameKey = 0;
            jsonParse(child, fun->mParams + fun->mParamSize++, vm);
            child = child->next;
        }
    } else if (json->type == cJSON_Number || json->type == cJSON_True || json->type == cJSON_False) {
        value->mValueType = BY_INT;
        value->mValue.mIntVal = json->valueint;
    } else if (json->type == cJSON_String) {
        //value->mValueType = BY_STRING;
        LInt len = LStrlen(_CS(json->valuestring));
        LInt8* ptr = NEW_ARRAY(LInt8, len, vm);
        LMemcpy(ptr, json->valuestring, len);
        //value->mValue.mStrVal.mPtr = ptr;
        //value->mValue.mStrVal.mLen = len;
        //GCAppendRef(ptr, BY_STRING, vm);
        BoyiaFunction* objBody = CreateStringObject(ptr, len, vm);
        value->mValueType = BY_CLASS;
        value->mValue.mObj.mPtr = (LIntPtr)objBody;
        value->mValue.mObj.mSuper = kBoyiaNull;
    }
}

LInt jsonParseWithCJSON(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    char* content = convertMStr2Str(GetStringBuffer(val));
    cJSON* json = cJSON_Parse(content);
    FREE_BUFFER(content);

    //BoyiaValue* value = (BoyiaValue*)GetNativeResult(vm);
    BoyiaValue value;
    jsonParse(json, &value, vm);
    // 设置给r0
    SetNativeResult(&value, vm);
    // 释放json
    cJSON_Delete(json);
    return kOpResultSuccess;
}

LInt createBoyiaDocument(LVoid* vm)
{
    KFORMATLOG("BoyiaViewDoc::loadHTML createBoyiaViewDoc %d", 1);
    //BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    //if (!val) {
    //    return kOpResultEnd;
    //}

    //BoyiaStr* urlStr = GetStringBuffer(val);
    //char* url = convertMStr2Str(urlStr);
    //String strUrl(_CS(url), LTrue, urlStr->mLen);
    //boyia::BoyiaViewDoc* doc = new boyia::BoyiaViewDoc(static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm)));
    //doc->loadHTML(strUrl);
    return kOpResultSuccess;
}

LInt appendView(LVoid* vm)
{
    //BoyiaValue* parent = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* child = (BoyiaValue*)GetLocalValue(1, vm);
    //if (!parent || !child) {
    //    return kOpResultEnd;
    //}

    //boyia::BoyiaViewGroup* parentView = (boyia::BoyiaViewGroup*)parent->mValue.mIntVal;
    //boyia::BoyiaView* childView = (boyia::BoyiaView*)child->mValue.mIntVal;

    //parentView->appendView(childView);
    return kOpResultSuccess;
}

BoyiaValue* FindProp(BoyiaFunction* fun, LUint key)
{
    LInt size = fun->mParamSize;
    while (size--) {
        if (fun->mParams[size].mNameKey == key) {
            return &fun->mParams[size];
        }
    }

    return kBoyiaNull;
}

LInt getRootDocument(LVoid* vm)
{
    //boyia::BoyiaRuntime* runtime = static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm));
    //boyia::BoyiaViewDoc* doc = new boyia::BoyiaViewDoc(runtime);
    //doc->setDocument(runtime->view()->getDocument());
    return kOpResultSuccess;
}

LInt setDocument(LVoid* vm)
{
    //BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    //boyia::BoyiaViewDoc* destDoc = (boyia::BoyiaViewDoc*)val->mValue.mIntVal;

    //val = (BoyiaValue*)GetLocalValue(1, vm);
    //boyia::BoyiaViewDoc* srcDoc = (boyia::BoyiaViewDoc*)val->mValue.mIntVal;
    //destDoc->setDocument(srcDoc->getDocument());

    return kOpResultSuccess;
}

LInt removeDocument(LVoid* vm)
{
    return kOpResultSuccess;
}

LInt removeView(LVoid* vm)
{
    //BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    //boyia::BoyiaView* view = (boyia::BoyiaView*)val->mValue.mIntVal;
    //view->removeView();
    return kOpResultSuccess;
}

LInt setViewXpos(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* posX = (BoyiaValue*)GetLocalValue(1, vm);

    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    //jsDoc->setX(posX->mValue.mIntVal);

    return kOpResultSuccess;
}

LInt setViewYpos(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* posY = (BoyiaValue*)GetLocalValue(1, vm);

    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    //jsDoc->setY(posY->mValue.mIntVal);

    return kOpResultSuccess;
}

LInt getViewXpos(LVoid* vm)
{
//    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
//    boyia::BoyiaView* jsDoc = (boyia::BoyiaView*)doc->mValue.mIntVal;
////    BoyiaValue val;
////    val.mValueType = BY_INT;
////    val.mValue.mIntVal = jsDoc->left();
////    SetNativeResult(&val, vm);
//    
//    resultInt(jsDoc->left(), vm);
    return kOpResultSuccess;
}

LInt getViewYpos(LVoid* vm)
{
//    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
//    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
////    BoyiaValue val;
////    val.mValueType = BY_INT;
////    val.mValue.mIntVal = jsDoc->top();
////    SetNativeResult(&val, vm);
//    
//    resultInt(jsDoc->top(), vm);
    return kOpResultSuccess;
}

LInt getViewWidth(LVoid* vm)
{
//    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
//    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
////    BoyiaValue val;
////    val.mValueType = BY_INT;
////    val.mValue.mIntVal = jsDoc->width();
////    SetNativeResult(&val, vm);
//
//    resultInt(jsDoc->width(), vm);
    return kOpResultSuccess;
}

LInt getViewHeight(LVoid* vm)
{
//    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
//    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
////    BoyiaValue val;
////    val.mValueType = BY_INT;
////    val.mValue.mIntVal = jsDoc->height();
////    SetNativeResult(&val, vm);
//    
//    resultInt(jsDoc->height(), vm);
    return kOpResultSuccess;
}

LInt setViewStyle(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* style = (BoyiaValue*)GetLocalValue(1, vm);
    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;

    //BoyiaStr* styleStr = GetStringBuffer(style);
    ////char* styleText = convertMStr2Str(styleStr);
    //String styleText(_CS(convertMStr2Str(styleStr)), LTrue, styleStr->mLen);

    //jsDoc->setStyle(styleText);

    return kOpResultSuccess;
}

LInt drawView(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    //jsDoc->drawView();

    return kOpResultSuccess;
}

LInt startScale(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* scale = (BoyiaValue*)GetLocalValue(1, vm);
    //BoyiaValue* duration = (BoyiaValue*)GetLocalValue(2, vm);
    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    //jsDoc->startScale(scale->mValue.mIntVal, duration->mValue.mIntVal);
    return kOpResultSuccess;
}

LInt startOpacity(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* opacity = (BoyiaValue*)GetLocalValue(1, vm);
    //BoyiaValue* duration = (BoyiaValue*)GetLocalValue(2, vm);
    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    //jsDoc->startOpacity(opacity->mValue.mIntVal, duration->mValue.mIntVal);
    return kOpResultSuccess;
}

LInt startTranslate(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* posx = (BoyiaValue*)GetLocalValue(1, vm);
    //BoyiaValue* posy = (BoyiaValue*)GetLocalValue(2, vm);
    //BoyiaValue* duration = (BoyiaValue*)GetLocalValue(3, vm);
    //boyia::BoyiaViewDoc* jsDoc = reinterpret_cast<boyia::BoyiaViewDoc*>(doc->mValue.mIntVal);
    //jsDoc->startTranslate(LPoint(posx->mValue.mIntVal, posy->mValue.mIntVal), duration->mValue.mIntVal);
    return kOpResultSuccess;
}

LInt loadDataFromNative(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* callback = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(2, vm);
    
    BoyiaStr* urlStr = GetStringBuffer(val);
    char* url = convertMStr2Str(urlStr);
    String strUrl(_CS(url), LTrue, urlStr->mLen);


    boyia::BoyiaRuntime* runtime = static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm));
    BoyiaFunction* fun = (BoyiaFunction*)callback->mValue.mObj.mSuper;
    BoyiaValue* klass = (BoyiaValue*)fun->mFuncBody;
    
    PrintValueKey(klass, vm);
    boyia::BoyiaNetwork* network = new boyia::BoyiaNetwork(
        callback, obj,
        runtime);
    
    // 默认是get请求，如果参数包含params，按照params中包含的属性进行处理
    if (GetLocalSize(vm) > 3) {
        // params是一个json串
        BoyiaValue* params = (BoyiaValue*)GetLocalValue(3, vm);
        if (params->mValue.mStrVal.mPtr) {
            BoyiaStr* str = GetStringBuffer(params);
            String paramsStr(_CS(convertMStr2Str(str)), LTrue, str->mLen);
            network->load(strUrl, paramsStr);
            return kOpResultSuccess;
        }
    }
    
    network->load(strUrl);
    return kOpResultSuccess;
}

LInt callStaticMethod(LVoid* vm)
{
#if ENABLE(BOYIA_ANDROID)
    BoyiaValue* clzz = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* method = (BoyiaValue*)GetLocalValue(1, vm);
    BoyiaValue* sign = (BoyiaValue*)GetLocalValue(2, vm);

    char* strClzz = convertMStr2Str(GetStringBuffer(clzz));

    BoyiaStr* methodArgStr = GetStringBuffer(method);
    BoyiaStr* signArgStr = GetStringBuffer(sign);
    char* strMethod = convertMStr2Str(methodArgStr);
    char* strSign = convertMStr2Str(signArgStr);
    char retFlag = strSign[signArgStr->mLen - 1];

    BOYIA_LOG("Boyia [info]: callStaticMethod---strClzz=%s strMethod=%s strSign=%s", strClzz, strMethod, strSign);
    if (retFlag == 'S') {
        strSign[method->mValue.mStrVal.mLen - 1] = 0;
    }

    LInt size = GetLocalSize(vm) - 3;
    BoyiaValue result;
    yanbo::JNIUtil::callStaticMethod(
        strClzz,
        strMethod,
        strSign,
        methodArgStr->mLen - 1,
        retFlag,
        clzz + 3,
        size,
        vm,
        &result);

    SetNativeResult(&result, vm);
    FREE_BUFFER(strClzz);
    FREE_BUFFER(strMethod);
    FREE_BUFFER(strSign);
#elif ENABLE(BOYIA_WINDOWS)
    printf("callStaticMethod");
#endif
    return kOpResultSuccess;
}

LInt getHtmlItem(LVoid* vm)
{
    //BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* idArg = (BoyiaValue*)GetLocalValue(1, vm);
    //boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;

    //BoyiaStr* idStr = GetStringBuffer(idArg);
    ////char* id = convertMStr2Str(idStr);
    //String id(_CS(convertMStr2Str(idStr)),
    //    LTrue, idStr->mLen);

    //jsDoc->getItemByID(id);

    return kOpResultSuccess;
}

LInt loadImageByUrl(LVoid* vm)
{
    //BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* urlArg = (BoyiaValue*)GetLocalValue(1, vm);

    //BoyiaStr* urlStr = GetStringBuffer(urlArg);
    ////char* url = convertMStr2Str(GetStringBuffer(urlArg));
    //String url(_CS(convertMStr2Str(urlStr)),
    //    LTrue, urlStr->mLen);

    //boyia::BoyiaImageView* image = reinterpret_cast<boyia::BoyiaImageView*>(itemArg->mValue.mIntVal);
    //image->loadImage(url);
    return kOpResultSuccess;
}

LInt setViewGroupText(LVoid* vm)
{
    //BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* textArg = (BoyiaValue*)GetLocalValue(1, vm);

    //BoyiaStr* textStr = GetStringBuffer(textArg);
    //String text(_CS(convertMStr2Str(textStr)), LTrue, textStr->mLen);

    //boyia::BoyiaViewGroup* view = (boyia::BoyiaViewGroup*)itemArg->mValue.mIntVal;
    //view->setText(text);
    //text.ReleaseBuffer();
    return kOpResultSuccess;
}

LInt setInputViewText(LVoid* vm)
{
    //BoyiaValue* input = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* textArg = (BoyiaValue*)GetLocalValue(1, vm);

    //BoyiaStr* textStr = GetStringBuffer(textArg);
    ////char* text = convertMStr2Str(GetStringBuffer(textArg));
    //String text(_CS(convertMStr2Str(GetStringBuffer(textArg))),
    //    LTrue, textStr->mLen);

    //boyia::BoyiaInputView* view = (boyia::BoyiaInputView*)input->mValue.mIntVal;
    //view->setText(text);
    return kOpResultSuccess;
}

LInt addEventListener(LVoid* vm)
{
    //BoyiaValue* navVal = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* type = (BoyiaValue*)GetLocalValue(1, vm);
    //BoyiaValue* callback = (BoyiaValue*)GetLocalValue(2, vm);

    //boyia::BoyiaView* navView = (boyia::BoyiaView*)navVal->mValue.mIntVal;
    //navView->addListener(type->mValue.mIntVal, callback);

    return kOpResultSuccess;
}

LInt setToNativeView(LVoid* vm)
{
    //BoyiaValue* navVal = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* byVal = (BoyiaValue*)GetLocalValue(1, vm);

    //boyia::BoyiaView* navView = (boyia::BoyiaView*)navVal->mValue.mIntVal;
    //navView->setBoyiaView(byVal);

    return kOpResultSuccess;
}

LInt createViewGroup(LVoid* vm)
{
    //BoyiaValue* idVal = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* selectVal = (BoyiaValue*)GetLocalValue(1, vm);

    //BoyiaStr* buffer = GetStringBuffer(idVal);
    //char* idStr = convertMStr2Str(buffer);
    //String strUrl(_CS(idStr), LTrue, buffer->mLen);

    //new boyia::BoyiaViewGroup(
    //    static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm)),
    //    strUrl,
    //    selectVal->mValue.mIntVal);

    return kOpResultSuccess;
}

LInt instanceOfClass(LVoid* vm)
{
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* cls = (BoyiaValue*)GetLocalValue(1, vm);

    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    LInt result = 0;
    
    if (obj->mValueType != BY_CLASS || !fun) {
        return resultInt(result, vm);
    }
    
    BoyiaValue* baseCls = (BoyiaValue*)fun->mFuncBody;

    BoyiaFunction* judgeFun = (BoyiaFunction*)cls->mValue.mObj.mPtr;
    BoyiaValue* judgeCls = (BoyiaValue*)judgeFun->mFuncBody;

    while (baseCls) {
        if (baseCls == judgeCls) {
            result = 1;
            break;
        }

        baseCls = (BoyiaValue*)baseCls->mValue.mObj.mSuper;
    }
    
    return resultInt(result, vm);
}

LInt setImageUrl(LVoid* vm)
{
    //BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* urlArg = (BoyiaValue*)GetLocalValue(1, vm);

    //BoyiaStr* urlStr = GetStringBuffer(urlArg);
    //String url(_CS(convertMStr2Str(urlStr)),
    //    LTrue, urlStr->mLen);

    //boyia::BoyiaImageView* image = (boyia::BoyiaImageView*)itemArg->mValue.mIntVal;
    //image->setImageUrl(url);
    //url.ReleaseBuffer();
    return kOpResultSuccess;
}

LInt viewCommit(LVoid* vm)
{
    //BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0, vm);
    //boyia::BoyiaView* view = (boyia::BoyiaView*)itemArg->mValue.mIntVal;
    //view->commit();

    return kOpResultSuccess;
}

LInt setViewVisible(LVoid* vm)
{
    //BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0, vm);
    //BoyiaValue* visibleArg = (BoyiaValue*)GetLocalValue(1, vm);
    //boyia::BoyiaView* view = (boyia::BoyiaView*)itemArg->mValue.mIntVal;

    //view->item()->setVisible((LInt)visibleArg->mValue.mIntVal);
    //view->commit();

    return kOpResultSuccess;
}

static cJSON* convertObjToJson(BoyiaValue* obj, LBool isArray, LVoid* vm)
{
    BoyiaFunction* props = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    cJSON* jsonObj = isArray ? cJSON_CreateArray() : cJSON_CreateObject();

    for (LInt i = 0; i < props->mParamSize; ++i) {
        BoyiaValue* prop = props->mParams + i;
        // Get key of object
        char* key = kBoyiaNull;
        if (!isArray) {
            BoyiaStr str;
            GetIdentName(prop->mNameKey, &str, vm);
            key = convertMStr2Str(&str);
        }

        switch (prop->mValueType) {
        case BY_CLASS: {
            // if the prop is array object
            LUintPtr objectId = GetBoyiaClassId(prop);
            if (objectId == kBoyiaString) {
                // add string item to json object
                char* value = convertMStr2Str(GetStringBuffer(prop));
                if (isArray) {
                    cJSON_AddItemToArray(jsonObj, cJSON_CreateString(value));
                } else {
                    cJSON_AddItemToObject(jsonObj, key, cJSON_CreateString(value));
                }

                FREE_BUFFER(value);
            } else {
                LBool isArrayProp = objectId == kBoyiaArray;
                if (isArray) {
                    cJSON_AddItemToArray(jsonObj, convertObjToJson(prop, isArrayProp, vm));
                } else {
                    // add object item to json object
                    cJSON_AddItemToObject(jsonObj, key, convertObjToJson(prop, isArrayProp, vm));
                }
            }
        } break;
        case BY_INT: {
            if (isArray) {
                // add number item to json array
                cJSON_AddItemToArray(jsonObj,
                    cJSON_CreateNumber(prop->mValue.mIntVal));
            } else {
                // add number item to json object
                cJSON_AddItemToObject(jsonObj, key,
                    cJSON_CreateNumber(prop->mValue.mIntVal));
            }
        } break;
        default:
            break;
        }

        if (key) {
            FREE_BUFFER(key);
        }
    }

    return jsonObj;
}

// Object convert to json string
LInt toJsonString(LVoid* vm)
{
    cJSON* json = kBoyiaNull;
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(0, vm);

    // only object can be convert to json string
    if (obj->mValueType == BY_CLASS) {
        json = convertObjToJson(obj, GetBoyiaClassId(obj) == kBoyiaArray, vm);
    }

    // convert json error
    if (!json) {
        return kOpResultEnd;
    }

    char* out = cJSON_Print(json);
    LInt len = LStrlen(_CS(out));
    cJSON_Delete(json);
    BOYIA_LOG("Boyia [info]: toJsonString %s, and length=%d", out, len);

    //BoyiaValue val;
    //val.mValueType = BY_STRING;
    //val.mValue.mStrVal.mPtr = out;
    //val.mValue.mStrVal.mLen = len;

    //SetNativeResult(&val, vm);
    //SetStringResult(out, len, vm);
    BoyiaValue val;
    CreateNativeString(&val, out, len, vm);
    SetNativeResult(&val, vm);
    return kOpResultSuccess;
}

LInt createSocket(LVoid* vm)
{
    BoyiaValue* wsUrl = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* msgCB = (BoyiaValue*)GetLocalValue(1, vm);

    BoyiaStr* urlStr = GetStringBuffer(wsUrl);
    char* url = convertMStr2Str(urlStr);
    String strUrl(_CS(url), LTrue, urlStr->mLen);

    new boyia::BoyiaSocket(strUrl, msgCB, static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm)));
    return kOpResultSuccess;
}

LInt sendSocketMsg(LVoid* vm)
{
    BoyiaValue* socketVal = (BoyiaValue*)GetLocalValue(0, vm);
    BoyiaValue* msgVal = (BoyiaValue*)GetLocalValue(1, vm);

    BoyiaStr* msgStr = GetStringBuffer(msgVal);
    String msg(_CS(convertMStr2Str(msgStr)), LTrue, msgStr->mLen);

    boyia::BoyiaSocket* socket = (boyia::BoyiaSocket*)socketVal->mValue.mIntVal;
    socket->send(msg);

    return kOpResultSuccess;
}

LInt getPlatformType(LVoid* vm)
{
    return resultInt((LInt)yanbo::PlatformBridge::getPlatformType(), vm);
}

LInt callPlatformApiHandler(LVoid* vm)
{
    // 方法参数
    BoyiaValue* params = (BoyiaValue*)GetLocalValue(0, vm);
    // 回调, 必须是属性函数
    BoyiaValue* callback = (BoyiaValue*)GetLocalValue(1, vm);
    
    if (callback->mValueType != BY_PROP_FUNC) {
        return kOpResultSuccess;
    }

    String paramStr;
    boyiaStrToNativeStr(params, paramStr);
    static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm))->callPlatformApi(paramStr, callback);
    return kOpResultSuccess;
}
