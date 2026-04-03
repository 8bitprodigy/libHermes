#ifndef HERMES_LABEL_H_
#define HERMES_LABEL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_element.h"


typedef struct UILabel {
    UIElement e;
    char     *label;
    ptrdiff_t labelBytes;
} UILabel;


//


UILabel *UILabelCreate(UIElement *parent, uint32_t flags, const char *string,
                       ptrdiff_t stringBytes);
void     UILabelSetContent(UILabel *label, const char *string, ptrdiff_t stringBytes);


#ifdef __cplusplus
}
#endif


#endif // HERMES_LABEL_H_
