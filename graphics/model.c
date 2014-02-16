/*
 * mesh.c
 *
 *  Created on: 24/mar/2012
 *      Author: enrico zamagni
 */

#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 256

// funzione per il caricamento di una libreria di materiali (.mtl)
MtlLib *loadMtlLib(const char *path, const char *fileName, TexParam *texparams) {
	static const MtlLib _emptyMtlLib;
	static const Material _emptyMaterial;
	char buff[BUFSIZE], fullPath[BUFSIZE];
	int wh = 0;
	Material *cur = NULL;

	// apro file di input
	strcpy(fullPath, path);
	strcat(fullPath, fileName);
	FILE *file = fopen(fullPath, "r");
	if (file == NULL ) {
		fprintf(stderr, "[ERROR]: could not open %s\n", fileName);
		return NULL ;
	}

	// istanzio libreria
	MtlLib *lib = malloc(sizeof(*lib));
	*lib = _emptyMtlLib;

	// carico materiali
	while (fgets(buff, BUFSIZE, file) != NULL ) {
		wh = 0;
		if (!strncmp(buff, "newmtl", 6)) {
			// trovato nuovo materiale
			lib->numMaterials++;
			lib->mtl = realloc(lib->mtl, lib->numMaterials * sizeof(Material));

			cur = &lib->mtl[lib->numMaterials - 1];
			*cur = _emptyMaterial;
			sscanf(buff, "newmtl %s", cur->name);

		} else {
			if (lib->numMaterials == 0)
				continue;

			// scarto whitespaces
			while (buff[wh] != '\0' && (buff[wh] == ' ' || buff[wh] == '\t'))
				wh++;

			if (!strncmp(&buff[wh], "Ka", 2)) {
				// ambient color
				sscanf(&buff[wh], "Ka %f %f %f", &cur->rgbAmbient[0],
						&cur->rgbAmbient[1], &cur->rgbAmbient[2]);

			} else if (!strncmp(&buff[wh], "Kd", 2)) {
				// diffuse color
				sscanf(&buff[wh], "Kd %f %f %f", &cur->rgbDiffuse[0],
						&cur->rgbDiffuse[1], &cur->rgbDiffuse[2]);

			} else if (!strncmp(&buff[wh], "Ks", 2)) {
				// specular color
				sscanf(&buff[wh], "Ks %f %f %f", &cur->rgbSpecular[0],
						&cur->rgbSpecular[1], &cur->rgbSpecular[2]);

			} else if (!strncmp(&buff[wh], "Ns", 2)) {
				// specular factor
				sscanf(&buff[wh], "Ns %f", &cur->specFactor);

			} else if (!strncmp(&buff[wh], "map_Kd", 6)) {
				// texture map
				char texFileName[BUFSIZE];
				sscanf(&buff[wh], "map_Kd %s", texFileName);
				if (bindTexture(&cur->textureName, path, texFileName,
						texparams)) {
					fprintf(stderr,
							"[ERROR]: Unable to load texture: %s for model: %s\n",
							texFileName, fileName);
				}
			}
		}
	}

	return lib;
}

// funzione per il caricamento di una mesh da file .obj
Model* loadModelFromOBJ(const char *path, const char *fileName, TexParam *texparams) {
	static const Model _emptyModel;
	int v = 0, vn = 0, vt = 0, f = 0;
	char buff[BUFSIZE], fullPath[BUFSIZE];
	int vbuf[FV], tcbuf[FV], nbuf[FV];

	FILE *file;
	Model *model;
	int m = 0;

	// apro file di input
	strcpy(fullPath, path);
	strcat(fullPath, fileName);
	file = fopen(fullPath, "r");
	if (file == NULL ) {
		fprintf(stderr, "[ERROR]: could not open %s!\n", fileName);
		return NULL ;
	}

	model = malloc(sizeof(Model));
	*model = _emptyModel;

	// PASSO 1: determino numero di vertici, facce e normali nella mesh
	while (fgets(buff, BUFSIZE, file) != NULL ) {
		switch (buff[0]) {
		case 'v':
			switch (buff[1]) {
			case 't':
				model->numTexCoords++;
				break;
			case 'n':
				model->numNormals++;
				break;
			case ' ':
				model->numVerts++;
				break;
			}
			break;
		case 'f':
			model->numFaces++;
			break;
		case 'm':
			if (!strncmp(buff, "mtllib", 6)) {
				if (model->mtlLib != NULL ) {
					fprintf(stderr,
							"[ERROR]: multiple mtllib reference in %s!\n",
							fileName);
					destroyModel(model);
					return NULL ;
				}

				// carico libreria di materiali
				char mtlFileName[BUFSIZE];
				sscanf(buff, "mtllib %s", mtlFileName);
				model->mtlLib = loadMtlLib(path, mtlFileName, texparams);
				if (model->mtlLib == NULL ) {
					fprintf(stderr,
							"[ERROR]: unable to load materials for %s!\n",
							fileName);
					destroyModel(model);
					return NULL ;
				}
			}
			break;
		case 'u':
			if (!strncmp(buff, "usemtl", 6)) {
				model->mtlLib->numFaceRefs++;
			}

			break;
		}
	}

	// allocazione memoria
	if (model->numVerts == 0 || model->numFaces == 0)
		return NULL ;
	model->vert = malloc(model->numVerts * sizeof(Vect3));
	model->face = malloc(model->numFaces * sizeof(MFace));
	model->norm = malloc(model->numNormals * sizeof(Vect3));
	model->texc = malloc(model->numTexCoords * sizeof(Vect2));
	if (model->mtlLib != NULL ) {
		model->mtlLib->faceIdx = malloc(
				model->mtlLib->numFaceRefs * sizeof(int));
		model->mtlLib->faceRef = malloc(
				model->mtlLib->numFaceRefs * sizeof(int));
	}
	rewind(file);

	// PASSO 2: leggo valori vertici/facce
	f = 0;
	while (fgets(buff, BUFSIZE, file) != NULL ) {
		switch (buff[0]) {
		case 'v':
			switch (buff[1]) {
			case 't':
				// COORDINATE TEXTURE
				sscanf(buff, "vt %f %f", &model->texc[vt].x,
						&model->texc[vt].y);
				vt++;
				break;
			case 'n':
				// NORMALE
				sscanf(buff, "vn %f %f %f", &model->norm[vn].x,
						&model->norm[vn].y, &model->norm[vn].z);
				vn++;
				break;
			case ' ':
				// VERTICE
				sscanf(buff, "v %f %f %f", &model->vert[v].x, &model->vert[v].y,
						&model->vert[v].z);
				v++;
				break;
			}
			break;

		case 'f':
			// lettura nel formato: f vertex/texcoord/normal
			if (sscanf(buff, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vbuf[0],
					&tcbuf[0], &nbuf[0], &vbuf[1], &tcbuf[1], &nbuf[1],
					&vbuf[2], &tcbuf[2], &nbuf[2]) < 9) {
				// lettura nel formato: f vertex//normal
				if (sscanf(buff, "f %d//%d %d//%d %d//%d", &vbuf[0], &nbuf[0],
						&vbuf[1], &nbuf[1], &vbuf[2], &nbuf[2]) < 6) {
					fprintf(stderr, "[ERROR]: unable to parse %s!\n", fileName);
					destroyModel(model);
					return NULL ;
				}
			}
			for (int i = 0; i < FV; i++) {
				//indici nel .obj partono da 1
				model->face[f].vert[i] = vbuf[i] - 1;
				model->face[f].texc[i] = tcbuf[i] - 1;
				model->face[f].norm[i] = nbuf[i] - 1;
			}
			f++;
			break;
		case 'u':
			// MATERIALE
			if (!strncmp(buff, "usemtl", 6)) {
				int found = 0;
				// associazione materiale
				for (int i = 0; i < model->mtlLib->numMaterials; i++) {
					if (!strncmp(model->mtlLib->mtl[i].name, &buff[7],
							strlen(model->mtlLib->mtl[i].name))) {
						found = 1;
						model->mtlLib->faceIdx[m] = f;
						model->mtlLib->faceRef[m] = i;
						m++;
						break;
					}
				}
				if (!found) {
					fprintf(stderr, "[ERROR]: unexpected material in %s!\n",
							fileName);
					destroyModel(model);
					return NULL ;
				}
			}
			break;
		}
	}

	return model;
}

void destroyModel(Model* model) {
	// scarico libreria materiali
	if (model->mtlLib != NULL ) {
		for(int m = 0; m < model->mtlLib->numMaterials; m++)
			glDeleteTextures(1, &model->mtlLib->mtl[m].textureName);
		free(model->mtlLib->mtl);
		free(model->mtlLib->faceIdx);
		free(model->mtlLib->faceRef);
		free(model->mtlLib);
	}

	// scarico topologia
	free(model->texc);
	free(model->norm);
	free(model->face);
	free(model->vert);

	// scarico struttura
	free(model);
}

void glRotatefr(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
	glRotatef(toGrad(angle), x, y, z);
}

void renderModel(const Model* m) {
	// controllo se la mesh ha dei materiali associati
	GLboolean hasMaterials = (m->mtlLib != NULL && m->mtlLib->numFaceRefs > 0);
	GLboolean isTextured = GL_FALSE;
	glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_LIGHTING_BIT);

	int mtlRef = 0;
	if (!hasMaterials)
		glEnable(GL_COLOR_MATERIAL);

	glBegin(GL_TRIANGLES);
	{
		for (int f = 0; f < m->numFaces; f++) {
			if (hasMaterials) {
				if (m->mtlLib->faceIdx[mtlRef] == f) {
					// switch tra materiali
					Material *mtl = &m->mtlLib->mtl[m->mtlLib->faceRef[mtlRef]];

					glMaterialfv(GL_FRONT, GL_AMBIENT, mtl->rgbAmbient);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mtl->rgbDiffuse);
					glMaterialfv(GL_FRONT, GL_SPECULAR, mtl->rgbSpecular);
					glMaterialf(GL_FRONT, GL_SHININESS, mtl->specFactor);

					// controllo se il materiale è mappato da una texture
					isTextured = mtl->textureName > 0;
					if (isTextured) {
						// materiale con texture
						glEnd();
						glEnable(GL_TEXTURE_2D);
						glBindTexture(GL_TEXTURE_2D, mtl->textureName);
						glBegin(GL_TRIANGLES);
					} else {
						// materiale senza texture
						if (glIsEnabled(GL_TEXTURE_2D)) {
							glEnd();
							glDisable(GL_TEXTURE_2D);
							glBegin(GL_TRIANGLES);
						}
					}

					if (mtlRef < m->mtlLib->numFaceRefs - 1)
						mtlRef++;
				}
			}

			for (int v = 0; v < FV; v++) {
				if (isTextured) {
					glTexCoord2f(m->texc[m->face[f].texc[v]].x,
							m->texc[m->face[f].texc[v]].y);
				}
				glNormal3f(m->norm[m->face[f].norm[v]].x,
						m->norm[m->face[f].norm[v]].y,
						m->norm[m->face[f].norm[v]].z);
				glVertex3f(m->vert[m->face[f].vert[v]].x,
						m->vert[m->face[f].vert[v]].y,
						m->vert[m->face[f].vert[v]].z);
			}
		}
	}
	glEnd();

	// ripristino stato precedente
	glPopAttrib();
}
