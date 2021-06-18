//#include "../../sw0/src/header.h"
#include "header.h"
#include "hamming_output.h"
#include "fft_ref.h"
#include "mel_ref.h"
#include "ref_logmel.h"
#include "ff.h"
#include "mfccref.h"
#include "xbasic_types.h"
#include "xparameters.h"

Xuint32 *counter_result = (Xuint32 *)XPAR_MYIP_0_S00_AXI_BASEADDR;
Xuint32 *counter_control = (Xuint32 *) 0x43c10004;
Xuint32 *counter_reset = (Xuint32 *) 0x43c10008;


FATFS FS_instance;

int main()
{
	cout << "Software version (Core 1)" <<endl;

	FRESULT result = f_mount(&FS_instance,"0:/", 1);
	if (result != 0) {
		print("Couldn't mount SD Card.\r\n");
	}
	#define maxFiles 32
	char files[maxFiles][32] = {0};
	int filesNum = 0;

	// Look for *.wav files and copy file names to files[]
	DIR dir;
	FRESULT res = f_opendir(&dir, "0:/");
	if (res != FR_OK) {
		print("Couldn't read root directory.\r\n");
	}
	for (;;) {
		FILINFO fno;
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0) {
			break;
		}
		if (fno.fattrib & AM_DIR) {                 // It's a directory
		}
		else if (strstr(fno.fname,".wav")!=NULL || strstr(fno.fname,".WAV")!=NULL) { // It's a WAV file
			strcpy(files[filesNum++],fno.fname);
		}
		else {										// It's a normal file
		}
	}
	f_closedir(&dir);


	static float waveform[SAMPLES];
	read_wave_form(files[0], waveform);
	static float padded_waveform[32240];
	static float sw_frame[400];
	static float sw_w_out[80000];
	static float sw_psd[102400];
	static float sw_mel[4600];
	static float sw_features[2600];


	for (int i=0; i<SAMPLES; i++) {
		padded_waveform[i] = waveform[i];
	}

	*counter_reset = 0xFFFFFFFF;
	*counter_reset = 0x00000000;
	*counter_control = 0xFFFFFFFF;
//	sleep(1);
//	cout << "Counter value "<<*counter_reset<<endl;
	for (int i=0; i<SAMPLES; i+=160) {
		for (int j=0; j<FRAME; j++) {
			sw_frame[j] = padded_waveform[i+j];
		}
		sw_mfcc(sw_frame, sw_w_out, sw_psd, sw_mel, sw_features);
	}
	*counter_control = 0x00000000;
    cout <<"Software Hamming window process, FFT, PSD, Mel Energies, Log Mel Energy, DCT, MFCC process completed" << endl;
    cout << "Number of cycles required is - " << *counter_result << endl;
    cout << "MFCC testing" << endl;

	for (int i = 0; i < 13; i++) {
		cout << "SW Output - " << sw_features[i] << "\t Reference output - " << ref_mfcc[i] << endl;
	}

//	cout <<"Template Hamming testing" << endl;
//
//	for (int i = 0; i < 10; i++) {
//		cout << "SW Output - " << sw_w_out[i+(400*5)] << "\t Reference output - " << hamming_output[i+(400*5)] << endl;
//	}
//
//	bool validate_windows = true;
//	float difference;
//	for (int i =0; i<80000; i++){
//		difference = sw_w_out[i]-hamming_output[i];
//		if((difference>0.01)||(difference<-0.01)) {
//			cout << "Error on " << i << " - " <<sw_w_out[i] << " - " << difference << endl;
//			validate_windows = false;
//		}
//	}
//    if(validate_windows) {
//    	cout << "TEST PASS. All the Windowed frames obtained matches with Reference." << endl;
//    }
//    else {
//    	cout <<"TEST FAIL" << endl;
//    }
//
//    cout <<"Software FFT and PSD process completed" << endl;
//
//	cout <<"PSD testing" << endl;
//
//	for (int i = 0; i < 10; i++) {
//		cout << "SW Output - " << sw_psd[i] << "\t Reference output - " << psd_ref[i] << endl;
//	}
//
//	cout << "Mel Energies testing" << endl;
//
//	for (int i = 0; i < 10; i++) {
//		cout << "SW Output - " << sw_mel[i] << "\t Reference output - " << mel_ref[i] << endl;
//	}

	return 0;
}
