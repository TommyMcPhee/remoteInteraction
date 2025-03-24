#include "ofApp.h"

//--------------------------------------------------------------
constexpr void ofApp::fillWavetable() {
	for (int a = 0; a < wavetableSize; a++) {
		wavetable[a] = sin(TWO_PI * (float)a / wavetableSize);
	}
}

void ofApp::setup() {
	fillWavetable();
	minimumFloat = std::numeric_limits<float>::min();
	for (int a = 0; a < dataBits; a++) {
		for (int b = 0; b < 2; b++) {
			pan[a][b] = sqrt(0.5);
		}
	}
	shader.load("remoteAccess");
	frameBuffer.allocate(ofGetScreenWidth(), ofGetScreenHeight());
	frameBuffer.begin();
	ofClear(0, 0, 0, 255);
	frameBuffer.end();
	shader.begin();
	shader.end();
	streamSettings.setOutListener(this);
	streamSettings.setApi(ofSoundDevice::Api::MS_WASAPI);
	streamSettings.sampleRate = sampleRate;
	streamSettings.bufferSize = bufferSize;
	streamSettings.numOutputChannels = channels;
	stream.setup(streamSettings);
}

void ofApp::ofSoundStreamSetup(ofSoundStreamSettings& settings) {

}

inline float ofApp::averageTwo(float inA, float inB, float mix) {
	return (1.0 - mix) * inA + (inB * mix);
}

inline float ofApp::triangle(float phase, float skew) {
	//IS SKEW NEEDED??
	return (1.0 - abs(phase - skew)) * (2.0 - (abs(skew - 0.5) * 2.0));
}

inline float ofApp::lookup(float phase) {
	float floatIndex = phase * (float)wavetableSize;
	float remainderIndex = fmod(floatIndex, 1.0);
	int intIndex = (int)floatIndex;
	if (intIndex >= wavetableSize - 1) {
		intIndex = wavetableSize - 2;
	}
	return averageTwo(wavetable[intIndex], wavetable[intIndex + 1], remainderIndex);
}

void ofApp::audioOut(ofSoundBuffer& soundBuffer) {
	for (int a = 0; a < soundBuffer.getNumFrames(); a++) {
		timer++;
		for (int b = 0; b < 4; b++) {
			for (int c = 0; c < dataBits; c++) {
				timers[b][c]++;
				parameters[b][c][0] = averageTwo(increments[b][c], averageIncrements[b][c], recipriocalTime);
				parameters[b][c][1] += parameters[b][c][0];
				parameters[b][c][1] = fmod(parameters[b][c][1], 1.0);
				parameters[b][c][2] = lookup(parameters[b][c][1]);
				switch (b) {
				case 0:
					phasePan[c][0] = sqrt(1.0 - parameters[b][c][2]);
					phasePan[c][1] = sqrt(parameters[b][c][2]);
					break;
				case 1:
					pan[c][0] = sqrt(1.0 - parameters[b][c][2]);
					pan[c][1] = sqrt(parameters[b][c][2]);
					break;
				case 3:
					for (int d = 0; d < channels; d++) {
						//FIX
						oscillators[c][0][d] = 0.25 * phasePan[c][d];
						//
						oscillators[c][1][d] += pow(oscillators[c][0][d], c + 1);
						oscillators[c][1][d] = fmod(oscillators[c][0][d], 1.0);
						//sample[d] += triangle(oscillators[c][1][d], 0.5) * pan[c][d] * parameters[3][c][2] / 8.0;
						sample[d] += lookup(oscillators[c][1][d]) * parameters[3][c][2] / 8.0;
					}
				}
			}
		}
		for (int b = 0; b < channels; b++) {
			//revisit amplitude
			soundBuffer[a * channels + b] = sample[b] * std::clamp(1.0 - (timer / time), 0.0, 1.0);
			//cout << sample[b] << endl;
			sample[b] = 0.0;
		}
	}
}

void ofApp::setUniforms() {
	shader.setUniform2f("window", window);
}

//--------------------------------------------------------------
void ofApp::draw() {
	width = (float)ofGetWidth();
	height = (float)ofGetHeight();
	window.set(width, height);
	frameBuffer.allocate(width, height);
	frameBuffer.begin();
	ofClear(0, 0, 0, 255);
	shader.begin();
	setUniforms();
	frameBuffer.draw(0.0, 0.0);
	shader.end();
	frameBuffer.end();
	frameBuffer.draw(0.0, 0.0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 44) {
		updateState(number, position);
		number = 0;
		position++;
	}
	if (key == 41) {
		updateState(number, position);
		number = 0;
		position = 0;
		if (timer > time) {
			time = timer;
			recipriocalTime = 1.0 / time;
			timer = 0;
		}
	}
	if (key > 47 && key < 59) {
		number *= 10;
		number += key - 48;
	}
}

void ofApp::updateState(int number, int position) {
	//cout << number << " " << position << endl;
	number %= 256;
	position %= 4;
	lastBits[position] = bits[position];
	bits[position] = bitset<dataBits>(number);
	for (int a = 0; a < 8; a++) {
		if (lastBits[position][a] != bits[position][a]) {
			increments[position][a] = 0.001;
			//increments[position][a] = 1.0 / (timers[position][a] + minimumFloat);
			parameters[position][a][0] = increments[position][a];
			timers[position][a] = 0.0;
			changes[position][a]++;
			averageIncrements[position][a] = (averageIncrements[position][a] * (changes[position][a] - 1.0) + increments[position][a]) / changes[position][a];
		}
	}
	values[number][position] += 1;
}

//--------------------------------------------------------------