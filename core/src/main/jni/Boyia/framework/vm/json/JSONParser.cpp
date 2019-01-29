#include "JSONParser.h"
#include "FileUtil.h"
#include <string.h>

namespace boyia
{
JSONIterator::JSONIterator(cJSON* json)
    : m_json(json)
{
}

JSONIterator JSONIterator::next()
{
    return JSONIterator(m_json->next);
}

cJSON* JSONIterator::operator * ()
{
    return m_json;
}

LBool JSONIterator::operator != (const JSONIterator& iter)
{
    return iter.m_json == m_json;
}

JSONIterator& JSONIterator::operator = (const JSONIterator& iter)
{
	m_json = iter.m_json;
	return *this;
}

JSONParser::JSONParser(const String& str, LBool isFile)
{
    if (isFile)
    {
    	String content;
		FileUtil::readFile(str, content);
		KFORMATLOG("boyia app JSONParse content=%s", GET_STR(content));
		m_json = cJSON_Parse(GET_STR(content));
    }
    else
    {
    	m_json = cJSON_Parse(GET_STR(str));
    }
}

JSONParser::~JSONParser()
{
	cJSON_Delete(m_json);
}

const cJSON* JSONParser::json() const
{
	return m_json;
}

cJSON* JSONParser::get(const char* key) const
{
	cJSON* child = m_json->child;
	while (child)
	{
		if (0 == strcmp(child->string, key))
		{
			return child;
		}

		child = child->next;
	}

	return NULL;
}

JSONIterator JSONParser::begin() const
{
	return JSONIterator(m_json->child);
}

}
