#Import the required modules
import numpy as np
from python_speech_features import mfcc
from python_speech_features import fbank
from python_speech_features import logfbank
import scipy.io.wavfile as wav
from python_speech_features import delta
import numpy as np

#Read the audio files
(rate,sig) = wav.read("data1.wav")
print(rate)
print(sig.shape)

#Create arrays 
frame = np.zeros(400)
hamming = np.hamming(400)
padded_sig = np.zeros(32240)
hamming_frame = np.zeros((200,400))
windowed_frame = np.zeros(512)
fft_frame = np.zeros(512)

#Padding the signal
for i in range(32000):
  padded_sig[i] = sig[i]
l = 0

#Applying hamming window and FFT
for i in range(0,32000,160):
  for j in range(400):
    frame[j] = padded_sig[i+j]
    windowed_frame[j] = hamming[j] * frame [j]
    hamming_frame[l][j]= hamming[j] * frame [j]
  fft_frame = np.fft.fft(windowed_frame, n=512)
  l= l+1
print(fft_frame.shape)

#Saving the output in file
file = open("hamming_output.txt","w+")  
file.write("float hamming_output[200][400] = {\n")
for i in range(200):
  for j in range(400):
    file.write("%f, \n" %(hamming_frame[i][j]))
file.write("}")
file.close()

#Obtain the mel energies and MFCC output
mel_energy = logfbank(sig, samplerate=16000, winlen=0.025, winstep=0.01, nfilt=23, nfft=512, lowfreq=0, highfreq=8000, preemph=0.97)
log_mel_energy = logfbank(sig, samplerate=16000, winlen=0.025, winstep=0.01, nfilt=23, nfft=512, lowfreq=0, highfreq=8000, preemph=0.97)
mfcc_feat = mfcc(sig,rate)
