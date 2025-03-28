#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {

public:
	static const int wavetableSize = 512, dataBits = 8, channels = 2, sampleRate = 48000, bufferSize = 256;
	float minimumFloat, startPan, timer = 0.0, time = 1.0, recipriocalTime = 1.0 / time, amplitude = 1.0, width, height;
	array<float, dataBits> phaseIncrements;
	void setup();
	array<float, wavetableSize> wavetable;
	constexpr void fillWavetable();
	ofSoundStreamSettings streamSettings;
	ofSoundStream stream;
	void ofSoundStreamSetup(ofSoundStreamSettings& settings);
	int number = 0, position = 0;
	array<bitset<dataBits>, 4> lastBits, bits;
	array<array<float, dataBits>, 4> timers, recipriocalTimers, increments, changes, averageIncrements;
	inline float averageTwo(float inA, float inB, float mix);
	inline float triangle(float phase);
	float lookup(float phase);
	array<array<array<float, 3>, dataBits>, 4> parameters;
	array<array<float, channels>, dataBits> phasePan, pan;
	array<array<float, 2>, dataBits> amplitudes;
	array<array<array<float, channels>, 2>, dataBits> oscillators;
	array<float, channels> sample;
	void audioOut(ofSoundBuffer& soundBuffer);
	ofVec2f window;
	ofFbo frameBuffer;
	ofShader shader;
	void setUniforms();
	void draw();
	void updateState(int number, int position);
	void keyPressed(int key);
};