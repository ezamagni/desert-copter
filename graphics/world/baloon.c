/*
 * baloon.c
 *
 *  Created on: 06/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include "baloon.h"
#include "../texture.h"
#include <stdlib.h>


Baloon* createBaloon(const Terrain* ter) {
	static const char modelPath[] = "models/baloon/";
	static const char modelFileName[] = "baloon.obj";
	static const GLfloat height_surplus = 5.8f;

	// inizializzo struttura
	Baloon *b = malloc(sizeof(Baloon));
	b->rot = 0;

	// scelgo una posizione casuale a bordo mappa
	float rand = ((float)random() / RAND_MAX) * ter->slotSize;
	switch (random() % 4) {
	case 0:
		// primo lato
		b->pos.x = ter->width * rand;
		b->pos.z = 0;
		break;
	case 1:
		// secondo lato
		b->pos.x = ter->width * ter->slotSize;
		b->pos.z = ter->depth * rand;
		break;
	case 2:
		// terzo lato
		b->pos.x = ter->width * rand;
		b->pos.z = ter->depth * ter->slotSize;
		break;
	default:
		// quarto lato
		b->pos.x = 0;
		b->pos.z = ter->width * rand;
		break;
	}

	// altezza superiore a punto più alto del terreno
	b->pos.y = ter->maxHeight + height_surplus;

	// imposto direzione
	Vect3 mapCenter = {0.5f * ter->width * ter->slotSize,
			0, 0.5f * ter->depth * ter->slotSize};
	b->dir = subtr(mapCenter, b->pos);
	b->dir.y = 0;
	b->dir = normalize(b->dir);

	// carico modello
	TexParam texparam = {GL_TRUE, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, GL_REPEAT, GL_RGB};
	b->model = loadModelFromOBJ(modelPath, modelFileName, &texparam);
	if (b->model == NULL) {
		destroyBaloon(&b);
		return NULL;
	}

	// compilo displaylist
	b->displist = glGenLists(1);
	glNewList(b->displist, GL_COMPILE);
	{
		renderModel(b->model);
	}
	glEndList();

	return b;
}

void destroyBaloon(Baloon** b) {
	destroyModel((*b)->model);
	glDeleteLists((*b)->displist, 1);
	free(*b);
	*b = NULL;
}

void updateBaloon(Baloon* b, const Terrain *ter) {
	static const GLfloat rot_speed = 0.002f;
	static const GLfloat mov_speed = 0.246f;

	// sposto e ruoto il pallone
	b->pos = sum(b->pos, scale(b->dir, mov_speed));
	b->rot += rot_speed;
	b->rot = remainderf(b->rot, M_2PI);
}

void renderBaloon(const Baloon* b) {
	glEnable(GL_LIGHTING);
	// vogliamo vedere anche l'interno del tessuto del pallone,
	// disabilitiamo momentaneamente il face culling
	glDisable(GL_CULL_FACE);
	glPushMatrix();
	{
		glTranslatef(b->pos.x, b->pos.y, b->pos.z);
		glRotatefr(b->rot, 0, 1, 0);

		// spazio coordinate pallone
		glCallList(b->displist);
	}
	glPopMatrix();
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
}


