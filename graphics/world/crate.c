/*
 * crate.c
 *
 *  Created on: 02/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include "crate.h"
#include "../texture.h"
#include <math.h>
#include <stdlib.h>
#include <GL/gl.h>


Crate* createCrate(TCoord coord, Terrain* ter, int ttl) {
	const char modelPath[] = "models/crate/";
	const char modelFileName[] = "crate.obj";

	// inizializzo struttura
	Crate *c = malloc(sizeof(Crate));
	c->bounce = 1;
	c->ttl = ttl;

	// carico modello
	TexParam texparam = {GL_TRUE, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST, GL_REPEAT, GL_RGB};
	c->model = loadModelFromOBJ(modelPath, modelFileName, &texparam);
	if (c->model == NULL) {
		destroyCrate(&c);
		return NULL;
	}

	// calcolo posizione a partire da coordinate mondo
	Vect3 cpos;
	cpos.x = ter->slotSize * (coord.x + 0.5f);
	cpos.z = ter->slotSize * (coord.z + 0.5f);
	cpos.y = ter->groundHeight[IDX(coord.x, coord.z, ter->width)] + 1.2f;
	c->pos = cpos;

	return c;
}

void destroyCrate(Crate** c) {
	destroyModel((*c)->model);
	free(*c);
	*c = NULL;
}

void updateCrate(Crate* c) {
	static const float bounce_speed = M_2PI / 380;
	static const float bounce_height_amount = 0.0043f;

	c->bounce += bounce_speed;
	c->bounce = remainderf(c->bounce, M_2PI);

	c->pos.y += sin(c->bounce) * bounce_height_amount;
	c->ttl--;
}

void renderCrate(Crate* c) {
	static const float bounce_scale_amount = 0.08f;
	GLfloat scale = 1 + sin(c->bounce) * bounce_scale_amount;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_LIGHTING);
	// la luce 1 simula il riflesso del terreno, non serve tenerne conto
	glDisable(GL_LIGHT1);
	glPushMatrix();
	{
		glTranslatef(c->pos.x, c->pos.y, c->pos.z);
		glRotatefr(c->bounce, 0, 1, 0);
		glScalef(scale, scale, scale);

		// renderizzo cassa
		renderModel(c->model);
	}
	glPopMatrix();
	glPopAttrib();
}
