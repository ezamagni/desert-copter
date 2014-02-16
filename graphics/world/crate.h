/*
 * crate.h
 *
 *  Created on: 02/lug/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef CRATE_H_
#define CRATE_H_

#include "../model.h"
#include "terrain.h"


typedef struct {
	Model *model;
	GLfloat bounce;
	Vect3 pos;
	int ttl;
} Crate;

// istanzio/libero risorse per cassa
Crate* createCrate(TCoord coord, Terrain* ter, int ttl);
void destroyCrate(Crate** c);

// aggiornamento cassa
void updateCrate(Crate* c);

// renderizzazione cassa
void renderCrate(Crate* c);

#endif /* CRATE_H_ */
