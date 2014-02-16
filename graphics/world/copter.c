/*
 * copter.c
 *
 *  Created on: 28/mag/2012
 *      Author: "Enrico Zamagni"
 */

#include "copter.h"
#include "../../options.h"
#include <math.h>
#include <stdlib.h>
#include <GL/gl.h>

/* Caratteristiche di comportamento elicottero */
// variabili controllo consumo benzina
const float fuel_consumpt_rate = 0.000079f;
const float fuel_consumpt_standby = 0.0000014f;
// variabili controllo decollo/atterraggio
const float eng_gain = 0.0026f;
const float eng_feedback = 0.9957f;
const float max_landquota = 0.85f;
const float landing_step = 0.018f;
const float max_landing_speed = 0.12f;
// variabili controllo salita/discesa
const float maxRotorAV = M_PI / 8.3f;
const float max_verticalV = 0.1938f;
const float vert_gain = 1.0f / 174.0f;
const float vert_feedback = 0.987f;
const float max_quota_surplus = 100;
// variabili controllo pitch
#define max_pitch M_PI_2 / 6
const float pitch_gain = max_pitch / 200;
const float pitch_feedback = 0.982f;
const float forward_gain = max_forwardV / 300;
const float forward_feedback = 0.985f;
// variabili controllo roll
#define max_roll M_PI_2 / 6
const float roll_gain = max_roll / 200;
const float roll_feedback = 0.982f;
#define max_lateralV 0.765f
const float lateral_gain = max_lateralV / 350;
const float lateral_feedback = 0.985f;
// variabili controllo yaw
const float yaw_gain = M_PI / 12000;
#define max_yawV M_PI / 220
const float yaw_feedback = 0.964f;
// variabili controllo urti
const float bump_feedback = 0.98f;
const float bump_strength = 0.92f;
// danni subiti rispetto a urto
const float damage_taken = 0.47f;


// confina un valore all'interno dell'intervallo specificato
GLfloat clamp(GLfloat val, GLfloat minval, GLfloat maxval) {
	if (val < minval)
		return minval;
	else if (val > maxval)
		return maxval;
	else
		return val;
}

// imposta la texture per l'environmental map
void createEnvTexture(Copter *c) {
	GLubyte *data;

	// alloco memoria per la texture
	// utilizzo calloc per inizializzare la porzione di memoria
	// con degli zeri (= texture completamente trasparente)
	data = calloc(sqr(ENVMAP_SIZE) * 3, sizeof(GLubyte));

	glGenTextures(1, &c->tex_env);
	glBindTexture(GL_TEXTURE_2D, c->tex_env);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ENVMAP_SIZE, ENVMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	free(data);
}

Copter* createCopter() {
	static const Copter _emptyCopt;
	const char modelPath[] = "models/copter/";
	const char *modelFileName[] = { "rotorv.obj", "body.obj", "glass.obj"};

	// inizializzo struttura
	Copter* c;
	c = malloc(sizeof(Copter));
	*c = _emptyCopt;
	c->isLanded = GL_TRUE;

	// carico modelli
	for(int i = 0; i < 3; i++) {
		c->model[i] = loadModelFromOBJ(modelPath, modelFileName[i], NULL);
		if (c->model[i] == NULL) {
			destroyCopter(c);
			return NULL;
		}
	}

	if(options.envmap) {
		// inizializzo la envmap texture
		createEnvTexture(c);
	}

	// compilo displaylists
	c->dispList[0] = glGenLists(3);
	c->dispList[1] = c->dispList[0] + 1;
	c->dispList[2] = c->dispList[1] + 1;
	for(int i = 0; i < 3; i++) {
		glNewList(c->dispList[i], GL_COMPILE);
		renderModel(c->model[i]);
		glEndList();
	}

	// imposto luce faro
	static const float coptLightAmbColor[4] = {0.4f, 0.42f, 0.32f, 1.0f};
	static const float coptLightDiffColor[4] = { 0.6781f, 0.6231f, 0.4529f, 1.0f };
	static const float coptLightSpecColor[4] = {0.9f, 1.0f, 0.92f, 1.0f};
	static const float coptLightAttenuation = 0.12f;
	static const int coptLightExponent = 0;
	static const int coptLightCutoff = 25;
	glLightfv(GL_LIGHT2, GL_AMBIENT, coptLightAmbColor);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, coptLightDiffColor);
	glLightfv(GL_LIGHT2, GL_SPECULAR, coptLightSpecColor);
	glLighti(GL_LIGHT2, GL_SPOT_EXPONENT, coptLightExponent);
	glLighti(GL_LIGHT2, GL_SPOT_CUTOFF, coptLightCutoff);
	glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, coptLightAttenuation);

	return c;
}

void destroyCopter(Copter *c) {
	for(int i = 0; i < 3; i++) {
		if(c->model[i] == NULL) continue;
		destroyModel(c->model[i]);
	}
	glDeleteLists(c->dispList[0], 3);
	glDeleteTextures(1, &c->tex_env);
	free(c);
}

// una morbida curva di salita tra 0 e 1
float f_rise(float x) {
	if (x >= 1)
		return 1;
	return sqrt(4 * x) - x;
}

CopterStatus updateCopter(Copter *c, InputState *input, Terrain *ter) {
	static int bouncing = 0;

	int sid = getSlotFromPos(c->pos.x, c->pos.z, ter);
	float gheight = ter->groundHeight[sid];

	float totSpeed = fabsf(c->speed.forward) + fabsf(c->speed.lateral) + fabsf(c->speed.vert) + fabsf(c->speed.yaw);
	if(totSpeed < 0.0001f) totSpeed = 0;
	CopterStatus status = {0, totSpeed * fuel_consumpt_rate, 0};

	KeyMap keymap = getKeymap(input, KM_COPTER);

	if (c->isLanded) {
		/*
		 * ELICOTTERO A TERRA
		 */
		if (keymap & VERT_UP) {
			// acceleratore premuto
			c->engpow += eng_gain * input->keys.value[TRIG_BACKLEFT];
		} else {
			c->engpow *= eng_feedback;
		}

		if (c->engpow >= 1) {
			// decollo!!
			c->engpow = 1;
			c->isLanded = GL_FALSE;
		}

		if (c->pos.y > gheight + 0.02f) {
			// fase finale atterraggio
			status.moved = 1;
			c->pos.y -= (c->pos.y - gheight) * landing_step;
		}
		c->angle.pitch *= pitch_feedback;
		c->angle.roll *= roll_feedback;

	} else {
		/*
		 * ELICOTTERO IN VOLO
		 */
		status.consumption += fuel_consumpt_standby;
		if(c->pos.y <= gheight + max_landquota) {
			// CONTROLLO COLLISIONE
			if (ter->slotFlat[sid] && !(keymap & VERT_UP) && totSpeed < max_landing_speed) {
				// atterraggio!!
				c->isLanded = GL_TRUE;
				c->speed.forward = 0;
				c->speed.vert = 0;
				c->speed.lateral = 0;
				c->bumpRemaining = 0;
				bouncing = 0;
			} else if (bouncing == 0 && totSpeed - c->speed.vert > 0.02f) {
				// urto!!
				bouncing = 1;
				c->bumpRemaining = 0.178f + totSpeed * bump_strength;
				c->bumpDirection = ter->groundNormal[sid];
				c->speed.forward = 0;
				c->speed.vert = 0;
				c->speed.lateral = 0;
				status.damage += totSpeed * damage_taken;
			} else {
				bouncing = 0;
			}
		}

		if(c->bumpRemaining > 0.1f) {
			// sto assorbendo un urto
			keymap = 0;
			Vect3 bounce = scale(c->bumpDirection, c->bumpRemaining * bump_strength);
			c->pos = sum(c->pos, bounce);
			c->bumpRemaining *= bump_feedback;
			status.moved = 1;
			totSpeed = mod(bounce);
		} else {
			c->bumpRemaining = 0;
		}

		// CONTROLLO SALITA/DISCESA
		if (keymap & (VERT_UP | VERT_DOWN)) {
			float value = (keymap & VERT_UP) ? input->keys.value[TRIG_BACKLEFT]
			                                                   : -input->keys.value[TRIG_BACKRIGHT];
			float delta_vspeed = max_verticalV * value - c->speed.vert;
			if( value < 0 ) {
				// si può perdere quota con maggiore velocità rispetto
				//a quanta se ne acquista durante una salita
				delta_vspeed = 2.3f * max_verticalV * value - c->speed.vert;;
			}
			delta_vspeed = clamp(delta_vspeed, -vert_gain, vert_gain);
			c->speed.vert += delta_vspeed;
		} else {
			c->speed.vert *= vert_feedback;
		}
		c->pos.y += c->speed.vert;

		// CONTROLLO PITCH (avanti/indietro)
		if (keymap & (FORWARD | BACKWARD)) {
			float delta_fspeed = max_forwardV * -input->keys.value[TRIG_LEFT_VERT] - c->speed.forward;
			float delta_pitch = max_pitch * -input->keys.value[TRIG_LEFT_VERT] - c->angle.pitch;
			delta_fspeed = clamp(delta_fspeed, -forward_gain, forward_gain);
			delta_pitch = clamp(delta_pitch, -pitch_gain, pitch_gain);
			c->speed.forward += delta_fspeed;
			c->angle.pitch += delta_pitch;
		} else {
			c->speed.forward *= forward_feedback;
			c->angle.pitch *= pitch_feedback;
		}

		// CONTROLLO ROLL
		if (keymap & (LEAN_RIGHT | LEAN_LEFT)) {
			int direction = (keymap & LEAN_LEFT) ? 1 : -1;
			float delta_lspeed = max_lateralV * direction - c->speed.lateral;
			float delta_roll = max_roll * -direction - c->angle.roll;
			delta_lspeed = clamp(delta_lspeed, -lateral_gain, lateral_gain);
			delta_roll = clamp(delta_roll, -roll_gain, roll_gain);
			c->speed.lateral += delta_lspeed;
			c->angle.roll += delta_roll;
		} else {
			c->speed.lateral *= lateral_feedback;
			c->angle.roll *= roll_feedback;
		}

		// CONTROLLO YAW
		if (keymap & (TURN_RIGHT | TURN_LEFT)) {
			float delta_yspeed = max_yawV * -input->keys.value[TRIG_LEFT_HORZ] - c->speed.yaw;
			delta_yspeed = clamp(delta_yspeed, -yaw_gain, yaw_gain);
			c->speed.yaw += delta_yspeed;
		} else {
			c->speed.yaw *= yaw_feedback;
		}
		c->angle.yaw += c->speed.yaw;

		// applico velocità
		c->pos.z += c->speed.forward * cos(c->angle.yaw)
				+ c->speed.lateral * cos(c->angle.yaw + M_PI_2);
		c->pos.x += c->speed.forward * sin(c->angle.yaw)
				+ c->speed.lateral * sin(c->angle.yaw + M_PI_2);

		if(keymap == 0) {
			// per evitare asintoti semi-infiniti devo azzerare le velocità
			// quando raggiungono una soglia critica
			const float min_threshold = 0.00005f;
			if(fabsf(c->speed.forward) < min_threshold) c->speed.forward = 0;
			if(fabsf(c->speed.lateral) < min_threshold) c->speed.lateral = 0;
			if(fabsf(c->speed.vert) < min_threshold) c->speed.vert = 0;
			if(fabsf(c->speed.yaw) < min_threshold) c->speed.yaw = 0;
		}

		if(c->speed.forward + c->speed.lateral + c->speed.vert + c->speed.yaw != 0) {
			status.moved = 1;
		}

		// controlli per evitare che l'elicottero esca dalla scena
		// (un po' brutali ma necessari)
		if(c->pos.x < 1) {
			c->pos.x = 1;
		} else if(c->pos.x > ter->slotSize * (ter->width - 1)) {
			c->pos.x = ter->slotSize * (ter->width - 1);
		}
		if (c->pos.z < 1) {
			c->pos.z = 1;
		} else if (c->pos.z > ter->slotSize * (ter->depth - 1)) {
			c->pos.z = ter->slotSize * (ter->depth - 1);
		}
		if(c->pos.y > ter->maxHeight + max_quota_surplus) {
			c->pos.y = ter->maxHeight + max_quota_surplus;
		}

	}

	// applico angoli
	float rotor_av = f_rise(c->engpow);
	c->angle.prevrotor = c->angle.rotor;
	c->angle.rotor += rotor_av * c->engpow * maxRotorAV;
	c->angle.rotor = remainderf(c->angle.rotor, M_2PI);
	c->angle.yaw = remainderf(c->angle.yaw, M_2PI);

	return status;
}

void renderCopter(Copter *c) {
	glEnable(GL_LIGHTING);

	static const float spotPos[4] = {0, 0, 5, 1};
	static const float spotDir[3] = {0, -0.7071, 0.7071};
	static const GLubyte glass_color[4] = {120, 200, 220, 235};
	static const GLubyte glass_color_env[4] = {255, 255, 255, 235};

	glPushMatrix();
	{
		glTranslatef(c->pos.x, c->pos.y, c->pos.z);
		glRotatefr(c->angle.yaw, 0, 1, 0);
		glRotatefr(c->angle.pitch, 1, 0, 0);
		glRotatefr(c->angle.roll, 0, 0, 1);

		// rendo corpo dell'elicottero
		// se voglio vedere l'interno dell'elicottero
		// devo temporaneamente disabilitare il face culling
		glDisable(GL_CULL_FACE);
		glCallList(c->dispList[COPT_BODY]);
		glEnable(GL_CULL_FACE);

		// rendo finestrini dell'elicottero
		glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4ubv(glass_color);

		if (options.envmap) {
			// effetto RIFLESSO
			glColor4ubv(glass_color_env);
			glBindTexture(GL_TEXTURE_2D, c->tex_env);
			glEnable(GL_TEXTURE_2D);
			// abilito generazione automatica coordinate texture
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		}

		// rendo vetro fusoliera
		glCallList(c->dispList[COPT_GLASS]);
		glPopAttrib();

		// posiziono luce faro
		glLightfv(GL_LIGHT2, GL_POSITION, spotPos);
		glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDir);
		glRotatefr(c->angle.rotor, 0, 1, 0);

		// rendo eliche
		glCallList(c->dispList[COPT_ROTOR]);
	}
	glPopMatrix();

	glDisable(GL_LIGHTING);
}

void renderPrevRotor(Copter *c) {
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_CURRENT_BIT);
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	glPushMatrix();
	{
		glTranslatef(c->pos.x, c->pos.y, c->pos.z);
		glRotatefr(c->angle.yaw, 0, 1, 0);
		glRotatefr(c->angle.pitch, 1, 0, 0);
		glRotatefr(c->angle.roll, 0, 0, 1);

		// spazio coordinate elicottero
		glRotatefr(c->angle.prevrotor, 0, 1, 0);
		//spazio coordinate rotore
		glColor4f(0.15f, 0.15f, 0.15f, 0.22f);
		glCallList(c->dispList[COPT_ROTOR]);
	}
	glPopMatrix();
	glDepthMask(GL_TRUE);
	glPopAttrib();
}

Vect3 getCopterDirection(Copter *c) {
	Vect3 Zaxis = {0, 0, 1};
	Vect3 result;

	result = rotateX(Zaxis, c->angle.pitch);
	result = rotateY(result, c->angle.yaw);

	return result;
}

Vect3 getCopterUpVector(Copter *c) {
	Vect3 Yaxis = {0, 1, 0};
	Vect3 result;

	result = rotateZ(Yaxis, c->angle.roll);
	result = rotateX(result, c->angle.pitch);
	result = rotateY(result, c->angle.yaw);

	return result;
}

Vect3 getCopterBackDisplacement(Copter *c, float distance, float upShift) {
	Vect3 back = inv(getCopterDirection(c));
	Vect3 up = getCopterUpVector(c);

	return scale(rotate(back, cprod(back, up), upShift), distance);
}
