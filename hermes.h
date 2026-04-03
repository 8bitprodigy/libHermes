// TODO UITextbox features - mouse input, undo, number dragging.
// TODO New elements - list view, menu bar.
// TODO Keyboard navigation in menus.
// TODO Easier to use fonts.

#ifndef HERMES_H_
#define HERMES_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hermes/automation_tests.h"
#include "hermes/font.h"
#include "hermes/inspector.h"
#include "hermes/timing.h"
#include "hermes/ui.h"
#include "hermes/ui_animation.h"
#include "hermes/ui_button.h"
#include "hermes/ui_checkbox.h"
#include "hermes/ui_clipboard.h"
#include "hermes/ui_code.h"
#include "hermes/ui_color.h"
#include "hermes/ui_dialog.h"
#include "hermes/ui_draw.h"
#include "hermes/ui_element.h"
#include "hermes/ui_event.h"
#include "hermes/ui_gauge.h"
#include "hermes/ui_image.h"
#include "hermes/ui_key.h"
#include "hermes/ui_label.h"
#include "hermes/ui_mdi.h"
#include "hermes/ui_menu.h"
#include "hermes/ui_painter.h"
#include "hermes/ui_pane.h"
#include "hermes/ui_panel.h"
#include "hermes/ui_rect.h"
#include "hermes/ui_scroll.h"
#include "hermes/ui_shortcut.h"
#include "hermes/ui_slider.h"
#include "hermes/ui_spacer.h"
#include "hermes/ui_string.h"
#include "hermes/ui_switcher.h"
#include "hermes/ui_table.h"
#include "hermes/ui_textbox.h"
#include "hermes/ui_theme.h"
#include "hermes/ui_window.h"
#include "hermes/utils.h"


#ifdef UI_IMPLEMENTATION

# include "hermes/font.c"
# include "hermes/inspector.c"
# include "hermes/platform.c"
# include "hermes/ui.c"
# include "hermes/ui_animation.c"
# include "hermes/ui_button.c"
# include "hermes/ui_checkbox.c"
# include "hermes/ui_clipboard.c"
# include "hermes/ui_code.c"
# include "hermes/ui_dialog.c"
# include "hermes/ui_draw.c"
# include "hermes/ui_element.c"
# include "hermes/ui_gauge.c"
# include "hermes/ui_image.c"
# include "hermes/ui_label.c"
# include "hermes/ui_mdi.c"
# include "hermes/ui_menu.c"
# include "hermes/ui_pane.c"
# include "hermes/ui_panel.c"
# include "hermes/ui_rect.c"
# include "hermes/ui_scroll.c"
# include "hermes/ui_slider.c"
# include "hermes/ui_spacer.c"
# include "hermes/ui_string.c"
# include "hermes/ui_switcher.c"
# include "hermes/ui_table.c"
# include "hermes/ui_textbox.c"
# include "hermes/ui_theme.c"
# include "hermes/ui_window.c"

#endif


#ifdef __cplusplus
}
#endif


#endif // HERMES_H_
