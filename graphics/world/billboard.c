/*
 * billboard.c
 *
 *  Created on: 14/lug/2012
 *      Author: EnricoZamagni
 */

#include "billboard.h"
#include <stdlib.h>


Billboard* createBillboard(const Terrain* ter) {
	static const char billbFile[] = "billboard.obj";
	static const char billbPath[] = "models/billboard/";

	if(ter->creditPos.x < 0) {
		// il terreno non prevede un tabellone
		return NULL;
	}

	// inizializzo struttura
	Billboard *b = malloc(sizeof(Billboard));

	// carico tabellone
	TexParam texp = {GL_TRUE, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_CLAMP, GL_RGB};
	b->model = loadModelFromOBJ(billbPath, billbFile, &texp);
	b->pos.x = ter->slotSize * (ter->creditPos.x + 0.5f);
	b->pos.z = ter->slotSize * (ter->creditPos.z + 0.5f);
	b->pos.y = ter->groundHeight[IDX(ter->creditPos.x, ter->creditPos.z, ter->width)];

	// compilo displaylist
	b->displist = glGenLists(1);
	glNewList(b->displist, GL_COMPILE);
	{
		renderModel(b->model);
	}
	glEndList();

	return b;
}

void destroyBillboard(Billboard** b) {
	destroyModel((*b)->model);
	glDeleteLists((*b)->displist, 1);
	free(*b);
	*b = NULL;
}

void renderBillboard(const Billboard* b) {
	glEnable(GL_LIGHTING);
	glPushMatrix();
	{
		glTranslatef(b->pos.x, b->pos.y, b->pos.z);
		glCallList(b->displist);
	}
	glPopMatrix();
	glDisable(GL_LIGHTING);
}
