#include "header.h"
#include "twiddle_factor.h"

// Bit reversal of output of FFT in order
void psd_bitrev512(BaseCplx xn[NUM_FFT], float *psd)
{
	for (int i = 0; i < NUM_FFT; i++) {
#pragma HLS pipeline
		BaseCplx x = xn[bit_rev_addr[i]];
		psd[i] = std::real(x * std::conj(x) * float(512.0f * 512.0f));
	}
}

// Compute of butterfly in FFT
void butterfly512(int Ulimit, int Llimit, int index,
		  BaseCplx * xn)
{
	BaseCplx a, b, c;
	BaseCplx data_U, data_L;
	data_U = xn[Ulimit];
	data_L = xn[Llimit];
	a = (data_U + data_L)/2.0f;
	c = (data_U - data_L)/2.0f;

	// Only forward FFT
//	b = c * twiddle512(index);
	b = c * BaseCplx(twiddle[index][0], twiddle[index][1]);
	xn[Ulimit] = a;
	xn[Llimit] = b;
}

// Fast fourier transform
void hw_fft(float windowed_frame[NUM_FFT], float psd[NUM_FFT]) {
	// Copy frame into local complex array
	// Perform forward FFT
	// While bitreversing, take |.|^2 to get PSD
	// This works right now only for 512 pt (16kHz, 25.0ms, padded)
	//assert (frame.Dim() == 512);
	BaseCplx xn[NUM_FFT];
	BaseCplx data_U, data_L;

	L2: for (int i=0; i<NUM_FFT; i++) {
#pragma HLS pipeline
		xn[i] = windowed_frame[i];
	}
	const int LOG2N = 9;	//log2(FFT_LENGTH512;
	int pass, span, stage, fly;
	int span_max;

	int Ulimit, Llimit, index;

	pass = 0;
	span = 0;

	// looping till number of stages
	L3: for (stage = 0; stage < LOG2N; stage++) {
		L4: for (fly = 0; fly < 256; fly++) {
#pragma HLS dependence variable=xn inter false
#pragma HLS pipeline
			span_max = (512) >> (stage + 1);
			int index = span << stage;
			Ulimit = span + ((pass * span_max) << 1);
			Llimit = Ulimit + span_max;
			data_U = xn[Ulimit];
			data_L = xn[Llimit];
			xn[Ulimit] = (data_U + data_L)/2.0f;
			xn[Llimit] = (data_U - data_L)/2.0f * BaseCplx(twiddle[index][0], twiddle[index][1]);
			if (span < (span_max) - 1) {
				span++;
			} else if (pass < POW2[stage] - 1) {
				span = 0;
				pass++;
			} else {
				span = 0;
				pass = 0;
			}
		}
	}
	psd_bitrev512(xn,psd);
}

// DFT operation
void hw_dft(float windowed_frame[NUM_FRAME], float psd[NUM_FFT]) {
	float xn[NUM_FFT];
	float xk_real[NUM_FFT];
	float xk_imaj[NUM_FFT];
	float temp_real;
	float temp_imaj;
	BaseCplx data_U, data_L;
	static float padded_frame[NUM_FFT];

	L1: for (int i=0; i<NUM_FRAME; i++) {
#pragma HLS pipeline
		padded_frame[i] = windowed_frame[i];
	}
	L2: for (int i=0; i<NUM_FFT; i++) {
#pragma HLS pipeline
		xn[i] = padded_frame[i];
		xk_real[i] = 0;
		xk_imaj[i] = 0;
	}

	L3: for (int k=0; k<512; k++) {
		temp_real = 0;
		temp_imaj = 0;
		L4: for (int n=0; n<512; n++) {
#pragma HLS pipeline
			temp_real += xn[n] * twiddle[n][0];
			temp_imaj += xn[n] * twiddle[n][1];
		}
		xk_real[k]= temp_real;
		xk_imaj[k]= temp_imaj;
	}

	for (int i = 0; i < NUM_FFT; i++) {
#pragma HLS pipeline
//		BaseCplx x = xn[i];
//		psd[i] = std::real(x * std::conj(x) * float(512.0f * 512.0f));
		psd[i] = (xk_real[i] * xk_real[i]) + (xk_imaj[i] * xk_imaj[i]);
	}
}

// HammingWindow function
void hamming_window(float padded_frame[NUM_FFT], float windowed_frame[NUM_FFT], float *raw_energy) {
    float energy = 0.0;
    RAW_ENERGY: for (int i = 0; i < 400; i++) {
#pragma HLS pipeline
    	energy += padded_frame[i] * padded_frame[i];
    }
    *raw_energy = log(energy);
    PREEMPH: for (int i = 400-1; i > 0; i--) {
#pragma HLS pipeline
    	padded_frame[i] -= padded_frame[i-1] * 0.97; //preemph_coeff is 0.97
    }
    padded_frame[0] -= padded_frame[0] * 0.97 ;
    HAMMING: for(int i=0; i<NUM_FRAME; i++) {
#pragma HLS pipeline
    	windowed_frame[i] = padded_frame[i]*HammingWindow[i];
    }
}

// Mel Banks
void hw_mel(float psd[NUM_FFT], float mel_energies[NUM_MEL]) {
    MEL1: for (int i = 0; i < NUM_MEL; i++) {
#pragma HLS pipeline
        float energy = 0.0;
        MEL2: for (int j = 0; j < 256; j++) {
#pragma HLS unroll
        	energy += MelBanks[i*256+j] * psd[j];
        }
        mel_energies[i] = log(energy);
    }
}

// Computing the MFCC features
void hw_mfcc(float mel_energies[NUM_MEL], float mfcc_feature[13]) {
    MFCC1: for (int i = 0; i < NUM_CEPS; i++)
#pragma HLS pipeline
    	mfcc_feature[i] = 0.0f;

    // Multiply mel_energies (23x1) by DCT matrix (13x23) to get ceps
    for (int i = 0; i < NUM_CEPS; i++) {
#pragma HLS pipeline
        for (int j = 0; j < NUM_MEL; j++) {
#pragma HLS unroll
        	mfcc_feature[i] += DCT[i*NUM_MEL+j] * mel_energies[j];
        }
    }
    // Cepstral liftering
    for (int i = 0; i < NUM_CEPS; i++) {
#pragma HLS pipeline
    	mfcc_feature[i] *= CepCoeffs[i];
    }
}

//Used for padding the frame equal to the length of FFT
void padding(float frame[400], float padded_frame[NUM_FFT]) {
	for (int i =0; i<400; i++) {
#pragma HLS pipeline
		padded_frame[i] = frame[i];
	}
}
