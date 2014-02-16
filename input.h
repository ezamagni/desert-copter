/*
 * input.h
 *
 *  Created on: 03/lug/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef INPUT_H_
#define INPUT_H_

#include <SDL/SDL.h>

// indici dei tasti del mouse
enum MButtons {
	MBUTTON_LEFT = 1,
	MBUTTON_MIDDLE = 2,
	MBUTTON_RIGHT = 4
};

// indici dei controlli analogici
// NB: non è prevista una configurazione
// assistita del gamepad. Per adattare il
// comportamento a un pad diverso,
// modificare i seguenti indici.
enum KPTriggers {
	TRIG_LEFT_HORZ = 0,
	TRIG_LEFT_VERT = 1,
	TRIG_RIGHT_VERT = 4,
	TRIG_RIGHT_HORZ = 3,
	TRIG_BACKLEFT = 2,
	TRIG_BACKRIGHT = 5
};

// indici dei tasti del gamepad
enum KPButtons {
	KPButton_ENTER = 0,
	KPButton_ESC = 6,
	KPButton_BACKLEFT = 4,
	KPButton_BACKRIGHT = 5
};

// tasti di controllo
#define KM_CONTROL 0
enum KeymapControl {
	ESC = 1,
	ENTER = 2
};

// tasti direzionali
#define KM_ARROWS 1
enum KeymapArrows {
	UP = 1,
	DOWN = 2,
	LEFT = 4,
	RIGHT = 8
};

// tasti elicottero
#define KM_COPTER 2
enum KeymapCopter {
	VERT_UP = 1,
	VERT_DOWN = 2,
	FORWARD = 4,
	BACKWARD = 8,
	LEAN_LEFT = 16,
	LEAN_RIGHT = 32,
	TURN_LEFT = 64,
	TURN_RIGHT = 128
};

// tasti telecamera
#define KM_CAMERA 3
enum KeymapCamera {
	CAM_UP = 1,
	CAM_DOWN = 2,
	CAM_LEFT = 4,
	CAM_RIGHT = 8,
	CAM_FORWARD = 16,
	CAM_BACKWARD = 32,
	CAM_LOCK_TOGGLE = 64,
};


// mappa tasti
typedef unsigned char KeyMap;

// stato della tastiera / keypad
typedef struct {
	KeyMap kState[4];
	float value[6];
} KeyState;

// stato del cursore
typedef struct {
	int x;
	int y;
	float xRel;
	float yRel;
	enum MButtons buttons;
} CursorState;

// stato complessivo dell'input
typedef struct {
	KeyState keys;
	CursorState cursor;
} InputState;


// ispeziona l'evento SDL fornito e costruisce lo stato corrente dell'input
// ritorna 0 se lo stato non è cambiato, 1 altrimenti
int getInput(const SDL_Event *e, InputState *curInput, int scrW, int scrH);

// estrae dallo stato corrente la mappa tasti richiesta
KeyMap getKeymap(InputState *input, int keyMap);

// restituisce 1 se - in una data mappa - il tasto indicato è premuto
// NB: in caso affermativo resetta lo stato di quel tasto (utile per
// avere un singolo feedback dalla pressione di un tasto)
int queryKey(InputState *in, int keyMap, int key);

#endif /* INPUT_H_ */
