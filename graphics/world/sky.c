/*
 * sky.c
 *
 *  Created on: 24/giu/2012
 *      Author: "Enrico Zamagni"
 */

#include "sky.h"
#include "../texture.h"
#include "../../options.h"

enum {
	SKY_LEFT = 0, SKY_BACK, SKY_RIGHT, SKY_FRONT, SKY_TOP
};

// texture di cielo per le facce della skybox
GLuint sky_tex[5];
// colore nebbia
GLfloat sky_fogColor[3] = {0, 0, 0};

void initSkybox(const GLfloat fogCol[3]) {
	static const char skyPath[] = "textures/skybox/";
	TexParam texparams = { GL_FALSE, GL_NEAREST, GL_LINEAR, GL_CLAMP, GL_RGB };

	bindTexture(&sky_tex[0], skyPath, "left.tga", &texparams);
	bindTexture(&sky_tex[1], skyPath, "back.tga", &texparams);
	bindTexture(&sky_tex[2], skyPath, "right.tga", &texparams);
	bindTexture(&sky_tex[3], skyPath, "front.tga", &texparams);
	bindTexture(&sky_tex[4], skyPath, "top.tga", &texparams);

	if(options.fog) {
		sky_fogColor[0] = fogCol[0];
		sky_fogColor[1] = fogCol[1];
		sky_fogColor[2] = fogCol[2];
	}
}

void destroySkybox() {
	glDeleteTextures(5, sky_tex);
}

void renderSkybox(GLfloat viewHeight) {
	static const GLfloat fogHeight = 0.25f;

	glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBindTexture(GL_TEXTURE_2D, sky_tex[SKY_BACK]);
	glBegin(GL_QUADS);
	{
		// faccia posteriore (Z-)
		glTexCoord2f(0, 0);	glVertex3f( 1,  1, 1);
		glTexCoord2f(0, 1);	glVertex3f( 1, -1, 1);
		glTexCoord2f(1, 1);	glVertex3f(-1, -1, 1);
		glTexCoord2f(1, 0);	glVertex3f(-1,  1, 1);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, sky_tex[SKY_LEFT]);
	glBegin(GL_QUADS);
	{
		// faccia sinistra (X+)
		glTexCoord2f(0, 0);	glVertex3f(-1,  1,  1);
		glTexCoord2f(0, 1);	glVertex3f(-1, -1,  1);
		glTexCoord2f(1, 1);	glVertex3f(-1, -1, -1);
		glTexCoord2f(1, 0);	glVertex3f(-1,  1, -1);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, sky_tex[SKY_FRONT]);
	glBegin(GL_QUADS);
	{
		// faccia frontale (Z+)
		glTexCoord2f(1, 0);	glVertex3f( 1,  1, -1);
		glTexCoord2f(0, 0);	glVertex3f(-1,  1, -1);
		glTexCoord2f(0, 1);	glVertex3f(-1, -1, -1);
		glTexCoord2f(1, 1);	glVertex3f( 1, -1, -1);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, sky_tex[SKY_RIGHT]);
	glBegin(GL_QUADS);
	{
		// faccia destra (X-)
		glTexCoord2f(0, 0);	glVertex3f(1,  1, -1);
		glTexCoord2f(0, 1);	glVertex3f(1, -1, -1);
		glTexCoord2f(1, 1);	glVertex3f(1, -1,  1);
		glTexCoord2f(1, 0);	glVertex3f(1,  1,  1);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, sky_tex[SKY_TOP]);
	glBegin(GL_QUADS);
	{
		// faccia superiore (Y+)
		glTexCoord2f(1, 0);	glVertex3f( 1, 1,  1);
		glTexCoord2f(0, 0);	glVertex3f(-1, 1,  1);
		glTexCoord2f(0, 1);	glVertex3f(-1, 1, -1);
		glTexCoord2f(1, 1);	glVertex3f( 1, 1, -1);
	}
	glEnd();

	// se la nebbia è abilitata, applico effetto gradiente
	if(options.fog) {
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLfloat yrel = -1.0f / (720 / viewHeight);
		glBegin(GL_QUADS);
		{
			//faccia verso Z+
			glColor4f(sky_fogColor[0], sky_fogColor[1], sky_fogColor[2], 0);
			glVertex3f(-1, fogHeight, 1); glVertex3f( 1, fogHeight, 1);
			glColor4f(sky_fogColor[0], sky_fogColor[1], sky_fogColor[2], 1);
			glVertex3f( 1, yrel, 1); glVertex3f(-1, yrel, 1);

			//faccia verso -Z
			glVertex3f( -1, yrel, -1); glVertex3f(1, yrel, -1);
			glColor4f(sky_fogColor[0], sky_fogColor[1], sky_fogColor[2], 0);
			glVertex3f( 1, fogHeight, -1); glVertex3f(-1, fogHeight, -1);

			//faccia verso X+
			glVertex3f( 1, fogHeight, 1); glVertex3f(1, fogHeight, -1);
			glColor4f(sky_fogColor[0], sky_fogColor[1], sky_fogColor[2], 1);
			glVertex3f( 1, yrel, -1); glVertex3f(1, yrel, 1);

			//faccia verso -X
			glVertex3f( -1, yrel, 1); glVertex3f(-1, yrel, -1);
			glColor4f(sky_fogColor[0], sky_fogColor[1], sky_fogColor[2], 0);
			glVertex3f( -1, fogHeight, -1); glVertex3f(-1, fogHeight, 1);
		}
		glEnd();
	}

	glPopAttrib();
}

