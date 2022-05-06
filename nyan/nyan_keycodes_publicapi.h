/*
	Файл	: nyan_keycodes_publicapi.h

	Описание: Коды клавиш

	История	: 18.04.14	Создан

*/

#ifndef NYAN_KEYCODES_PUBLICAPI_H
#define NYAN_KEYCODES_PUBLICAPI_H

// Статусы нажатия кнопок
#define NK_STATUS_UNPRESSED 0
#define NK_STATUS_PRESSED 1
#define NK_STATUS_RELEASED 2

// Коды кнопок

#define NK_BACK 0x08 // VK_BACK, BACKSPACE key
#define NK_TAB 0x09 // VK_TAB
#define NK_RETURN 0x0d // VK_RETURN, ENTER key

#define NK_SHIFT 0x10 // VK_SHIFT, SHIFT key
#define NK_CONTROL 0x11 // VK_CONTROL, CTRL key
#define NK_MENU 0x12 // VK_ALT, ALT key
	#define NK_ALT NK_MENU
#define NK_PAUSE 0x13 // VK_PAUSE, PAUSE key

#define NK_CAPSLOCK 0x14 // VK_CAPITAL, CAPS LOCK key

#define NK_ESCAPE 0x1b // VK_ESCAPE, ESC key

#define NK_SPACE 0x20 // SPACEBAR
#define NK_PRIOR 0x21 // VK_PRIOR, PAGE UP key
	#define NK_PAGEUP NK_PRIOR
#define NK_NEXT 0x22 // VK_NEXT, PAGE DOWN key
	#define NK_PAGEDOWN NK_NEXT
#define NK_END 0x23 // END key
#define NK_HOME 0x24 // HOME key

#define NK_LEFT 0x25 // VK_LEFT
#define NK_UP 0x26 // VK_UP
#define NK_RIGHT 0x27 // VK_RIGHT
#define NK_DOWN 0x28 // VK_DOWN

#define NK_PRINTSCREEN 0x2c // VK_SNAPSHOT, PRINT SCREEN key

#define NK_INSERT 0x2d // INS key
#define NK_DELETE 0x2e // DEL key

#define NK_0 0x30 // 0
#define NK_1 0x31 // 1
#define NK_2 0x32 // 2
#define NK_3 0x33 // 3
#define NK_4 0x34 // 4
#define NK_5 0x35 // 5
#define NK_6 0x36 // 6
#define NK_7 0x37 // 7
#define NK_8 0x38 // 8
#define NK_9 0x39 // 9

#define NK_A 0x41 // A
#define NK_B 0x42 // B
#define NK_C 0x43 // C
#define NK_D 0x44 // D
#define NK_E 0x45 // E
#define NK_F 0x46 // F
#define NK_G 0x47 // G
#define NK_H 0x48 // H
#define NK_I 0x49 // I
#define NK_J 0x4A // J
#define NK_K 0x4B // K
#define NK_L 0x4C // L
#define NK_M 0x4D // M
#define NK_N 0x4E // N
#define NK_O 0x4F // O
#define NK_P 0x50 // P
#define NK_Q 0x51 // Q
#define NK_R 0x52 // R
#define NK_S 0x53 // S
#define NK_T 0x54 // T
#define NK_U 0x55 // U
#define NK_V 0x56 // V
#define NK_W 0x57 // W
#define NK_X 0x58 // X
#define NK_Y 0x59 // Y
#define NK_Z 0x5A // Z

#define NK_LWIN 0x5B // Left windows key
#define NK_RWIN 0x5C // Right windows key
#define NK_APP 0x5D // Applications key (which opens RMB menu)

#define NK_NUMPAD0 0x60 // NUMPAD 0
#define NK_NUMPAD1 0x61 // NUMPAD 1
#define NK_NUMPAD2 0x62 // NUMPAD 2
#define NK_NUMPAD3 0x63 // NUMPAD 3
#define NK_NUMPAD4 0x64 // NUMPAD 4
#define NK_NUMPAD5 0x65 // NUMPAD 5
#define NK_NUMPAD6 0x66 // NUMPAD 6
#define NK_NUMPAD7 0x67 // NUMPAD 7
#define NK_NUMPAD8 0x68 // NUMPAD 8
#define NK_NUMPAD9 0x69 // NUMPAD 9

#define NK_MULTIPLY 0x6A // Multiply key
#define NK_ADD 0x6B // Add key
#define NK_SEPARATOR 0x6C // Separator key
#define NK_SUBSTRACT 0x6D // Substract key
#define NK_DECIMAL 0x6E // Decimal key
#define NK_DIVIDE 0x6F // Divide key

#define NK_F1 0x70 // F1 key
#define NK_F2 0x71 // F2 key
#define NK_F3 0x72 // F3 key
#define NK_F4 0x73 // F4 key
#define NK_F5 0x74 // F5 key
#define NK_F6 0x75 // F6 key
#define NK_F7 0x76 // F7 key
#define NK_F8 0x77 // F8 key
#define NK_F9 0x78 // F9 key
#define NK_F10 0x79 // F10 key
#define NK_F11 0x7A // F11 key
#define NK_F12 0x7B // F12 key

#define NK_NUMLOCK 0x90 // NUM LOCK key
#define NK_SCROLL 0x91 // SCROLL LOCK key

#define NK_LSHIFT 0xA0 // Left SHIFT key
#define NK_RSHIFT 0xA1 // Right SHIFT key
#define NK_LCONTROL 0xA2 // Left CTRL key
#define NK_RCONTROL 0xA3 // Right CTRL key
#define NK_LMENU 0xA4 // VK_LMENU, Left ALT key
	#define NK_LALT NK_LMENU
#define NK_RMENU 0xA5 // VK_RMENU, Right ALT key
	#define NK_RALT NK_RMENU

#define NK_OEM_1 0xba // VK_OEM_1, ; or :
#define NK_OEM_PLUS 0xbb // VK_OEM_PLUS, = or +
#define NK_OEM_COMMA 0xbc // VK_OEM_COMMA, , or <
#define NK_OEM_MINUS 0xbd // VK_OEM_MINUS, - or _
#define NK_OEM_PERIOD 0xbe // VK_OEM_PERIOD, . or >
#define NK_OEM_2 0xbf // VK_OEM_2, / or ?
#define NK_OEM_3 0xc0 // VK_OEM_3, ` or ~

#define NK_OEM_4 0xdb // VK_OEM_4, [ or {
#define NK_OEM_5 0xdc // VK_OEM_5, \ or |
#define NK_OEM_6 0xdd // VK_OEM_6, ] or }
#define NK_OEM_7 0xde // VK_OEM_7, ' or "

#endif
