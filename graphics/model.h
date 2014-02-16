/*
 * model.h
 *
 *  Created on: 24/mar/2012
 *      Author: enrico zamagni
 */

#ifndef MODEL_H_
#define MODEL_H_

#include "../mathext.h"
#include "texture.h"
#include <GL/gl.h>

#define FV 3	// 3 vertici per ogni faccia

// faccia di un modello mesh
typedef struct {
    int vert[FV];
    int texc[FV];
    int norm[FV];
} MFace;

// descrizione materiale
typedef struct {
	char name[80];			// nome del materiale
	GLfloat rgbAmbient[3];	// colore ambientale
	GLfloat rgbDiffuse[3];	// colore diffuso
	GLfloat rgbSpecular[3];	// colore speculare
	GLfloat specFactor;		// fattore speculare
	GLuint textureName;		// nome texture
} Material;

// libreria materiali
typedef struct {
	int numMaterials;
	Material *mtl;
	int numFaceRefs;
	int *faceIdx;
	int *faceRef;
} MtlLib;

// modello mesh
typedef struct {
	int numVerts;
	int numFaces;
    int numNormals;
    int numTexCoords;
	MFace *face;
	Vect3 *vert;
	Vect3 *norm;
	Vect2 *texc;

	// libreria di materiali
	MtlLib *mtlLib;
} Model;

// libreria di modelli
typedef struct {
	int numModels;
	int *id;
	Model **mdl;
} MdlLib;


// restituisce un modello caricato da file .obj (più eventuali materiali)
// se il modello prevede delle texture, queste vengono caricate con i parametri
// indicati in texparams
Model* loadModelFromOBJ(const char *path, const char *fileName,
		TexParam *texparams);

// libera le risorse ocupate da un modello, inclusi eventuali materiali
// e texture caricate
void destroyModel(Model* model);

// equivalente a glRotatef ma per angoli espressi in radianti
void glRotatefr(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

// renderizza il modello specificato
void renderModel(const Model* model);

#endif /* MODEL_H_ */
