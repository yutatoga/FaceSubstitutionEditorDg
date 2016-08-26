#include "ofApp.h"

using namespace ofxCv;

const int ofApp::lines [] = {22,27,27,21,21,22,22,23,23,21,21,20,20,23,23,24,24,25,25,26,26,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,17,17,18,18,19,19,20,27,28,28,29,29,30,30,31,30,32,30,33,30,34,30,35,35,34,34,33,33,32,32,31,31,48,31,49,31,50,32,50,33,50,33,51,33,52,34,52,35,52,35,53,35,54,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,48,48,60,60,61,61,62,62,54,54,63,63,64,64,65,65,48,49,60,60,50,50,61,61,51,61,52,52,62,62,53,55,63,63,56,56,64,64,57,64,58,58,65,65,59,36,37,37,38,38,39,39,40,40,41,41,36,42,43,43,44,44,45,45,46,46,47,47,42,27,42,42,22,42,23,43,23,43,24,43,25,44,25,44,26,45,26,45,16,45,15,46,15,46,14,47,14,29,47,47,28,28,42,27,39,39,21,39,20,38,20,38,19,38,18,37,18,37,17,36,17,36,0,36,1,41,1,41,2,40,2,2,29,29,40,40,28,28,39,29,31,31,3,3,29,29,14,14,35,35,29,3,48,48,4,48,6,6,59,59,7,7,58,58,8,8,57,8,56,56,9,9,55,55,10,10,54,54,11,54,12,54,13,13,35};

ofVec2f ofApp::rotateCoord(ofVec2f p,float rad){
    return ofVec2f(p.x*cos(rad)-p.y*sin(rad), p.x*sin(rad)+p.y*cos(rad));
}

void ofApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;
    cam.initGrabber(camWidth, camHeight);
    substitutionWidth = camWidth;
    substitutionHeight = camHeight;
	clone.setup(camWidth, camHeight);
	ofFbo::Settings settings;
	settings.width = camWidth;
	settings.height = camHeight;
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
    // decolation
    decolationMaskFbo.allocate(settings);
    decolationFbo.allocate(settings);
    
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
	selectArea = false;
    
    // gui
    showGui = true;
    panel.setup();
    panel.add(showCamMeshWireFrame.set("showCamMeshWireFrame", false));
    panel.add(enableBlurMix.set("enableBlurMix", false));
    panel.add(enableEvent.set("enableEvent", false));
    panel.add(mixStrength.set("mixStrength", 50, 0, 500));
    panel.add(substitutionCamScale.set("substitutionCamScale", 2, 1, 10));
    panel.add(substitutionSrcScale.set("substitutionSrcScale", 1, 1, 10));

    // image
    fixedFrontImage.load("logo.png");
    
    // event
    timeFaceDetection = 0;
    didEvent = false;
    player.load("sound.mp3");
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
        ofImage mirroredCam;
        mirroredCam.setFromPixels(cam.getPixels());
        mirroredCam.mirror(false, true);
		camTracker.update(toCv(mirroredCam));
		cloneReady = camTracker.getFound();
		if(cloneReady) {
            // event
            if (enableEvent){
                if (timeFaceDetection == 0) {
                    timeFaceDetection = ofGetElapsedTimef();
                } else if (ofGetElapsedTimef() - timeFaceDetection > 3) {
                    if (didEvent == false) {
                        // do event
                        enableBlurMix = false;
                        player.play();
                        
                        didEvent = true;
                    }
                }
            }
            
            // cam
            camMesh = camTracker.getImageMesh();
            camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
            ofVec2f camFaceCenter = camTracker.getPosition();
            float faceScale = camTracker.getScale();
            faceScale = ofClamp(faceScale, 0, 10);
            substitutionWidth = ofMap(faceScale, 0, 10, 0, camWidth*substitutionCamScale);
            substitutionHeight = ofMap(faceScale, 0, 10, 0, camHeight*substitutionCamScale);
            
            ofVec2f camZero = ofVec2f(-substitutionWidth/2.0, -substitutionHeight/2.0); // for rotation
            // - step
            float camStepUpperSide = substitutionWidth/8.0f;
            float camStepRightSide, camStepLeftSide;
            camStepRightSide = camStepLeftSide = substitutionHeight/5.0f;
            float camStepLowerSide = substitutionWidth/7.0f;
            
            // src
            ofVec2f srcFaceCenter = srcTracker.getPosition();
            float srcWidth = camWidth * substitutionSrcScale; // src.getWidth();
            float srcHeight = camHeight * substitutionSrcScale; // src.getHeight();
            ofVec2f srcZero = ofVec2f(srcFaceCenter.x-srcWidth/2.0f, srcFaceCenter.y-srcHeight/2.0f);
            float srcStepUpperSide = srcWidth/8.0f;
            float srcStepRightSide, srcStepLeftSide;
            // - step
            srcStepRightSide = srcStepLeftSide = srcHeight/5.0f;
            float srcStepLowerSide = srcWidth/7.0f;
            
            // face angle for rotation
            float faceAngle = camTracker.getOrientation().z;

            // vertices
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide*4, camZero.y), faceAngle)  + camFaceCenter);                   // 66 - upper center
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide*5, camZero.y), faceAngle) + camFaceCenter);                    // 67
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide*6, camZero.y), faceAngle) + camFaceCenter);                    // 68
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide*7, camZero.y), faceAngle) + camFaceCenter);                    // 69
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+substitutionWidth, camZero.y) , faceAngle) + camFaceCenter);                    // 70 - upper right corner
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+substitutionWidth, camZero.y+camStepRightSide), faceAngle) + camFaceCenter);    // 71
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+substitutionWidth, camZero.y+camStepRightSide*2), faceAngle) + camFaceCenter);  // 72
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+substitutionWidth, camZero.y+camStepRightSide*3), faceAngle) + camFaceCenter);  // 73
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+substitutionWidth, camZero.y+camStepRightSide*4), faceAngle) + camFaceCenter);  // 74
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+substitutionWidth, camZero.y+substitutionHeight) , faceAngle) + camFaceCenter); // 75 - lower right corner
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepLowerSide*6, camZero.y+substitutionHeight), faceAngle) + camFaceCenter); // 76
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepLowerSide*5, camZero.y+substitutionHeight), faceAngle) + camFaceCenter); // 77
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepLowerSide*4, camZero.y+substitutionHeight), faceAngle) + camFaceCenter); // 78
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepLowerSide*3, camZero.y+substitutionHeight), faceAngle) + camFaceCenter); // 79
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepLowerSide*2, camZero.y+substitutionHeight), faceAngle) + camFaceCenter); // 80
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepLowerSide, camZero.y+substitutionHeight), faceAngle) + camFaceCenter);   // 81
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x, camZero.y+substitutionHeight) , faceAngle) + camFaceCenter);                   // 82 - lower left corner
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x, camZero.y+camStepLeftSide*4), faceAngle) + camFaceCenter);                     // 83
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x, camZero.y+camStepLeftSide*3), faceAngle) + camFaceCenter);                     // 84
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x, camZero.y+camStepLeftSide*2), faceAngle) + camFaceCenter);                     // 85
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x, camZero.y+camStepLeftSide), faceAngle) + camFaceCenter);                       // 86
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x, camZero.y), faceAngle) + camFaceCenter);                                       // 87 - upper left corner
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide, camZero.y), faceAngle) + camFaceCenter);                      // 88
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide*2, camZero.y), faceAngle) + camFaceCenter);                    // 89
            camMesh.addVertex(rotateCoord(ofVec2f(camZero.x+camStepUpperSide*3, camZero.y), faceAngle) + camFaceCenter);                    // 90

            // coords
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*4, srcZero.y));          // 66 - upper center
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*5, srcZero.y));          // 67
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*6, srcZero.y));          // 68
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*7, srcZero.y));          // 69
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y));                    // 70 - upper right corner
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide));   // 71
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide*2)); // 72
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide*3)); // 73
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide*4)); // 74
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcHeight));          // 75 - lower right corner
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*6, srcZero.y+srcHeight));// 76
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*5, srcZero.y+srcHeight));// 77
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*4, srcZero.y+srcHeight));// 78
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*3, srcZero.y+srcHeight));// 79
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*2, srcZero.y+srcHeight));// 80
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide, srcZero.y+srcHeight));  // 81
            camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcHeight));                   // 82 - lower left corner
            camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide*4));           // 83
            camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide*3));           // 84
            camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide*2));           // 85
            camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide));             // 86
            camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y));                             // 87 - upper left corner
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide, srcZero.y));            // 88
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*2, srcZero.y));          // 89
            camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*3, srcZero.y));          // 90
            
            // extra indices
            vector<ofIndexType> extraIndices
            {
                66,20,23,
                66,23,67,
                67,23,24,
                67,24,68,
                68,24,25,
                68,25,69,
                69,25,26,
                69,26,70,
                70,26,16,
                70,16,71,
                71,16,15,
                71,15,72,
                72,15,14,
                72,14,73,
                73,14,13,
                73,13,74,
                74,13,12,
                74,12,75,
                75,12,11,
                75,11,76,
                76,10,11,
                77,10,76,
                77,9,10,
                78,9,77,
                78,8,9,
                79,8,78,
                79,7,8,
                80,7,79,
                80,6,7,
                81,6,80,
                81,5,6,
                82,5,81,
                82,5,4,
                82,4,83,
                83,4,3,
                84,83,3,
                84,3,2,
                85,84,2,
                85,2,1,
                86,85,1,
                86,1,0,
                87,86,0,
                87,0,17,
                88,87,17,
                88,17,18,
                89,88,18,
                89,18,19,
                90,89,19,
                90,19,20,
                66,90,20
            };
            camMesh.addIndices(extraIndices);
			
            // mask for decolation
            decolationMaskFbo.begin();
            ofClear(0, 0, 0, 0);
            // mask path
            ofPath maskPath;
            maskPath.lineTo(0, 0);
            maskPath.lineTo(camWidth, 0);
            maskPath.lineTo(camWidth, camHeight);
            maskPath.lineTo(0, camHeight);
            maskPath.lineTo(0, 0);
            maskPath.lineTo(camMesh.getVertex(0));
            maskPath.lineTo(camMesh.getVertex(17));
            maskPath.lineTo(camMesh.getVertex(18));
            maskPath.lineTo(camMesh.getVertex(19));
            maskPath.lineTo(camMesh.getVertex(20));
            maskPath.lineTo(camMesh.getVertex(23));
            maskPath.lineTo(camMesh.getVertex(24));
            maskPath.lineTo(camMesh.getVertex(25));
            maskPath.lineTo(camMesh.getVertex(26));
            maskPath.lineTo(camMesh.getVertex(16));
            maskPath.lineTo(camMesh.getVertex(15));
            maskPath.lineTo(camMesh.getVertex(14));
            maskPath.lineTo(camMesh.getVertex(13));
            maskPath.lineTo(camMesh.getVertex(12));
            maskPath.lineTo(camMesh.getVertex(11));
            maskPath.lineTo(camMesh.getVertex(10));
            maskPath.lineTo(camMesh.getVertex(9));
            maskPath.lineTo(camMesh.getVertex(8));
            maskPath.lineTo(camMesh.getVertex(7));
            maskPath.lineTo(camMesh.getVertex(6));
            maskPath.lineTo(camMesh.getVertex(5));
            maskPath.lineTo(camMesh.getVertex(4));
            maskPath.lineTo(camMesh.getVertex(3));
            maskPath.lineTo(camMesh.getVertex(2));
            maskPath.lineTo(camMesh.getVertex(1));
            maskPath.lineTo(camMesh.getVertex(0));
            maskPath.draw();
            decolationMaskFbo.end();
            
            // decolation
            decolationFbo.begin();
            ofClear(0, 0, 0, 0);
            // comic effect
            float radius = max(camWidth, camHeight)*2;
            for (float i = 0; i < TWO_PI; i+=0.2) {
                ofDrawTriangle(camTracker.getPosition().x, camTracker.getPosition().y,
                               camWidth/2.0f+radius*cos(i), camHeight/2.0f+radius*sin(i),
                               camWidth/2.0f+radius*cos(i)+50, camHeight/2.0f+radius*sin(i)+50);
            }
            decolationFbo.end();
            
            if (enableBlurMix) {
                maskFbo.begin();
                ofClear(0, 255);
                camMesh.draw();
                maskFbo.end();
                
                srcFbo.begin();
                ofClear(0, 255);
                src.bind();
                camMesh.draw();
                src.unbind();
                srcFbo.end();
                
                clone.setStrength(mixStrength); // how much mix the substitution
                clone.update(srcFbo.getTexture(),
                             mirroredCam.getTexture(),
                             maskFbo.getTexture());
            }
		}
	}
}

void ofApp::draw() {
	ofSetColor(255);
	
	int xOffset = cam.getWidth();
	
	if(src.getWidth() > 0 && cloneReady) {
        if (enableBlurMix) {
            // 1. mix with blur
            clone.draw(0, 0);
        } else {
            // 2. not using blur
            // draw webcam
            cam.draw(camWidth, 0, -camWidth, camHeight); // mirrored
            
            // decolation
            decolationFbo.getTexture().setAlphaMask(decolationMaskFbo.getTexture());
            decolationFbo.draw(0, 0);
            
            // draw substitution
            src.bind();
            camMesh.draw();
            src.unbind();
        }
	} else {
		cam.draw(camWidth, 0, -camWidth, camHeight); // mirrored
	}
	
	if(!camTracker.getFound()) {
		ofDrawBitmapStringHighlight("camera face not found", 5, cam.getHeight()-5);

        // event
        if (enableEvent) {
            timeFaceDetection = 0;
            enableBlurMix = true;
            didEvent = false;
        }
	}
	if(src.getWidth() == 0) {
		ofDrawBitmapStringHighlight("drag an image here", ofGetWidth()*0.75 - 50, ofGetHeight()/2.0f);
	}
	
	if (src.getWidth() > 0) {
		src.draw(xOffset, 0);
	}
	
	if (srcPoints.size() > 0) {
		for (int i = 0; i < sizeof(lines) / sizeof(int) - 1; i += 2) {
			ofVec2f p0 = srcPoints[lines[i]];
			ofVec2f p1 = srcPoints[lines[i+1]];
			ofDrawLine(xOffset + p0[0], p0[1], xOffset + p1[0], p1[1]);
		}
	}
	
	ofFill();
	for (int i = 0; i < srcPoints.size(); i++) {
		ofVec2f p = srcPoints[i];
		
		ofSetColor(255,128,0);
		ofDrawCircle(xOffset + p[0], p[1], 6);
		
		ofSetColor(255,255,255);
		ofDrawCircle(xOffset + p[0], p[1], 4);
	}
	
	for (int i = 0; i < selectedPoints.size(); i++) {
		ofVec2f p = srcPoints[selectedPoints[i]];
		
		ofSetColor(255,255,255);
		ofDrawCircle(xOffset + p[0], p[1], 6);
		
		ofSetColor(255,128,0);
		ofDrawCircle(xOffset + p[0], p[1], 4);
	}
	ofSetColor(255,255,255);
	
	if (selectArea) {
		int startX = selectAreaStart[0];
		int startY = selectAreaStart[1];
		
		ofNoFill();
		ofSetColor(255, 255, 255);
		ofDrawRectangle(startX, startY, mouseX - startX, mouseY - startY);
	}
    
    // image
    fixedFrontImage.draw(0, 0);
    
    //debug
    if (showCamMeshWireFrame) camMesh.drawWireframe();
    stringstream ss;
    ss << "framerate: " << ofToString(ofGetFrameRate(), 0);
    ofDrawBitmapString(ss.str(), 5, 10);
    
    // gui
    if (showGui) panel.draw();
}

void ofApp::loadPoints(string filename) {
	ofFile file;
	file.open(ofToDataPath(filename), ofFile::ReadWrite, false);

    ofBuffer buff = file.readToBuffer();
    ofBuffer::Lines lines = buff.getLines();
    ofBuffer::Line iter = lines.begin();
    
    srcPoints = vector<ofVec2f>();
    while (iter != lines.end()) {
        string line = *iter;
        vector<string> tokens = ofSplitString(line, "\t");
        srcPoints.push_back(ofVec2f(ofToFloat(tokens[0]), ofToFloat(tokens[1])));
    }
	cout << "Read " << filename << "." << endl;
}

void ofApp::loadFace(string filename){
	src.load(filename);
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	for (int i = 0; i < dragInfo.files.size(); i++) {
		string filename = dragInfo.files[i];
		vector<string> tokens = ofSplitString(filename, ".");
		string extension = tokens[tokens.size() - 1];
		if (extension == "tsv") {
			loadPoints(filename);
		}
		else {
			loadFace(filename);
		}
	}
}

void ofApp::mouseMoved(int x, int y ) {
	
}

void ofApp::mouseDragged(int x, int y, int button) {
	int xOffset = cam.getWidth();
	if (x < xOffset) {
		
	}
	else {
		x -= xOffset;
		
		if (button == 0) {
			for (int i = 0; i < dragPoints.size(); i++) {
				ofVec2f d = dragPointsToMouse[i];
				srcPoints[dragPoints[i]].set(x + d[0], y + d[1]);
			}
		}
	}
}

void ofApp::mousePressed(int x, int y, int button) {
	mousePressedTime = ofGetSystemTime();
	dragPointsToMouse.clear();
	
	int xOffset = cam.getWidth();
	if (x < xOffset) {
		
	}
	else {
		x -= xOffset;
		
		float nearestDsq = std::numeric_limits<float>::max();
		int nearestIndex = -1;
		for (int i = 0; i < srcPoints.size(); i++) {
			float dx = srcPoints[i][0] - x;
			float dy = srcPoints[i][1] - y;
			float dsq = dx * dx + dy * dy;
			if (dsq < nearestDsq) {
				nearestDsq = dsq;
				nearestIndex = i;
			}
		}
		
		if (nearestDsq < 25) {
			if (find(selectedPoints.begin(), selectedPoints.end(), nearestIndex) != selectedPoints.end()) {
				// If the user pressed a selected point then drag the selected points.
				dragPoints = selectedPoints;
				for (int i = 0; i < dragPoints.size(); i++) {
					ofVec2f p = srcPoints[selectedPoints[i]];
					dragPointsToMouse.push_back(ofVec2f(p[0] - x, p[1] - y));
				}
			}
			else {
				// If the user pressed an unselected point then drag it and ignore the selection.
				ofVec2f p = srcPoints[nearestIndex];
				dragPoints.push_back(nearestIndex);
				dragPointsToMouse.push_back(ofVec2f(p[0] - x, p[1] - y));
			}
		}
		else {
			selectAreaStart.set(xOffset + x, y);
			selectArea = true;
		}
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	int xOffset = cam.getWidth();
	
	if (selectArea) {
		selectedPoints.clear();
		ofRectangle r = ofRectangle(fmin(selectAreaStart[0], x) - xOffset,
									fmin(selectAreaStart[1], y),
									abs(selectAreaStart[0] - x),
									abs(selectAreaStart[1] - y));
		for (int i = 0; i < srcPoints.size(); i++) {
			ofVec2f p = srcPoints[i];
			if (r.x < p.x && p.x < r.x + r.width
				&& r.y < p.y && p.y < r.y + r.height) {
				selectedPoints.push_back(i);
			}
		}
	}
	
	if (x < xOffset) {
	}
	else {
		x -= xOffset;
		
		if (ofGetSystemTime() - mousePressedTime < 300) {
			float nearestDsq = std::numeric_limits<float>::max();
			int nearestIndex = -1;
			for (int i = 0; i < srcPoints.size(); i++) {
				float dx = srcPoints[i][0] - x;
				float dy = srcPoints[i][1] - y;
				float dsq = dx * dx + dy * dy;
				if (dsq < nearestDsq) {
					nearestDsq = dsq;
					nearestIndex = i;
				}
			}
			
			if (nearestDsq < 25) {
				if (find(selectedPoints.begin(), selectedPoints.end(), nearestIndex) == selectedPoints.end()) {
					selectedPoints.clear();
					selectedPoints.push_back(nearestIndex);
				}
				else {
					selectedPoints.erase(std::remove(selectedPoints.begin(), selectedPoints.end(), nearestIndex), selectedPoints.end());
				}
			}
			else {
				selectedPoints.clear();
			}	
		}
	}
	
	selectArea = false;
	dragPoints.clear();
}

void ofApp::keyPressed(int key) {
	ofFile file;
	ofBuffer buff;
	
	switch (key) {
        case 'h': // hide or show gui
            showGui = !showGui;
            break;
		case 'q': // Read point locations from source image.
			if(src.getWidth() > 0) {
                ofImage graySrc = src;
                graySrc.setImageType(ofImageType::OF_IMAGE_COLOR);
				srcTracker.update(toCv(graySrc));
				srcPoints = srcTracker.getImagePoints();
			}
			cout << "Calculated points from source image." << endl;
			break;
		
		case 'r': // Read points from points.tsv.
			loadPoints("points.tsv");
			break;
			
		case 's': // Save points to points.tsv.
			if (srcPoints.size() > 0) {
				ofBuffer points;
				string header = "x\ty\n";
				points.append(header.c_str(), header.size());
				
				for (int i = 0; i < srcPoints.size(); i++) {
					string srcPoint = ofToString(srcPoints[i][0]) + "\t" + ofToString(srcPoints[i][1]) + "\n";
					points.append(srcPoint.c_str(), srcPoint.size());
				}
				
				bool wrote = ofBufferToFile("points.tsv", points);
				cout << "Wrote points.tsv." << endl;
			}
			break;
			
		case 'c': // Clear the selection.
			selectedPoints.clear();
			break;
	}
}

void ofApp::keyReleased(int key) {
}
