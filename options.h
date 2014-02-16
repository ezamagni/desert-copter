/*
 * options.h
 *
 *  Created on: 10/lug/2012
 *      Author: EnricoZamagni
 */
#ifndef OPTIONS_H_
#define OPTIONS_H_

typedef unsigned char Opt;

// stato delle opzioni attuali
typedef struct {
	// impostazioni selezionabili via GUI
	Opt antialiasing;
	Opt fog;
	Opt mblur;
	Opt envmap;
} Options;

// impostazioni accessibili da tutti i sorgenti
Options options;

#endif
