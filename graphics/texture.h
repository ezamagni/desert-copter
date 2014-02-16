/*
 * texture.h
 *
 *  Created on: 30/giu/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <GL/gl.h>

// parametri per gestire il binding di una texture
typedef struct {
	GLboolean buildMipMap;  // generazione automatica mipmap
	GLint texMinFilter;     // minification filter
	GLint texMagFilter;     // magnification filter
	GLint wrap;             // wrap mode
	GLenum format;          // formato texture
} TexParam;

// carica un file immagine .tga e lo riversa in un'area di memoria
// viene restituito il puntatore ai dati caricati e le dimensioni
// dell'immagine in w e h
unsigned char *loadTextureFromTGA(const char *filename, int *w, int *h);

// effettua il binding in OpenGl del file di immagine specificato
// viene generato automaticamente un indice opengl per la texture e restituito in texName
// vengono utilizzati i parametri texture forniti con texinfo
int bindTexture(GLuint *texName, const char *path, const char *fileName, const TexParam *texinfo);

#endif /* TEXTURE_H_ */
