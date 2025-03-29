#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {

public:
	static const int wavetableSize = 512, dataBits = 8, channels = 2, sampleRate = 48000, bufferSize = 256;
	double minimumFloat, startPan, timer = 0.0, time = 1.0, recipriocalTime = 1.0 / time, amplitude = 1.0, lowPass, highPass, width, height;
	array<double, dataBits> phaseIncrements;
	void setup();
	array<double, wavetableSize> wavetable;
	constexpr void fillWavetable();
	ofSoundStreamSettings streamSettings;
	ofSoundStream stream;
	void ofSoundStreamSetup(ofSoundStreamSettings& settings);
	int number = 0, position = 0;
	array<int, 4> lastValues, values;
	array<double, 4> differenceValues, filter;
	array<bitset<dataBits>, 4> lastBits, bits;
	array<array<double, dataBits>, 4> timers, recipriocalTimers, increments, changes, averageIncrements;
	inline double averageTwo(double inA, double inB, double mix);
	inline double triangle(double phase);
	double lookup(double phase);
	array<array<array<double, 3>, dataBits>, 4> parameters;
	array<array<double, channels>, dataBits> phasePan, pan;
	array<array<double, 2>, dataBits> amplitudes;
	array<array<array<double, channels>, 2>, dataBits> oscillators;
	array<double, channels> lastSample, sample;
	void audioOut(ofSoundBuffer& soundBuffer);
	float parameterValues[64];
	ofVec4f filterVec;
	ofVec2f window;
	ofFbo frameBuffer;
	ofShader shader;
	void setUniforms();
	void draw();
	void updateState(int number, int position);
	void keyPressed(int key);
};