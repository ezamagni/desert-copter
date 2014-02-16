/*
 * hud.c
 *
 *  Created on: 04/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include "hud.h"
#include "../graphics/texture.h"
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>

// elementi grafici
GLuint texNumbers;
GLuint texIcons;

static const int ASCII_NUMBASE = 48;
static const int iconSize = 28;
static const int barWidth = 150;
static const int barHeight = 14;
static const int barBorder = 2;
static const float barTransparency = 0.63f;

enum Icons {ICO_POINTS = 0, ICO_FUEL, ICO_DAMAGE};


int loadHUD() {
	static const char texPath[] = "textures/gui/";
	static const char *texName[] = {"numbers.tga", "icons.tga"};

	int err = 0;
	TexParam texparam = {GL_FALSE, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_RGB};

	err += bindTexture(&texNumbers, texPath, texName[0], &texparam);
	texparam.format = GL_RGBA;
	err += bindTexture(&texIcons, texPath, texName[1], &texparam);

	if(err > 0) {
		fprintf(stderr, "[ERROR]: Unable to load HUD assets\n");
		return 1;
	}

	return 0;
}

void destroyHUD() {
	glDeleteTextures(1, &texNumbers);
	glDeleteTextures(1, &texIcons);
}

void drawIcon(enum Icons icon) {
	glBindTexture(GL_TEXTURE_2D, texIcons);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0, (1.0f / 3) * icon);
		glVertex2f(0, 0);
		glTexCoord2f(0, (1.0f / 3) * (icon + 1));
		glVertex2f(0, iconSize);
		glTexCoord2f(1, (1.0f / 3) * (icon + 1));
		glVertex2f(iconSize, iconSize);
		glTexCoord2f(1, (1.0f / 3) * icon);
		glVertex2f(iconSize, 0);
	}
	glEnd();
}

void drawPoints(int points, int x, int y) {
	static const int digitWidth = 20;
	static const int digitHeight = 30;

	// ottengo stringa di cifre da rappresentare
	char numstr[32];
	sprintf(numstr, "%d", points);
	int strlen = strnlen(numstr, 32);

	int ascii;
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	{
		// disegno icona punti
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTranslatef(x, y, 0);
		drawIcon(ICO_POINTS);
		glTranslatef(iconSize * 1.5f, (iconSize - digitHeight) / 2, 0);

		// disegno cifre
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
		glBindTexture(GL_TEXTURE_2D, texNumbers);
		for(int i = 0; i < strlen; i++) {
			ascii = numstr[i] - ASCII_NUMBASE;
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.1f * ascii, 0);
				glVertex2f(0, 0);
				glTexCoord2f(0.1f * ascii, 1);
				glVertex2f(0, digitHeight);
				glTexCoord2f(0.1f * (ascii + 1), 1);
				glVertex2f(digitWidth, digitHeight);
				glTexCoord2f(0.1f * (ascii + 1), 0);
				glVertex2f(digitWidth, 0);
			}
			glEnd();
			glTranslatef(digitWidth, 0, 0);
		}
	}
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void drawProgressBar(float val) {
	// calcolo larghezza riempimento
	// barW : maxwidth = val : 1
	float barW = ((float)(barWidth * val));

	glPushMatrix();
	{
		//disegno bordo
		glPolygonMode(GL_FRONT, GL_LINE);
		glColor4f(0, 0, 0, 1);
		glPushMatrix();
		{
			glBegin(GL_QUADS);
			{
				glVertex2f(0, 0);
				glVertex2f(0, barHeight);
				glVertex2f(barWidth, barHeight);
				glVertex2f(barWidth, 0);
			}
			glEnd();
		}
		glPopMatrix();

		// riempio barra
		glPolygonMode(GL_FRONT, GL_FILL);
		glColor4f(1 - val, val, 0, barTransparency);
		glBegin(GL_QUADS);
		{
			glVertex2f(barBorder, barBorder);
			glVertex2f(barBorder, barHeight - barBorder);
			glVertex2f(barW - barBorder * 2 + 1, barHeight - barBorder);
			glVertex2f(barW - barBorder * 2 + 1, barBorder);
		}
		glEnd();
		glColor3f(1, 1, 1);
	}
	glPopMatrix();
}

void drawFuel(float fuel, int x, int y) {
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	{
		// disegno icona benzina
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTranslatef(x, y, 0);
		drawIcon(ICO_FUEL);
		glDisable(GL_TEXTURE_2D);

		// disegno barra
		glTranslatef(iconSize * 1.2f, (iconSize - barHeight) / 2, 0);
		drawProgressBar(fuel);
	}
	glPopMatrix();
}

void drawDamage(float damage, int x, int y) {
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	{
		// disegno icona danni
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTranslatef(x, y, 0);
		drawIcon(ICO_DAMAGE);
		glDisable(GL_TEXTURE_2D);

		// disegno barra
		glTranslatef(iconSize * 1.2f, (iconSize - barHeight) / 2, 0);
		drawProgressBar(1 - damage);
	}
	glPopMatrix();
}
