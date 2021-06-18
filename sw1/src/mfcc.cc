#include "header.h"
#include "twiddle.h"

//BaseCplx twiddle512(int i)
//{
//	BaseCplx result;
//	result = BaseCplx(cos(2*M_PI*i/512), -sin(2*M_PI*i/512));
//    return result;
//}




void psd_bitrev512(BaseCplx xn[512], float *psd)
{
	for (int i = 0; i < 512; i++) {
		BaseCplx x = xn[bit_rev_addr[i]];
		psd[i] = std::real(x * std::conj(x) * float(512.0f * 512.0f));
	}
}

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


void fft(float windowed_frame[400], float psd[512]) {
	// Copy frame into local complex array
	// Perform forward FFT
	// While bitreversing, take |.|^2 to get PSD
	// This works right now only for 512 pt (16kHz, 25.0ms, padded)
	//assert (frame.Dim() == 512);
	BaseCplx xn[512];
	static float padded_frame[512];

	for (int i=0; i<400; i++) {
		padded_frame[i] = windowed_frame[i];
	}
	for (int i=0; i<512; i++) {
		xn[i] = padded_frame[i];
	}
	const int LOG2N = 9;	//log2(FFT_LENGTH512;
	int stage, pass, span, fly;
	int Ulimit, Llimit, span_max;

	pass = 0;
	span = 0;
	for (stage = 0; stage < LOG2N; stage++) {
		for (fly = 0; fly < 512 / 2; fly++) {
			span_max = (512) >> (stage + 1);
			int index = span << stage;
			Ulimit = span + ((pass * span_max) << 1);
			Llimit = Ulimit + span_max;
			butterfly512(Ulimit, Llimit, index, xn);
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

void mel_powerbank(float *psd, float mel_energies[NUM_MEL]) {
    for (int i = 0; i < NUM_MEL; i++) {
    	float energy = 0.0;
        for (int j = 0; j < NUM_FFT/2; j++) {
        	energy += MelBanks[i*512/2+j] * psd[j];
        }
        mel_energies[i] = log(energy);
    }
}

void hamming_window(float sw_frame[400], float windowed_frame[400], float *raw_energy) {

    float energy = 0.0;
    for (int i = 0; i < 400; i++) energy += sw_frame[i] * sw_frame[i];
    *raw_energy = log(energy);
    for (int i = 400 - 1; i > 0; i--)
    	sw_frame[i] -= sw_frame[i-1] * 0.97; //preemph_coeff is 0.97
    sw_frame[0] -= sw_frame[0] * 0.97 ;
	for (int i=0; i<400; i++) {
		windowed_frame[i] = sw_frame[i] * HammingWindow[i];
	}
}

void mfcc_dct(float mel_energies[NUM_MEL], float features[NUM_CEPS]) {
    for (int i = 0; i < NUM_CEPS; i++)
        features[i] = 0.0f;

    // Multiply mel_energies (23x1) by DCT matrix (13x23) to get ceps
    for (int i = 0; i < NUM_CEPS; i++) {
        for (int j = 0; j < NUM_MEL; j++) {
            features[i] += DCT[i*NUM_MEL+j] * mel_energies[j];
        }
    }
    // Cepstral liftering
    for (int i = 0; i < NUM_CEPS; i++) {
            features[i] *= CepCoeffs[i];
    }
}

void sw_mfcc(float sw_frame[400], float sw_w_out[80000], float sw_psd[102400],
		float sw_mel[4600], float sw_features[2600]) {
	static float windowed_frame[400];
	static float psd[512];
	static float mel_energies[23];
	static float features[13];
	static int l;
	float raw_energy = 0.0;
	hamming_window(sw_frame, windowed_frame, &raw_energy);
	for (int j=0; j<FRAME; j++) {
		sw_w_out[(l*400)+j] = windowed_frame[j];
	}

	fft(windowed_frame, psd);
	for (int j=0; j<512; j++) {
		sw_psd[(l*512)+j] = psd[j];
	}

	mel_powerbank(psd, mel_energies);

	for (int j=0; j<23;j++) {
		sw_mel[(l*23)+j] = mel_energies[j];
	}

	mfcc_dct(mel_energies, features);

	features[0] = raw_energy;

	for (int j=0; j<13;j++) {
		sw_features[(l*13)+j] = features[j];
	}
	l = l+1;
}
