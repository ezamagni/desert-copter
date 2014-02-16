/*
 * baloon.h
 *
 *  Created on: 06/lug/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef BALOON_H_
#define BALOON_H_

#include "../model.h"
#include "terrain.h"

typedef struct {
	Model *model;
	GLuint displist;
	GLfloat rot;
	Vect3 pos;
	Vect3 dir;
} Baloon;

// crea un nuovo pallone in una posizione casuale
Baloon* createBaloon(const Terrain* ter);

// libero risorse usate dal pallone
void destroyBaloon(Baloon** b);

// aggiorno stato del pallone
void updateBaloon(Baloon* b, const Terrain *ter);

// renderizzo pallone
void renderBaloon(const Baloon* b);

#endif /* BALOON_H_ */
