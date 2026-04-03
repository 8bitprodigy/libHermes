// TODO UITextbox features - mouse input, undo, number dragging.
// TODO New elements - list view, menu bar.
// TODO Keyboard navigation in menus.
// TODO Easier to use fonts.

#ifndef HERMES_H_
#define HERMES_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "include/automation_tests.h"
#include "include/font.h"
#include "include/inspector.h"
#include "include/timing.h"
#include "include/software_painter.h"
#include "include/ui.h"
#include "include/ui_animation.h"
#include "include/ui_button.h"
#include "include/ui_checkbox.h"
#include "include/ui_clipboard.h"
#include "include/ui_code.h"
#include "include/ui_color.h"
#include "include/ui_dialog.h"
#include "include/ui_draw.h"
#include "include/ui_element.h"
#include "include/ui_event.h"
#include "include/ui_gauge.h"
#include "include/ui_image.h"
#include "include/ui_key.h"
#include "include/ui_label.h"
#include "include/ui_mdi.h"
#include "include/ui_menu.h"
#include "include/ui_painter.h"
#include "include/ui_pane.h"
#include "include/ui_panel.h"
#include "include/ui_rect.h"
#include "include/ui_scroll.h"
#include "include/ui_shortcut.h"
#include "include/ui_slider.h"
#include "include/ui_spacer.h"
#include "include/ui_string.h"
#include "include/ui_switcher.h"
#include "include/ui_table.h"
#include "include/ui_textbox.h"
#include "include/ui_theme.h"
#include "include/ui_window.h"
#include "include/utils.h"


#ifdef UI_IMPLEMENTATION

#include "hermes/font.c"
#include "hermes/inspector.c"
#include "hermes/platform.c"
#include "hermes/software_painter.c"
#include "hermes/ui.c"
#include "hermes/ui_animation.c"
#include "hermes/ui_button.c"
#include "hermes/ui_checkbox.c"
#include "hermes/ui_clipboard.c"
#include "hermes/ui_code.c"
#include "hermes/ui_dialog.c"
#include "hermes/ui_draw.c"
#include "hermes/ui_element.c"
#include "hermes/ui_gauge.c"
#include "hermes/ui_image.c"
#include "hermes/ui_label.c"
#include "hermes/ui_mdi.c"
#include "hermes/ui_menu.c"
#include "hermes/ui_pane.c"
#include "hermes/ui_panel.c"
#include "hermes/ui_rect.c"
#include "hermes/ui_scroll.c"
#include "hermes/ui_slider.c"
#include "hermes/ui_spacer.c"
#include "hermes/ui_string.c"
#include "hermes/ui_switcher.c"
#include "hermes/ui_table.c"
#include "hermes/ui_textbox.c"
#include "hermes/ui_theme.c"
#include "hermes/ui_window.c"

#endif


#ifdef __cplusplus
}
#endif


#endif // HERMES_H_
