#include "ofApp.h"

//--------------------------------------------------------------
constexpr void ofApp::fillWavetable() {
	for (int a = 0; a < wavetableSize; a++) {
		wavetable[a] = sin(TWO_PI * (double)a / wavetableSize);
	}
}

void ofApp::setup() {
	fillWavetable();
	startPan = sqrt(0.5);
	for (int a = 0; a < dataBits; a++) {
		phaseIncrements[a] = pow(0.5, a + 2);
		for (int b = 0; b < 2; b++) {
			oscillators[a][0][b] = phaseIncrements[a];
			phasePan[a][b] = startPan;
			pan[a][b] = startPan;
		}
	}
	shader.load("remoteInteraction");
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

inline double ofApp::averageTwo(double inA, double inB, double mix) {
	return (1.0 - mix) * inA + (inB * mix);
}

inline double ofApp::triangle(double phase) {
	return 1.0 - (2.0 * abs(phase - 0.5));
}

double ofApp::lookup(double phase) {
	double floatIndex = phase * (double)wavetableSize;
	double remainderIndex = fmod(floatIndex, 1.0);
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
				recipriocalTimers[b][c] = 1.0 / timers[b][c];
				increments[b][c] *= 1.0 - increments[b][c];
				parameters[b][c][0] = averageTwo(increments[b][c], averageIncrements[b][c], 1.0 - recipriocalTimers[b][c]);
				parameters[b][c][1] += parameters[b][c][0];
				parameters[b][c][1] = fmod(parameters[b][c][1], 1.0);
				parameters[b][c][2] = triangle(parameters[b][c][1]);
				if (b < 2) {
					parameters[b][c][2] *= abs(parameters[b][c][2] - 0.5) * 2.0;
				}
				else {
					parameters[b][c][2] *= parameters[b][c][2];
				}
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
					for (int d = 0; d < 2; d++) {
						amplitudes[c][d] *= pow(1.0 - (parameters[d][c][0] * parameters[2 + d][c][0]), amplitudes[c][d]);
					}
					for (int d = 0; d < channels; d++) {
						oscillators[c][0][d] += phaseIncrements[c] * abs(oscillators[c][0][d] - phaseIncrements[c]) * ofRandomf() * parameters[2][c][2] * amplitudes[c][0] / (1.0 - phaseIncrements[c]);
						oscillators[c][0][d] = fmod(abs(oscillators[c][0][d]), 1.0);
						oscillators[c][1][d] += pow(oscillators[c][0][d], c + 1);
						oscillators[c][1][d] = fmod(oscillators[c][1][d], 1.0);
						sample[d] += lookup(oscillators[c][1][d]) * pan[c][d] * parameters[3][c][2] * amplitudes[c][1] / 8.0;
					}
				}
			}
		}
		amplitude *= (1.0 - (1.0 / timer)) * ofClamp(timer / time, 0.0, 1.0);
		for (int b = 0; b < channels; b++){
			sample[b] *= amplitude;
			highPass = averageTwo(sample[b], -1.0 * lastSample[b], filter[b]);
			lowPass = averageTwo(sample[b], lastSample[b], filter[b + 2]);
			sample[b] = averageTwo(highPass, lowPass, 0.5);
			lastSample[b] = sample[b];
			soundBuffer[a * channels + b] = sample[b];
			sample[b] = 0.0;
		}
	}
}

void ofApp::setUniforms() {
	shader.setUniform2f("window", window);
	shader.setUniform1f("amplitude", amplitude);
	for (int a = 0; a < dataBits; a++) {
		for (int b = 0; b < 4; b++) {
			int vecIndex = a * 4;
			parameterValues[vecIndex + b] = (float)parameters[b][a][0];
			parameterValues[vecIndex + 32 + b] = (float)parameters[b][a][2];
		}
	}
	shader.setUniform4fv("parameterValues", parameterValues, 16);
	shader.setUniform4f("filterVec", filterVec);
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
		time = timer;
		recipriocalTime = 1.0 / time;
		amplitude = 1.0;
	}
	if (key > 47 && key < 59) {
		number *= 10;
		number += key - 48;
	}
}

void ofApp::updateState(int number, int position) {
	cout << number << " " << position << endl;
	number %= 256;
	position %= 4;
	lastValues[position] = values[position];
	values[position] = number;
	differenceValues[position] = 1.0 / (double)(abs(lastValues[position] - values[position]) + 1);
	if (position < 2) {
		filter[0] = sqrt(sqrt(1.0 - differenceValues[0]) * differenceValues[2]);
		filter[1] = sqrt(sqrt(differenceValues[0]) * differenceValues[2]);
	}
	else {
		filter[2] = sqrt(sqrt(1.0 - differenceValues[1]) * differenceValues[3]);
		filter[3] = sqrt(sqrt(differenceValues[1]) * differenceValues[3]);
	}
	filterVec.set((float)filter[0], (float)filter[1], (float)filter[2], (float)filter[3]);
	lastBits[position] = bits[position];
	bits[position] = bitset<dataBits>(number);
	for (int a = 0; a < dataBits; a++) {
		if (lastBits[position][a] != bits[position][a]) {
			if (position == 3) {
				amplitudes[a][0] = 1.0;
				amplitudes[a][1] = 1.0;
			}
			increments[position][a] = (oscillators[a][0][0] * oscillators[a][0][1]) * pow(recipriocalTimers[position][a], parameters[position][a][2] * (1.0 - recipriocalTime) + recipriocalTime);
			parameters[position][a][0] = increments[position][a];
			timers[position][a] = 0.0;
			changes[position][a]++;
			averageIncrements[position][a] = (averageIncrements[position][a] * (changes[position][a] - 1.0) + increments[position][a]) / changes[position][a];
		}
	}
}

//--------------------------------------------------------------