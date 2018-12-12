#include "DOMBuilder.h"
#include "TextView.h"
#include "InputView.h"
#include "LinkView.h"
#include "ImageView.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "UIView.h"
#include "VideoView.h"

namespace yanbo
{
DOMBuilder::DOMBuilder()
    : m_htmlDoc(NULL)
    , m_cssParser(NULL)
    , m_xmlDoc(NULL)
{
}

DOMBuilder::~DOMBuilder()
{
	if (m_xmlDoc)
	{
		delete m_xmlDoc;
	}
}

void DOMBuilder::createDocument(
		const String& buffer,
		HtmlDocument* doc,
		util::CssParser* cssParser)
{
	m_htmlDoc = doc;
	m_cssParser = cssParser;

	if (NULL == m_xmlDoc)
	{
		m_xmlDoc = new XMLDocument;
		m_xmlDoc->Parse((const LCharA*)buffer.GetBuffer());
	}

	XMLElement *root = m_xmlDoc->RootElement();
	if (root)
	{
	    createRenderTree(root, NULL, NULL);
	}

	m_htmlDoc->sortIds();
}

void DOMBuilder::createRenderTree(XMLNode* elem, XMLNode* parentElem, HtmlView* parent)
{
	//KLOG("createRenderTree");
	HtmlView* item = createHtmlView(elem, parentElem, parent);
	XMLNode* child = NULL;

	for (child = elem->FirstChild(); child; child = child->NextSibling())
	{
	    createRenderTree(child, elem, item);
	}
}

HtmlView* DOMBuilder::createHtmlView(XMLNode* node, XMLNode* parentElem, HtmlView* parent)
{
	HtmlView* item = NULL;
	HtmlTags* htmlTags = HtmlTags::getInstance();

	if (node->ToText() == NULL && node->ToComment() == NULL)
	{
		XMLElement* elem = node->ToElement();
		if (!elem)
		{
			return NULL;
		}
	    String tagType = _CS(elem->Value());
	    String tagId = _CS(elem->Attribute("id"));
	    String tagName = _CS(elem->Attribute("name"));
	    String tagClass = _CS(elem->Attribute("class"));

	    //KLOG("type");
	    //KSTRLOG8(tagType);
		int type = htmlTags->symbolAsInt(tagType);

		switch (type)
		{
		case HtmlTags::HTML:
			{
			}
			break;
		case HtmlTags::TITLE:
			{
			}
			break;
		case HtmlTags::HEAD:
			{
			}
			break;
		case HtmlTags::LINK:
			{
				String src = _CS(elem->Attribute("href"));
				if (src.GetLength() > 0 && m_htmlDoc)
			    {
					m_htmlDoc->getView()->getLoader()->load(src, ResourceLoader::CACHECSS);
				}
			}
			break;
		case HtmlTags::META:
			{

			}
			break;
		case HtmlTags::BASE:
			{
			}
			break;
		case HtmlTags::STYLE:
			{
			}
			break;
		case HtmlTags::SCRIPT:
			{
				String src = _CS(elem->Attribute("src"));
				if (src.GetLength() > 0 && m_htmlDoc)
				{
					m_htmlDoc->getView()->getLoader()->load(src, ResourceLoader::CACHEJS);
				}

			}
			break;
		// only body and its child can create render, render tree root is body item
		case HtmlTags::BODY:
		case HtmlTags::VIEW:
			{
			    item = new BlockView(tagId, LFalse);
				if (m_htmlDoc)
				{
					m_htmlDoc->setRenderTreeRoot(item);
				}

				KLOG("HtmlTags RootView construct");
			}
			break;
		case HtmlTags::FORM:
			{
				String tagAction = _CS(elem->Attribute("action"));
				String tagMethod = _CS(elem->Attribute("method"));
				String tagEncoding = _CS(elem->Attribute("enctype"));
				item = new HtmlForm(
						            tagId,
						            tagAction,
						            tagMethod,
						            tagEncoding,
						            LFalse);
			}
			break;
		case HtmlTags::DIV:
		case HtmlTags::P:
			{
				item = new BlockView(tagId, LFalse);
			}
			break;
		case HtmlTags::IMG:
			{
			    String imageSrc = _CS(elem->Attribute("src"));
				item = new ImageView(tagId, LFalse, imageSrc);
			}
			break;
		case HtmlTags::A:
			{
				String tagUrl = _CS(elem->Attribute("href"));
				item = new LinkView(tagId, tagUrl);
			}
			break;
		case HtmlTags::INPUT:
			{
			    String tagValue = _CS(elem->Attribute("value"));
			    String tagTitle = _CS(elem->Attribute("title"));
			    String tagInputType = _CS(elem->Attribute("type"));
			    String tagImageUrl = _CS(elem->Attribute("src"));

				item = new InputView(
						tagId,
						tagName,
						tagValue,
						tagTitle,
						tagInputType,
						tagImageUrl);
			}
			break;
		case HtmlTags::VIDEO:
		    {
                item = new VideoView(tagId, LFalse, _CS(elem->Attribute("src")));
		    }
		    break;
		default:
			{
				item = new InlineView(tagId, LFalse);
			}
			break;
		}

		if (item)
		{
			item->setParent(parent);
			item->setClassName(tagClass);
			item->setTagName(tagType);
			item->setTagType((HtmlTags::HtmlType)type);
		    if (parent)
			{
				parent->addChild(item);
			}
		}

		if (tagId.GetLength() && m_htmlDoc)
		{
			KFORMATLOG("HtmlView id %s", (const char*)tagId.GetBuffer());
			m_htmlDoc->putItemID(tagId, item);
		}
	}
	else if (node->ToText())// text
	{
		XMLText* elem = node->ToText();
		if (parentElem)
		{
			String tagType = _CS(parentElem->Value());
			int type = htmlTags->symbolAsInt(tagType);
			String text = _CS(elem->Value());
			switch (type)
			{
			case HtmlTags::STYLE:
			    {
					util::InputStream is(text);
					m_cssParser->parseCss(is);
			    }
				break;
			case HtmlTags::TITLE:
				m_htmlDoc->setPageTitle(text);
				break;
			case HtmlTags::SCRIPT:
			    break;
			default:
			    {
			    	StringUtils::replaceSpecialChars(text);
			    	if (text.GetLength())
			    	{
			    	    item = new TextView(_CS(""), text, LFalse);
			    		if (parent)
			    		{
			    			item->setParent(parent);
			    			parent->addChild(item);
			    		}

			    	    KFORMATLOG("TextView text=%s", (const char*)text.GetBuffer());
			    	}
			    }
				break;
			}
		}
	}

	if (item && m_htmlDoc)
	{
	    m_htmlDoc->addHtmlView(item);
		item->setDocument(m_htmlDoc);
	}

	return item;
}

}
