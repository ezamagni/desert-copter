/*
 * texture.c
 *
 *  Created on: 30/giu/2012
 *      Author: "Enrico Zamagni"
 */

#include "texture.h"
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 256


// funzione per il caricamento di una immagine da file TGA
unsigned char *loadTextureFromTGA(const char *filename, int *w, int *h) {
	unsigned char header[20];
	FILE *file = fopen(filename, "rb");

	if (file == NULL) {
		fprintf(stderr, "[ERROR]: Could not open texture file: %s\n", filename);
		return NULL;
	}

	//read all 18 bytes of the header
	fread(header, sizeof(char), 18, file);

	//should be image type 2 (color) or type 10 (rle compressed color)
	if (header[2] != 2 && header[2] != 10) {
		fclose(file);
		fprintf(stderr, "[ERROR]: Unknown texture header: %s\n", filename);
		return NULL;
	}

	if (header[0]) {
		fseek(file, header[0], SEEK_CUR);
	}

	// get the size and bitdepth from the header
	int m_width = header[13] * 256 + header[12];
	int m_height = header[15] * 256 + header[14];
	int m_bpp = header[16] / 8;
	*w = m_width;
	*h = m_height;

	if (m_bpp != 3 && m_bpp != 4) {
		fclose(file);
		fprintf(stderr, "[ERROR]: Error reading texture: %s\n", filename);
		return NULL;
	}

	int imageSize = m_width * m_height * m_bpp;

	//allocate memory for image data
	unsigned char *data = malloc(imageSize * sizeof(unsigned char));

	//read the uncompressed image data if type 2
	if (header[2] == 2) {
		fread(data, sizeof(unsigned char), imageSize, file);
	}

	long ctpixel = 0, ctloop = 0;

	//read the compressed image data if type 10
	if (header[2] == 10) {
		// stores the rle header and the temp color data
		unsigned char rle;
		unsigned char color[4];

		while (ctpixel < imageSize) {
			// reads the the RLE header
			fread(&rle, 1, 1, file);

			// if the rle header is below 128 it means that what folows is just raw data with rle+1 pixels
			if (rle < 128) {
				fread(&data[ctpixel], m_bpp, rle + 1, file);
				ctpixel += m_bpp * (rle + 1);
			}

			// if the rle header is equal or above 128 it means that we have a string of rle-127 pixels
			// that use the folowing pixels color
			else {
				// read what color we should use
				fread(&color[0], 1, m_bpp, file);

				// insert the color stored in tmp into the folowing rle-127 pixels
				ctloop = 0;
				while (ctloop < (rle - 127)) {
					data[ctpixel] = color[0];
					data[ctpixel + 1] = color[1];
					data[ctpixel + 2] = color[2];
					if (m_bpp == 4) {
						data[ctpixel + 3] = color[3];
					}

					ctpixel += m_bpp;
					ctloop++;
				}
			}
		}
	}

	ctpixel = 0;

	//Because TGA file store their colors in BGRA format we need to swap the red and blue color components
	unsigned char temp;
	while (ctpixel < imageSize) {
		temp = data[ctpixel];
		data[ctpixel] = data[ctpixel + 2];
		data[ctpixel + 2] = temp;
		ctpixel += m_bpp;
	}

	//close file
	fclose(file);
	return data;
}

int bindTexture(GLuint *texName, const char *path, const char *fileName, const TexParam *texinfo) {
	unsigned char *textureData;
	int textureW, textureH;
	char fullPath[BUFSIZE];

	// carico file di texture
	strcpy(fullPath, path);
	strcat(fullPath, fileName);
	textureData = loadTextureFromTGA(fullPath, &textureW, &textureH);
	if (textureData == NULL)
		return 1;

	// genero texture OpenGL
	glGenTextures(1, texName);
	glBindTexture(GL_TEXTURE_2D, *texName);

	if(texinfo == NULL) {
		// utilizzo parametri texture di default
		TexParam defaultTexInfo;
		defaultTexInfo.buildMipMap = GL_FALSE;
		defaultTexInfo.wrap = GL_REPEAT;
		defaultTexInfo.texMagFilter = defaultTexInfo.texMinFilter = GL_LINEAR;
		defaultTexInfo.format = GL_RGB;
		texinfo = &defaultTexInfo;
	}

	if (texinfo->buildMipMap) {
		// costruisco mipmap
		gluBuild2DMipmaps(GL_TEXTURE_2D, texinfo->format, textureW, textureH,
				texinfo->format, GL_UNSIGNED_BYTE, textureData);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, texinfo->format, textureW, textureH, 0,
				texinfo->format, GL_UNSIGNED_BYTE, textureData);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texinfo->wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texinfo->wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texinfo->texMagFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texinfo->texMinFilter);

	free(textureData);
	return 0;
}
