/*
 * gui.c
 *
 *  Created on: 03/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include "gui.h"
#include "../graphics/texture.h"
#include <GL/gl.h>
#include <stdio.h>

enum MnuMainButtons {
	MNUM_NEW = 0, MNUM_OPT, MNUM_QUIT
};

enum MnuPauseButtons {
	MNUP_RET = 0, MNUP_QUIT
};

enum MnuOptionButtons {
	MNUO_AA = 0, MNUO_FOG, MNUO_BLUR, MNUO_ENV, MNUO_ABORT, MNUO_ACCEPT
};

// elementi grafici
GLuint tex_backgr;
GLuint tex_buttons[3];

// stato del menu di gioco
int btnSelected = 0;
int mnuType = MNUT_MAIN;

// impostazioni precedenti
Options prevOptions;


// funzioni di callback
void (*menuCallback[4])(void);


int loadGUI(void (*menucallbacks[])(void)) {
	static const char texPath[] = "textures/gui/";
	static const char *texName[] = {"backgr.tga", "buttons1.tga", "buttons2.tga", "buttons3.tga"};

	menuCallback[0] = menucallbacks[0];
	menuCallback[1] = menucallbacks[1];
	menuCallback[2] = menucallbacks[2];
	menuCallback[3] = menucallbacks[3];

	int err = 0;
	TexParam texparam = {GL_FALSE, GL_NEAREST, GL_NEAREST, GL_CLAMP, GL_RGBA};

	err += bindTexture(&tex_backgr, texPath, texName[0], &texparam);
	err += bindTexture(&tex_buttons[0], texPath, texName[1], &texparam);
	err += bindTexture(&tex_buttons[1], texPath, texName[2], &texparam);
	err += bindTexture(&tex_buttons[2], texPath, texName[3], &texparam);

	if(err > 0) {
		fprintf(stderr, "[ERROR]: Unable to load menu assets\n");
		return 1;
	}

	return 0;
}

void destroyGUI() {
	glDeleteTextures(1, &tex_backgr);
	glDeleteTextures(3, tex_buttons);
}

void toggle(Opt *o) {
	*o = ~*o;
}

void updateMenu(InputState *input) {

	if(queryKey(input, KM_ARROWS, UP)) {
		btnSelected -= 1;
		if(btnSelected < 0) btnSelected = 0;

	} else if (queryKey(input, KM_ARROWS, DOWN)) {
		btnSelected += 1;
		if(mnuType == MNUT_MAIN && btnSelected > MNUM_QUIT) {
			btnSelected = MNUM_QUIT;
		} else if (mnuType == MNUT_PAUSE && btnSelected > MNUP_QUIT) {
			btnSelected = MNUP_QUIT;
		} else if (mnuType == MNUT_OPTIONS && btnSelected > MNUO_ACCEPT) {
			btnSelected = MNUO_ACCEPT;
		}

	} else if(queryKey(input, KM_CONTROL, ENTER)) {
		// ENTER
		switch(mnuType) {
		case MNUT_MAIN:
			switch(btnSelected) {
			case MNUM_NEW:
				menuCallback[CALL_NEWGAME]();
				break;
			case MNUM_OPT:
				switchMenu(MNUT_OPTIONS);
				break;
			case MNUM_QUIT:
				menuCallback[CALL_EXIT]();
				break;
			}
			break;
		case MNUT_PAUSE:
			switch (btnSelected) {
			case MNUP_RET:
				menuCallback[CALL_RETURN]();
				break;
			case MNUP_QUIT:
				menuCallback[CALL_EXIT]();
				break;
			}
			break;
		case MNUT_OPTIONS:
			switch(btnSelected) {
			case MNUO_AA:
				toggle(&options.antialiasing);
				break;
			case MNUO_FOG:
				toggle(&options.fog);
				break;
			case MNUO_BLUR:
				toggle(&options.mblur);
				break;
			case MNUO_ENV:
				toggle(&options.envmap);
				break;
			case MNUO_ABORT:
				options = prevOptions;
				switchMenu(MNUT_MAIN);
				break;
			case MNUO_ACCEPT:
				switchMenu(MNUT_MAIN);
				menuCallback[CALL_OPTIONS]();
				break;
			}
			break;
		}

	} else if (queryKey(input, KM_CONTROL, ESC)) {
		// ESC
		switch(mnuType) {
		case MNUT_MAIN:
			// nessuna azione
			break;
		case MNUT_PAUSE:
			btnSelected = 0;
			menuCallback[CALL_RETURN]();
			break;
		case MNUT_OPTIONS:
			options = prevOptions;
			switchMenu(MNUT_MAIN);
			break;
		}
	}
}

void switchMenu(int menuType) {
	mnuType = menuType;
	btnSelected = 0;
	if(menuType == MNUT_OPTIONS)
		prevOptions = options;
}

Opt getOptions(int index) {
	switch(index) {
	case MNUO_AA:
		return options.antialiasing;
	case MNUO_FOG:
		return options.fog;
	case MNUO_ENV:
		return options.envmap;
	default:
		return options.mblur;
	}
}

void drawMenuOptions(int scrW, int scrH) {
	static const int btnWidth = 100;
	static const int btnHeight = 30;
	static const int btnSpacing = 10;
	static const int optSpacing = 24;
	static const int optSize = 20;

	int bmax = MNUO_ACCEPT + 1;
	float x = scrW / 2 - (btnWidth / 2 + optSpacing / 2 + optSize / 2);
	float y = scrH / 2 - (bmax * (btnHeight + btnSpacing) * 0.5f);

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, tex_buttons[mnuType]);
	glPushMatrix();
	{
		glTranslatef(x, y, 0);
		for(int i = 0; i < bmax; i++) {
			if (btnSelected == i)
				glColor3f(1, 1, 0);
			else
				glColor3f(0, 1, 0);

			// disegno bottone
			glBegin(GL_QUADS);
			{
				glTexCoord2f(0, (1.0f / bmax) * i);
				glVertex2f(0, 0);
				glTexCoord2f(0, (1.0f / bmax) * (i + 1));
				glVertex2f(0, btnHeight);
				glTexCoord2f(1, (1.0f / bmax) * (i + 1));
				glVertex2f(btnWidth, btnHeight);
				glTexCoord2f(1, (1.0f / bmax) * i);
				glVertex2f(btnWidth, 0);
			}
			glEnd();

			if(i < MNUO_ABORT) {
				// disegno checkbox
				glPushMatrix();
				{
					glTranslatef(btnWidth + optSpacing, 5, 0);
					glDisable(GL_TEXTURE_2D);
					if(!getOptions(i)) {
						// l'opzione non è impostata
						// disegno rettangolo vuoto
						glPolygonMode(GL_FRONT, GL_LINE);
					}
					glColor3f(0, 1, 0);
					glBegin(GL_QUADS);
					{
						glVertex2f(0, 0);
						glVertex2f(0, optSize);
						glVertex2f(optSize, optSize);
						glVertex2f(optSize, 0);
					}
					glEnd();
					glPolygonMode(GL_FRONT, GL_FILL);
					glEnable(GL_TEXTURE_2D);
				}
				glPopMatrix();
			}
			glTranslatef(0, btnSpacing + btnHeight, 0);
		}
	}
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}

void drawMenu(int scrW, int scrH) {
	static const int btnWidth = 180;
	static const int btnHeight = 50;
	static const int btnSpacing = 20;
	static const int border = 60;

	if(mnuType == MNUT_OPTIONS) {
		// per non complicare troppo questa funzione
		// realizziamo il disegno del menù opzioni in
		// una procedura dedicata.
		drawMenuOptions(scrW, scrH);
		return;
	}

	int backgrW = btnWidth + border * 2;
	int backgrH[2] = {border * 2 + btnHeight * 3 + btnSpacing * 2,
			border * 2 + btnHeight * 2 + btnSpacing};

	float x = scrW / 2 - backgrW / 2;
	float y = scrH / 2 - backgrH[mnuType] / 2;

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix();
	{
		// disegno sfondo
		glBindTexture(GL_TEXTURE_2D, tex_backgr);
		glTranslatef(x, y, 0);
		glBegin(GL_QUADS);
		{
			glTexCoord2f(0, 0);
			glVertex2f(0, 0);
			glTexCoord2f(0, 1);
			glVertex2f(0, backgrH[mnuType]);
			glTexCoord2f(1, 1);
			glVertex2f(backgrW, backgrH[mnuType]);
			glTexCoord2f(1, 0);
			glVertex2f(backgrW, 0);
		}
		glEnd();

		// disegno bottoni
		int bmax;
		if (mnuType == MNUT_MAIN)
			bmax = 3;
		else
			bmax = 2;
		glBindTexture(GL_TEXTURE_2D, tex_buttons[mnuType]);
		glTranslatef(border, border, 0);
		for (int i = 0; i < bmax; i++) {
			if (btnSelected == i)
				glColor3f(1, 1, 0.57f);
			else
				glColor3f(1, 1, 1);

			glBegin(GL_QUADS);
			{
				glTexCoord2f(0, (1.0f / bmax) * i);
				glVertex2f(0, 0);
				glTexCoord2f(0, (1.0f / bmax) * (i + 1));
				glVertex2f(0, btnHeight);
				glTexCoord2f(1, (1.0f / bmax) * (i + 1));
				glVertex2f(btnWidth, btnHeight);
				glTexCoord2f(1, (1.0f / bmax) * i);
				glVertex2f(btnWidth, 0);
			}
			glEnd();
			glTranslatef(0, btnSpacing + btnHeight, 0);
		}
	}
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}
