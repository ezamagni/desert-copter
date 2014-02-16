/*
 * billboard.h
 *
 *  Created on: 14/lug/2012
 *      Author: EnricoZamagni
 */

#ifndef BILLBOARD_H_
#define BILLBOARD_H_

#include "../model.h"
#include "terrain.h"

typedef struct {
	Model *model;
	GLuint displist;
	Vect3 pos;
} Billboard;

// carica il tabellone nella posizione indicata dal terreno
// NB: se il terreno non prevede un tabellone viene restituito NULL
Billboard* createBillboard(const Terrain* ter);

// libero risorse del tabellone
void destroyBillboard(Billboard** b);

// renderizzo tabellone
void renderBillboard(const Billboard* b);
#endif //BILLBOARD_H_
