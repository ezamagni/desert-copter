/*
 * hud.h
 *
 *  Created on: 04/lug/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef HUD_H_
#define HUD_H_

#include "../graphics/world/terrain.h"
#include "../graphics/world/copter.h"

#define mmapZoom 1.2f
#define mmapWidth 200
#define mmapHeight 200

// inizializzazione e scaricamento HUD
int loadHUD();
void destroyHUD();

// visualizza i punti totalizzati
void drawPoints(int points, int x, int y);
// visualizza il carburante rimanente (fuel: [0,1])
void drawFuel(float fuel, int x, int y);
// visualizza i danni subiti (damage: [0,1])
void drawDamage(float damage, int x, int y);

// inizializza la minimappa per il terreno fornito in input
void loadMMap(const Terrain *ter, const Copter *copt);
void destroyMMap();

// funzioni di aggiornamento della minimappa:
// specifica la posizione corrente della cassa
// e un flag che indica se la cassa è presente o
// meno nella mappa
void updateMMapCratePos(float xPos, float zPos, int isPresent);
void updateMMap();

// disegna la minimappa al punto indicato
void drawMMap(int x, int y);


#endif /* HUD_H_ */
