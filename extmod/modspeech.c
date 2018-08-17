#include "py/nlr.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/binary.h"
#include <stdio.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "speech/debug.h"
#include "speech/reciter.h"
#include "speech/sam.h"

int debug = 0;

void WriteWav(char* filename, char* buffer, int bufferlength)
{
	FILE *file = fopen(filename, "wb");
	if (file == NULL) return;
	//RIFF header
	fwrite("RIFF", 4, 1,file);
	unsigned int filesize=bufferlength + 12 + 16 + 8 - 8;
	fwrite(&filesize, 4, 1, file);
	fwrite("WAVE", 4, 1, file);

	//format chunk
	fwrite("fmt ", 4, 1, file);
	unsigned int fmtlength = 16;
	fwrite(&fmtlength, 4, 1, file);
	unsigned short int format=1; //PCM
	fwrite(&format, 2, 1, file);
	unsigned short int channels=1;
	fwrite(&channels, 2, 1, file);
	unsigned int samplerate = 22050;
	fwrite(&samplerate, 4, 1, file);
	fwrite(&samplerate, 4, 1, file); // bytes/second
	unsigned short int blockalign = 1;
	fwrite(&blockalign, 2, 1, file);
	unsigned short int bitspersample=8;
	fwrite(&bitspersample, 2, 1, file);

	//data chunk
	fwrite("data", 4, 1, file);
	fwrite(&bufferlength, 4, 1, file);
	fwrite(buffer, bufferlength, 1, file);

	fclose(file);
}

STATIC mp_obj_t speech_say(mp_obj_t phrase_obj) {
    const char *phrase = mp_obj_str_get_str(phrase_obj);

    char input[256];
    strcpy(input, phrase);

    int i;
    for(i=0; input[i] != 0; i++) {
		input[i] = toupper((int)input[i]);
    }
    strcat(input, "[");

    TextToPhonemes(input);
    SetInput(input);
    SAMMain();

    char *wavfilename = "test.wav";
    WriteWav(wavfilename, GetBuffer(), GetBufferLength()/50);

    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(speech_say_obj, speech_say);

STATIC const mp_map_elem_t speech_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_speech) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_say), (mp_obj_t)&speech_say_obj },
};

STATIC MP_DEFINE_CONST_DICT (
    mp_module_speech_globals,
    speech_globals_table
);

const mp_obj_module_t mp_module_speech = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_speech_globals,
};
