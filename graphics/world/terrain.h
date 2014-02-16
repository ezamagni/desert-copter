/*
 * terrain.h
 *
 *  Created on: 26/giu/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef TERRAIN_H_
#define TERRAIN_H_

#include <GL/gl.h>
#include "../../mathext.h"

#define IDX(x, y, s) ((y) * (s) + (x))

// coordinate slot terreno
typedef struct {
	int x;
	int z;
} TCoord;

// struttura del terreno di gioco
typedef struct {
	unsigned char *hdata;
	int width, depth;
	TCoord startPos;
	TCoord creditPos;
	GLfloat slotSize;
	GLuint dispList;
	GLuint texName;
	Vect3 *groundNormal;
	GLboolean *slotFlat;
	GLfloat *groundHeight;
	GLfloat maxHeight;
	GLfloat minHeight;
} Terrain;

// carica un terreno dal file mappa indicato.
// NB: sono accettate soltanto immagini .tga SENZA compressione RLE
Terrain* loadTerrain(const char *path, const char *fileName);

// libera tutte le risorse occupate dal terreno
void destroyTerrain(Terrain *ter);

// compila un terreno precedentemente caricato specificando dimensione degli slot
// e altezza massima raggiungibile
void compileTerrain(Terrain *terrain, GLfloat slotSize, GLfloat maxHeight);

// ricava l'indice dello slot di un terreno già compilato in base alle coordinate x e z
int getSlotFromPos(GLfloat x, GLfloat z, Terrain *ter);

#endif /* TERRAIN_H_ */
