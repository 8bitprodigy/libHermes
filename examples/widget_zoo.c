#define UI_LINUX
#define UI_IMPLEMENTATION
#include "../hermes.h"

#include <stdio.h>
#include <string.h>


// Helpers

const char *FindSystemFont(const char *name) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "fc-match --format=%%{file} '%s'", name);
    FILE *f = popen(cmd, "r");
    static char path[512];
    fgets(path, sizeof(path), f);
    pclose(f);
    path[strcspn(path, "\n")] = 0;
    return path;
}


// Table data

static const char *tableNames[]  = { "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune" };
static const char *tableMasses[] = { "0.055", "0.815", "1.000", "0.107", "317.8", "95.2", "14.5", "17.1" };

int PlanetTableMessage(UIElement *element, UIMessage message, int di, void *dp) {
    if (message == UI_MSG_TABLE_GET_ITEM) {
        UITableGetItem *m = (UITableGetItem *) dp;
        if (m->column == 0) return snprintf(m->buffer, m->bufferBytes, "%s", tableNames[m->index]);
        if (m->column == 1) return snprintf(m->buffer, m->bufferBytes, "%s", tableMasses[m->index]);
    }
    return 0;
}


// Slider -> Gauge linkage

static UIGauge *linkedGauge;

int SliderMessage(UIElement *element, UIMessage message, int di, void *dp) {
    if (message == UI_MSG_VALUE_CHANGED) {
        UISlider *slider = (UISlider *) element;
        UIGaugeSetPosition(linkedGauge, (float) slider->position);
        UIElementRepaint(&linkedGauge->e, NULL);
    }
    return 0;
}


// Checkbox

static UILabel *checkLabel;

void CheckboxInvoke(void *cp) {
    UICheckbox *cb = (UICheckbox *) cp;
    const char *states[] = { "Unchecked", "Checked", "Indeterminate" };
    UILabelSetContent(checkLabel, states[cb->check], -1);
    UIElementRepaint(&checkLabel->e, NULL);
}


// Radio button
 
static UILabel *radioLabel;
 
void RadioGroupInvoke(void *cp, int selected) {
    const char *options[] = { "Option 1", "Option 2", "Option 3" };
    UILabelSetContent(radioLabel, options[selected], -1);
    UIElementRepaint(&radioLabel->e, NULL);
}


// Button counter

static UILabel *counterLabel;
static int      counterValue = 0;

int ButtonMessage(UIElement *element, UIMessage message, int di, void *dp) {
    if (message == UI_MSG_CLICKED) {
        char buf[32];
        counterValue++;
        snprintf(buf, sizeof(buf), "Clicked %d time(s)", counterValue);
        UILabelSetContent(counterLabel, buf, -1);
        UIElementRepaint(&counterLabel->e, NULL);
    }
    return 0;
}


// Switcher buttons

static UISwitcher *switcher;
static UIElement  *switcherPages[3];
static int         switcherActive = 0;

int SwitcherButtonMessage(UIElement *element, UIMessage message, int di, void *dp) {
    if (message == UI_MSG_CLICKED) {
        int index = (int)(intptr_t) element->cp;
        switcherActive = index;
        UISwitcherSwitchTo(switcher, switcherPages[index]);
        UIElementRepaint(&switcher->e, NULL);
    }
    return 0;
}


// Main

int 
main() 
{
     Hermes_InitConfig config = { false };
     Hermes_Init(&config);

#ifdef UI_FREETYPE
    UIFontActivate(UIFontCreate(FindSystemFont("DejaVu sans"), 11));
#endif

     Hermes_CreateWindow(0, UI_ELEMENT_PARENT_PUSH, "Widget Zoo", 800, 600);

    // Root panel
    UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_COLOR_1 | UI_PANEL_EXPAND);

    // Tab pane with all sections
    UITabPane *tabs = UITabPaneCreate(0, UI_ELEMENT_H_FILL | UI_ELEMENT_V_FILL,
                                      "Basic Controls\tTable\tSplit & Wrap\tSwitcher\tCode");


    // Tab 0: Basic Controls

    UIPanelCreate(&tabs->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_EXPAND | UI_PANEL_LARGE_SPACING | UI_PANEL_SCROLL);

        // Button row
        UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_HORIZONTAL | UI_PANEL_MEDIUM_SPACING);
            UIButton *btn = UIButtonCreate(0, 0, "Click me", -1);
            btn->e.messageUser = ButtonMessage;
            counterLabel = UILabelCreate(0, UI_ELEMENT_H_FILL, "Clicked 0 time(s)", -1);
        UIParentPop();

        UISpacerCreate(0, 0, 0, 8);

        // Checkboxes
        UILabelCreate(0, 0, "Checkboxes:", -1);
        UICheckbox *cb1 = UICheckboxCreate(0, 0, "Option A", -1);
        UICheckbox *cb2 = UICheckboxCreate(0, 0, "Option B", -1);
        UICheckbox *cb3 = UICheckboxCreate(0, UI_CHECKBOX_ALLOW_INDETERMINATE, "Option C (indeterminate allowed)", -1);
        checkLabel = UILabelCreate(0, 0, "Unchecked", -1);
        cb1->invoke = cb2->invoke = cb3->invoke = CheckboxInvoke;
        cb1->e.cp   = cb1;
        cb2->e.cp   = cb2;
        cb3->e.cp   = cb3;

        UISpacerCreate(0, 0, 0, 8);

        // Radio buttons
        UILabelCreate(0, 0, "Radio Buttons:", -1);
        UIRadioGroup *radioGroup = UIRadioGroup_create(0, 0, RadioGroupInvoke, NULL);
        UIRadioButton_create(radioGroup, 0, "Option 1", -1, NULL, NULL);
        UIRadioButton_create(radioGroup, 0, "Option 2", -1, NULL, NULL);
        UIRadioButton_create(radioGroup, 0, "Option 3", -1, NULL, NULL);
        radioLabel = UILabelCreate(0, 0, "None selected", -1);
 
        UISpacerCreate(0, 0, 0, 8);

        // Textbox
        UILabelCreate(0, 0, "Textbox:", -1);
        UITextboxCreate(0, UI_ELEMENT_H_FILL);

        UISpacerCreate(0, 0, 0, 8);

        // Slider → Gauge
        UILabelCreate(0, 0, "Slider -> Gauge:", -1);
        UISlider *slider = UISliderCreate(0, UI_ELEMENT_H_FILL);
        slider->e.messageUser = SliderMessage;
        UISliderSetPosition(slider, 0.4, false);
        linkedGauge = UIGaugeCreate(0, UI_ELEMENT_H_FILL);
        UIGaugeSetPosition(linkedGauge, 0.4f);

    UIParentPop(); // basic controls panel


    // Tab 1: Table

    UITable *table = UITableCreate(&tabs->e, UI_ELEMENT_H_FILL | UI_ELEMENT_V_FILL,
                                   "Planet\tMass (Earths)");
    table->e.messageUser = PlanetTableMessage;
    table->itemCount     = 8;
    UITableResizeColumns(table);


    // Tab 2: Split & Wrap

    UISplitPane *split = UISplitPaneCreate(&tabs->e, UI_ELEMENT_H_FILL | UI_ELEMENT_V_FILL, 0.4f);

        UIPanelCreate(&split->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_COLOR_2 | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);
            UILabelCreate(0, 0, "Left pane (40%)", -1);
            UIButtonCreate(0, UI_ELEMENT_H_FILL, "Button A", -1);
            UIButtonCreate(0, UI_ELEMENT_H_FILL, "Button B", -1);
            UIButtonCreate(0, UI_ELEMENT_H_FILL, "Button C", -1);
        UIParentPop();

        UIPanelCreate(&split->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);
            UILabelCreate(0, 0, "Right pane (60%) --— wrap panel below:", -1);
            UIWrapPanel *wrap = UIWrapPanelCreate(0, UI_ELEMENT_H_FILL);
            for (int i = 0; i < 12; i++) {
                char buf[16];
                snprintf(buf, sizeof(buf), "Item %d", i + 1);
                UIButtonCreate(&wrap->e, 0, buf, -1);
            }
        UIParentPop();


    // Tab 3: Switcher

    UIPanelCreate(&tabs->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);

        // Switcher tab buttons
        UIPanelCreate(0, UI_ELEMENT_PARENT_PUSH | UI_PANEL_HORIZONTAL | UI_PANEL_SMALL_SPACING);
            const char *pageNames[] = { "Page One", "Page Two", "Page Three" };
            for (int i = 0; i < 3; i++) {
                UIButton *pb = UIButtonCreate(0, 0, pageNames[i], -1);
                pb->e.messageUser = SwitcherButtonMessage;
                pb->e.cp          = (void *)(intptr_t) i;
            }
        UIParentPop();

        switcher = UISwitcherCreate(0, UI_ELEMENT_H_FILL | UI_ELEMENT_V_FILL);

        // Page 0
        UIPanelCreate(&switcher->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_COLOR_1 | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);
            UILabelCreate(0, 0, "This is Page One.", -1);
            UIButtonCreate(0, 0, "A button on page one", -1);
            switcherPages[0] = UIParentPop();

        // Page 1
        UIPanelCreate(&switcher->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_COLOR_2 | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);
            UILabelCreate(0, 0, "This is Page Two.", -1);
            UISliderCreate(0, UI_ELEMENT_H_FILL);
            switcherPages[1] = UIParentPop();

        // Page 2
        UIPanelCreate(&switcher->e, UI_ELEMENT_PARENT_PUSH | UI_PANEL_EXPAND | UI_PANEL_MEDIUM_SPACING);
            UILabelCreate(0, 0, "This is Page Three.", -1);
            UITextboxCreate(0, UI_ELEMENT_H_FILL);
            switcherPages[2] = UIParentPop();

        UISwitcherSwitchTo(switcher, switcherPages[0]);

    UIParentPop(); // switcher tab panel


    // Tab 4: Code view

    UICode *code = UICodeCreate(&tabs->e, UI_ELEMENT_H_FILL | UI_ELEMENT_V_FILL);
    code->font = UIFontCreate(FindSystemFont("monospace"), 11);
    UICodeInsertContent(
            code,
            "// libHermes widget zoo\n"
            "int main() {\n"
            "     Hermes_InitConfig config = { false };\n"
            "     Hermes_Init(&config);\n"
            "     Hermes_CreateWindow(0, UI_ELEMENT_PARENT_PUSH, \"Zoo\", 800, 600);\n"
            "    // ... build your UI ...\n"
            "    return  Hermes_Loop();\n"
            "}\n",
            -1, 
            false
        );


    // Done

    UIParentPop(); // root panel

    return  Hermes_Loop();
}
