#include "header.h"
#include "ff.h"
#include "data1.h"
#include "hamming_output.h"
#include "fft_ref.h"
#include "mel_ref.h"
#include "ref_logmel.h"
#include "mfccref.h"
#include "xbasic_types.h"
#include "xparameters.h"

Xuint32 *counter_result = (Xuint32 *)XPAR_MYIP_0_S00_AXI_BASEADDR;
Xuint32 *counter_control = (Xuint32 *) 0x43c10004;
Xuint32 *counter_reset = (Xuint32 *) 0x43c10008;

FATFS FS_instance;

// IP config pointers and handlers
XFeature_extraction mfcc_hw;
XFeature_extraction_Config *mfcc_cfg;

void initPeripherals()
{
	//Initialize MFCC core
	cout << "Initializing MFCC HLS IP Core" << endl;
	mfcc_cfg = XFeature_extraction_LookupConfig(XPAR_FEATURE_EXTRACTION_0_DEVICE_ID);
	if(mfcc_cfg) {
		int status = XFeature_extraction_CfgInitialize(&mfcc_hw, mfcc_cfg);
		if (status != XST_SUCCESS) {
			cout << "Error Initializing MFCC IP core" << endl;
		}
	}
}

int main()
{
	init_platform();
	cout << "Feature Extraction from Audio Signal Project" << endl;
	print("Mounting SD Card\n\r");
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
	if (filesNum == 0) {
		print("No WAV files found. \r\n");
		getchar();
	}
	cout << "Number of wav files found is " << filesNum << endl;

	static float waveform[SAMPLES];
	static float padded_waveform[32240];
	float *frame = (float *)0x40000000;
//	float *windowed_frame = (float *)0x42000000;
//	float *psd = (float *)0x42000000;
//	float *mel_energy = (float *)0x42000000;
	float *mfcc_feature = (float *)0x42000000;

	static float mfcc_output[2600];
	read_wave_form(files[0], waveform);

	cout << "Template validation with reference" << endl;
	for (int i=0; i<10; i++) {
		cout << "Sample number - " << i << "\t Reference value - " << samples[i] <<  "\tActual Value - " << waveform[i] << endl;
	}
	cout << "Complete validation" << endl;
	bool validate_samples = true;

	for (int i =0; i<SAMPLES; i++){
		if (samples[i]!=waveform[i]) {
			validate_samples = false;
		}
	}
    if(validate_samples) {
    	cout << "TEST PASS. All the Samples obtained matches with Reference." << endl;
    }
    else {
    	cout <<"TEST FAIL" << endl;
    }

    initPeripherals();

    //Padding samples to be even to 25ms frame
	for (int i=0; i<SAMPLES; i++) {
		padded_waveform[i] = waveform[i];
	}

	int l=0;
	//Framing the samples every 25ms with timestep of 10ms

	*counter_reset = 0xFFFFFFFF;
	*counter_reset = 0x00000000;
	*counter_control = 0xFFFFFFFF;
	for (int i=0; i<SAMPLES; i+=160) {
		for (int j=0; j<FRAME; j++) {
			frame[j] = padded_waveform[i+j];
		}
		XFeature_extraction_Start(&mfcc_hw);
		while(!XFeature_extraction_IsDone(&mfcc_hw));
		for (int j=0; j<13; j++) {
			mfcc_output[(l*13)+j] = mfcc_feature[j];
		}
		l=l+1;
	}
	*counter_control = 0x00000000;

	cout << "Number of cycles required is - " << *counter_result << endl;
	cout<<"Hardware Hamming window, FFT, PSD, and Log Mel Energy Bank process completed" << endl;


	cout <<"MFCC testing" << endl;

	for (int i = 0; i < 13; i++) {
		cout << "HW Output - " << mfcc_output[i] << "\t Reference output - " << ref_mfcc[i] << endl;
	}


	cleanup_platform();
	return 0;
}
