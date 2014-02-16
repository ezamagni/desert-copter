/*
 * minimap.c
 *
 *  Created on: 05/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include "hud.h"
#include "../graphics/texture.h"
#include <stdlib.h>

#define MAPW terrain->width
#define MAPH terrain->depth

/*
 * Texture di terreno
 * Questa texture rappresenta la conformazione del terreno
 * attraverso diverse gradazioni di colore.
 * Viene generata da loadMMap (1 pixel per ogni slot del terreno).
 */
GLuint tex_terrain;

/*
 * Texture di maskera
 * Questa texture viene applicata allo stencil buffer per
 * delimitare l'area circolare di viewport nella quale
 * poter disegnare la minimappa.
 * Come funzione accessoria, fornisce lo sfondo scuro per
 * le aree esterne al terreno di gioco.
 */
GLuint tex_mask;

/*
 * Texture di bordo
 * Viene disegnata sucessivamente alla minimappa come
 * elemento decorativo e per coprire i bordi netti dello
 * stencil buffer.
 */
GLuint tex_border;

// puntatore al terreno attuale
const Terrain *terrain;
// puntatore all'elicottero
const Copter *copter;

// gestisce il lampeggìo degli elementi sulla mappa
const int TIME_BTW_BLINKS = 17;
short curTime = 0;
char blinkff = 0;

// ricorda la posizione della cassa
float crateX, crateZ;
int cratePresent = 0;


int getIDX(int d, int w, int width, int bpp) {
	return d * width * bpp + w * bpp;
}

void loadMMap(const Terrain *ter, const Copter *copt) {
	static const GLubyte browncol[3] = {180, 145, 100};
	static const short darkspan = 85;
	static const char texPath[] = "textures/gui/";
	static const char *texName[] = {"mmapmask.tga", "mmapborder.tga"};

	// carico texture maschera
	TexParam texparams = { GL_FALSE, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_RGBA };
	bindTexture(&tex_mask, texPath, texName[0], &texparams);
	texparams.texMagFilter = texparams.texMinFilter = GL_LINEAR;
	bindTexture(&tex_border, texPath, texName[1], &texparams);

	// inizializzo dati texture terreno
	GLubyte *data_terrain;
	data_terrain = malloc(ter->width * ter->depth * 3 * sizeof(GLubyte));

	// genero dati texture
	for(int d = 0; d < ter->depth; d++) {
		for(int w = 0; w < ter->width; w++) {
			// ottengo altezza relativa dello slot corrente
			// hrel : 1 = (slotH - minH) : (maxH - minH)
			float hrel = (ter->groundHeight[IDX(w, d, ter->width)] - ter->minHeight)
					/ (ter->maxHeight - ter->minHeight);

			// gradazione di marrone a seconda dell'altezza slot
			// dark : darkspan = hrel : 1
			short darkfactor = darkspan * hrel;
			data_terrain[getIDX(d, w, ter->width, 3)] =
					(GLubyte) (browncol[0] - darkfactor);
			data_terrain[getIDX(d, w, ter->width, 3) + 1] =
					(GLubyte) (browncol[1] - darkfactor);
			data_terrain[getIDX(d, w, ter->width, 3) + 2] =
					(GLubyte) (browncol[2] - darkfactor);
		}
	}

	// carico texture terreno
	glGenTextures(1, &tex_terrain);
	glBindTexture(GL_TEXTURE_2D, tex_terrain);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ter->width, ter->depth, 0, GL_RGB,
			GL_UNSIGNED_BYTE, data_terrain);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	terrain = ter;
	copter = copt;

	// libero dati terreno
	free(data_terrain);
}

void destroyMMap() {
	// libero risorse
	glDeleteTextures(1, &tex_terrain);
	glDeleteTextures(1, &tex_mask);
	glDeleteTextures(1, &tex_border);
	// resetto stato
	terrain = NULL;
	copter = NULL;
	curTime = 0;
	cratePresent = 0;
}

void drawMMap(int x, int y) {
	// ricavo posizioni elicottero e cassa relative alla mappa
	float coptXrel = (copter->pos.x / terrain->slotSize);
	float coptZrel = (copter->pos.z / terrain->slotSize);
	float crateXrel = (int)(crateX / terrain->slotSize);
	float crateZrel = (int)(crateZ / terrain->slotSize);

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_TEXTURE_2D);

	// vogliamo rempire lo stencil con valori a 1:
	glStencilFunc(GL_ALWAYS, 1, 1);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	// impostiamo alpha test: disegneremo la texture di
	// maschera ma vogliamo che i pixel completamente trasparenti
	// vengano scartati (lo stencil prenderà la forma della maschera)
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_NOTEQUAL, 0);

	// disegno la texture di maschera per dare una forma
	// circolare al nostro stencil test
	glPushMatrix();
	{
		glTranslatef(x, y, 0);
		glBindTexture(GL_TEXTURE_2D, tex_mask);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, 0); glVertex2f(0, 0);
			glTexCoord2f(0, 1); glVertex2f(0, mmapHeight);
			glTexCoord2f(1, 1); glVertex2f(mmapWidth, mmapHeight);
			glTexCoord2f(1, 0); glVertex2f(mmapWidth, 0);
		}
		glEnd();
	}
	glPopMatrix();

	// ora si vuole disegnare solo dove lo stencil vale 1
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glPushMatrix();
	{
		// (3) porto la mappa nel punto desiderato
		glTranslatef(mmapWidth / 2 + x, mmapHeight / 2 + y, 0);
		// (2) ruoto la mappa seguendo facing dell'elicottero
		glRotatefr((copter->angle.yaw + M_PI), 0, 0, 1);
		glScalef(mmapZoom, mmapZoom, mmapZoom);
		// (1) centro la mappa
		glTranslatef(-coptXrel, -coptZrel, 0);

		// finalmente.. disegno la minimappa!
		glBindTexture(GL_TEXTURE_2D, tex_terrain);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, 0); glVertex2f(0, 0);
			glTexCoord2f(0, 1); glVertex2f(0, MAPH);
			glTexCoord2f(1, 1); glVertex2f(MAPW, MAPH);
			glTexCoord2f(1, 0); glVertex2f(MAPW, 0);
		}
		glEnd();

		// disegno marker cassa
		if(cratePresent && blinkff) {
			glDisable(GL_TEXTURE_2D);
			glColor3f(0, 0, 0);
			glTranslatef(crateXrel, crateZrel, 0);
			glBegin(GL_QUADS);
			{
				glVertex2f(0, 0);
				glVertex2f(0, 3);
				glVertex2f(3, 3);
				glVertex2f(3, 0);
			}
			glEnd();
		}
	}
	glPopMatrix();

    // disabilito stencil test
	glDisable(GL_STENCIL_TEST);

   	// disegno texture di bordo della minimappa
	glEnable(GL_TEXTURE_2D);
	glTranslatef(x - 4, y - 4, 0);
	glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, tex_border);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, 0); glVertex2f(0, 0);
			glTexCoord2f(0, 1); glVertex2f(0, mmapHeight + 8);
			glTexCoord2f(1, 1); glVertex2f(mmapWidth + 8, mmapHeight + 8);
			glTexCoord2f(1, 0); glVertex2f(mmapWidth + 8, 0);
		}
		glEnd();

	glPopAttrib();
}

void updateMMapCratePos(float xPos, float zPos, int isPresent) {
	cratePresent = isPresent;
	crateX = xPos;
	crateZ = zPos;
}

void updateMMap() {
	// prendo semplicemente nota del tempo che passa
	curTime++;
	if(curTime > TIME_BTW_BLINKS) {
		blinkff = ~blinkff;
		curTime = 0;
	}
}

