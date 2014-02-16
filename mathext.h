/*
 * mathext.h
 *
 *  Created on: 12/apr/2012
 *      Author: enrico zamagni
 */

#ifndef MATHEXT_H_
#define MATHEXT_H_

#include <math.h>

#define M_2PI 2 * M_PI
#define toRad(degrees) degrees * M_PI / 180.0f
#define toGrad(radiants) radiants * 180.0f / M_PI
#define sqr(x) x*x

// vettore bidimensionale
typedef struct {
	float x;
	float y;
} Vect2;

// vettore tridimensionale
typedef struct {
	float x;
	float y;
	float z;
} Vect3;

// FUNZIONI MATEMATICHE PER CALCOLO VETTORIALE //
float dprod(Vect3 a, Vect3 b);
Vect3 cprod(Vect3 a, Vect3 b);
float mod(Vect3 v);
Vect3 normalize(Vect3 v);
Vect3 scale(Vect3 v, float s);
Vect3 inv(Vect3 v);
Vect3 subtr(Vect3 a, Vect3 b);
Vect3 sum(Vect3 a, Vect3 b);
Vect3 reflect(Vect3 ray, Vect3 normal);

Vect3 rotate(Vect3 v, Vect3 axis, float tht);
Vect3 rotateX(Vect3 v, float tht);
Vect3 rotateY(Vect3 v, float tht);
Vect3 rotateZ(Vect3 v, float tht);


#endif /* MATHEXT_H_ */
