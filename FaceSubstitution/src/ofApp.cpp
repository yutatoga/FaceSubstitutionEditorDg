#include "ofApp.h"

using namespace ofxCv;

const int ofApp::lines [] = {22,27,27,21,21,22,22,23,23,21,21,20,20,23,23,24,24,25,25,26,26,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,17,17,18,18,19,19,20,27,28,28,29,29,30,30,31,30,32,30,33,30,34,30,35,35,34,34,33,33,32,32,31,31,48,31,49,31,50,32,50,33,50,33,51,33,52,34,52,35,52,35,53,35,54,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,48,48,60,60,61,61,62,62,54,54,63,63,64,64,65,65,48,49,60,60,50,50,61,61,51,61,52,52,62,62,53,55,63,63,56,56,64,64,57,64,58,58,65,65,59,36,37,37,38,38,39,39,40,40,41,41,36,42,43,43,44,44,45,45,46,46,47,47,42,27,42,42,22,42,23,43,23,43,24,43,25,44,25,44,26,45,26,45,16,45,15,46,15,46,14,47,14,29,47,47,28,28,42,27,39,39,21,39,20,38,20,38,19,38,18,37,18,37,17,36,17,36,0,36,1,41,1,41,2,40,2,2,29,29,40,40,28,28,39,29,31,31,3,3,29,29,14,14,35,35,29,3,48,48,4,48,6,6,59,59,7,7,58,58,8,8,57,8,56,56,9,9,55,55,10,10,54,54,11,54,12,54,13,13,35};

void ofApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;
	cam.initGrabber(640, 480);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);
	
	selectArea = false;
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
            // cam
            camMesh = camTracker.getImageMesh();
            camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
            
            ofVec2f camFaceCenter = camTracker.getPosition();
            float camWidth = cam.getWidth(); //     default: 640
            float camHeight = cam.getHeight();//    default: 480
            ofVec2f camZero = ofVec2f(camFaceCenter.x-camWidth/2.0f, camFaceCenter.y-camHeight/2.0f);

            // - step
            float camStepUpperSide = camWidth/8.0f;
            float camStepRightSide, camStepLeftSide;
            camStepRightSide = camStepLeftSide = camHeight/5.0f;
            float camStepLowerSide = camWidth/7.0f;
            
            // src
            ofVec2f srcFaceCenter = srcTracker.getPosition();
            float srcWidth = cam.getWidth(); // src.getWidth();
            float srcHeight = cam.getHeight(); // src.getHeight();
            ofVec2f srcZero = ofVec2f(srcFaceCenter.x-srcWidth/2.0f, srcFaceCenter.y-srcHeight/2.0f);
            float srcStepUpperSide = srcWidth/8.0f;
            float srcStepRightSide, srcStepLeftSide;
            // - step
            srcStepRightSide = srcStepLeftSide = srcHeight/5.0f;
            float srcStepLowerSide = srcWidth/7.0f;
            
            // extra vertices and texCoods
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide*4, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*4, srcZero.y));                     // 66 - upper center
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide*5, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*5, srcZero.y));                     // 67
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide*6, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*6, srcZero.y));                     // 68
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide*7, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*7, srcZero.y));                     // 69
            camMesh.addVertex(ofVec2f(camZero.x+camWidth, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y));                                         // 70 - upper right corner
            camMesh.addVertex(ofVec2f(camZero.x+camWidth, camZero.y+camStepRightSide)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide));       // 71
            camMesh.addVertex(ofVec2f(camZero.x+camWidth, camZero.y+camStepRightSide*2)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide*2));   // 72
            camMesh.addVertex(ofVec2f(camZero.x+camWidth, camZero.y+camStepRightSide*3)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide*3));   // 73
            camMesh.addVertex(ofVec2f(camZero.x+camWidth, camZero.y+camStepRightSide*4)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcStepRightSide*4));   // 74
            camMesh.addVertex(ofVec2f(camZero.x+camWidth, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcWidth, srcZero.y+srcHeight));                     // 75 - lower right corner
            camMesh.addVertex(ofVec2f(camZero.x+camStepLowerSide*6, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*6, srcZero.y+srcHeight)); // 76
            camMesh.addVertex(ofVec2f(camZero.x+camStepLowerSide*5, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*5, srcZero.y+srcHeight)); // 77
            camMesh.addVertex(ofVec2f(camZero.x+camStepLowerSide*4, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*4, srcZero.y+srcHeight)); // 78
            camMesh.addVertex(ofVec2f(camZero.x+camStepLowerSide*3, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*3, srcZero.y+srcHeight)); // 79
            camMesh.addVertex(ofVec2f(camZero.x+camStepLowerSide*2, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide*2, srcZero.y+srcHeight)); // 80
            camMesh.addVertex(ofVec2f(camZero.x+camStepLowerSide, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepLowerSide, srcZero.y+srcHeight));     // 81
            camMesh.addVertex(ofVec2f(camZero.x, camZero.y+camHeight)); camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcHeight));                                       // 82 - lower left corner
            camMesh.addVertex(ofVec2f(camZero.x, camZero.y+camStepLeftSide*4)); camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide*4));                       // 83
            camMesh.addVertex(ofVec2f(camZero.x, camZero.y+camStepLeftSide*3)); camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide*3));                       // 84
            camMesh.addVertex(ofVec2f(camZero.x, camZero.y+camStepLeftSide*2)); camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide*2));                       // 85
            camMesh.addVertex(ofVec2f(camZero.x, camZero.y+camStepLeftSide)); camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y+srcStepLeftSide));                           // 86
            camMesh.addVertex(ofVec2f(camZero.x, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x, srcZero.y));                                                           // 87 - upper left corner
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide, srcZero.y));                         // 88
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide*2, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*2, srcZero.y));                     // 89
            camMesh.addVertex(ofVec2f(camZero.x+camStepUpperSide*3, camZero.y)); camMesh.addTexCoord(ofVec2f(srcZero.x+srcStepUpperSide*3, srcZero.y));                     // 90
            
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
			
			clone.setStrength(49); // how much mix the substitution
			clone.update(srcFbo.getTexture(),
						 cam.getTexture(),
						 maskFbo.getTexture());
			
		}
	}
}

void ofApp::draw() {
	ofSetColor(255);
	
	int xOffset = cam.getWidth();
	
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		cam.draw(0, 0);
	}
	
	if(!camTracker.getFound()) {
		ofDrawBitmapStringHighlight("camera face not found", 10, 10);
	}
	if(src.getWidth() == 0) {
		ofDrawBitmapStringHighlight("drag an image here", 10, 30);
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
    
    //debug
    //camMesh.drawWireframe();
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
