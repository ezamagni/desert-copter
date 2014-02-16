/*
 * camera.h
 *
 *  Created on: 07/giu/2012
 *      Author: Enrico Zamagni
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include "../mathext.h"
#include "../input.h"

#define CAMLOCK_FREE 0x00
#define CAMLOCK_ALL 0xff
#define CAMLOCK_EYE 0x02
#define CAMLOCK_ROLL 0x04
#define CAMLOCK_POSITION 0x01

// info per inseguimento target
typedef struct {
	Vect3 pos;
	Vect3 up;
	Vect3 displ;
	Vect3 curDispl;
} CameraTarget;

// definizione di telecamera
typedef struct {
	Vect3 pos;
	Vect3 eye;
	Vect3 up;
	CameraTarget target;
	unsigned char locks;
} Camera;


// inizializza telecamera
void setupCamera(Camera *c);

// sposta la camera (in coordinate mondo)
// in base all'input corrente
void moveCamera(Camera *c, KeyMap keymap);
// orienta la telecamera rispetto ai due
// angoli orizzontale e verticale
void rotateCamera(Camera *c, float aH, float aV);
// inclina la telecamera
void rollCamera(Camera *c, float angle);
// aggiorna e gestisce la telecamera in base a input e modalità
void updateCamera(Camera *c, InputState *input);
// imposta un nuovo bersaglio per la telecamera. Il versaglio verrà
// inseguito in base ai lock impostati.
void setCameraTarget(Camera *c, Vect3 target, Vect3 up, Vect3 displacement);

#endif /* CAMERA_H_ */
