#include "ui_theme.h"


const char *themeItems[] = {
    "panel1",           "panel2",        "selected",       "border",        "text",
    "textDisabled",     "textSelected",  "buttonNormal",   "buttonHovered", "buttonPressed",
    "buttonDisabled",   "textboxNormal", "textboxFocused", "codeFocused",   "codeBackground",
    "codeDefault",      "codeComment",   "codeString",     "codeNumber",    "codeOperator",
    "codePreprocessor", "accent1",       "accent2",
};


UITheme uiThemeClassic = {
    .panel1   = 0xFF999999,
    .panel2   = 0xFF4d4d4d,
    .selected = 0xFF94BEFE,
    .border   = 0xFF404040,

    .text         = 0xFF000000,
    .textDisabled = 0xFF404040,
    .textSelected = 0xFFE6E6E6,

    .buttonNormal   = 0xFFCCCCCC,
    .buttonHovered  = 0xFFF2F2F2,
    .buttonPressed  = 0xFF808080,
    .buttonDisabled = 0xFF808080,

    .textboxNormal  = 0xFFE6E6E6,
    .textboxFocused = 0xFFFFFFFF,

    .codeFocused      = 0xFFE0E0E0,
    .codeBackground   = 0xFFFFFFFF,
    .codeDefault      = 0xFF000000,
    .codeComment      = 0xFFA11F20,
    .codeString       = 0xFF037E01,
    .codeNumber       = 0xFF213EF1,
    .codeOperator     = 0xFF7F0480,
    .codePreprocessor = 0xFF545D70,

    .gauge = 0xFF6f7C91,

    .accent1 = 0xA02C2C,
    .accent2 = 0x2CA02C,
};

UITheme uiThemeDark = {
    .panel1   = 0xFF555555,
    .panel2   = 0xFF444444,
    .selected = 0xFF94BEFE,
    .border   = 0xFF000000,

    .text         = 0xFFFFFFFF,
    .textDisabled = 0xFF787D81,
    .textSelected = 0xFF000000,

    .buttonNormal   = 0xFF555555,
    .buttonHovered  = 0xFF4B5874,
    .buttonPressed  = 0xFF444444,
    .buttonDisabled = 0xFF1B1F23,

    .textboxNormal  = 0xFF31353C,
    .textboxFocused = 0xFF4D4D59,

    .codeFocused      = 0xFF505055,
    .codeBackground   = 0xFF212126,
    .codeDefault      = 0xFFFFFFFF,
    .codeComment      = 0xFFB4B4B4,
    .codeString       = 0xFFF5DDD1,
    .codeNumber       = 0xFFC3F5D3,
    .codeOperator     = 0xFFF5D499,
    .codePreprocessor = 0xFFF5F3D1,

    .gauge = 0xFF4B5874,

    .accent1 = 0xF01231,
    .accent2 = 0x45F94E,
};
