/*
 * mathext.c
 *
 *  Created on: 12/apr/2012
 *      Author: enrico zamagni
 */

#include "mathext.h"

// prodotto scalare
float dprod(Vect3 a, Vect3 b) {
	return a.x * b.x + a.y * b.y;
}

// prodotto vettoriale
Vect3 cprod(Vect3 a, Vect3 b) {
	Vect3 result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

// inverso
Vect3 inv(Vect3 v) {
	Vect3 result;
	result.x = -v.x;
	result.y = -v.y;
	result.z = -v.z;
	return result;
}

// modulo
float mod(Vect3 v) {
	return sqrt(sqr(v.x) + sqr(v.y) + sqr(v.z));
}

// normalizzazione
Vect3 normalize(Vect3 v) {
	Vect3 result;
	float vmod = mod(v);
	if(vmod == 0) {
        // salviamoci da una divisione per zero:
        // se qualche furbone tenta di normalizzare un vettore "nullo"
        // restituiamo per convenzione il versore y
        result.x = result.z = 0;
        result.y = 1;
        return result;
    } 
	result.x = v.x / vmod;
	result.y = v.y / vmod;
	result.z = v.z / vmod;
	return result;
}

// differenza (a - b)
Vect3 subtr(Vect3 a, Vect3 b) {
	Vect3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}

// addizione
Vect3 sum(Vect3 a, Vect3 b) {
	Vect3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

// moltiplicazione per scalare
Vect3 scale(Vect3 v, float s) {
	Vect3 result;
	result.x = v.x * s;
	result.y = v.y * s;
	result.z = v.z * s;
	return result;
}

// vettore riflesso alla normale
Vect3 reflect(Vect3 ray, Vect3 normal) {
	Vect3 result;
	// Res = 2(R*N)N - R
	result = scale(normal, 2 * dprod(ray, normal));
	result = subtr(result, ray);
	return result;
}

// rotazione antioraria rispetto ad asse
Vect3 rotate(Vect3 v, Vect3 axis, float tht) {
	Vect3 result;
	float rmatrix[3][3];

	rmatrix[0][0] = sqr(axis.x) + (1 - sqr(axis.x)) * cos(tht);
	rmatrix[0][1] = (1 - cos(tht)) * axis.x * axis.y - axis.z * sin(tht);
	rmatrix[0][2] = (1 - cos(tht)) * axis.x * axis.z + sin(tht) * axis.y;
	rmatrix[1][0] = (1 - cos(tht)) * axis.y * axis.x + sin(tht) * axis.z;
	rmatrix[1][1] = sqr(axis.y) + (1 - sqr(axis.y)) * cos(tht);
	rmatrix[1][2] = (1 - cos(tht)) * axis.y * axis.z - sin(tht) * axis.x;
	rmatrix[2][0] = (1 - cos(tht)) * axis.z * axis.x - sin(tht) * axis.y;
	rmatrix[2][1] = (1 - cos(tht)) * axis.z * axis.y + sin(tht) * axis.x;
	rmatrix[2][2] = sqr(axis.z) + (1 - sqr(axis.z)) * cos(tht);

	result.x = v.x * rmatrix[0][0] + v.y * rmatrix[0][1] + v.z * rmatrix[0][2];
	result.y = v.x * rmatrix[1][0] + v.y * rmatrix[1][1] + v.z * rmatrix[1][2];
	result.z = v.x * rmatrix[2][0] + v.y * rmatrix[2][1] + v.z * rmatrix[2][2];

	return result;
}

// rotazione antioraria attorno ad asse X
Vect3 rotateX(Vect3 v, float tht) {
	Vect3 result = v;
	result.y = v.y * cos(tht) - v.z * sin(tht);
	result.z = v.y * sin(tht) + v.z * cos(tht);
	return result;
}

// rotazione antioraria attorno ad asse Y
Vect3 rotateY(Vect3 v, float tht) {
	Vect3 result = v;
	result.z = v.z * cos(tht) - v.x * sin(tht);
	result.x = v.z * sin(tht) + v.x * cos(tht);
	return result;
}

// rotazione antioraria attorno ad asse Z
Vect3 rotateZ(Vect3 v, float tht) {
	Vect3 result = v;
	result.x = v.x * cos(tht) - v.y * sin(tht);
	result.y = v.x * sin(tht) + v.y * cos(tht);
	return result;
}
