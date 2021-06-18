#include "header.h"
#include "ff.h"

struct wav_header_t
{
    uint8_t chunkID[4]; //"RIFF" = 0x46464952
    uint32_t chunkSize; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
    uint8_t format[4]; //"WAVE" = 0x45564157
    uint8_t subchunk1ID[4]; //"fmt " = 0x20746D66
    uint32_t subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

//Chunks
struct chunk_t
{
    uint8_t ID[4]; //"data" = 0x61746164
    uint32_t size;  //Chunk data bytes
};

int read_wave_form(char *filename, float waveform[SAMPLES]) {
    wav_header_t header;
    int samp_rate;
    int num_samples;
    FIL file;
    UINT nBytesRead=0;
    UINT nChannels=1;
    FRESULT res = f_open(&file, filename, FA_READ);
    if (res!=0) {
    	cout << "File not found" << endl;
    }
    printf("Loading %s\r\n",filename);

    res = f_read(&file,(void*)&header,sizeof(header),&nBytesRead);
    if (res!=0) {
    	cout << "Failed to read file" << endl;
    }

    if (strncmp((const char*)(header.chunkID), "RIFF", 4) == 0) {
        cout << "File Type RIFF was found!"<< endl;
    } else {
        cout << "Wrong file type?"<< endl;
        exit(1);
    }
    if (header.audioFormat != 1) {  // PCM Wave format
        cout << "Only PCM WAV format is accepted at present" <<endl;
        exit(1);
    }
    if (header.numChannels > 1) {
        cout << "WARNING: Multiple channels found in WAV, taking only 1"<<endl;
    }
    samp_rate = header.sampleRate;
    cout << "Sampling rate is " << samp_rate << endl;
    cout << "Duration of Audio is " << duration << " seconds" << endl;

    //Reading file
    chunk_t chunk;
    //go to data chunk
    int skip = 0;
    while (true)
    {
        res = f_read(&file, (void*)&chunk, sizeof(chunk), &nBytesRead);
        if (res!=0) {
        	cout << "Failed to read file" << endl;
        }
        if (*(unsigned int *)&chunk.ID == 0x61746164)
            break;
        //skip chunk data bytes
        DWORD fp = f_tell(&file);
        f_lseek(&file,fp + chunk.size);
        cout << "Skipped " << skip++ << endl;
        if (skip > 100) exit(1);
    }
    //Number of samples
    int sample_size = header.bitsPerSample / 8;
    int samples_count = chunk.size * 8 / header.bitsPerSample;
    samples_count /= header.numChannels;
    num_samples = samples_count;
    cout << "Samples count = " << samples_count << endl;

    short int x[header.numChannels];
    //Reading data
    for (int i = 0; i < samples_count; i++)
    {
        f_read(&file, &x, sample_size, &nChannels);
        waveform[i] = (float)(x[0]);
    }
    f_close(&file);
    return 0;
}


//void framing(float waveform[SAMPLES], float frame[FRAME]) {
//	static float padded_waveform[32240];
//	for (int i=0; i<SAMPLES; i++) {
//		padded_waveform[i] = waveform[i];
//	}
//	for (int i=0; i<SAMPLES; i+160) {
//		for (int j=0; j<FRAME; j++) {
//			if((i+j)<SAMPLES) {
//			frame[j] = padded_waveform[i+j];
//			}
//		}
//	}
//}
