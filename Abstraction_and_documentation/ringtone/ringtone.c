

#include "ringtone.h"

/* FORMAT OF THE RING TONE TEXT TRANFER PROTOCOL (ringtone)
Every section is separated by a ":"
TITLE
    No more than 10 characters
DEFAULT PARAMETERS
    d 	Default duration
            1 	a full note
            2 	a half note
            4 	a quarter note
            8 	an eighth note
            16 	a sixteenth note
            32 	a thirty-second note
    o 	Default octave
            5 	Note A is 440Hz
            6 	Note A is 880Hz
            7 	Note A is 1760Hz
            8 	Note A is 3520Hz
    b 	Default bpm (beats per minute)
            25, 28, 31, 35, 40, 45, 
            50, 56, 63, 70, 80, 90,
            100, 112, 125, 140, 160, 
            180, 200, 225, 250, 285, 
            320, 355, 400, 450, 500, 
            565, 635, 715, 800 and 900.
SONGDATA
    The notes are encoded as follows:
        [duration] note [scale] [special-duration],[duration] note [scale] [special-duration], etc...
        P 	Pause
        C 	Note C
        C# 	Note Cis
        D 	Note D
        D# 	Note Dis
        E 	Note E
        F 	Note F
        F# 	Note Fis
        G 	Note G
        G# 	Note Gis
        A 	Note A
        A# 	Note Ais
        H 	Note H
        
        http://www.smssolutions.net/tutorials/smart/ringtone/
*/


//Frequency of all notes in the 5th octave
//https://pages.mtu.edu/~suits/notefreqs.html



RingtoneTypeDef current_ringtone;
NoteTypeDef current_note;
unsigned int current_index;

uint8_t playing = 0;


static unsigned int my_pow(unsigned int base,unsigned int degree);



void ringtone_start_timers(void){

	uc_timer_start(RINGTONE_TIMER);
	uc_timer_start(RINGTONE_PWM_TIMER);
}

void ringtone_stop_timers(void){

	uc_timer_stop(RINGTONE_TIMER);
	uc_timer_stop(RINGTONE_PWM_TIMER);
}

void ringtone_isr(void){
    ringtone_update();
}

void ringtone_stop(void){
    NoteTypeDef note = {0.0, 0.0};
    ringtone_update_timers(&note);
    ringtone_stop_timers();
    playing = 0;
}

void ringtone_init(void){

	/****** Configure timer as up counting*****/
	uc_timer_setup(RINGTONE_TIMER_CLOCK, RINGTONE_TIMER, RINGTONE_TIMER_PRESCALER);
	uc_timer_config_period(RINGTONE_TIMER, 0);
	uc_timer_enable_interrupt(RINGTONE_TIMER, RINGTONE_TIMER_INTERRUPT);

    uc_interrupt_attatch_function(RINGTONE_TIMER_INTERRUPT_ISR, ringtone_isr);

	/****** Configure timer as PWM*****/
	uc_timer_pwm_pin_setup(RINGTONE_PWM_PIN_PORT_CLOCK, RINGTONE_PWM_PIN_PORT, RINGTONE_PWM_PIN);
	uc_timer_pwm_setup(RINGTONE_PWM_TIMER_CLOCK, RINGTONE_PWM_TIMER, RINGTONE_PWM_TIMER_CHANNEL, RINGTONE_PRESCALER_PWM); 


}

void ringtone_update_timers(NoteTypeDef * note){
	float frequency, duration;
	uint32_t counts;
	frequency = note->frequency;
	duration = note->duration;
	
	if(frequency == 0.0){
		uc_timer_config_period(RINGTONE_PWM_TIMER, 0);
        uc_timer_pwm_config_duty_cycle(RINGTONE_PWM_TIMER, RINGTONE_PWM_TIMER_CHANNEL, 0);
	}else{
        counts = (uint32_t)((float)F_CLK/(float)RINGTONE_PRESCALER_PWM/frequency);
        uc_timer_config_period(RINGTONE_PWM_TIMER, counts);
	    uc_timer_pwm_config_duty_cycle(RINGTONE_PWM_TIMER, RINGTONE_PWM_TIMER_CHANNEL, counts/2);
	}

	if(duration == 0.0){
		uc_timer_config_period(RINGTONE_TIMER, 0);
	}else{
        counts = (uint32_t)((float)F_CLK/(float)RINGTONE_TIMER_PRESCALER/duration);
        counts*= 4; // don't know why
        uc_timer_config_period(RINGTONE_TIMER, counts);
	}

}

void ringtone_play(char * ringtone_txt){

    ringtone_stop();
    playing = 1;
	current_ringtone = ringtone_init_ringtone(ringtone_txt);
	current_index = current_ringtone.start_index;

	current_note = ringtone_parse_note(&current_ringtone, &current_index);

	ringtone_update_timers(&current_note);
	ringtone_start_timers();


}

void ringtone_update(void){

	ringtone_update_timers(&current_note);
	current_note = ringtone_parse_note(&current_ringtone, &current_index);
}

RingtoneTypeDef ringtone_init_ringtone(char * ringtone_txt){
	unsigned int j,k;
	char buffer[5];
	
	RingtoneTypeDef ringtone;

	ringtone.ringtone_txt = ringtone_txt;
	

	/******* Get Title*****/
    j = 0;
	do{
		ringtone.title[j] = ringtone_txt[j];
	}while(ringtone_txt[++j] != ':');
	ringtone.title[j] = 0;

	/******* Parse default duration *****/
    do{}while(ringtone_txt[++j] != '=');    
    j++; // skip '='

	k = 0;
    do{
		if (ringtone_txt[j] != ' '){
			buffer[k++] = ringtone_txt[j];
		}
    }while(ringtone_txt[++j] != ',');

	buffer[k] = 0;
    ringtone.default_duration = (float)atoi(buffer);

	/******* Parse default octave *****/
    do{}while(ringtone_txt[++j] != '=');    
    j++; // skip '='

	k = 0;
    do{
		if (ringtone_txt[j] != ' '){
			buffer[k++] = ringtone_txt[j];
		}
    }while(ringtone_txt[++j] != ',');

	buffer[k] = 0;
    ringtone.default_octave = atoi(buffer);

	/******* Parse default tempo *****/
    do{}while(ringtone_txt[++j] != '=');    
    j++; // skip '='

	k = 0;
    do{
		if (ringtone_txt[j] != ' '){
			buffer[k++] = ringtone_txt[j];
		}
    }while(ringtone_txt[++j] != ':');

	buffer[k] = 0;
    ringtone.default_tempo = (float)atoi(buffer);
	//ringtone.default_tempo = (float)(60*4/ringtone.default_tempo);

	j++;
	ringtone.start_index = j;

	return ringtone;
}

NoteTypeDef ringtone_parse_note(RingtoneTypeDef * ringtone, unsigned int * note_index){
	unsigned int octave,j, index;
	float duration,frequency;
	char buffer[5];
	char * song;

	duration = ringtone->default_duration;
	octave = ringtone->default_octave;
	index = *note_index;
	song = ringtone->ringtone_txt;


	if(song[index] == ' '){
		index++;
	}
	
	/*********Parse duration (optional)**********/
	j = 0;
	while( song[index] >= '0' && song[index] <= '9'){
		buffer[j++] = song[index++];
	}
	if(j > 0){
		buffer[j] = 0;
		duration = (float)atoi(buffer);
	}
	
	/*********Parse Note**********/
	j = 0;
	
	switch(song[index++]){
		case 'p':
			frequency = P;
			break;
		case 'c': 
			if(song[index] == '#'){
				frequency = Cis;
				index++;
			}
			else frequency = C;
			break;
		case 'd': 
			if(song[index] == '#'){
				frequency = Dis;
				index++;
			}
			else frequency = D;
			break;
		case 'e':
			frequency = E;
			break;
		case 'f': 
			if(song[index] == '#'){
				frequency = Fis;
				index++;
			}
			else frequency = F;
			break;
		case 'g': 
			if(song[index] == '#'){
				frequency = Gis;
				index++;
			}
			else frequency = G;
			break;
		case 'a': 
			if(song[index] == '#'){
				frequency = Ais;
				index++;
			}
			else frequency = A;
			break;
		case 'h':
			frequency = H;
			break;
		case 'b':
			frequency = H;
			break;
		default:
			frequency = P;
			break;
	}
	
	/*********Parse special-duration (optional)**********/
	if(song[index] == '.') 
	{
		duration = duration*0.6666;
		index++;
	}

	/*********Parse scale  (optional)**********/

	j = 0;
	if( song[index] >= '0' && song[index] <= '9'){
		buffer[j++]=song[index];
		buffer[j]=0;
		octave = atoi(buffer);
		index++;
	}

	if(song[index] == ','){
		index++;
	} 
	if(song[index] == 0) index = ringtone->start_index;

	*note_index = index;
	
	NoteTypeDef  note;
	note.frequency = frequency * (float)my_pow(2, (octave-5));
	//note.duration= duration/(ringtone->default_tempo);
	note.duration= (duration*ringtone->default_tempo)/60;

	return note;

}

static unsigned int my_pow(unsigned int base,unsigned int degree){ 
    unsigned int result = 1;
    unsigned int i;
    for ( i = 0; i < degree; ++i)
        result *= base;

    return result;
}


