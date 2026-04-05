#include "ui_radiobutton.h"
#include "ui_button.h"
#include "ui_draw.h"
#include "ui_event.h"
#include "ui_key.h"
#include "ui_painter.h"
#include "ui_string.h"
#include "utils.h"


/******************
	RADIO GROUP
******************/
static int
_UIRadioGroupMessage(
	UIElement *element, 
	UIMessage  message,
	int        di,
	void      *dp
)
{
	UIRadioGroup *group = (UIRadioGroup*)element;

	switch (message) {
	case UI_MSG_VALUE_CHANGED: {
			/*  di = index of the button that was clicked
				Uncheck all siblings, check the selected one
			*/
			for (uint32_t i = 0; i < element->childCount; i++) {
				UIRadioButton *btn = (UIRadioButton*)element->children[i];
				btn->base.check = (i == (uint32_t)di) ? 
						UI_CHECK_CHECKED
						: UI_CHECK_UNCHECKED;
			}
			group->selected = di;
			UIElementRepaint(element, NULL);

			if (group->Invoke)
				group->Invoke(group->cp, group->selected);

			break;
		}
	case UI_MSG_LAYOUT: {
			/* Stack children vertically like a panel */
			int y = element->bounds.t;
			for (uint32_t i = 0; i < element->childCount; i++) {
				UIElement *child = element->children[i];
				int h = UIElementMessage(child, UI_MSG_GET_HEIGHT, 0, NULL);
				UIElementMove(
						child, 
						UI_RECT_4(element->bounds.l, element->bounds.r, y, y+h),
						true
					);

				y += h;
			}

			break;
		}
	case UI_MSG_GET_HEIGHT: {
			int total = 0;
			for (uint32_t i = 0; i < element->childCount; i++)
				total += UIElementMessage(
						element->children[i], 
						UI_MSG_GET_HEIGHT, 
						0, 
						NULL
					);
			return total;
		}
	case UI_MSG_GET_WIDTH: {
			int max = 0;
			for (uint32_t i = 0; i < element->childCount; i++) {
				int w = UIElementMessage(
						element->children[i], 
						UI_MSG_GET_WIDTH, 
						0, 
						NULL
					);
				if (max < w) max = w;
			}
			return max;
		}
	}
	
	return 0;
}


UIRadioGroup *
UIRadioGroup_create(
	UIElement           *parent,
	uint32_t             flags,
	UIRadioButtonInvoke  invoke,
	void                *cp
)
{
	UIRadioGroup *group = (UIRadioGroup*)UIElementCreate(
			sizeof(UIRadioGroup),
			parent,
			flags,
			_UIRadioGroupMessage,
			"RadioGroup"
		);
	
	group->selected = -1;
	group->Invoke   = invoke;
	group->cp       = cp;
	
	return group;
}


/*******************
	RADIO BUTTON
*******************/
static int
_UIRadioButtonMessage(
	UIElement *element, 
	UIMessage  message, 
	int        di, 
	void      *dp
)
{
	UIRadioButton *btn = (UIRadioButton*)element;
	UICheckbox    *box = &btn->base;

	switch (message) {
	case UI_MSG_GET_HEIGHT: 
		return UI_SIZE_BUTTON_HEIGHT * element->window->scale;
	
	case UI_MSG_GET_WIDTH:
		int labelSize = UIMeasureStringWidth(box->label, box->labelBytes);
		return (labelSize + UI_SIZE_CHECKBOX_BOX + UI_SIZE_CHECKBOX_GAP) 
			*element->window->scale;
	
	case UI_MSG_PAINT: {
			UIDrawControl(
					(UIPainter*)dp, 
					element->bounds,
					UI_DRAW_CONTROL_RADIOBUTTON
						| (box->check == UI_CHECK_CHECKED ? 
								UI_DRAW_CONTROL_STATE_CHECKED
								: 0
							) 
						| UI_DRAW_CONTROL_STATE_FROM_ELEMENT(element),
					box->label,
					box->labelBytes,
					0,
					element->window->scale
				);
			break;
		}
	case UI_MSG_UPDATE: 
		UIElementRepaint(element, NULL);
		break;

	case UI_MSG_DEALLOCATE:
		UI_FREE(box->label);
		break;

	case UI_MSG_KEY_TYPED:
		UIKeyTyped *m = (UIKeyTyped*)dp;
		if (m->textBytes == 1 && m->text[0] == ' ') {
			UIElementMessage(element, UI_MSG_CLICKED, 0, 0);
			UIElementRepaint(element, NULL);
		}
		break;

	case UI_MSG_CLICKED: {
			if (box->check == UI_CHECK_CHECKED)
				return 0; /* Already selected, do nothing */

			/* Find our index in the parent group and notify it */
			UIElement *parent = element->parent;
			for (uint32_t i = 0; i < parent->childCount; i++) {
				if (parent->children[i] == element) {
					UIElementMessage(parent, UI_MSG_VALUE_CHANGED, (int)i, NULL);
					break;
				}
			}

			if (box->invoke)
				box->invoke(element->cp);
			break;
		}
		
	}

	return 0;
}


UIRadioButton *
UIRadioButton_create(
	UIRadioGroup        *group,
	uint32_t             flags,
	const char          *label,
	ptrdiff_t            label_bytes,
	UIRadioButtonInvoke  invoke,
	void                *cp
)
{
	UIRadioButton *btn = (UIRadioButton*)UIElementCreate(
			sizeof(UIRadioButton),
			&group->e,
			flags | UI_ELEMENT_TAB_STOP,
			_UIRadioButtonMessage,
			"RadioButton"
		);

	btn->base.label  = UIStringCopy(label, (btn->base.labelBytes = label_bytes));
	btn-> base.check = UI_CHECK_UNCHECKED;
	btn->base.invoke = (void(*)(void*))invoke;
	btn->base.e.cp   = cp;

	return btn;
}
