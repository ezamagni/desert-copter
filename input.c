/*
 * input.c
 *
 *  Created on: 03/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include "input.h"
#include <math.h>

static const float analog_sensitivity = 1.0f / 750000;


int getInput(const SDL_Event *e, InputState *curInput, int scrW, int scrH) {
	int stateChanged = 1;

	switch (e->type) {
	case SDL_JOYAXISMOTION:
		if (abs(e->jaxis.value) > 3270) {
			// lo stick si è mosso
			float val = (float) e->jaxis.value / 32768.0f;
			switch (e->jaxis.axis) {
			case TRIG_LEFT_HORZ:
				curInput->keys.value[TRIG_LEFT_HORZ] = val;
				if (val < 0) {
					curInput->keys.kState[KM_COPTER] |= TURN_LEFT;
					curInput->keys.kState[KM_COPTER] &= ~TURN_RIGHT;
				} else {
					curInput->keys.kState[KM_COPTER] |= TURN_RIGHT;
					curInput->keys.kState[KM_COPTER] &= ~TURN_LEFT;
				}
				break;
			case TRIG_LEFT_VERT:
				curInput->keys.value[TRIG_LEFT_VERT] = val;
				if (val > 0) {
					curInput->keys.kState[KM_COPTER] |= BACKWARD;
					curInput->keys.kState[KM_COPTER] &= ~FORWARD;
				} else {
					curInput->keys.kState[KM_COPTER] |= FORWARD;
					curInput->keys.kState[KM_COPTER] &= ~BACKWARD;
				}
				break;
			case TRIG_RIGHT_HORZ:
				curInput->keys.value[TRIG_RIGHT_HORZ] = val;
				curInput->cursor.buttons |= MBUTTON_LEFT;
				curInput->cursor.xRel = scrW * val * analog_sensitivity;

				break;
			case TRIG_RIGHT_VERT:
				curInput->keys.value[TRIG_RIGHT_VERT] = val;
				curInput->cursor.buttons |= MBUTTON_LEFT;
				curInput->cursor.yRel = scrH * val * analog_sensitivity;
				break;
			case TRIG_BACKLEFT:
				if (val < 0) {
					curInput->keys.value[TRIG_BACKLEFT] = 0;
					curInput->keys.kState[KM_COPTER] &= ~VERT_UP;
				} else {
					curInput->keys.value[TRIG_BACKLEFT] = val;
					curInput->keys.kState[KM_COPTER] |= VERT_UP;
				}
				break;
			case TRIG_BACKRIGHT:
				if (val < 0) {
					curInput->keys.value[TRIG_BACKRIGHT] = 0;
					curInput->keys.kState[KM_COPTER] &= ~VERT_DOWN;
				} else {
					curInput->keys.value[TRIG_BACKRIGHT] = val;
					curInput->keys.kState[KM_COPTER] |= VERT_DOWN;
				}
				break;
			default:
				stateChanged = 0;
				break;
			}
		} else {
			// lo stick si è riposizionato
			switch (e->jaxis.axis) {
			case TRIG_LEFT_HORZ:
				curInput->keys.value[TRIG_LEFT_HORZ] = 0;
				curInput->keys.kState[KM_COPTER] &= ~TURN_LEFT;
				curInput->keys.kState[KM_COPTER] &= ~TURN_RIGHT;
				break;
			case TRIG_LEFT_VERT:
				curInput->keys.value[TRIG_LEFT_VERT] = 0;
				curInput->keys.kState[KM_COPTER] &= ~BACKWARD;
				curInput->keys.kState[KM_COPTER] &= ~FORWARD;
				break;
			case TRIG_RIGHT_HORZ:
				curInput->keys.value[TRIG_RIGHT_HORZ] = 0;
				curInput->cursor.buttons &= ~MBUTTON_LEFT;
				curInput->cursor.xRel = 0;
				break;
			case TRIG_RIGHT_VERT:
				curInput->keys.value[TRIG_RIGHT_VERT] = 0;
				curInput->cursor.buttons &= ~MBUTTON_LEFT;
				curInput->cursor.yRel = 0;
				break;
			case TRIG_BACKLEFT:
				curInput->keys.value[TRIG_BACKLEFT] = 0;
				curInput->keys.kState[KM_COPTER] &= ~VERT_UP;
				break;
			case TRIG_BACKRIGHT:
				curInput->keys.value[TRIG_BACKRIGHT] = 0;
				curInput->keys.kState[KM_COPTER] &= ~VERT_DOWN;
				break;
			default:
				stateChanged = 0;
				break;
			}
		}
		break;

	case SDL_JOYHATMOTION:
		if (e->jhat.value & SDL_HAT_UP) {
			curInput->keys.kState[KM_ARROWS] |= UP;
		}
		if (e->jhat.value & SDL_HAT_LEFT) {
			curInput->keys.kState[KM_ARROWS] |= LEFT;
		}
		if (e->jhat.value & SDL_HAT_RIGHT) {
			curInput->keys.kState[KM_ARROWS] |= RIGHT;
		}
		if (e->jhat.value & SDL_HAT_DOWN) {
			curInput->keys.kState[KM_ARROWS] |= DOWN;
		}
		break;

	case SDL_JOYBUTTONDOWN:
		/*
		 * TASTO GAMEPAD PREMUTO
		 */
		switch (e->jbutton.button) {
		case KPButton_ENTER:
			curInput->keys.kState[KM_CONTROL] |= ENTER;
			break;
		case KPButton_ESC:
			curInput->keys.kState[KM_CONTROL] |= ESC;
			break;
		case KPButton_BACKLEFT:
			curInput->keys.kState[KM_COPTER] |= LEAN_LEFT;
			break;
		case KPButton_BACKRIGHT:
			curInput->keys.kState[KM_COPTER] |= LEAN_RIGHT;
			break;
		default:
			stateChanged = 0;
			break;
		}

		break;

	case SDL_JOYBUTTONUP:
		/*
		 * TASTO GAMEPAD RILASCIATO
		 */
		switch (e->jbutton.button) {
		case KPButton_ENTER:
			curInput->keys.kState[KM_CONTROL] &= ~ENTER;
			break;
		case KPButton_ESC:
			curInput->keys.kState[KM_CONTROL] &= ~ESC;
			break;
		case KPButton_BACKLEFT:
			curInput->keys.kState[KM_COPTER] &= ~LEAN_LEFT;
			break;
		case KPButton_BACKRIGHT:
			curInput->keys.kState[KM_COPTER] &= ~LEAN_RIGHT;
			break;
		default:
			stateChanged = 0;
			break;
		}
		break;

	case SDL_KEYDOWN:
		/*
		 * TASTO PREMUTO
		 */
		switch (e->key.keysym.sym) {
		case SDLK_ESCAPE:
			curInput->keys.kState[KM_CONTROL] |= ESC;
			break;
		case SDLK_RETURN:
			curInput->keys.kState[KM_CONTROL] |= ENTER;
			break;

		case SDLK_UP:
			curInput->keys.kState[KM_ARROWS] |= UP;
			break;
		case SDLK_DOWN:
			curInput->keys.kState[KM_ARROWS] |= DOWN;
			break;
		case SDLK_LEFT:
			curInput->keys.kState[KM_ARROWS] |= LEFT;
			break;
		case SDLK_RIGHT:
			curInput->keys.kState[KM_ARROWS] |= RIGHT;
			break;

		case SDLK_q:
		case SDLK_KP7:
			curInput->keys.kState[KM_COPTER] |= VERT_UP;
			curInput->keys.value[TRIG_BACKLEFT] = 1;
			break;
		case SDLK_e:
		case SDLK_KP9:
			curInput->keys.kState[KM_COPTER] |= VERT_DOWN;
			curInput->keys.value[TRIG_BACKRIGHT] = 1;
			break;
		case SDLK_w:
		case SDLK_KP8:
			curInput->keys.kState[KM_COPTER] |= FORWARD;
			curInput->keys.value[TRIG_LEFT_VERT] = -1;
			break;
		case SDLK_s:
		case SDLK_KP2:
			curInput->keys.kState[KM_COPTER] |= BACKWARD;
			curInput->keys.value[TRIG_LEFT_VERT] = 1;
			break;
		case SDLK_1:
		case SDLK_KP1:
			curInput->keys.kState[KM_COPTER] |= LEAN_LEFT;
			break;
		case SDLK_3:
		case SDLK_KP3:
			curInput->keys.kState[KM_COPTER] |= LEAN_RIGHT;
			break;
		case SDLK_a:
		case SDLK_KP4:
			curInput->keys.kState[KM_COPTER] |= TURN_LEFT;
			curInput->keys.value[TRIG_LEFT_HORZ] = -1;
			break;
		case SDLK_d:
		case SDLK_KP6:
			curInput->keys.kState[KM_COPTER] |= TURN_RIGHT;
			curInput->keys.value[TRIG_LEFT_HORZ] = 1;
			break;

		case SDLK_u:
			curInput->keys.kState[KM_CAMERA] |= CAM_UP;
			break;
		case SDLK_o:
			curInput->keys.kState[KM_CAMERA] |= CAM_DOWN;
			break;
		case SDLK_j:
			curInput->keys.kState[KM_CAMERA] |= CAM_LEFT;
			break;
		case SDLK_l:
			curInput->keys.kState[KM_CAMERA] |= CAM_RIGHT;
			break;
		case SDLK_i:
			curInput->keys.kState[KM_CAMERA] |= CAM_FORWARD;
			break;
		case SDLK_k:
			curInput->keys.kState[KM_CAMERA] |= CAM_BACKWARD;
			break;
		case SDLK_c:
			curInput->keys.kState[KM_CAMERA] |= CAM_LOCK_TOGGLE;
			break;
		default:
			stateChanged = 0;
			break;
		}
		break;

	case SDL_KEYUP:
		/*
		 * TASTO RILASCIATO
		 */
		switch (e->key.keysym.sym) {
		case SDLK_ESCAPE:
			curInput->keys.kState[KM_CONTROL] &= ~ESC;
			break;
		case SDLK_RETURN:
			curInput->keys.kState[KM_CONTROL] &= ~ENTER;
			break;

		case SDLK_UP:
			curInput->keys.kState[KM_ARROWS] &= ~UP;
			break;
		case SDLK_DOWN:
			curInput->keys.kState[KM_ARROWS] &= ~DOWN;
			break;
		case SDLK_LEFT:
			curInput->keys.kState[KM_ARROWS] &= ~LEFT;
			break;
		case SDLK_RIGHT:
			curInput->keys.kState[KM_ARROWS] &= ~RIGHT;
			break;

		case SDLK_q:
		case SDLK_KP7:
			curInput->keys.kState[KM_COPTER] &= ~VERT_UP;
			curInput->keys.value[TRIG_BACKLEFT] = 0;
			break;
		case SDLK_e:
		case SDLK_KP9:
			curInput->keys.kState[KM_COPTER] &= ~VERT_DOWN;
                //curInput->keys.value[TRIG_BACKRIGHT] = 0;
			break;
		case SDLK_w:
		case SDLK_KP8:
			curInput->keys.kState[KM_COPTER] &= ~FORWARD;
                //curInput->keys.value[TRIG_LEFT_VERT] = 0;
			break;
		case SDLK_s:
		case SDLK_KP2:
			curInput->keys.kState[KM_COPTER] &= ~BACKWARD;
                //curInput->keys.value[TRIG_LEFT_VERT] = 0;
			break;
		case SDLK_1:
		case SDLK_KP1:
			curInput->keys.kState[KM_COPTER] &= ~LEAN_LEFT;
			break;
		case SDLK_3:
		case SDLK_KP3:
			curInput->keys.kState[KM_COPTER] &= ~LEAN_RIGHT;
			break;
		case SDLK_a:
		case SDLK_KP4:
			curInput->keys.kState[KM_COPTER] &= ~TURN_LEFT;
                //curInput->keys.value[TRIG_LEFT_HORZ] = 0;
			break;
		case SDLK_d:
		case SDLK_KP6:
			curInput->keys.kState[KM_COPTER] &= ~TURN_RIGHT;
                //curInput->keys.value[TRIG_LEFT_HORZ] = 0;
			break;

		case SDLK_u:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_UP;
			break;
		case SDLK_o:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_DOWN;
			break;
		case SDLK_j:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_LEFT;
			break;
		case SDLK_l:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_RIGHT;
			break;
		case SDLK_i:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_FORWARD;
			break;
		case SDLK_k:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_BACKWARD;
			break;
		case SDLK_c:
			curInput->keys.kState[KM_CAMERA] &= ~CAM_LOCK_TOGGLE;
			break;
		default:
			stateChanged = 0;
			break;
		}
		break;

	case SDL_MOUSEMOTION:
		/*
		 * INPUT MOUSE
		 */
		curInput->cursor.x = e->motion.x;
		curInput->cursor.y = e->motion.y;
		curInput->cursor.xRel = (float) e->motion.xrel / scrW;
		curInput->cursor.yRel = (float) e->motion.yrel / scrH;
		break;

	case SDL_MOUSEBUTTONDOWN:
		/*
		 * BOTTONE CURSORE PREMUTO
		 */
		switch (e->button.button) {
		case SDL_BUTTON_LEFT:
			curInput->cursor.buttons |= MBUTTON_LEFT;
			break;
		case SDL_BUTTON_MIDDLE:
			curInput->cursor.buttons |= MBUTTON_MIDDLE;
			break;
		case SDL_BUTTON_RIGHT:
			curInput->cursor.buttons |= MBUTTON_RIGHT;
			break;
		}
		break;

	case SDL_MOUSEBUTTONUP:
		/*
		 * BOTTONE CURSORE RILASCIATO
		 */
		switch (e->button.button) {
		case SDL_BUTTON_LEFT:
			curInput->cursor.buttons &= ~MBUTTON_LEFT;
			break;
		case SDL_BUTTON_MIDDLE:
			curInput->cursor.buttons &= ~MBUTTON_MIDDLE;
			break;
		case SDL_BUTTON_RIGHT:
			curInput->cursor.buttons &= ~MBUTTON_RIGHT;
			break;
		}
		break;

	default:
		stateChanged = 0;
		break;
	}

	return stateChanged;
}

KeyMap getKeymap(InputState *input, int keyMap) {
	return input->keys.kState[keyMap];
}

int queryKey(InputState *in, int keyMap, int key) {
	if (in->keys.kState[keyMap] & key) {
		in->keys.kState[keyMap] &= ~key;
		return 1;
	} else {
		return 0;
	}
}
