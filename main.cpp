#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	ofGLFWWindowSettings settings;
	settings.setGLVersion(3, 2); //we define the OpenGL version we want to use
	settings.setSize(1024, 768);
	ofCreateWindow(settings);
	ofSetFullscreen(false);
	ofRunApp(new ofApp());
}