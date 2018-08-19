#include <Arduino.h>
#include <HID-Project.h>

#include "inline.h"
#include "keymap.h"

#define CMD_SERIAL Serial1
#define CMD_SERIAL_SPEED 115200

#define CMD_MOUSE_LEFT			0b10000000
#define CMD_MOUSE_LEFT_STATE	0b00001000
#define CMD_MOUSE_RIGHT			0b01000000
#define CMD_MOUSE_RIGHT_STATE	0b00000100


// -----------------------------------------------------------------------------
INLINE void cmdResetHid() { // 0 bytes
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	BootKeyboard.releaseAll();
	AbsoluteMouse.releaseAll();
}

INLINE void cmdKeyEvent() { // 2 bytes
	KeyboardKeycode code = keymap((uint8_t)CMD_SERIAL.read());
	uint8_t state = CMD_SERIAL.read();
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	if (code != KEY_ERROR_UNDEFINED) {
		if (state) {
			BootKeyboard.press(code);
		} else {
			BootKeyboard.release(code);
		}
	}
}

INLINE void cmdMouseMoveEvent() { // 4 bytes
	int x = (int)CMD_SERIAL.read() << 8;
	x |= (int)CMD_SERIAL.read();
	int y = (int)CMD_SERIAL.read() << 8;
	y |= (int)CMD_SERIAL.read();
	AbsoluteMouse.moveTo(x, y);
}

INLINE void cmdMouseButtonEvent() { // 1 byte
	uint8_t state = CMD_SERIAL.read();
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	if (state & CMD_MOUSE_LEFT) {
		if (state & CMD_MOUSE_LEFT_STATE) {
			AbsoluteMouse.press(MOUSE_LEFT);
		} else {
			AbsoluteMouse.release(MOUSE_LEFT);
		}
	}
	if (state & CMD_MOUSE_RIGHT) {
		if (state & CMD_MOUSE_RIGHT_STATE) {
			AbsoluteMouse.press(MOUSE_RIGHT);
		} else {
			AbsoluteMouse.release(MOUSE_RIGHT);
		}
	}
}

INLINE void cmdMouseWheelEvent() { // 2 bytes
	CMD_SERIAL.read(); // delta_x is not supported by hid-project now
	signed char delta_y = CMD_SERIAL.read();
	CMD_SERIAL.read(); // unused
	CMD_SERIAL.read(); // unused
	AbsoluteMouse.move(0, 0, delta_y);
}


// -----------------------------------------------------------------------------
void setup() {
	CMD_SERIAL.begin(CMD_SERIAL_SPEED);
	BootKeyboard.begin();
	AbsoluteMouse.begin();
	CMD_SERIAL.write(0);
}

void loop() {
	if (CMD_SERIAL.available() >= 5) {
		switch ((uint8_t)CMD_SERIAL.read()) {
			case 0: cmdResetHid(); break;
			case 1: cmdKeyEvent(); break;
			case 2: cmdMouseMoveEvent(); break;
			case 3: cmdMouseButtonEvent(); break;
			case 4: cmdMouseWheelEvent(); break;
			default: break;
		}
		CMD_SERIAL.write(0);
	}
}
