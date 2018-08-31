#include "MiniLib.h"
#include "MiniCore.h"
#include "MiniMemory.h"
#include "cJSON.h"
#include "StringUtils.h"
#include "JSViewDoc.h"
#include "UIView.h"
#include "JSNetwork.h"
#include "JNIUtil.h"
#include "JSImageView.h"
#include "JSViewGroup.h"
#include "JSInputView.h"
#include <android/log.h>
#include <stdio.h>

extern void jsLog(const char* format, ...);
extern LVoid MiniSort(MiniValue* vT, LInt len);
extern LVoid GCAppendRef(LVoid* address, LUint8 type);

char* convertMStr2Str(MiniStr* str)
{
	char* newStr = new char[str->mLen + 1];
	LMemset(newStr, 0, str->mLen + 1);

	LMemcpy(newStr, str->mPtr, str->mLen);
	return newStr;
}

LInt getFileContent()
{
	// 0 索引第一个参数
	MiniValue* value = (MiniValue*)GetLocalValue(0);
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
	LMemset(buf, 0, len + 1);
	rewind(file);
	fread(buf, sizeof(char), len, file);
	fclose(file);

	MiniValue val;
	val.mValueType = M_STRING;
	val.mValue.mStrVal.mPtr = buf;
	val.mValue.mStrVal.mLen = len;

	SetNativeResult(&val);

	printf("getFileContent data=%s\n", buf);
	return 1;
}

LInt addElementToVector()
{
	jsLog("addElementToVector %d", 1);
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	MiniValue* element = (MiniValue*)GetLocalValue(1);

	MiniFunction* fun = (MiniFunction*)val->mValue.mObj.mPtr;
	if (fun->mParamSize >= fun->mParamCount) {
		MiniValue* value = fun->mParams;
		LInt count = fun->mParamCount;
		fun->mParams = NEW_ARRAY(MiniValue, (count + 10));
		fun->mParamCount = count + 10;
		LMemcpy(fun->mParams, value, count * sizeof(MiniValue));
		DELETE(value);
	}
	jsLog("addElementToVector %d", 2);
	//fun->mParams[fun->mParamSize++] = *element;
	ValueCopy(fun->mParams + fun->mParamSize++, element);

	return 1;
}

LInt getElementFromVector()
{
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	MiniValue* index = (MiniValue*)GetLocalValue(1);
	//MiniValue* deltaIndex = (MiniValue*)GetLocalValue(2);

	MiniFunction* fun = (MiniFunction*)val->mValue.mObj.mPtr;
	MiniValue* result = &fun->mParams[index->mValue.mIntVal];
	SetNativeResult(result);
	return 1;
}

static LBool compareValue(MiniValue* src, MiniValue* dest) {
	if (src->mValueType != dest->mValueType) {
		return LFalse;
	}

	switch (src->mValueType)
	{
	case M_CHAR:
	case M_INT:
	case M_NAVCLASS:
		return src->mValue.mIntVal == dest->mValue.mIntVal ? LTrue : LFalse;
	case M_CLASS:
	case M_JSFUN:
		return src->mValue.mObj.mPtr == dest->mValue.mObj.mPtr ? LTrue : LFalse;
	case M_STRING:
		return MStrcmp(&src->mValue.mStrVal, &dest->mValue.mStrVal);
	default:
		break;
	}

	return LFalse;
}

LInt removeElementFromVector() {
	MiniValue* array = (MiniValue*)GetLocalValue(0);
	//MiniValue* deltaIndex = (MiniValue*)GetLocalValue(1);
	MiniValue* val = (MiniValue*)GetLocalValue(1);

	MiniFunction* fun = (MiniFunction*)array->mValue.mObj.mPtr;
	//LInt size = fun->mParamSize - deltaIndex->mValue.mIntVal;
	LInt idx = fun->mParamSize-1;
	while (idx >= 0) {
		MiniValue* elem = fun->mParams + idx;
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
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	//MiniValue* deltaIndex = (MiniValue*)GetLocalValue(1);
	MiniFunction* fun = (MiniFunction*)val->mValue.mObj.mPtr;

	MiniValue value;
	value.mValueType = M_INT;
	value.mValue.mIntVal = fun->mParamSize;

	SetNativeResult(&value);

	return 1;
}

LInt clearVector() {
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	MiniValue* deltaIndex = (MiniValue*)GetLocalValue(1);
	MiniFunction* fun = (MiniFunction*)val->mValue.mObj.mPtr;
	fun->mParamSize = deltaIndex->mValue.mIntVal;

	return 1;
}

LInt logPrint()
{
	//__android_log_print(ANDROID_LOG_INFO, "MiniJS", "logPrint execute");
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	if (val->mValueType == M_INT)
	{
		__android_log_print(ANDROID_LOG_INFO, "MiniJS", "MiniJS Log: %d", (int)val->mValue.mIntVal);
	}
	else if (val->mValueType == M_STRING) 
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

static MiniFunction* getObjFun(cJSON* json) {
	MiniFunction* fun = NEW(MiniFunction);
	LInt size = cJSON_GetArraySize(json);
	fun->mParams = NEW_ARRAY(MiniValue, size);
	fun->mParamSize = size;
	return fun;
}

LVoid jsonParse(cJSON* json, MiniValue* value) {
	if (json->valuestring) {
		value->mNameKey = getJsonObjHash(json);
	}

	if (json->type == cJSON_Object) {
		value->mValueType = M_CLASS;
		cJSON* child = json->child;
		LInt index = 0;
		
		MiniFunction* fun = getObjFun(json);
		value->mValue.mObj.mPtr = (LInt)fun;
		value->mValue.mObj.mSuper = 0;
		GCAppendRef(fun, M_CLASS);

		while (child) {
			fun->mParams[index].mNameKey = getJsonObjHash(child);
			jsonParse(child, fun->mParams + index++);
			child = child->next;
		}
	} else if (json->type == cJSON_Array) {
		value->mValueType = M_CLASS;

		LInt size = cJSON_GetArraySize(json);
		MiniFunction* fun = (MiniFunction*)CopyObject(GenIdentByStr("Array", 5), size);
		value->mValue.mObj.mPtr = (LInt)fun;
		value->mValue.mObj.mSuper = 0;
		GCAppendRef(fun, M_CLASS);

		cJSON* child = json->child;
		while (child) {
			fun->mParams[fun->mParamSize].mNameKey = 0;
			jsonParse(child, fun->mParams + fun->mParamSize++);
			child = child->next;
		}
	} else if (json->type == cJSON_Number || json->type == cJSON_True || json->type == cJSON_False) {
		value->mValueType = M_INT;
		value->mValue.mIntVal = json->valueint;
	} else if (json->type == cJSON_String) {
		value->mValueType = M_STRING;
		value->mValue.mStrVal.mPtr = json->valuestring; 
		value->mValue.mStrVal.mLen = LStrlen(_CS(json->valuestring));
	}
}

LInt jsonParseWithCJSON()
{
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	char* fileName = convertMStr2Str(&val->mValue.mStrVal);
	cJSON* json = cJSON_Parse(fileName);
    delete[] fileName;
	MiniValue* value = (MiniValue*) GetNativeResult();
	jsonParse(json, value);

	//SetNativeResult(&value);

	return 1;
}

LInt createJSDocument()
{
	KFORMATLOG("JSViewDoc::loadHTML createJSViewDoc %d", 1);
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	if (!val)
	{
		return 0;
	}

	char* url = convertMStr2Str(&val->mValue.mStrVal);
	String strUrl(_CS(url), LTrue, val->mValue.mStrVal.mLen);
	mjs::JSViewDoc* doc = new mjs::JSViewDoc();
	doc->loadHTML(strUrl);
	return 1;
}

LInt appendView()
{
	MiniValue* parent = (MiniValue*)GetLocalValue(0);
	MiniValue* child = (MiniValue*)GetLocalValue(1);
	if (!parent || !child)
	{
		return 0;
	}

	mjs::JSViewGroup* parentView = (mjs::JSViewGroup*) parent->mValue.mIntVal;
	mjs::JSView* childView = (mjs::JSView*) child->mValue.mIntVal;

	parentView->appendView(childView);
	return 1;
}

MiniValue* FindProp(MiniFunction* fun, LUint key)
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
    mjs::JSViewDoc* doc = new mjs::JSViewDoc();
    doc->setDocument(yanbo::UIView::getInstance()->getDocument());
    return 1;
}

LInt setDocument()
{
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	mjs::JSViewDoc* destDoc = (mjs::JSViewDoc*) val->mValue.mIntVal;

	val = (MiniValue*)GetLocalValue(1);
	mjs::JSViewDoc* srcDoc = (mjs::JSViewDoc*) val->mValue.mIntVal;
	destDoc->setDocument(srcDoc->getDocument());

	return 1;
}

LInt removeDocument()
{
    return 1;
}

LInt setViewXpos()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* posX = (MiniValue*)GetLocalValue(1);

	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	jsDoc->setX(posX->mValue.mIntVal);

	return 1;
}

LInt setViewYpos()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* posY = (MiniValue*)GetLocalValue(1);

	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	jsDoc->setY(posY->mValue.mIntVal);

	return 1;
}

LInt getViewXpos()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	mjs::JSView* jsDoc = (mjs::JSView*) doc->mValue.mIntVal;
	MiniValue val;
	val.mValueType = M_INT;
	val.mValue.mIntVal = jsDoc->left();
	SetNativeResult(&val);

	return 1;
}

LInt getViewYpos()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	MiniValue val;
	val.mValueType = M_INT;
	val.mValue.mIntVal = jsDoc->top();
	SetNativeResult(&val);

	return 1;
}

LInt getViewWidth()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	MiniValue val;
	val.mValueType = M_INT;
	val.mValue.mIntVal = jsDoc->width();
	SetNativeResult(&val);

	return 1;
}

LInt getViewHeight()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	MiniValue val;
	val.mValueType = M_INT;
	val.mValue.mIntVal = jsDoc->height();
	SetNativeResult(&val);

	return 1;
}

LInt setViewStyle()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* style = (MiniValue*)GetLocalValue(1);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;

	char* styleText = convertMStr2Str(&style->mValue.mStrVal);
	String styleStr(_CS(styleText), LTrue, style->mValue.mStrVal.mLen);

	jsDoc->setStyle(styleStr);

	return 1;
}

LInt drawView()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	jsDoc->drawView();

	return 1;
}

LInt startScale()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* scale = (MiniValue*)GetLocalValue(1);
	MiniValue* duration = (MiniValue*)GetLocalValue(2);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	jsDoc->startScale(scale->mValue.mIntVal, duration->mValue.mIntVal);
    return 1;
}

LInt startOpacity()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* opacity = (MiniValue*)GetLocalValue(1);
	MiniValue* duration = (MiniValue*)GetLocalValue(2);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	jsDoc->startOpacity(opacity->mValue.mIntVal, duration->mValue.mIntVal);
    return 1;
}

LInt startTranslate()
{
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* posx = (MiniValue*)GetLocalValue(1);
	MiniValue* posy = (MiniValue*)GetLocalValue(2);
	MiniValue* duration = (MiniValue*)GetLocalValue(3);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;
	jsDoc->startTranslate(LPoint(posx->mValue.mIntVal, posy->mValue.mIntVal), duration->mValue.mIntVal);
    return 1;
}

LInt loadDataFromNative()
{
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	MiniValue* callback = (MiniValue*)GetLocalValue(1);
	MiniValue* obj = (MiniValue*)GetLocalValue(2);
	mjs::JSNetwork* network = new mjs::JSNetwork(callback, obj);
	char* url = convertMStr2Str(&val->mValue.mStrVal);
	String strUrl(_CS(url), LTrue, val->mValue.mStrVal.mLen);
	network->load(strUrl);

	return 1;
}

LInt setJSTouchCallback()
{
	MiniValue* val = (MiniValue*)GetLocalValue(0);
	yanbo::UIView::getInstance()->jsHandler()->setTouchCallback(val);

	return 1;
}

LInt callStaticMethod()
{
	MiniValue* clzz = (MiniValue*)GetLocalValue(0);
	MiniValue* method = (MiniValue*)GetLocalValue(1);
	MiniValue* sign = (MiniValue*)GetLocalValue(2);

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
	MiniValue result;
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
	MiniValue* doc = (MiniValue*)GetLocalValue(0);
	MiniValue* idArg = (MiniValue*)GetLocalValue(1);
	mjs::JSViewDoc* jsDoc = (mjs::JSViewDoc*) doc->mValue.mIntVal;

	char* id = convertMStr2Str(&idArg->mValue.mStrVal);
	String idStr(_CS(id), LTrue, idArg->mValue.mStrVal.mLen);

    jsDoc->getItemByID(idStr);

	return 1;
}

LInt loadImageByUrl()
{
	MiniValue* itemArg = (MiniValue*)GetLocalValue(0);
	MiniValue* urlArg = (MiniValue*)GetLocalValue(1);

	char* url = convertMStr2Str(&urlArg->mValue.mStrVal);
	String urlStr(_CS(url), LTrue, urlArg->mValue.mStrVal.mLen);

	mjs::JSImageView* image = (mjs::JSImageView*)itemArg->mValue.mIntVal;
	image->loadImage(urlStr);
	return 1;
}

LInt setViewGroupText()
{
	MiniValue* itemArg = (MiniValue*)GetLocalValue(0);
	MiniValue* textArg = (MiniValue*)GetLocalValue(1);

	char* text = convertMStr2Str(&textArg->mValue.mStrVal);
	String textStr(_CS(text), LTrue, textArg->mValue.mStrVal.mLen);

	mjs::JSViewGroup* view = (mjs::JSViewGroup*)itemArg->mValue.mIntVal;
	view->setText(textStr);
	textStr.ReleaseBuffer();
	return 1;
}

LInt setInputViewText() {
	MiniValue* input = (MiniValue*)GetLocalValue(0);
	MiniValue* textArg = (MiniValue*)GetLocalValue(1);

	char* text = convertMStr2Str(&textArg->mValue.mStrVal);
	String textStr(_CS(text), LTrue, textArg->mValue.mStrVal.mLen);

	mjs::JSInputView* view = (mjs::JSInputView*)input->mValue.mIntVal;
	view->setText(textStr);
	return 1;
}

LInt addEventListener() {
	MiniValue* navVal = (MiniValue*)GetLocalValue(0);
	MiniValue* type = (MiniValue*)GetLocalValue(1);
	MiniValue* callback = (MiniValue*)GetLocalValue(2);

	mjs::JSBase* navView = (mjs::JSBase*) navVal->mValue.mIntVal;
	navView->addListener(type->mValue.mIntVal, callback);

    return 1;
}

LInt setJSViewToNativeView() {
	MiniValue* navVal = (MiniValue*)GetLocalValue(0);
	MiniValue* jsVal = (MiniValue*)GetLocalValue(1);

	mjs::JSBase* navView = (mjs::JSBase*) navVal->mValue.mIntVal;
	navView->setJSView(jsVal);

	return 1;
}

LInt createViewGroup() {
	MiniValue* idVal = (MiniValue*)GetLocalValue(0);
	MiniValue* selectVal = (MiniValue*)GetLocalValue(1);

	char* idStr = convertMStr2Str(&idVal->mValue.mStrVal);
	String strUrl(_CS(idStr), LTrue, idVal->mValue.mStrVal.mLen);
	new mjs::JSViewGroup(strUrl, selectVal->mValue.mIntVal);

	return 1;
}

LInt instanceOfClass() {
	MiniValue* obj = (MiniValue*)GetLocalValue(0);
	MiniValue* cls = (MiniValue*)GetLocalValue(1);

	MiniFunction* fun = (MiniFunction*)obj->mValue.mObj.mPtr;
	MiniValue* baseCls = (MiniValue*)fun->mFuncBody;

	MiniFunction* judgeFun = (MiniFunction*)cls->mValue.mObj.mPtr;
	MiniValue* judgeCls = (MiniValue*)judgeFun->mFuncBody;

	LInt result = 0;
	while (baseCls) {
		if (baseCls == judgeCls) {
			result = 1;
			break;
		}

		baseCls = (MiniValue*) baseCls->mValue.mObj.mSuper;
	}

	MiniValue value;
	value.mValueType = M_INT;
	value.mValue.mIntVal = result;
	SetNativeResult(&value);
	return 1;
}

LInt setImageUrl() {
	MiniValue* itemArg = (MiniValue*)GetLocalValue(0);
	MiniValue* urlArg = (MiniValue*)GetLocalValue(1);

	char* url = convertMStr2Str(&urlArg->mValue.mStrVal);
	String urlStr(_CS(url), LTrue, urlArg->mValue.mStrVal.mLen);

	mjs::JSImageView* image = (mjs::JSImageView*)itemArg->mValue.mIntVal;
	image->setImageUrl(urlStr);
	urlStr.ReleaseBuffer();
	return 1;
}

LInt viewCommit() {
	MiniValue* itemArg = (MiniValue*)GetLocalValue(0);
	mjs::JSView* view = (mjs::JSView*)itemArg->mValue.mIntVal;
	view->commit();

	return 1;
}
