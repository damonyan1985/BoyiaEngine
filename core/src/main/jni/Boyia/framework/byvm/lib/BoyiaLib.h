#ifndef BoyiaLib_h
#define BoyiaLib_h

#include "BoyiaCore.h"

enum BYValueType
{
	BY_CHAR = 2,
	BY_INT,
	BY_STRING,
	BY_FUN,
	BY_NAVCLASS,
	BY_CLASS,
};

char* convertMStr2Str(BoyiaStr* str);

LInt getFileContent();

LInt addElementToVector();

LInt getElementFromVector();

LInt getVectorSize();

LInt clearVector();

LInt logPrint();

LInt jsonParseWithCJSON();

LInt createJSDocument();

LInt appendView();

LInt getRootDocument();

LInt setDocument();

LInt removeDocument();

LInt setViewXpos();

LInt setViewYpos();

LInt drawView();

LInt startScale();

LInt startOpacity();

LInt removeElementWidthIndex();

LInt removeElementFromVector();

LInt getViewXpos();

LInt getViewYpos();

LInt getViewWidth();

LInt getViewHeight();

LInt setViewStyle();

LInt loadDataFromNative();

LInt setJSTouchCallback();

LInt callStaticMethod();

LInt startTranslate();

LInt getHtmlItem();

LInt loadImageByUrl();

LInt setViewGroupText();

LInt setInputViewText();

LInt addEventListener();

LInt setToNativeView();

LInt instanceOfClass();

LInt createViewGroup();

LInt setImageUrl();

LInt viewCommit();

#endif // !BoyiaLib_h

