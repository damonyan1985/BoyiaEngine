#include "BoyiaLib.h"
#include "BoyiaCore.h"
#include "BoyiaImageView.h"
#include "BoyiaInputView.h"
#include "BoyiaMemory.h"
#include "BoyiaNetwork.h"
#include "BoyiaViewDoc.h"
#include "BoyiaViewGroup.h"
#if ENABLE(BOYIA_ANDROID)
#include "JNIUtil.h"
#endif
#include "SalLog.h"
#include "StringUtils.h"
#include "UIView.h"
#include "cJSON.h"
//#include <android/log.h>
#include <stdio.h>

//extern void BoyiaLog(const char* format, ...);
extern LVoid GCAppendRef(LVoid* address, LUint8 type);

char* convertMStr2Str(BoyiaStr* str)
{
    char* newStr = new char[str->mLen + 1];
    LMemset(newStr, 0, str->mLen + 1);

    LMemcpy(newStr, str->mPtr, str->mLen);
    return newStr;
}

LInt getFileContent(LVoid* vm)
{
    // 0 索引第一个参数
    BoyiaValue* value = (BoyiaValue*)GetLocalValue(0);
    if (!value) {
        return 0;
    }

    char* fileName = convertMStr2Str(&value->mValue.mStrVal);
    FILE* file = fopen(fileName, "r");
    delete[] fileName;
    fseek(file, 0, SEEK_END);
    int len = ftell(file); //获取文件长度
    LInt8* buf = NEW_ARRAY(LInt8, (len + 1), vm);
    GCAppendRef(buf, BY_STRING);
    LMemset(buf, 0, len + 1);
    rewind(file);
    fread(buf, sizeof(char), len, file);
    fclose(file);

    BoyiaValue val;
    val.mValueType = BY_STRING;
    val.mValue.mStrVal.mPtr = buf;
    val.mValue.mStrVal.mLen = len;

    SetNativeResult(&val, vm);

    printf("getFileContent data=%s\n", buf);
    return 1;
}

LInt addElementToVector(LVoid* vm)
{
    BOYIA_LOG("addElementToVector %d", 1);
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* element = (BoyiaValue*)GetLocalValue(1);

    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
    if (fun->mParamSize >= fun->mParamCount) {
        BoyiaValue* value = fun->mParams;
        LInt count = fun->mParamCount;
        fun->mParams = NEW_ARRAY(BoyiaValue, (count + 10), vm);
        fun->mParamCount = count + 10;
        LMemcpy(fun->mParams, value, count * sizeof(BoyiaValue));
		VM_DELETE(value, vm);
    }
    BOYIA_LOG("addElementToVector %d", 2);
    //fun->mParams[fun->mParamSize++] = *element;
    ValueCopy(fun->mParams + fun->mParamSize++, element);

    return 1;
}

LInt getElementFromVector(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* index = (BoyiaValue*)GetLocalValue(1);
    //BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(2);

    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
    BoyiaValue* result = &fun->mParams[index->mValue.mIntVal];
    SetNativeResult(result, vm);
    return 1;
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
    BoyiaValue* array = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* idxVal = (BoyiaValue*)GetLocalValue(1);

    LInt idx = idxVal->mValue.mIntVal;
    BoyiaFunction* fun = (BoyiaFunction*)array->mValue.mObj.mPtr;
    for (LInt i = idx; i < fun->mParamSize - 1; ++i) {
        ValueCopy(fun->mParams + i, fun->mParams + i + 1);
    }

    --fun->mParamSize;
	return 1;
}

LInt removeElementFromVector(LVoid* vm)
{
    BoyiaValue* array = (BoyiaValue*)GetLocalValue(0);
    //BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(1);

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

    return 1;
}

LInt getVectorSize(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    //BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;

    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = fun->mParamSize;

    SetNativeResult(&value, vm);

    return 1;
}

LInt clearVector(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
    BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
    fun->mParamSize = deltaIndex->mValue.mIntVal;

    return 1;
}

LInt logPrint(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    if (val->mValueType == BY_INT) {
        BOYIA_LOG("Boyia [info]: %d", (int)val->mValue.mIntVal);
    } else if (val->mValueType == BY_STRING) {
        char* log = convertMStr2Str(&val->mValue.mStrVal);
        BOYIA_LOG("Boyia [info]: %s", (const char*)log);
        delete[] log;
    }

    return 1;
}

static LUint getJsonObjHash(cJSON* json)
{
    return GenIdentByStr(json->string, LStrlen(_CS(json->string)));
}

static BoyiaFunction* getObjFun(cJSON* json, LVoid* vm)
{
    BoyiaFunction* fun = NEW(BoyiaFunction, vm);
    LInt size = cJSON_GetArraySize(json);
    fun->mParams = NEW_ARRAY(BoyiaValue, size, vm);
    fun->mParamSize = size;
    return fun;
}

LVoid jsonParse(cJSON* json, BoyiaValue* value, LVoid* vm)
{
    if (json->valuestring) {
        value->mNameKey = getJsonObjHash(json);
    }

    if (json->type == cJSON_Object) {
        value->mValueType = BY_CLASS;
        cJSON* child = json->child;
        LInt index = 0;

        BoyiaFunction* fun = getObjFun(json, vm);
        value->mValue.mObj.mPtr = (LIntPtr)fun;
        value->mValue.mObj.mSuper = 0;
        GCAppendRef(fun, BY_CLASS);

        while (child) {
            fun->mParams[index].mNameKey = getJsonObjHash(child);
            jsonParse(child, fun->mParams + index++, vm);
            child = child->next;
        }
    } else if (json->type == cJSON_Array) {
        value->mValueType = BY_CLASS;

        LInt size = cJSON_GetArraySize(json);
        BoyiaFunction* fun = (BoyiaFunction*)CopyObject(GenIdentByStr("Array", 5), size, vm);
        value->mValue.mObj.mPtr = (LIntPtr)fun;
        value->mValue.mObj.mSuper = 0;
        GCAppendRef(fun, BY_CLASS);

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
        value->mValueType = BY_STRING;
        LInt len = LStrlen(_CS(json->valuestring));
        LInt8* ptr = NEW_ARRAY(LInt8, len, vm);
        LMemcpy(ptr, json->valuestring, len);
        value->mValue.mStrVal.mPtr = ptr;
        value->mValue.mStrVal.mLen = len;
        GCAppendRef(ptr, BY_STRING);
    }
}

LInt jsonParseWithCJSON(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    char* content = convertMStr2Str(&val->mValue.mStrVal);
    cJSON* json = cJSON_Parse(content);
    delete[] content;
    BoyiaValue* value = (BoyiaValue*)GetNativeResult(vm);
    jsonParse(json, value, vm);
    cJSON_Delete(json);
    return 1;
}

LInt createJSDocument(LVoid* vm)
{
    KFORMATLOG("BoyiaViewDoc::loadHTML createBoyiaViewDoc %d", 1);
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    if (!val) {
        return 0;
    }

    char* url = convertMStr2Str(&val->mValue.mStrVal);
    String strUrl(_CS(url), LTrue, val->mValue.mStrVal.mLen);
    boyia::BoyiaViewDoc* doc = new boyia::BoyiaViewDoc(static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm)));
    doc->loadHTML(strUrl);
    return 1;
}

LInt appendView(LVoid* vm)
{
    BoyiaValue* parent = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* child = (BoyiaValue*)GetLocalValue(1);
    if (!parent || !child) {
        return 0;
    }

    boyia::BoyiaViewGroup* parentView = (boyia::BoyiaViewGroup*)parent->mValue.mIntVal;
    boyia::BoyiaView* childView = (boyia::BoyiaView*)child->mValue.mIntVal;

    parentView->appendView(childView);
    return 1;
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
    boyia::BoyiaRuntime* runtime = static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm));
    boyia::BoyiaViewDoc* doc = new boyia::BoyiaViewDoc(runtime);
    doc->setDocument(runtime->view()->getDocument());
    return 1;
}

LInt setDocument(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaViewDoc* destDoc = (boyia::BoyiaViewDoc*)val->mValue.mIntVal;

    val = (BoyiaValue*)GetLocalValue(1);
    boyia::BoyiaViewDoc* srcDoc = (boyia::BoyiaViewDoc*)val->mValue.mIntVal;
    destDoc->setDocument(srcDoc->getDocument());

    return 1;
}

LInt removeDocument(LVoid* vm)
{
    return 1;
}

LInt setViewXpos(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* posX = (BoyiaValue*)GetLocalValue(1);

    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    jsDoc->setX(posX->mValue.mIntVal);

    return 1;
}

LInt setViewYpos(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* posY = (BoyiaValue*)GetLocalValue(1);

    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    jsDoc->setY(posY->mValue.mIntVal);

    return 1;
}

LInt getViewXpos(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaView* jsDoc = (boyia::BoyiaView*)doc->mValue.mIntVal;
    BoyiaValue val;
    val.mValueType = BY_INT;
    val.mValue.mIntVal = jsDoc->left();
    SetNativeResult(&val, vm);

    return 1;
}

LInt getViewYpos(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    BoyiaValue val;
    val.mValueType = BY_INT;
    val.mValue.mIntVal = jsDoc->top();
    SetNativeResult(&val, vm);

    return 1;
}

LInt getViewWidth(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    BoyiaValue val;
    val.mValueType = BY_INT;
    val.mValue.mIntVal = jsDoc->width();
    SetNativeResult(&val, vm);

    return 1;
}

LInt getViewHeight(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    BoyiaValue val;
    val.mValueType = BY_INT;
    val.mValue.mIntVal = jsDoc->height();
    SetNativeResult(&val, vm);

    return 1;
}

LInt setViewStyle(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* style = (BoyiaValue*)GetLocalValue(1);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;

    char* styleText = convertMStr2Str(&style->mValue.mStrVal);
    String styleStr(_CS(styleText), LTrue, style->mValue.mStrVal.mLen);

    jsDoc->setStyle(styleStr);

    return 1;
}

LInt drawView(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    jsDoc->drawView();

    return 1;
}

LInt startScale(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* scale = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* duration = (BoyiaValue*)GetLocalValue(2);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    jsDoc->startScale(scale->mValue.mIntVal, duration->mValue.mIntVal);
    return 1;
}

LInt startOpacity(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* opacity = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* duration = (BoyiaValue*)GetLocalValue(2);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    jsDoc->startOpacity(opacity->mValue.mIntVal, duration->mValue.mIntVal);
    return 1;
}

LInt startTranslate(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* posx = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* posy = (BoyiaValue*)GetLocalValue(2);
    BoyiaValue* duration = (BoyiaValue*)GetLocalValue(3);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;
    jsDoc->startTranslate(LPoint(posx->mValue.mIntVal, posy->mValue.mIntVal), duration->mValue.mIntVal);
    return 1;
}

LInt loadDataFromNative(LVoid* vm)
{
    BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* callback = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(2);
    boyia::BoyiaNetwork* network = new boyia::BoyiaNetwork(callback, obj, vm);
    char* url = convertMStr2Str(&val->mValue.mStrVal);
    String strUrl(_CS(url), LTrue, val->mValue.mStrVal.mLen);
    network->load(strUrl);

    return 1;
}

LInt callStaticMethod(LVoid* vm)
{
#if ENABLE(BOYIA_ANDROID)
    BoyiaValue* clzz = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* method = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* sign = (BoyiaValue*)GetLocalValue(2);

    char* strClzz = convertMStr2Str(&clzz->mValue.mStrVal);
    char* strMethod = convertMStr2Str(&method->mValue.mStrVal);
    char* strSign = convertMStr2Str(&sign->mValue.mStrVal);
    char retFlag = strSign[sign->mValue.mStrVal.mLen - 1];

    BOYIA_LOG("Boyia [info]: callStaticMethod---strClzz=%s strMethod=%s strSign=%s", strClzz, strMethod, strSign);
    if (retFlag == 'S') {
        strSign[method->mValue.mStrVal.mLen - 1] = 0;
    }

    LInt size = GetLocalSize() - 3;
    BoyiaValue result;
    yanbo::JNIUtil::callStaticMethod(
        strClzz,
        strMethod,
        strSign,
        method->mValue.mStrVal.mLen - 1,
        retFlag,
        clzz + 3,
        size,
        &result);

    SetNativeResult(&result, vm);
    delete[] strClzz;
    delete[] strMethod;
    delete[] strSign;
#endif
    return 1;
}

LInt getHtmlItem(LVoid* vm)
{
    BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* idArg = (BoyiaValue*)GetLocalValue(1);
    boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*)doc->mValue.mIntVal;

    char* id = convertMStr2Str(&idArg->mValue.mStrVal);
    String idStr(_CS(id), LTrue, idArg->mValue.mStrVal.mLen);

    jsDoc->getItemByID(idStr);

    return 1;
}

LInt loadImageByUrl(LVoid* vm)
{
    BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* urlArg = (BoyiaValue*)GetLocalValue(1);

    char* url = convertMStr2Str(&urlArg->mValue.mStrVal);
    String urlStr(_CS(url), LTrue, urlArg->mValue.mStrVal.mLen);

    boyia::BoyiaImageView* image = (boyia::BoyiaImageView*)itemArg->mValue.mIntVal;
    image->loadImage(urlStr);
    return 1;
}

LInt setViewGroupText(LVoid* vm)
{
    BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* textArg = (BoyiaValue*)GetLocalValue(1);

    char* text = convertMStr2Str(&textArg->mValue.mStrVal);
    String textStr(_CS(text), LTrue, textArg->mValue.mStrVal.mLen);

    boyia::BoyiaViewGroup* view = (boyia::BoyiaViewGroup*)itemArg->mValue.mIntVal;
    view->setText(textStr);
    textStr.ReleaseBuffer();
    return 1;
}

LInt setInputViewText(LVoid* vm)
{
    BoyiaValue* input = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* textArg = (BoyiaValue*)GetLocalValue(1);

    char* text = convertMStr2Str(&textArg->mValue.mStrVal);
    String textStr(_CS(text), LTrue, textArg->mValue.mStrVal.mLen);

    boyia::BoyiaInputView* view = (boyia::BoyiaInputView*)input->mValue.mIntVal;
    view->setText(textStr);
    return 1;
}

LInt addEventListener(LVoid* vm)
{
    BoyiaValue* navVal = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* type = (BoyiaValue*)GetLocalValue(1);
    BoyiaValue* callback = (BoyiaValue*)GetLocalValue(2);

    boyia::BoyiaBase* navView = (boyia::BoyiaBase*)navVal->mValue.mIntVal;
    navView->addListener(type->mValue.mIntVal, callback);

    return 1;
}

LInt setToNativeView(LVoid* vm)
{
    BoyiaValue* navVal = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* jsVal = (BoyiaValue*)GetLocalValue(1);

    boyia::BoyiaBase* navView = (boyia::BoyiaBase*)navVal->mValue.mIntVal;
    navView->setBoyiaView(jsVal);

    return 1;
}

LInt createViewGroup(LVoid* vm)
{
    BoyiaValue* idVal = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* selectVal = (BoyiaValue*)GetLocalValue(1);

    char* idStr = convertMStr2Str(&idVal->mValue.mStrVal);
    String strUrl(_CS(idStr), LTrue, idVal->mValue.mStrVal.mLen);
    
    new boyia::BoyiaViewGroup(
        static_cast<boyia::BoyiaRuntime*>(GetVMCreator(vm)), 
        strUrl, 
        selectVal->mValue.mIntVal);

    return 1;
}

LInt instanceOfClass(LVoid* vm)
{
    BoyiaValue* obj = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* cls = (BoyiaValue*)GetLocalValue(1);

    BoyiaFunction* fun = (BoyiaFunction*)obj->mValue.mObj.mPtr;
    BoyiaValue* baseCls = (BoyiaValue*)fun->mFuncBody;

    BoyiaFunction* judgeFun = (BoyiaFunction*)cls->mValue.mObj.mPtr;
    BoyiaValue* judgeCls = (BoyiaValue*)judgeFun->mFuncBody;

    LInt result = 0;
    while (baseCls) {
        if (baseCls == judgeCls) {
            result = 1;
            break;
        }

        baseCls = (BoyiaValue*)baseCls->mValue.mObj.mSuper;
    }

    BoyiaValue value;
    value.mValueType = BY_INT;
    value.mValue.mIntVal = result;
    SetNativeResult(&value, vm);
    return 1;
}

LInt setImageUrl(LVoid* vm)
{
    BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* urlArg = (BoyiaValue*)GetLocalValue(1);

    char* url = convertMStr2Str(&urlArg->mValue.mStrVal);
    String urlStr(_CS(url), LTrue, urlArg->mValue.mStrVal.mLen);

    boyia::BoyiaImageView* image = (boyia::BoyiaImageView*)itemArg->mValue.mIntVal;
    image->setImageUrl(urlStr);
    urlStr.ReleaseBuffer();
    return 1;
}

LInt viewCommit(LVoid* vm)
{
    BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
    boyia::BoyiaView* view = (boyia::BoyiaView*)itemArg->mValue.mIntVal;
    view->commit();

    return 1;
}

LInt setViewVisible(LVoid* vm)
{
    BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* visibleArg = (BoyiaValue*)GetLocalValue(1);
    boyia::BoyiaView* view = (boyia::BoyiaView*)itemArg->mValue.mIntVal;

    view->item()->getStyle()->displayType = visibleArg->mValue.mIntVal;
    view->commit();

    return 1;
}
