#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp {

public:
	static const int wavetableSize = 512, dataBits = 8, channels = 2, sampleRate = 48000, bufferSize = 256;
	float minimumFloat, maxRoot, timer = 0.0, time = 1.0, recipriocalTime = 1.0 / time, width, height;
	void setup();
	array<float, wavetableSize> wavetable;
	constexpr void fillWavetable();
	ofSoundStreamSettings streamSettings;
	ofSoundStream stream;
	void ofSoundStreamSetup(ofSoundStreamSettings& settings);
	int number = 0, position = 0;
	//values?
	array<array<int, 4>, 256> values;
	array<bitset<dataBits>, 4> lastBits;
	array<bitset<dataBits>, 4> bits;
	array<array<float, dataBits>, 4> timers;
	array<array<float, dataBits>, 4> increments;
	array<array<float, dataBits>, 4> changes;
	array<array<float, dataBits>, 4> averageIncrements;
	// averageTwo?
	inline float averageTwo(float inA, float inB, float mix);
	inline float triangle(float phase, float skew);
	inline float lookup(float phase);
	array<array<array<float, 3>, dataBits>, 4> parameters;
	// samples?
	array<array<float, channels>, dataBits> phasePan, pan;
	array<array<array<float, channels>, 2>, dataBits> oscillators;
	// lastSample?
	array<float, channels> sample;

	void audioOut(ofSoundBuffer& soundBuffer);
	ofVec2f window;
	//data?
	float data[256 * 4];
	ofFbo frameBuffer;
	ofShader shader;
	void setUniforms();
	void draw();
	void updateState(int number, int position);
	void keyPressed(int key);
};