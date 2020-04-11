#ifndef RINGTONE_H_   /* Include guard */
#define RINGTONE_H_


#include <stdlib.h>
#include "../system_common/system_common.h"
#include "../uc_timer/uc_timer.h"
#include "../uc_interrupt/uc_interrupt.h"
#include "ringtone_params.h"


#define P 0.0
#define C 261.63
#define Cis 277.18
#define D 293.66 
#define Dis 311.13
#define E 329.63 	
#define F 349.23
#define Fis 369.99
#define G 392.00
#define Gis 415.30
#define A 440.00
#define Ais 466.16
#define H 493.88

typedef struct  {
   	char * ringtone_txt;
	char title[30];
	float default_duration;
	unsigned int default_octave;    //261.63Hz to 3951.07Hz
	float default_tempo;     		//0.4166Hz to 15Hz
	unsigned int start_index;
} RingtoneTypeDef;

typedef struct  {
	float duration;
	float frequency; 
} NoteTypeDef;


void ringtone_init(void);
RingtoneTypeDef ringtone_init_ringtone(char * ringtone_txt);
NoteTypeDef ringtone_parse_note(RingtoneTypeDef * ringtone, unsigned int * note_index);
void ringtone_play(char * ringtone_txt);
void ringtone_stop(void);
void ringtone_update(void);
void ringtone_start_timers(void);
void ringtone_stop_timers(void);
void ringtone_update_timers(NoteTypeDef * note);
void ringtone_isr(void);


#endif // RINGTONE_H_