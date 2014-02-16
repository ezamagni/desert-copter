/*
 * gui.h
 *
 *  Created on: 03/lug/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef GUI_H_
#define GUI_H_

#include "../input.h"
#include "../options.h"

// funzioni di callback per il menu
enum CallFunc {
	CALL_EXIT = 0, CALL_RETURN, CALL_NEWGAME, CALL_OPTIONS
};

// tipologia di menu
enum MenuType {
	MNUT_MAIN = 0, MNUT_PAUSE, MNUT_OPTIONS
};


// inizializza le risorse grafiche per i menu
int loadGUI(void (**menucallbacks)(void));

// libera le risorse grafice utilizzate dai manu
void destroyGUI();

// disegna il menu corrente in base alle dimensioni del viewport
void drawMenu(int scrW, int scrH);

// passa da un tipo di menu ad un altro
void switchMenu(int menuType);

// aggiorna lo stato dei menu in base all'input dell'utente
void updateMenu(InputState *input);

#endif /* GUI_H_ */
