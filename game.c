/*
 * game.c
 *
 *  Created on: 02/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "options.h"
#include "input.h"
#include "gui/hud.h"
#include "graphics/camera.h"
#include "graphics/world/copter.h"
#include "graphics/world/terrain.h"
#include "graphics/world/sky.h"
#include "graphics/world/crate.h"
#include "graphics/world/baloon.h"
#include "graphics/world/billboard.h"

// elementi di scena
Camera camera;
Terrain *terrain = NULL;
Copter *copter = NULL;
Crate* crate = NULL;
Baloon* baloon = NULL;
Billboard* billboard = NULL;

// stato della partita
int points = 0;
float fuel = 1;
float damage = 0;
int gamelost = 0;

// gestisce il fading a fine partita
GLfloat fadeAmount = 1;

// parametri illuminazione
const GLfloat sun_pos[] = { 1.0f, 0.85f, -0.9f, 0.0f };
const GLfloat sun_diffcolor[4] = { 0.6781f, 0.6231f, 0.4529f, 1.0f };
const GLfloat sun_speccolor[4] = { 0.5f, 0.45f, 0.045f, 1.0f };
//const GLfloat backsun_pos[] = { -0.32f, -0.85f, 0.12, 0.0f };
const GLfloat backsun_pos[] = { 0.0f, -1.0f, 0.0f, 0.0f };
const GLfloat backsun_diffcolor[4] = { 0.302f * 0.85f, 0.274f * 0.85f, 0.2f * 0.85f, 1.0f };

// parametri nebbia
const GLfloat fogColor[4]= {0.6863f, 0.549f, 0.4353f, 1.0f};
const GLfloat fogStart = 115.0f;
const GLfloat fogEnd = 700.0f;

extern void gameOver(int points);

void renderEnvMap(int scrW, int scrH) {
	// imposto viewport pari a dimensione envmap
	glViewport(0, 0, ENVMAP_SIZE, ENVMAP_SIZE);

	// imposto proiezione per envmap
	glMatrixMode(GL_PROJECTION);
	// salvo la vista corrente
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(60, 1.5f, 0.2f, 500);

	glMatrixMode(GL_MODELVIEW);
	// imposto punto di vista (posizione e verso elicottero)
	// ma prima salvo le trasformazioni attuali
	glPushMatrix();
	glLoadIdentity();
	Vect3 coptDir = getCopterDirection(copter);
	Vect3 coptUp = getCopterUpVector(copter);
	gluLookAt(0, 0, 0, coptDir.x, coptDir.y, coptDir.z, -coptUp.x, -coptUp.y, -coptUp.z);

	// renderizzo il cielo
	renderSkybox(copter->pos.y);

	// salvo il framebuffer nella envmap
	glBindTexture(GL_TEXTURE_2D, copter->tex_env);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, ENVMAP_SIZE, ENVMAP_SIZE, 0);

	// ripristino viewport, proiezione e vista precedenti
	glViewport(0, 0, scrW, scrH);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// ripulisco il frame buffer e il depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderScene(int scrW, int scrH) {
	// se l'effetto RIFLESSO è attivo devo renderizzare la envmap
	// prima di procedere con il rendering vero e proprio
	if(options.envmap) {
		renderEnvMap(scrW, scrH);
	}

	// applico prima trasformazione di vista (orientamento)
	Vect3 lookDir = subtr(camera.eye, camera.pos);
	gluLookAt(0, 0, 0, lookDir.x, lookDir.y, lookDir.z, camera.up.x, camera.up.y, camera.up.z);
	// disegno skybox
	renderSkybox(camera.pos.y);
	// posiziono luci
	glLightfv(GL_LIGHT0, GL_POSITION, sun_pos);
	glLightfv(GL_LIGHT1, GL_POSITION, backsun_pos);
	// applico seconda trasformazione di vista (posizione)
	glTranslatef(-camera.pos.x, -camera.pos.y, -camera.pos.z);

	// accendo/spengo luce elicottero
	if(copter->isLanded)
		glDisable(GL_LIGHT2);
	else
		glEnable(GL_LIGHT2);

	// disegno terreno
	glCallList(terrain->dispList);

	//disegno eventuale cassa
	if(crate != NULL) {
		renderCrate(crate);
	}

	// disegno eventuale mongolfiera
	if(baloon != NULL) {
		renderBaloon(baloon);
	}

	// disegno tabellone
	if(billboard != NULL) {
		renderBillboard(billboard);
	}

	if(options.mblur) {
		if(!copter->isLanded) {
			// prevediamo anche un semplice effetto
			// di "sfocatura" per le eliche dell'elicottero
			renderPrevRotor(copter);
		}

		// effetto MOTION BLUR: intensità sfocatura tanto
		// maggiore quanto più andiamo veloci.
		static const float maxBlur = 0.54f;
		static const float thresholdSpeed = max_forwardV / 6;
		if (copter->speed.forward >= thresholdSpeed) {
			// blur : maxBlur = (speed  - threshold): (maxSpeed - threshold)
			float blurAmount = maxBlur
					* (copter->speed.forward - thresholdSpeed)
					/ (max_forwardV - thresholdSpeed);
			// (1) il contenuto dell'AB viene "ridotto"
			glAccum(GL_MULT, blurAmount);
			// (2) all'AB viene sommato il frame corrente
			glAccum(GL_ACCUM, 1 - blurAmount);
			// (3) l'AB viene scaricato nel color buffer
			glAccum(GL_RETURN, 1.0f);
		}
	}

	//disegno elicottero
	renderCopter(copter);
}

void drawSceneHUD(int scrW, int scrH) {
	static const int HUDspacing = 30;

	if (!gamelost) {
		int x = HUDspacing, y = HUDspacing;
		// indicatore punteggio
		drawPoints(points, x, y);
		y += 50;

		// indicatore benzina
		drawFuel(fuel, x, y);
		y += 35;

		// indicatore danni
		drawDamage(damage, x, y);

		// minimappa
		drawMMap(x, scrH - mmapHeight - HUDspacing);

	} else {
		// effetto OFFUSCAMENTO scena prima di
		// ritornare al menù principale
		glPushAttrib(GL_COLOR_BUFFER_BIT);
		glBlendFunc(GL_ONE, GL_SRC_COLOR);
		glColor3f(fadeAmount,fadeAmount * fadeAmount, fadeAmount * fadeAmount);
		glBegin(GL_QUADS);
		{
			glVertex2f(scrW, 0);
			glVertex2f(0, 0);
			glVertex2f(0, scrH);
			glVertex2f(scrW, scrH);
		}
		glEnd();
		glPopAttrib();
	}
}

int initScene() {
	static const char terFile[] = "terrain.tga";

	// setup illuminazione
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_diffcolor);
	glLightfv(GL_LIGHT0, GL_SPECULAR, sun_speccolor);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, backsun_diffcolor);

	if(options.fog) {
		// setup nebbia
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glHint(GL_FOG_HINT, GL_NICEST);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_START, fogStart);
		glFogf(GL_FOG_END, fogEnd);
		glEnable(GL_FOG);
	}

	// carico terreno di gioco
	terrain = loadTerrain("textures/", terFile);
	if (terrain == NULL)
		return 1;
	compileTerrain(terrain, 3.76f, 85);

	// inizializzo skybox
	initSkybox(fogColor);

	// inizializzo elicottero
	copter = createCopter();
	if (copter == NULL)
		return 1;
	Vect3 copterpos =
			{ terrain->startPos.x * terrain->slotSize,
					terrain->groundHeight[IDX(terrain->startPos.x, terrain->startPos.z, terrain->width)],
					terrain->startPos.z * terrain->slotSize };
	copter->pos = copterpos;

	// imposto telecamera
	setupCamera(&camera);
	setCameraTarget(&camera, copter->pos, getCopterUpVector(copter),
			getCopterBackDisplacement(copter, 30, M_PI_4 / 2));
	camera.locks = CAMLOCK_ALL;

	// carico tabellone
	billboard = createBillboard(terrain);

	// carico elementi dell'HUD
	if(loadHUD())
		return 1;
	loadMMap(terrain, copter);

	return 0;
}

int nextRndInt(int min, int max) {
	return random() % (max - min + 1) + min;
}

void handleCrate() {
	static const int mintime_between_crate = 200;
	static const int max_time_surplus_crate = 150;
	static const int safety_border = 4;
	static const int crate_duration = 6000;
	static const float min_crate_distance = 3.48f;

	static int time_to_next_crate = 0;
	int resetTimer = 0;

	if( gamelost ) {
		// stiamo chiudendo
		// resetta lo stato ed esci
		time_to_next_crate = 0;
		return;
	}

	if (crate == NULL) {
		time_to_next_crate--;
		if (time_to_next_crate <= 0) {
			// creiamo una nuova cassa
			TCoord cpos = { nextRndInt(safety_border, terrain->width - safety_border),
					nextRndInt(safety_border, terrain->depth - safety_border) };
			crate = createCrate(cpos, terrain, crate_duration);
			updateMMapCratePos(crate->pos.x, crate->pos.z, 1);
			resetTimer = 1;
		}
	} else {
		updateCrate(crate);
		if(crate->ttl <= 0) {
			// cassa scaduta
			destroyCrate(&crate);
			updateMMapCratePos(0, 0, 0);
			resetTimer = 1;
		} else if(mod(subtr(copter->pos, crate->pos)) <= min_crate_distance) {
			// collisione con cassa
			points++;
			destroyCrate(&crate);
			updateMMapCratePos(0, 0, 0);
			resetTimer = 1;
		}
	}

	if(resetTimer) {
		time_to_next_crate = mintime_between_crate + nextRndInt(0, max_time_surplus_crate);
	}
}

void handleBaloon() {
	static const GLfloat far_distance = 230.0f;
	static const int mintime_between_baloon = 350;
	static const int max_time_surplus_baloon = 1500;
	static const float min_baloon_distance = 12.7f;
	static const float earn_fuel = 0.18f;
	static const float earn_damage = 0.10f;

	static int time_to_next_baloon = 0;
	int resetTimer = 0;

	if(baloon == NULL) {
		time_to_next_baloon--;
		if(time_to_next_baloon <= 0) {
			// creiamo una nuova mongolfiera
			baloon = createBaloon(terrain);
			resetTimer = 1;
		}
	} else {
		updateBaloon(baloon, terrain);
		// controllo che la mongolfiera non sia uscito dalla mappa
		float mapWdist = terrain->width * terrain->slotSize + far_distance;
		float mapDdist = terrain->depth * terrain->slotSize + far_distance;
		if (baloon->pos.x < -far_distance || baloon->pos.x > mapWdist
				|| baloon->pos.z < -far_distance || baloon->pos.z > mapDdist) {
			// elimino questa mongolfiera
			destroyBaloon(&baloon);
			resetTimer = 1;
		} else if(mod(subtr(copter->pos, baloon->pos)) <= min_baloon_distance) {
			// collisione con mongolfiera
			// scelgo un premio casuale
			if(random() % 3) {
				fuel += earn_fuel;
				if(fuel > 1) fuel = 1;
			} else {
				damage -= earn_damage;
				if(damage < 0) damage = 0;
			}

			// distruggo mongolfiera
			destroyBaloon(&baloon);
			resetTimer = 1;
		}
	}

	if(resetTimer) {
		time_to_next_baloon = mintime_between_baloon + nextRndInt(0, max_time_surplus_baloon);
	}
}

void updateScene(InputState *input) {
	// gestico cassa
	handleCrate();

	// gestsco mongolfiera
	handleBaloon();

	// aggiorno elicottero
	if (!gamelost) {
		CopterStatus simulResult = updateCopter(copter, input, terrain);
		if (simulResult.moved) {
			// l'elicottero si è spostato
			// consumo benzina
			fuel -= simulResult.consumption;
			// prendo nota di eventuali danni
			damage += simulResult.damage;
			// aggiorno target telecamera
			setCameraTarget(&camera, copter->pos, getCopterUpVector(copter),
					getCopterBackDisplacement(copter,
							11 + copter->isLanded * 9.5f, M_PI_4 / 2));
		}
	}

	// aggiorno telecamera
	updateCamera(&camera, input);

	// aggiorno minimappa
	updateMMap();

	// verifico condizioni di perdita
	gamelost = (fuel <= 0 || damage >= 1);
	if (gamelost) {
		static const float fade_rate = 0.991f;
		// animazione finale prima di abbandonare la scena
		fadeAmount *= fade_rate;
		if(fadeAmount < 0.05f) {
			// è finita!
			gameOver(points);
		}
	}
}

void destroyScene() {
	// libero risorse
	destroyTerrain(terrain);
	destroySkybox();
	destroyCopter(copter);
	if(crate != NULL) {
		destroyCrate(&crate);
	}
	if(baloon != NULL) {
		destroyBaloon(&baloon);
	}
	if(billboard != NULL) {
		destroyBillboard(&billboard);
	}
	destroyHUD();
	destroyMMap();

	// resetto stato
	points = 0;
	fuel = 1;
	damage = 0;
	fadeAmount = 1;
	gamelost = 0;
}
