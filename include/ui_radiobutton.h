#ifndef UI_RADIOBUTTON_H
#define UI_RADIOBUTTON_H


#ifdef __cplusplus
extern "C" {
#endif


#include "ui_checkbox.h"


typedef void (*UIRadioButtonInvoke)(void *cp, int selected);

typedef struct
UIRadioGroup
{
	UIElement            e;
	int                  selected;
	UIRadioButtonInvoke  Invoke;
	void                *cp;
}
UIRadioGroup;

typedef struct
UIRadioButton 
{
	UICheckbox  base;
}
UIRadioButton;

/*******************
	CONSTRUCTORS
*******************/
UIRadioGroup *UIRadioGroup_create( 
	UIElement *parent, 
	uint32_t flags, 
	UIRadioButtonInvoke invoke,
	void *cp
);

UIRadioButton *UIRadioButton_create(
	UIRadioGroup        *group,
	uint32_t             flags,
	const char          *label,
	ptrdiff_t            label_bytes,
	UIRadioButtonInvoke  invoke,
	void                *cp
);


#ifdef __cplusplus
}
#endif


#endif /* UI_RADIOBUTTON_H */
