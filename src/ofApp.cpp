#include "ofApp.h"

using namespace ofxCv;
using namespace std;

const int ofApp::lines [] = {22,27,27,21,21,22,22,23,23,21,21,20,20,23,23,24,24,25,25,26,26,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,0,0,17,17,18,18,19,19,20,27,28,28,29,29,30,30,31,30,32,30,33,30,34,30,35,35,34,34,33,33,32,32,31,31,48,31,49,31,50,32,50,33,50,33,51,33,52,34,52,35,52,35,53,35,54,48,49,49,50,50,51,51,52,52,53,53,54,54,55,55,56,56,57,57,58,58,59,59,48,48,60,60,61,61,62,62,54,54,63,63,64,64,65,65,48,49,60,60,50,50,61,61,51,61,52,52,62,62,53,55,63,63,56,56,64,64,57,64,58,58,65,65,59,36,37,37,38,38,39,39,40,40,41,41,36,42,43,43,44,44,45,45,46,46,47,47,42,27,42,42,22,42,23,43,23,43,24,43,25,44,25,44,26,45,26,45,16,45,15,46,15,46,14,47,14,29,47,47,28,28,42,27,39,39,21,39,20,38,20,38,19,38,18,37,18,37,17,36,17,36,0,36,1,41,1,41,2,40,2,2,29,29,40,40,28,28,39,29,31,31,3,3,29,29,14,14,35,35,29,3,48,48,4,48,6,6,59,59,7,7,58,58,8,8,57,8,56,56,9,9,55,55,10,10,54,54,11,54,12,54,13,13,35};
float mata;

std::vector<ofVec2f> vecConversion(std::vector<ofVec3f>& vectorThree){
    std::vector<ofVec2f> vectorTwo;
    for(int i=0; i< vectorThree.size(); i++){
        vectorTwo.push_back(ofVec2f(vectorThree.at(i).x, vectorThree.at(i).y));
    }
    
    return vectorTwo;
}

void ofApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../../../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;
	cam.initGrabber(1280, 780);
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
    
    camFbo.allocate(settings);
	
	selectArea = false;
    
    santaHat.loadModel("/Users/andreiantonescu/Desktop/blender-santa/santa-cut3.3ds", true);
    santaHat.setScale(0.3, 0.3,0.3);

    glShadeModel(GL_SMOOTH);
    
    light.setAmbientColor(ofFloatColor(0.2,0.2,0.2));
    light.setDiffuseColor(ofFloatColor(0.5,0.5,0.5));
    light.setSpecularColor(ofFloatColor(0.15, 0.05, 0.05));
    
    light.setPosition(0, 300, 0);
    
    mata = 0;
    
    beard.loadImage("/Users/andreiantonescu/Desktop/blender-santa/beard.png");
    
    beardFbo.allocate(1280, 780);

}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(initialFramePreproc(toCv(cam)));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
            
            // get rotation
            ofMatrix4x4 rot = camTracker.getRotationMatrix();
            float angle,x,y,z;
            rot.getRotate().getRotate(angle, x, y, z);
            
            // push into smooth vecs
            faceRotateX.push_back(x); faceRotateY.push_back(y); faceRotateZ.push_back(z); faceRotateAngle.push_back(angle);
            camTrackerX.push_back(camTracker.getPosition().x);
            camTrackerY.push_back(camTracker.getPosition().y);
            faceOutlineX.push_back(camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox().position.x);
            faceOutlineY.push_back(camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox().position.y);
            faceRectW.push_back(camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox().width);
            faceScale.push_back(camTracker.getScale());
            
            //smooth
            gaussianiir1d(&camTrackerX[0], camTrackerX.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&camTrackerY[0], camTrackerY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceOutlineX[0], faceOutlineX.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceOutlineY[0], faceOutlineY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRectW[0], faceRectW.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateX[0], faceRotateX.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateY[0], faceRotateY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateZ[0], faceRotateZ.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateAngle[0], faceRotateAngle.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceScale[0], faceScale.size(), smoothAlpha, smoothSteps);

            // get smoothed translation
            hatTranslation = ofPoint(faceOutlineX[faceOutlineX.size()-1], faceOutlineY[faceOutlineY.size()-1]);
            hatTranslation.x+= faceRectW[faceRectW.size()-1]/2;
            hatTranslation = hatTranslation - ofPoint(camTrackerX[camTrackerX.size()-1], camTrackerY[camTrackerY.size()-1]);
            
            
			ofMesh camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
			
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
            
            camFbo.begin();
            ofClear(0,255);
            cam.draw(0, 0);
            camFbo.end();
			
			clone.setStrength(20);
			clone.update(srcFbo.getTextureReference(),
						 camFbo.getTextureReference(),
						 maskFbo.getTextureReference());
			
		}
	}
}

void ofApp::draw() {
    
    //beard mesh try
    ofMesh triangulated;
    if(camTracker.getFound()){
        ofMesh faceMesh = camTracker.getMeanObjectMesh();
        
		ofxDelaunay delaunay;
		
		// add main face points
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			delaunay.addPoint(ofVec2f(faceMesh.getVertex(i).x * camTracker.getScale(), faceMesh.getVertex(i).y * camTracker.getScale())
                              + camTracker.getPosition() );
		}
        
        // add boundary face points
		float scaleFactor = 1.6;
		ofPolyline outline = camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
		ofVec2f position = camTracker.getPosition();
		for(int i = 0; i < outline.size(); i++) {
			ofVec2f point((outline[i] - position) * scaleFactor + position);
			delaunay.addPoint(point);
		}
        
		// add the image corners
		int w = 1280, h = 780;
		delaunay.addPoint(ofVec2f(0, 0));
		delaunay.addPoint(ofVec2f(w, 0));
		delaunay.addPoint(ofVec2f(w, h));
		delaunay.addPoint(ofVec2f(0, h));
        
        delaunay.triangulate();
		triangulated = delaunay.triangleMesh;
		triangulated.drawWireframe();
    }
    
    
    glDisable(GL_LIGHTING);
	ofSetColor(255);
	
	int xOffset = cam.getWidth();
	
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
        camTracker.draw();
	} else {
		cam.draw(0, 0);
        camTracker.draw();
	}
    
    ofPushMatrix();
    if(camTrackerX.size() && camTrackerY.size()){
        //position model
        float scaleRatio = 0.08;
        
        
        //beard
        beardFbo.begin();
        ofPushMatrix();
        ofClear(255);
        ofTranslate(ofVec2f(camTrackerX[camTrackerX.size()-1], camTrackerY[camTrackerY.size() - 1]));
        ofRotate(faceRotateAngle[faceRotateAngle.size() - 1], 0, -faceRotateY[faceRotateY.size() - 1],
                 faceRotateZ[faceRotateZ.size() - 1]);
        ofScale(faceScale[faceScale.size()-1]*scaleRatio, faceScale[faceScale.size()-1]*scaleRatio, faceScale[faceScale.size()-1]*scaleRatio);
        ofTranslate(0, 200);
        ofSetColor(255, 255, 255);
        beard.draw(-beard.width/2, -beard.height/2);
        
        ofPopMatrix();
        beardFbo.end();
        
        scaleRatio = 0.24;
        // hat
        ofTranslate(ofVec3f(camTrackerX[camTrackerX.size()-1], camTrackerY[camTrackerY.size() - 1],-100));
        ofRotate(faceRotateAngle[faceRotateAngle.size() - 1], -faceRotateX[faceRotateX.size() - 1], -faceRotateY[faceRotateY.size() - 1],
                 faceRotateZ[faceRotateZ.size() - 1]);
        cout<<mata<<endl;
        
        ofScale(faceScale[faceScale.size()-1]*scaleRatio, faceScale[faceScale.size()-1]*scaleRatio, faceScale[faceScale.size()-1]*scaleRatio);
        ofTranslate(hatTranslation);
        
        //draw model
        light.enable();
        light.setOrientation( ofVec3f( 0, cos(ofGetElapsedTimef()) * RAD_TO_DEG, 0) );
        light.setPosition(0,0, 200);
        ofEnableDepthTest();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        santaHat.drawFaces();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofDisableDepthTest();
        light.disable();
        glDisable(GL_LIGHTING);
    }
    ofPopMatrix();
	
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 10, 10);

	}
	if(src.getWidth() == 0) {
		drawHighlightString("drag an image here", 10, 30);
	}
	
	if (src.getWidth() > 0) {
		src.draw(xOffset, 0);
	}
	
	if (srcPoints.size() > 0) {
		for (int i = 0; i < sizeof(lines) / sizeof(int) - 1; i += 2) {
			ofVec2f p0 = srcPoints[lines[i]];
			ofVec2f p1 = srcPoints[lines[i+1]];
			ofLine(xOffset + p0[0], p0[1], xOffset + p1[0], p1[1]);
		}
	}
	
	ofFill();
	for (int i = 0; i < srcPoints.size(); i++) {
		ofVec2f p = srcPoints[i];
		
		ofSetColor(255,128,0);
		ofCircle(xOffset + p[0], p[1], 6);
		
		ofSetColor(255,255,255);
		ofCircle(xOffset + p[0], p[1], 4);
	}
	
	for (int i = 0; i < selectedPoints.size(); i++) {
		ofVec2f p = srcPoints[selectedPoints[i]];
		
		ofSetColor(255,255,255);
		ofCircle(xOffset + p[0], p[1], 6);
		
		ofSetColor(255,128,0);
		ofCircle(xOffset + p[0], p[1], 4);
	}
	ofSetColor(255,255,255);
	
	if (selectArea) {
		int startX = selectAreaStart[0];
		int startY = selectAreaStart[1];
		
		ofNoFill();
		ofSetColor(255, 255, 255);
		ofRect(startX, startY, mouseX - startX, mouseY - startY);
	}
    
    
//    ofMesh finalMesh;
//    finalMesh.setMode(OF_PRIMITIVE_TRIANGLES);
//    triangulated.addTexCoords(vecConversion(triangulated.getVertices()));
//    
//    beardFbo.getTextureReference().bind();
//    camTracker.getImageMesh().draw();
//    beardFbo.getTextureReference().unbind();
    ofPushMatrix();
    triangulated.drawWireframe();
    ofPopMatrix();
//    finalMesh.drawWireframe();

}

void ofApp::loadPoints(string filename) {
	ofFile file;
	file.open(ofToDataPath(filename), ofFile::ReadWrite, false);
	ofBuffer buff = file.readToBuffer();
	
	// Discard the header line.
	if (!buff.isLastLine()) buff.getNextLine();
	
	srcPoints = vector<ofVec2f>();
	
	while (!buff.isLastLine()) {
		string line = buff.getNextLine();
		vector<string> tokens = ofSplitString(line, "\t");
		srcPoints.push_back(ofVec2f(ofToFloat(tokens[0]), ofToFloat(tokens[1])));
	}
	cout << "Read " << filename << "." << endl;
}

void ofApp::loadFace(string filename){
	src.loadImage(filename);
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
				srcTracker.update(toCv(src));
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
            
        case 'a': // Clear the selection.
            mata-=0.1;
            break;
            
        case 'z': // Clear the selection.
            mata+=0.1;
            break;
	}
}

void ofApp::keyReleased(int key) {
}