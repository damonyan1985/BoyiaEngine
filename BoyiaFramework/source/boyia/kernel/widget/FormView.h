/*
 * FormView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef FormView_h
#define FormView_h

#include "InlineView.h"

namespace yanbo {

class FormView : public InlineView {
public:
    FormView(const String& id,
        const String& name,
        const String& inValue,
        const String& inTitle);

public:
    void setValue(const String& inValue);
    const String& getValue() const;

    void setTitle(const String& title);
    const String& getTitle() const;

    void setName(const String& name);
    const String& getName() const;

protected:
    virtual ~FormView();

    String m_name;
    String m_value;
    String m_title;
};
}
#endif /* FormView_H_ */
