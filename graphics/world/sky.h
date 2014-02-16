/*
 * sky.h
 *
 *  Created on: 24/giu/2012
 *      Author: "Enrico Zamagni"
 */

#ifndef SKY_H_
#define SKY_H_

#include <GL/gl.h>
#include "../../options.h"

// inizializza/libera le risorse per la skybox
void initSkybox(const GLfloat fogCol[3]);
void destroySkybox();

// renderizza la skybox
void renderSkybox(GLfloat viewHeight);

#endif /* SKY_H_ */
