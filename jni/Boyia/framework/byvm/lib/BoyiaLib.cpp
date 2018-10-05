#include "BoyiaLib.h"
#include "BoyiaCore.h"
#include "BoyiaMemory.h"
#include "cJSON.h"
#include "StringUtils.h"
#include "BoyiaViewDoc.h"
#include "UIView.h"
#include "BoyiaNetwork.h"
#include "JNIUtil.h"
#include "BoyiaImageView.h"
#include "BoyiaViewGroup.h"
#include "BoyiaInputView.h"
#include <android/log.h>
#include <stdio.h>

extern void jsLog(const char* format, ...);
//extern LVoid MiniSort(BoyiaValue* vT, LInt len);
extern LVoid GCAppendRef(LVoid* address, LUint8 type);

char* convertMStr2Str(BoyiaStr* str)
{
	char* newStr = new char[str->mLen + 1];
	LMemset(newStr, 0, str->mLen + 1);

	LMemcpy(newStr, str->mPtr, str->mLen);
	return newStr;
}

LInt getFileContent()
{
	// 0 索引第一个参数
	BoyiaValue* value = (BoyiaValue*)GetLocalValue(0);
	if (!value)
	{
		return 0;
	}

	char* fileName = convertMStr2Str(&value->mValue.mStrVal);
	FILE* file = fopen(fileName, "r");
	delete[] fileName;
	fseek(file, 0, SEEK_END);
	int len = ftell(file); //获取文件长度
	LInt8* buf = NEW_ARRAY(LInt8, (len + 1));
	GCAppendRef(buf, STRING);
	LMemset(buf, 0, len + 1);
	rewind(file);
	fread(buf, sizeof(char), len, file);
	fclose(file);

	BoyiaValue val;
	val.mValueType = BY_STRING;
	val.mValue.mStrVal.mPtr = buf;
	val.mValue.mStrVal.mLen = len;

	SetNativeResult(&val);

	printf("getFileContent data=%s\n", buf);
	return 1;
}

LInt addElementToVector()
{
	jsLog("addElementToVector %d", 1);
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* element = (BoyiaValue*)GetLocalValue(1);

	BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
	if (fun->mParamSize >= fun->mParamCount) {
		BoyiaValue* value = fun->mParams;
		LInt count = fun->mParamCount;
		fun->mParams = NEW_ARRAY(BoyiaValue, (count + 10));
		fun->mParamCount = count + 10;
		LMemcpy(fun->mParams, value, count * sizeof(BoyiaValue));
		DELETE(value);
	}
	jsLog("addElementToVector %d", 2);
	//fun->mParams[fun->mParamSize++] = *element;
	ValueCopy(fun->mParams + fun->mParamSize++, element);

	return 1;
}

LInt getElementFromVector()
{
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* index = (BoyiaValue*)GetLocalValue(1);
	//BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(2);

	BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
	BoyiaValue* result = &fun->mParams[index->mValue.mIntVal];
	SetNativeResult(result);
	return 1;
}

static LBool compareValue(BoyiaValue* src, BoyiaValue* dest) {
	if (src->mValueType != dest->mValueType) {
		return LFalse;
	}

	switch (src->mValueType)
	{
	case BY_CHAR:
	case BY_INT:
	case BY_NAVCLASS:
		return src->mValue.mIntVal == dest->mValue.mIntVal ? LTrue : LFalse;
	case BY_CLASS:
	case BY_FUN:
		return src->mValue.mObj.mPtr == dest->mValue.mObj.mPtr ? LTrue : LFalse;
	case BY_STRING:
		return MStrcmp(&src->mValue.mStrVal, &dest->mValue.mStrVal);
	default:
		break;
	}

	return LFalse;
}

LInt removeElementWidthIndex() {
    BoyiaValue* array = (BoyiaValue*)GetLocalValue(0);
    BoyiaValue* idxVal = (BoyiaValue*)GetLocalValue(1);

    LInt idx = idxVal->mValue.mIntVal;
    BoyiaFunction* fun = (BoyiaFunction*)array->mValue.mObj.mPtr;
    for (LInt i = idx; i < fun->mParamSize - 1; ++i) {
        ValueCopy(fun->mParams + i, fun->mParams + i + 1);
    }

    --fun->mParamSize;
}

LInt removeElementFromVector() {
	BoyiaValue* array = (BoyiaValue*)GetLocalValue(0);
	//BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(1);

	BoyiaFunction* fun = (BoyiaFunction*)array->mValue.mObj.mPtr;
	//LInt size = fun->mParamSize - deltaIndex->mValue.mIntVal;
	LInt idx = fun->mParamSize-1;
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

LInt getVectorSize() {
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	//BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
	BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;

	BoyiaValue value;
	value.mValueType = BY_INT;
	value.mValue.mIntVal = fun->mParamSize;

	SetNativeResult(&value);

	return 1;
}

LInt clearVector() {
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* deltaIndex = (BoyiaValue*)GetLocalValue(1);
	BoyiaFunction* fun = (BoyiaFunction*)val->mValue.mObj.mPtr;
	fun->mParamSize = deltaIndex->mValue.mIntVal;

	return 1;
}

LInt logPrint()
{
	//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "logPrint execute");
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	if (val->mValueType == BY_INT)
	{
		__android_log_print(ANDROID_LOG_INFO, "MiniJS", "MiniJS Log: %d", (int)val->mValue.mIntVal);
	}
	else if (val->mValueType == BY_STRING)
	{
		char* log = convertMStr2Str(&val->mValue.mStrVal);
		__android_log_print(ANDROID_LOG_INFO, "MiniJS", "MiniJS Log: %s", (const char*)log);
		delete[] log;
	}

	return 1;
}

static LUint getJsonObjHash(cJSON* json) 
{
	//return StringUtils::hashCode(String(_CS(json->string), LFalse, LStrlen(_CS(json->string))));
    return GenIdentByStr(json->string, LStrlen(_CS(json->string)));
}

static BoyiaFunction* getObjFun(cJSON* json) {
	BoyiaFunction* fun = NEW(BoyiaFunction);
	LInt size = cJSON_GetArraySize(json);
	fun->mParams = NEW_ARRAY(BoyiaValue, size);
	fun->mParamSize = size;
	return fun;
}

LVoid jsonParse(cJSON* json, BoyiaValue* value) {
	if (json->valuestring) {
		value->mNameKey = getJsonObjHash(json);
	}

	if (json->type == cJSON_Object) {
		value->mValueType = BY_CLASS;
		cJSON* child = json->child;
		LInt index = 0;
		
		BoyiaFunction* fun = getObjFun(json);
		value->mValue.mObj.mPtr = (LIntPtr)fun;
		value->mValue.mObj.mSuper = 0;
		GCAppendRef(fun, BY_CLASS);

		while (child) {
			fun->mParams[index].mNameKey = getJsonObjHash(child);
			jsonParse(child, fun->mParams + index++);
			child = child->next;
		}
	} else if (json->type == cJSON_Array) {
		value->mValueType = BY_CLASS;

		LInt size = cJSON_GetArraySize(json);
		BoyiaFunction* fun = (BoyiaFunction*)CopyObject(GenIdentByStr("Array", 5), size);
		value->mValue.mObj.mPtr = (LIntPtr)fun;
		value->mValue.mObj.mSuper = 0;
		GCAppendRef(fun, BY_CLASS);

		cJSON* child = json->child;
		while (child) {
			fun->mParams[fun->mParamSize].mNameKey = 0;
			jsonParse(child, fun->mParams + fun->mParamSize++);
			child = child->next;
		}
	} else if (json->type == cJSON_Number || json->type == cJSON_True || json->type == cJSON_False) {
		value->mValueType = BY_INT;
		value->mValue.mIntVal = json->valueint;
	} else if (json->type == cJSON_String) {
		value->mValueType = BY_STRING;
		LInt len = LStrlen(_CS(json->valuestring));
        LInt8* ptr = NEW_ARRAY(LInt8, len);
        LMemcpy(ptr, json->valuestring, len);
		value->mValue.mStrVal.mPtr = ptr;
		value->mValue.mStrVal.mLen = len;
		GCAppendRef(ptr, STRING);
	}
}

LInt jsonParseWithCJSON()
{
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	char* content = convertMStr2Str(&val->mValue.mStrVal);
	cJSON* json = cJSON_Parse(content);
    delete[] content;
	BoyiaValue* value = (BoyiaValue*) GetNativeResult();
	jsonParse(json, value);
	cJSON_Delete(json);
	return 1;
}

LInt createJSDocument()
{
	KFORMATLOG("BoyiaViewDoc::loadHTML createBoyiaViewDoc %d", 1);
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	if (!val)
	{
		return 0;
	}

	char* url = convertMStr2Str(&val->mValue.mStrVal);
	String strUrl(_CS(url), LTrue, val->mValue.mStrVal.mLen);
	boyia::BoyiaViewDoc* doc = new boyia::BoyiaViewDoc();
	doc->loadHTML(strUrl);
	return 1;
}

LInt appendView()
{
	BoyiaValue* parent = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* child = (BoyiaValue*)GetLocalValue(1);
	if (!parent || !child)
	{
		return 0;
	}

	boyia::BoyiaViewGroup* parentView = (boyia::BoyiaViewGroup*) parent->mValue.mIntVal;
	boyia::BoyiaView* childView = (boyia::BoyiaView*) child->mValue.mIntVal;

	parentView->appendView(childView);
	return 1;
}

BoyiaValue* FindProp(BoyiaFunction* fun, LUint key)
{
	LInt size = fun->mParamSize;
    while (size--)
    {
        if (fun->mParams[size].mNameKey == key)
        {
        	return &fun->mParams[size];
        }
    }

    return NULL;
}

LInt getRootDocument()
{
    boyia::BoyiaViewDoc* doc = new boyia::BoyiaViewDoc();
    doc->setDocument(yanbo::UIView::getInstance()->getDocument());
    return 1;
}

LInt setDocument()
{
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaViewDoc* destDoc = (boyia::BoyiaViewDoc*) val->mValue.mIntVal;

	val = (BoyiaValue*)GetLocalValue(1);
	boyia::BoyiaViewDoc* srcDoc = (boyia::BoyiaViewDoc*) val->mValue.mIntVal;
	destDoc->setDocument(srcDoc->getDocument());

	return 1;
}

LInt removeDocument()
{
    return 1;
}

LInt setViewXpos()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* posX = (BoyiaValue*)GetLocalValue(1);

	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	jsDoc->setX(posX->mValue.mIntVal);

	return 1;
}

LInt setViewYpos()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* posY = (BoyiaValue*)GetLocalValue(1);

	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	jsDoc->setY(posY->mValue.mIntVal);

	return 1;
}

LInt getViewXpos()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaView* jsDoc = (boyia::BoyiaView*) doc->mValue.mIntVal;
	BoyiaValue val;
	val.mValueType = BY_INT;
	val.mValue.mIntVal = jsDoc->left();
	SetNativeResult(&val);

	return 1;
}

LInt getViewYpos()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	BoyiaValue val;
	val.mValueType = BY_INT;
	val.mValue.mIntVal = jsDoc->top();
	SetNativeResult(&val);

	return 1;
}

LInt getViewWidth()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	BoyiaValue val;
	val.mValueType = BY_INT;
	val.mValue.mIntVal = jsDoc->width();
	SetNativeResult(&val);

	return 1;
}

LInt getViewHeight()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	BoyiaValue val;
	val.mValueType = BY_INT;
	val.mValue.mIntVal = jsDoc->height();
	SetNativeResult(&val);

	return 1;
}

LInt setViewStyle()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* style = (BoyiaValue*)GetLocalValue(1);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;

	char* styleText = convertMStr2Str(&style->mValue.mStrVal);
	String styleStr(_CS(styleText), LTrue, style->mValue.mStrVal.mLen);

	jsDoc->setStyle(styleStr);

	return 1;
}

LInt drawView()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	jsDoc->drawView();

	return 1;
}

LInt startScale()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* scale = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* duration = (BoyiaValue*)GetLocalValue(2);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	jsDoc->startScale(scale->mValue.mIntVal, duration->mValue.mIntVal);
    return 1;
}

LInt startOpacity()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* opacity = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* duration = (BoyiaValue*)GetLocalValue(2);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	jsDoc->startOpacity(opacity->mValue.mIntVal, duration->mValue.mIntVal);
    return 1;
}

LInt startTranslate()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* posx = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* posy = (BoyiaValue*)GetLocalValue(2);
	BoyiaValue* duration = (BoyiaValue*)GetLocalValue(3);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;
	jsDoc->startTranslate(LPoint(posx->mValue.mIntVal, posy->mValue.mIntVal), duration->mValue.mIntVal);
    return 1;
}

LInt loadDataFromNative()
{
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* callback = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* obj = (BoyiaValue*)GetLocalValue(2);
	boyia::BoyiaNetwork* network = new boyia::BoyiaNetwork(callback, obj);
	char* url = convertMStr2Str(&val->mValue.mStrVal);
	String strUrl(_CS(url), LTrue, val->mValue.mStrVal.mLen);
	network->load(strUrl);

	return 1;
}

LInt setJSTouchCallback()
{
	BoyiaValue* val = (BoyiaValue*)GetLocalValue(0);
	yanbo::UIView::getInstance()->jsHandler()->setTouchCallback(val);

	return 1;
}

LInt callStaticMethod()
{
	BoyiaValue* clzz = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* method = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* sign = (BoyiaValue*)GetLocalValue(2);

	char* strClzz = convertMStr2Str(&clzz->mValue.mStrVal);
	char* strMethod = convertMStr2Str(&method->mValue.mStrVal);
	char* strSign = convertMStr2Str(&sign->mValue.mStrVal);
	char retFlag = strSign[sign->mValue.mStrVal.mLen - 1];

	__android_log_print(ANDROID_LOG_INFO, "MiniJS", "strClzz=%s strMethod=%s strSign=%s", strClzz, strMethod, strSign);
	if (retFlag == 'S')
	{
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

	SetNativeResult(&result);
	delete[] strClzz;
    delete[] strMethod;
    delete[] strSign;
	return 1;
}

LInt getHtmlItem()
{
	BoyiaValue* doc = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* idArg = (BoyiaValue*)GetLocalValue(1);
	boyia::BoyiaViewDoc* jsDoc = (boyia::BoyiaViewDoc*) doc->mValue.mIntVal;

	char* id = convertMStr2Str(&idArg->mValue.mStrVal);
	String idStr(_CS(id), LTrue, idArg->mValue.mStrVal.mLen);

    jsDoc->getItemByID(idStr);

	return 1;
}

LInt loadImageByUrl()
{
	BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* urlArg = (BoyiaValue*)GetLocalValue(1);

	char* url = convertMStr2Str(&urlArg->mValue.mStrVal);
	String urlStr(_CS(url), LTrue, urlArg->mValue.mStrVal.mLen);

	boyia::BoyiaImageView* image = (boyia::BoyiaImageView*)itemArg->mValue.mIntVal;
	image->loadImage(urlStr);
	return 1;
}

LInt setViewGroupText()
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

LInt setInputViewText() {
	BoyiaValue* input = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* textArg = (BoyiaValue*)GetLocalValue(1);

	char* text = convertMStr2Str(&textArg->mValue.mStrVal);
	String textStr(_CS(text), LTrue, textArg->mValue.mStrVal.mLen);

	boyia::BoyiaInputView* view = (boyia::BoyiaInputView*)input->mValue.mIntVal;
	view->setText(textStr);
	return 1;
}

LInt addEventListener() {
	BoyiaValue* navVal = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* type = (BoyiaValue*)GetLocalValue(1);
	BoyiaValue* callback = (BoyiaValue*)GetLocalValue(2);

	boyia::BoyiaBase* navView = (boyia::BoyiaBase*) navVal->mValue.mIntVal;
	navView->addListener(type->mValue.mIntVal, callback);

    return 1;
}

LInt setToNativeView() {
	BoyiaValue* navVal = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* jsVal = (BoyiaValue*)GetLocalValue(1);

	boyia::BoyiaBase* navView = (boyia::BoyiaBase*) navVal->mValue.mIntVal;
	navView->setBoyiaView(jsVal);

	return 1;
}

LInt createViewGroup() {
	BoyiaValue* idVal = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* selectVal = (BoyiaValue*)GetLocalValue(1);

	char* idStr = convertMStr2Str(&idVal->mValue.mStrVal);
	String strUrl(_CS(idStr), LTrue, idVal->mValue.mStrVal.mLen);
	new boyia::BoyiaViewGroup(strUrl, selectVal->mValue.mIntVal);

	return 1;
}

LInt instanceOfClass() {
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

		baseCls = (BoyiaValue*) baseCls->mValue.mObj.mSuper;
	}

	BoyiaValue value;
	value.mValueType = BY_INT;
	value.mValue.mIntVal = result;
	SetNativeResult(&value);
	return 1;
}

LInt setImageUrl() {
	BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
	BoyiaValue* urlArg = (BoyiaValue*)GetLocalValue(1);

	char* url = convertMStr2Str(&urlArg->mValue.mStrVal);
	String urlStr(_CS(url), LTrue, urlArg->mValue.mStrVal.mLen);

	boyia::BoyiaImageView* image = (boyia::BoyiaImageView*)itemArg->mValue.mIntVal;
	image->setImageUrl(urlStr);
	urlStr.ReleaseBuffer();
	return 1;
}

LInt viewCommit() {
	BoyiaValue* itemArg = (BoyiaValue*)GetLocalValue(0);
	boyia::BoyiaView* view = (boyia::BoyiaView*)itemArg->mValue.mIntVal;
	view->commit();

	return 1;
}
