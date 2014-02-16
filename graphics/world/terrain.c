/*
 * terrain.c
 *
 *  Created on: 26/giu/2012
 *      Author: "Enrico Zamagni"
 */

#include "terrain.h"
#include "../model.h"
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 256
#define SPEC_STARTPOS 0xff
#define SPEC_CREDITPOS 0xaa
#define GROUND_FLAT_TOLERANCE 0.0286f
#define GROUND_STEEP_TOLERANCE 0.042f


Terrain* loadTerrain(const char *path, const char *fileName) {
	static const Terrain _emptyTerrain;
	unsigned char header[20];
	char fullPath[BUFSIZE];

	// carico file di texture
	strcpy(fullPath, path);
	strcat(fullPath, fileName);
	FILE *file = fopen(fullPath, "rb");
	if (file == NULL) {
		fprintf(stderr, "[ERROR]: could not open terrain file: %s\n", fileName);
		return NULL;
	}

	//read all 18 bytes of the header
	fread(header, sizeof(char), 18, file);

	//must be image type 2 (color))
	if (header[2] != 2) {
		fclose(file);
		fprintf(stderr, "[ERROR]: Wrong terrain format: %s\n", fileName);
		return NULL ;
	}

	if (header[0]) {
		fseek(file, header[0], SEEK_CUR);
	}

	// get the size and bitdepth from the header
	int terrW = header[13] * 256 + header[12];
	int terrH = header[15] * 256 + header[14];
	int bpp = header[16] / 8;

	if (bpp != 3 && bpp != 4) {
		fclose(file);
		fprintf(stderr, "[ERROR]: Unexpected terrain format: %s\n", fileName);
		return NULL ;
	}

	if (terrW < 2 || terrH < 2) {
		fclose(file);
		fprintf(stderr, "[ERROR]: Wrong terrain size: %s\n", fileName);
		return NULL ;
	}

	// allocate memory and read data
	int terrainSize = terrW * terrH;
	int imageSize = terrainSize * bpp;
	unsigned char *data = malloc(imageSize * sizeof(unsigned char));
	fread(data, sizeof(unsigned char), imageSize, file);
	fclose(file);

	// inizializzo terreno
	Terrain *terr = malloc(sizeof(*terr));
	*terr = _emptyTerrain;
	terr->creditPos.x = terr->creditPos.z = -1;
	terr->hdata = malloc(terrainSize * sizeof(unsigned char));
	terr->width = terrW - 1;
	terr->depth = terrH - 1;
	int slotSize = terr->width * terr->depth;
	terr->groundNormal = malloc(slotSize * sizeof(Vect3));
	terr->slotFlat = malloc(slotSize * sizeof(GLboolean));
	terr->groundHeight = malloc(slotSize * sizeof(GLfloat));

	// carico dati relativi al terreno
	for (int i = 0; i < imageSize; i++) {
		if (i % bpp == 0) {
			// componente B --> height data
			terr->hdata[i / bpp] = data[i];
		} else if (i % bpp == 2) {
			// componente R --> special purpose
			if(data[i] == SPEC_STARTPOS) {
				terr->startPos.x = (i / bpp) % (terr->width + 1);
				terr->startPos.z = (i / bpp) / (terr->depth + 1);
			} else if(data[i] == SPEC_CREDITPOS) {
				terr->creditPos.x = (i / bpp) % (terr->width + 1);
				terr->creditPos.z = (i / bpp) / (terr->depth + 1);
			}
		} else {
			// componenti G, A ignorati
			continue;
		}
	}

	free(data);
	return terr;
}

void destroyTerrain(Terrain *ter) {
	glDeleteLists(ter->dispList, 1);
	glDeleteTextures(1, &ter->texName);
	free(ter->hdata);
	free(ter->groundNormal);
	free(ter->slotFlat);
	free(ter->groundHeight);
	free(ter);
}

void compileTerrain(Terrain *t, GLfloat sSize, GLfloat maxH) {
	static const char terrPath[] = "textures/world/";
	static const char terrFile[] = "ground.tga";
	static const int farplane_dist = 1000;

	GLuint listName = glGenLists(1);
	GLfloat maxTHeight = 0, minTHeight = maxH;

	// carico texture terreno
	TexParam texparams = {GL_TRUE, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_REPEAT, GL_RGB};
	if(bindTexture(&t->texName, terrPath ,terrFile , &texparams)) {
		fprintf(stderr, "[ERROR]: Unable to load texture: %s\n", terrFile);
	}

	// compilo displaylist
	glNewList(listName, GL_COMPILE);
	{
		glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);
		glEnable(GL_TEXTURE_2D);

		const GLfloat terColorDiffuse[] = { 0.86, 0.82, 0.82 };
		const GLfloat terColorAmbient[] = { 0.4, 0.4, 0.4 };
		glMaterialfv(GL_FRONT, GL_DIFFUSE, terColorDiffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, terColorAmbient);
		glMaterialf(GL_FRONT, GL_SHININESS, 0);
		glEnable(GL_LIGHTING);
		// la luce 1 simula il riflesso del terreno, non serve tenerne conto
		glDisable(GL_LIGHT1);
		glBindTexture(GL_TEXTURE_2D, t->texName);

		// disegno un enorme piano orizzontale esteso oltre i limiti.
		// bruttino a vedersi, ma si ha l'idea - con poco sforzo -
		// che il mondo esista anche fuori dalla nostra "gabbia"
		glDepthMask(GL_FALSE);
		glBegin(GL_QUADS);
		{
			glNormal3f(0, 1, 0);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-farplane_dist, 0, -farplane_dist);
			glTexCoord2f(0.0f, 500.0f); glVertex3f(-farplane_dist, 0, t->slotSize * t->depth + farplane_dist * 2);
			glTexCoord2f(500.0f, 500.0f); glVertex3f(t->slotSize * t->width + farplane_dist * 2, 0, t->slotSize * t->depth + farplane_dist * 2);
			glTexCoord2f(500.0f, 0.0f); glVertex3f(t->slotSize * t->width + farplane_dist * 2, 0, -farplane_dist * 2);
		}
		glEnd();
		glDepthMask(GL_TRUE);

		// compilo slot del terreno
		glBegin(GL_TRIANGLES);
		Vect3 norm[2], p[4];
		/*
		 * [x0z0y0]----[x1z0y3]		width--->
		 * 	|               |		depth
		 * 	|               |			|
		 * [x0z1y1]----[x1z1y2]			v
		 */
		for (int d = 0; d < t->depth - 1; d++) {
			for (int w = 0; w < t->width - 1; w++) {
				// calcolo vertici
				p[0].x = w * sSize;
				p[0].z = d * sSize;
				p[0].y = (float)t->hdata[IDX(w, d, t->width + 1)] / 0xff * maxH;

				p[1].x = p[0].x;
				p[1].z = p[0].z + sSize;
				p[1].y = (float)t->hdata[IDX(w, d + 1, t->width + 1)] / 0xff * maxH;

				p[2].x = p[0].x + sSize;
				p[2].z = p[1].z;
				p[2].y = (float)t->hdata[IDX(w + 1, d + 1, t->width + 1)] / 0xff * maxH;

				p[3].x = p[2].x;
				p[3].z = p[0].z;
				p[3].y = (float)t->hdata[IDX(w + 1, d, t->width + 1)] / 0xff * maxH;

				// calcolo normali
				norm[0] = normalize(
						cprod(subtr(p[1], p[0]), subtr(p[3], p[0])));
				norm[1] = normalize(
						cprod(subtr(p[3], p[2]), subtr(p[1], p[2])));
				t->groundNormal[IDX(w, d, t->width)] = normalize(sum(norm[0], norm[1]));
				GLfloat slotHeight = (p[0].y + p[1].y + p[2].y + p[3].y) / 4;
				t->groundHeight[IDX(w, d, t->width)] = slotHeight;
				if(slotHeight > maxTHeight) maxTHeight = slotHeight;
				if(slotHeight < minTHeight) minTHeight = slotHeight;

				// determino se lo slot è sufficientemente "piatto" per consentire un atterraggio
				Vect3 up = { 0, 1, 0 };
				t->slotFlat[IDX(w, d, t->width)] = (1 - dprod(norm[0], norm[1]))
						< GROUND_FLAT_TOLERANCE
						&& (1 - dprod(up, t->groundNormal[IDX(w, d, t->width)]))
								< GROUND_STEEP_TOLERANCE;

				// disegno primo triangolo
				glNormal3f(norm[0].x, norm[0].y, norm[0].z);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(p[0].x, p[0].y, p[0].z);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(p[1].x, p[1].y, p[1].z);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(p[3].x, p[3].y, p[3].z);

				// disegno secondo triangolo
				glNormal3f(norm[1].x, norm[1].y, norm[1].z);
				glVertex3f(p[3].x, p[3].y, p[3].z);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(p[1].x, p[1].y, p[1].z);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(p[2].x, p[2].y, p[2].z);
			}
		}
		glEnd();
		glPopAttrib();
	}
	glEndList();
	t->dispList = listName;
	t->slotSize = sSize;
	t->maxHeight = maxTHeight;
}

int getSlotFromPos(GLfloat x, GLfloat z, Terrain *ter) {
	int w = x / ter->slotSize, d = z / ter->slotSize;
	return IDX(w, d, ter->width);
}
