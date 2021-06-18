#include "header.h"

// Without Batching
void feature_extraction(float frame[400], float mfcc[13]) {
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
#pragma HLS INTERFACE bram port=frame
#pragma HLS INTERFACE bram port=mfcc
// Using BRAM as interface between PS and PL
	static float padded_frame[512];
	static float windowed_frame[512];
	static float psd[512];
	static float mel_energy[23];
	static float mfcc_feature[13];
	float raw_energy = 0.0;
	//Calling the hardware functions to do all the subtasks
	padding(frame, padded_frame);
	hamming_window(padded_frame, windowed_frame, &raw_energy);
	hw_fft(windowed_frame, psd);
	hw_mel(psd, mel_energy);
	hw_mfcc(mel_energy, mfcc_feature);
	mfcc[0] = raw_energy;
	for (int i = 1; i < 13; i++) {
#pragma HLS pipeline
		mfcc[i] = mfcc_feature[i];
	}
}

//With Batching
//void feature_extraction(float padded_waveform[32240], float mfcc[2600]) {
//#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
//#pragma HLS INTERFACE bram port=padded_waveform
//#pragma HLS INTERFACE bram port=mfcc
//	static float padded_frame_1[512];
//	static float windowed_frame_1[512];
//	static float psd_1[512];
//	static float mel_energy_1[23];
//	static float mfcc_feature_1[13];
//	static float padded_frame_2[512];
//	static float windowed_frame_2[512];
//	static float psd_2[512];
//	static float mel_energy_2[23];
//	static float mfcc_feature_2[13];
//	float raw_energy = 0.0;
//
//	for (int i=0; i<32000; i+=160) {
//		for (int j=0; j<400; j++) {
//#pragma HLS pipeline
//			padded_frame_1[j] = padded_waveform[i+j];
//		}
////		padding(frame, padded_frame);
//		hamming_window(padded_frame_2, windowed_frame_1, &raw_energy);
//		hw_fft(windowed_frame_2, psd_1);
//		hw_mel(psd_2, mel_energy_1);
//		hw_mfcc(mel_energy_2, mfcc_feature_1);
//		mfcc[0] = raw_energy;
//		for (int j = 1; j < 13; j++) {
//#pragma HLS pipeline
//			mfcc[(i*13)+j] = mfcc_feature_2[j];
//		}
//		for (int k = 1; k < 13; k++) {
//#pragma HLS pipeline
//			mfcc_feature_2[k] = mfcc_feature_1[k];
//			mfcc_feature_1[k] = 0;
//
//		}
//		for (int k = 1; k < 23; k++) {
//#pragma HLS pipeline
//			mel_energy_2[k] = mel_energy_1[k];
//			mel_energy_1[k] = 0;
//		}
//		for (int k = 1; k < 512; k++) {
//#pragma HLS pipeline
//			psd_2[k] = psd_1[k];
//			psd_1[k] = 0;
//			windowed_frame_2[k] = windowed_frame_1[k];
//			windowed_frame_1[k] = 0;
//			padded_frame_2[k] = padded_frame_1[k];
//			padded_frame_1[k] = 0;
//		}
//	}
//}

