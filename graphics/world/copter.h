/*
 * copter.h
 *
 *  Created on: 28/mag/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef COPTER_H_
#define COPTER_H_

#include "../../input.h"
#include "../model.h"
#include "terrain.h"

#define max_forwardV 0.82f
#define ENVMAP_SIZE 256

enum CopterParts { COPT_ROTOR = 0, COPT_BODY, COPT_GLASS };

// valori angoli
struct Copt_a {
	GLfloat rotor;
	GLfloat prevrotor;
	GLfloat yaw;
	GLfloat pitch;
	GLfloat roll;
};

// valori velocità
struct Copt_v {
	// velocità
	GLfloat vert;
	GLfloat forward;
	GLfloat lateral;

	// velocità angolari
	GLfloat yaw;
};

// struttura elicottero
typedef struct {
	Vect3 pos;			// posizione XYZ
	float engpow;		// potenza motore verticale [0-1]
	GLboolean isLanded;
	float bumpRemaining;	// quantità d'urto residua
	Vect3 bumpDirection;	// direzione spinta dell'urto

	// risorse grafiche
	Model *model[3];		// modelli
	GLuint dispList[3];		// display lists
	GLuint tex_env;			// texture per envmapping

	struct Copt_a angle;	// angoli
	struct Copt_v speed;	// velocità
} Copter;


// informazioni di stato sull'elicottero
// utili per la logica di gioco
// (info relative a un singolo step di simulazione)
typedef struct {
	int moved;			// si è mosso?
	float consumption;	// quanto ha consumato?
	float damage;		// quanti danni ha subìto?
} CopterStatus;


// inizializza/distrugge risorse per elicottero
Copter* createCopter();
void destroyCopter(Copter *c);

// renderizza elicottero
void renderCopter(Copter *c);
// renderizza pale elicottero (per effetto motion blur)
void renderPrevRotor(Copter *c);

// aggiorna la simulazione fisica dell'elicottero in base al terreno e input correnti
CopterStatus updateCopter(Copter *c, InputState *input, Terrain *ter);

// restituisce versore di direzione dell'elicottero
Vect3 getCopterDirection(Copter *c);
// restituisce upvector dell'elicottero
Vect3 getCopterUpVector(Copter *c);
// costruisce il displacement relativo all'elicottero per il target della
// telecamera in base a distanza e angolo verticale
Vect3 getCopterBackDisplacement(Copter *c, float distance, float upShift);

#endif /* COPTER_H_ */

