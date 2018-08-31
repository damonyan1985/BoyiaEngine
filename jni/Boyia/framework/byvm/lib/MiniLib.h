#ifndef MiniLib_h
#define MiniLib_h

#include "MiniCore.h"

enum MValueType
{
	M_CHAR = 2,
	M_INT,
	M_STRING,
	M_JSFUN,
	M_NAVCLASS,
	M_CLASS,
};

char* convertMStr2Str(MiniStr* str);

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

LInt setJSViewToNativeView();

LInt instanceOfClass();

LInt createViewGroup();

LInt setImageUrl();

LInt viewCommit();

#endif // !MiniLib_h

