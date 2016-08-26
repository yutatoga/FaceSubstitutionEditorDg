#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "Clone.h"
#include "ofxFaceTracker.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofxGui.h"

#include <limits>

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void dragEvent(ofDragInfo dragInfo);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
	void loadPoints(string filename);
	void loadFace(string filename);
	
	void keyPressed(int key);
	void keyReleased(int key);

    ofVec2f rotateCoord(ofVec2f p,float rad);
    
    
    
	ofxFaceTrackerThreaded camTracker;
    ofVideoGrabber cam;
    const float camWidth = 640;
    const float camHeight = 480;
    float substitutionWidth;
    float substitutionHeight;

	ofxFaceTracker srcTracker;
	ofImage src;
	vector<ofVec2f> srcPoints;
	vector<int> selectedPoints;
	vector<int> dragPoints;
	vector<ofVec2f> dragPointsToMouse;
	
	bool selectArea;
	ofVec2f selectAreaStart;
	
	bool cloneReady;
	Clone clone;
	ofFbo srcFbo, maskFbo, mainViewFbo;
    // decolationFbo is the layer which is front of webcam and back from face, which means decolation without covering eye and mouse.
    ofFbo decolationFbo, decolationMaskFbo;
    
	static const int lines [];
	
	long mousePressedTime;

    ofFbo myFbo;
    
    ofMesh  camMesh;
    
    //gui
    bool showGui;
    ofxPanel panel;
    ofParameter<bool> showCamMeshWireFrame;
    ofParameter<bool> enableBlurMix;
    ofParameter<bool> enableEvent;
    ofParameter<bool> enableFullScreenMainView;
    ofParameter<float> mixStrength;
    ofParameter<float> substitutionCamScale;
    ofParameter<float> substitutionSrcScale;
    
    // image
    ofImage fixedFrontImage;
    
    // event
    float timeFaceDetection;
    bool didEvent;
    
    // sound
    ofSoundPlayer player;
};
