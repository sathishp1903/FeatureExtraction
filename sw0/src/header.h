#include <iostream>
#include <fstream>
#include "platform.h"
#include "xparameters.h"
#include "sleep.h"
#include "xfeature_extraction.h"


using namespace std;

#define SAMPLES 32000
#define FRAME 400
#define duration 2

int read_wave_form(char *filename, float waveform[SAMPLES]);

