# Feature extraction from Audio Signal for Automatic Speech Recognition

Vivado
------
To open the Vivado design -
> Open project_1.xpr 

HLS
---
The MFCC feature extraction process is designed in HLS.

> The source code for HLS is available in hls_mfcc/src/ folder


Counter IP
----------
A counter IP is designed to measure the performance of hardware and software performance.

> The verilog code to implement the counter can be found at custom_counter/myip_1.0/hdl/myip_v1_0_S00_AXI

Processor Code - SDK
--------------------

The software code is written for both the core of the Zynq processor using SDK application.

> The core 0 code which implements the hardware MFCC version can be found in sw0/src/

> The core 1 code which implements the software MFCC version can be found in sw1/src/

Instructions to run the application
-----------------------------------

> Record the audio using audacity application for 2 secs in wav format with 16kHz sample rate in mono format.
> Load that audio in the SD card.
> Or load the already recorded audio file which is available in the audio_files folder and load in the SD Card.
> Open the vivado project, export the hardware and launch SDK.
> Enable the XilFFS library for both the software files to mount the data from SD card.
> Switch ON the Zed board and insert the SD card with the audio file.
> From the SDK, program the FPGA using Xilinx/Program FPGA.
> Turn on serial monitor for the required port with baud rate as 115200.
> Run sw0, to evaluate the hardware mode of MFCC and visualize the  performance.
> Run sw1, to evaluate the software mode of MFCC and visualize the  performance.


Reference version
-----------------
The python code which is used as reference version is found at python/code.py
