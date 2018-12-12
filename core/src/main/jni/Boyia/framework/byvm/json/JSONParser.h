// Author Yanbo
#ifndef JSONParser_h
#define JSONParser_h

#include "cJSON.h"
#include "UtilString.h"

namespace boyia
{
class JSONIterator
{
public:
	JSONIterator(cJSON* json);
	JSONIterator next();
	cJSON* operator * ();
	JSONIterator& operator = (const JSONIterator& iter);
	LBool operator != (const JSONIterator& iter);

private:
	cJSON* m_json;
};

class JSONParser
{
public:
	JSONParser(const String& str, LBool isFile = LTrue);
	~JSONParser();

	const cJSON* json() const;
	cJSON* get(const char* key) const;

	JSONIterator begin() const;

private:
	cJSON* m_json;
};
}
#endif
