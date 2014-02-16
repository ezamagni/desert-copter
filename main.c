/*
 * main.c
 *
 *  Created on: 02/lug/2012
 *      Author: "Enrico Zamagni"
 */

#include <stdlib.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include "gui/gui.h"
#include "input.h"

// numero di millisec simulati da un passo di fisica
const int PHYS_SAMPLING_STEP = 10;

enum GameState {GAME_STOPPED = 0, GAME_RUNNING, GAME_PAUSED};

SDL_Surface *surf;
Uint32 nstep = 0; // numero di passi di fisica simulati
enum GameState gameState = GAME_STOPPED;
InputState input;
short quit = 0;

// impostazioni globali
int scrW = 1024, scrH = 768, fov = 90;
const GLfloat farPlane = 1000;
Options options;

// prototipi per funzioni di scena
extern int initScene();
extern void destroyScene();
extern void updateScene(InputState *input);
extern void renderScene(int scrW, int scrH);
extern void drawSceneHUD(int scrW, int scrH);

// prototipi funzioni GUI
void menuExit();
void menuReturn();
void menuNewGame();
void menuOptions();


void renderFrame() {
	// pulisco framebuffer e depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glColor3f(1, 1, 1);

	// FASE 1: renderizzo scena
	if (gameState != GAME_STOPPED) {
		// applico proiezione
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, 		//field-of-view
				(float)scrW / scrH,	//aspect ratio,
				0.25, farPlane); 	// zNear, zFar

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// disegno la scena
		renderScene(scrW, scrH);
	}

	// FASE 2: disegno interfaccia utente
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    // imposto proiezione ortogonale
	gluOrtho2D(0, scrW, scrH, 0);
	glMatrixMode(GL_MODELVIEW);
    // resetto matrice trasformazioni
	glLoadIdentity();
	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	if(gameState != GAME_RUNNING) {
		//menu
		drawMenu(scrW, scrH);
	} else {
        // hud
		drawSceneHUD(scrW, scrH);
	}
	glPopAttrib();

	// scambio backbuffer con frontbuffer
	SDL_GL_SwapBuffers();
}

int SDLSetup(SDL_Surface **surf) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(2);
	}

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

	if(options.antialiasing) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	} else {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	}

	if(options.mblur) {
		SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 16);
	}

	*surf = SDL_SetVideoMode(scrW, scrH, 0,
			SDL_OPENGL | SDL_RESIZABLE | SDL_HWSURFACE);// | SDL_FULLSCREEN);
	if (surf == NULL) {
		fprintf(stderr, "Can't set video mode: %s\n", SDL_GetError());
		SDL_Quit();
		return 2;
	}

	SDL_WM_SetCaption("Desert Copter", "Desert Copter");

	// inizializzo joystick
	if(SDL_NumJoysticks() > 0 ) {
		// rilevato joystick
		SDL_JoystickEventState(SDL_ENABLE);
		SDL_JoystickOpen(0);
	}

	return 0;
}

// libera tutte le risorse utilizzate dal gioco
void gameclose() {
	destroyGUI();
	if(gameState == GAME_RUNNING) {
		destroyScene();
	}

	SDL_FreeSurface(surf);
	SDL_Quit();
}

// inizializza le risorse per il gioco
void gameinit() {
	if (SDLSetup(&surf))
		exit(2);

	// re-inizializzazione GUI
	void (*calls[])(void) = {menuExit, menuReturn, menuNewGame, menuOptions};
	if (loadGUI(calls))
		exit(4);

	// re-inizializzazione OpenGL
	glViewport(0, 0, scrW, scrH);

	if(options.mblur)
		glClear(GL_ACCUM_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);
}

// chiude e reinizializza le risorse del gioco
void gamereinit() {
	// chiudo quello che è aperto..
	gameclose();

	// ..e reinizializzo
	gameinit();
}

int main(int argc, char* argv[]) {
	// inizializzazione
	gameinit();

	// main loop
	while (!quit) {
		SDL_Event e;
		// processo eventi SDL
		short newinput = 0;

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_VIDEORESIZE:
				if(gameState == GAME_STOPPED) {
					// il resize a partita in corso non è supportato,
					// si ridimensiona solo a partita terminata
					scrW = e.resize.w;
					scrH = e.resize.h;
					gamereinit();
				}
				break;
			}

			// aggiorno eventuale input
			newinput = getInput(&e, &input, scrW, scrH);
		}
		// nessun altro evento: procedo con simulazione
		if (newinput) {
			// c'è del nuovo input: lo processo
			if (gameState == GAME_RUNNING) {
				if (queryKey(&input, KM_CONTROL, ESC)) {
					// ESCAPE e gioco in corso: pausa
					gameState = GAME_PAUSED;
				}
			} else {
				updateMenu(&input);
			}
		}

		Uint32 timeNow = SDL_GetTicks();

		while (nstep * PHYS_SAMPLING_STEP < timeNow) {
			nstep++;
			if (gameState == GAME_RUNNING)
				updateScene(&input);

			if(!newinput) {
				// piccolo hack: se non arrivano eventi non ho modo di sapere che il
				// mouse si è fermato! Occorre far decadere col tempo le coordinate relative
				if(input.keys.value[TRIG_RIGHT_HORZ] == 0)
					input.cursor.xRel *= 0.87f;
				if(input.keys.value[TRIG_RIGHT_VERT] == 0)
					input.cursor.yRel *= 0.87f;
			}
		}

		// disegno nuovo frame
		renderFrame();

	} //main loop

	// chiudi tutto
	gameclose();
	return 0;
}

// partita finita: l'utente ha perso
// viene richiamata da game.c
void gameOver(int points) {
	// partita persa: chiudo tutto
	// e torno al menù iniziale
	switchMenu(MNUT_MAIN);
	gameState = GAME_STOPPED;
	printf("Hai realizzato %d punti!\n", points);
	destroyScene();
}

/*
 * GUI: funzioni di callback dal menu
 */
void menuExit() {
	if(gameState == GAME_PAUSED) {
		switchMenu(MNUT_MAIN);
		gameState = GAME_STOPPED;
		destroyScene();
	} else {
		quit = 1;
	}
}

void menuReturn() {
	// l'utente è tornato alla partita
	gameState = GAME_RUNNING;
}

void menuNewGame() {
	// inizializzazione scena
	gameState = GAME_RUNNING;
	switchMenu(MNUT_PAUSE);
	srandom(SDL_GetTicks());
	if (initScene())
		exit(3);
}

void menuOptions() {
	// l'utente ha cambiato impostazioni
	gamereinit();
}
