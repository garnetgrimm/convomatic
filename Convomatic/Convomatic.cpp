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

void convolve(double *g, double *h, double *x, int lenH, int lenX)
{
	int nconv = lenH + lenX - 1;
	int i, j, h_start, x_start, x_end;

	for (i = 0; i < min(200000,nconv); i++)
	{
		if (i % 1000 == 0) {
			cout << i << " out of " << nconv << endl;
		}
		x_start = max(0, i - lenH + 1);
		x_end = min(i + 1, lenX);
		h_start = min(i, lenH - 1);
		for (j = x_start; j < x_end; j++)
		{
			g[i] += h[h_start--] * x[j];
		}
	}
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
	outputFile.setNumSamplesPerChannel(inputFile.getNumSamplesPerChannel());
	outputFile.setNumChannels(1);

	auto start = high_resolution_clock::now();
	double* result;
	double* startInput = &inputFile.samples[channel][0];
	double* startImpulse = &impulseFile.samples[channel][0];
	result = (double*)calloc(inputFile.getNumSamplesPerChannel(), sizeof(double));
	convolve(result, startImpulse, startInput, impulseFile.getNumSamplesPerChannel(), inputFile.getNumSamplesPerChannel());
	
	
	//convolve(result, startInput, startImpulse, inputFile.getNumSamplesPerChannel(), impulseFile.getNumSamplesPerChannel());
	//memcpy(&outputFile.samples[channel][0], &result, sizeof(double) * inputFile.getNumSamplesPerChannel());
	
	//WORKING
	for (int i = 0; i < inputFile.getNumSamplesPerChannel(); i++) {
		outputFile.samples[channel][i] = result[i] * 0.15;
	}

	/*
	for (int i = 0; i < 5000; i++)
	{
		int inputSamplesLeft = inputFile.getNumSamplesPerChannel() - i;
		int impulseSampleCount = impulseFile.getNumSamplesPerChannel();
		if (i % 1000 == 0) {
			cout << i << " out of " << inputFile.getNumSamplesPerChannel() << endl;
		}
		for (int impulseIdx = 0; impulseIdx < std::min(inputSamplesLeft, impulseSampleCount); impulseIdx++) {
			double result = inputFile.samples[channel][i] * impulseFile.samples[channel][impulseIdx];
			outputFile.samples[channel][i + impulseIdx] += result;
		}
	}
	*/
	auto end = high_resolution_clock::now();
	double durr = std::chrono::duration<double>(end - start).count();
	printf("Took %f seconds (%f per each sample)\r\n", durr, (inputFile.getNumSamplesPerChannel()) / durr);
	outputFile.save("res/impulse-input.wav", AudioFileFormat::Wave);

	//graphFFT visualFFT;
	//visualFFT.drawGraph((char*)"res/440Hz.wav");
	return 0;
}