#include "PlatformLib.h"

typedef union DOMValueImpl {
	char* json;
	LInt loopIndex;
} DOMValueImpl;

class DOMValue {
public:
	DOMValue()
	{
		value.json = NULL;
	}
	LInt type;
	DOMValueImpl value;
};