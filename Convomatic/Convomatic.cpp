#include <chrono>
#include "AudioFile.cpp"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace std::chrono;

AudioFile<double> inputFile;
AudioFile<double> outputFile;
AudioFile<double> impulseFile;

void preview(AudioFile<double> *file, int channel, int size) {
	cout << std::fixed;
	cout << std::setprecision(3);
	for (int i = 0; i < size; i++) {
		double inputVal = file->samples[channel][i];
		if (inputVal >= 0) cout << " ";
		cout << file->samples[channel][i] << " ";
		if (i+1 % 10 == 0) cout << endl;
	}
	cout << endl;
}

double convolve( double *bufStart, double* currBuff, int bufSize, double *impulse, int impulseLen)
{
	double result = 0;
	for (int i = 0; i < impulseLen; i++)
	{
		if (currBuff <= bufStart) {
			currBuff = bufStart + bufSize - 1;
		}
		result += *(currBuff--) * impulse[i];
	}
	return result;
}


int main() {
	int channel = 0;
	cout << "Loading Files" << endl;
	
	inputFile.load("res/quicc.wav");
	cout << "Loaded input file" << endl;
	preview(&inputFile, channel, 100);

	impulseFile.load("res/slap.wav");
	cout << "Loaded impulse file" << endl;
	preview(&impulseFile, channel, 100);

	if (!inputFile.isMono()) inputFile.samples.resize(1);
	if (!impulseFile.isMono()) impulseFile.samples.resize(1);

	cout << "Beginning transformation" << endl;

	auto start = high_resolution_clock::now();

	int inputSamples = inputFile.getNumSamplesPerChannel();
	int impulseSamples = impulseFile.getNumSamplesPerChannel();
	int outputLen = inputSamples + impulseSamples - 1;

	outputFile.setNumSamplesPerChannel(outputLen);
	outputFile.setNumChannels(1);

	double* result;
	double* startInput = &inputFile.samples[channel][0];
	double* startImpulse = &impulseFile.samples[channel][0];

	const int bufSize = 50000;
	double* buffer = (double*)calloc(bufSize, sizeof(double));
	double *currBufPtr = &buffer[0];
	memset(&buffer[0], 0, sizeof(double) * bufSize);

	for (int i = 0; i < outputLen; i++)
	{
		//get current sample
		double sample = inputFile.samples[channel][i];

		//store input at current location
		*currBufPtr = sample;

		double conv = convolve(&buffer[0], currBufPtr, bufSize, startImpulse, impulseSamples);

		//circular buffer
		currBufPtr++;
		if (currBufPtr > &buffer[0] + bufSize) {
			currBufPtr = &buffer[0];
		}

		//write to output
		outputFile.samples[channel][i] = conv * 0.05;

		//report status
		if (i % 1000 == 0) {
			cout << i << " out of " << outputLen << endl;
		}
	}

	auto end = high_resolution_clock::now();
	double durr = std::chrono::duration<double>(end - start).count();
	printf("Took %f seconds (%f per each sample)\r\n", durr, (inputFile.getNumSamplesPerChannel()) / durr);
	outputFile.save("res/output1.wav", AudioFileFormat::Wave);

	//graphFFT visualFFT;
	//visualFFT.drawGraph((char*)"res/440Hz.wav");
	return 0;
}