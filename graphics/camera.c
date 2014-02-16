/*
 * camera.c
 *
 *  Created on: 07/giu/2012
 *      Author: "Enrico Zamagni"
 */

#include "camera.h"
#include <GL/glu.h>

/* Caratteristiche telecamera */
const float target_pos_chasespeed = 0.049f;
const float target_eye_chasespeed = 0.097f;
const float target_up_chasespeed = 0.17f;
const float target_up_chasedispl = 0.075f;
const float target_camera_sens = 8.91f;
const float free_camera_speed = 0.62f;
const float free_camera_sens = 4.21f;

enum VectorFollowMode { FOLLOW_ORIENTATION = 0, FOLLOW_POSITION };

void setupCamera(Camera* c) {
	static const CameraTarget _emptyTarget;

	// imposta camera libera
	// che guarda circa verso asse Y positivo
	c->pos.x = c->pos.z = 5;
	// telecamera molto in alto per avere effetto "caduta"
	c->pos.y = 1000;
	c->eye.x = c->eye.z = 6;
	c->eye.y = 16;
	c->up.x = c->up.y = 0;
	c->up.y = 1;
	c->target = _emptyTarget;
	c->locks = CAMLOCK_FREE;
}

Vect3 follow(Vect3 v, Vect3 position, float amount, enum VectorFollowMode mode) {
	Vect3 direction = scale(subtr(position, v), amount);

	if(mode == FOLLOW_ORIENTATION) {
		// vogliamo che il vettore risultante segua la
		// direzione ma resti immutato in modulo
		float modOrig = mod(v);
		v = sum(v, direction);
		v = scale(normalize(v), modOrig);
	} else {
		// vogliamo che il vettore risultate insegua
		// posizione, verso e modulo della direzione
		v = sum(v, direction);
	}

	return v;
}

void moveCamera(Camera *c, KeyMap keymap) {
	// evito lunghi calcoli se non devo spostarmi lungo x o z
	if (keymap > CAM_DOWN) {
		// ricavo angoli con assi
		Vect3 eyedir = subtr(c->eye, c->pos);
		float phi; // phi: angolo con piano YZ
		if (eyedir.z == 0) {
			phi = M_PI_2;
		} else {
			phi = atanf(eyedir.x / eyedir.z);
			if (eyedir.z < 0)
				phi += M_PI;
		}

		float dx, dz;
		// avanti-indietro
		if (keymap & CAM_FORWARD) {
			dz = free_camera_speed * cos(phi);
			dx = free_camera_speed * sin(phi);
		}
		if (keymap & CAM_BACKWARD) {
			dz = -free_camera_speed * cos(phi);
			dx = -free_camera_speed * sin(phi);
		}

		// sinistra-destra
		if (keymap & CAM_LEFT) {
			dz = free_camera_speed * -sin(phi);
			dx = free_camera_speed * cos(phi);
		}
		if (keymap & CAM_RIGHT) {
			dz = -free_camera_speed * -sin(phi);
			dx = -free_camera_speed * cos(phi);
		}

		c->eye.x += dx;
		c->eye.z += dz;
		c->pos.x += dx;
		c->pos.z += dz;
	}

	// su-giu
	if (keymap & CAM_UP) {
		c->pos.y += free_camera_speed;
		c->eye.y += free_camera_speed;
	}
	if (keymap & CAM_DOWN) {
		c->pos.y -= free_camera_speed;
		c->eye.y -= free_camera_speed;
	}
}

void rotateCamera(Camera *c, float aH, float aV) {
	static const Vect3 Yaxis = { 0, 1, 0 };
	Vect3 eyedir = normalize(subtr(c->eye, c->pos));

	// ruoto orizzontalmente
	if(fabsf(dprod(Yaxis, eyedir)) < 0.95f) {
		// ruotare attorno ad asse Y se lo sguardo corrisponde
		// a tale asse farebbe impazzire la telecamera
		eyedir = rotateY(eyedir, aH);
	}
	// ruoto verticalmente
	Vect3 rightAxis = cprod(eyedir, Yaxis);
	eyedir = rotate(eyedir, rightAxis, aV);

	c->up = cprod(rightAxis, eyedir);
	c->eye = sum(c->pos, eyedir);
}

void rollCamera(Camera *c, float angle) {
	Vect3 eyedir = subtr(c->eye, c->pos);
	c->up = rotate(c->up, normalize(eyedir), angle);
}

void setCameraTarget(Camera *c, Vect3 target, Vect3 up, Vect3 displacement) {
	CameraTarget camtarget;
	camtarget.pos = target;
	camtarget.up = up;
	camtarget.displ = displacement;
	if(c->target.curDispl.x == 0 && c->target.curDispl.y == 0 && c->target.curDispl.z == 0) {
		camtarget.curDispl = displacement;
	} else {
		camtarget.curDispl = c->target.curDispl;
	}

	c->target = camtarget;
}

void updateCamera(Camera *c, InputState *input) {

	if (queryKey(input, KM_CAMERA, CAM_LOCK_TOGGLE)) {
		// abilito-disabilito locks su telecamera
		if (c->locks > CAMLOCK_FREE) {
			c->locks = CAMLOCK_FREE;
		} else {
			c->locks = CAMLOCK_ALL;
		}
	}

	if (c->locks & CAMLOCK_POSITION) {
		if (input->cursor.buttons & MBUTTON_LEFT) {
			// faccio ruotare il curDisplacement attorno al target
			c->target.curDispl = rotateY(c->target.curDispl,
					input->cursor.xRel * target_camera_sens);
			Vect3 rightAxis = normalize(cprod(c->target.curDispl, c->target.up));
			c->target.curDispl = rotate(c->target.curDispl, rightAxis,
					input->cursor.yRel * target_camera_sens);
		} else {
			// riporto il curDisplacement alla posizione corretta
			c->target.curDispl = follow(c->target.curDispl, c->target.displ,
					target_up_chasedispl, FOLLOW_POSITION);
		}
		// accorcia distanza con target
		Vect3 desiredPos = sum(c->target.pos, c->target.curDispl);
		c->pos = follow(c->pos, desiredPos, target_pos_chasespeed, FOLLOW_POSITION);
		// si dovrebbe muovere c->eye solidalmente a c->pos, ma al 99% dei casi non serve
	} else {
		moveCamera(c, getKeymap(input, KM_CAMERA));
	}

	if (c->locks & CAMLOCK_EYE) {
		c->eye = follow(c->eye, c->target.pos, target_eye_chasespeed, FOLLOW_POSITION);
	} else if (input->cursor.buttons & MBUTTON_LEFT) {
		rotateCamera(c, -input->cursor.xRel * free_camera_sens,
				-input->cursor.yRel * free_camera_sens);
	}

	if (c->locks & CAMLOCK_ROLL) {
		c->up = follow(c->up, c->target.up, target_up_chasespeed, FOLLOW_ORIENTATION);
	} else if (input->cursor.buttons & MBUTTON_MIDDLE) {
		rollCamera(c, input->cursor.xRel);
	}
}
