#pragma once

#include "ofMain.h"

#include "ofxCv.h"
#include "ofxFaceTrackerThreaded.h"
#include "ofxAssimpModelLoader.h"
#include "helpers.h"
#include "gaussian1d.h"
#include "ofxPostGlitch.h"

class ofApp : public ofBaseApp{
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
	
    void exit();
    
	void keyPressed(int key);
	void keyReleased(int key);
    
	ofxFaceTrackerThreaded camTracker;
	ofVideoGrabber cam;
    
    ofxAssimpModelLoader santaHat;
    ofxAssimpModelLoader beardModel;
    ofLight light;
    
    //vecs for smoothing
    std::vector<float> camTrackerX;
    std::vector<float> camTrackerY;
    std::vector<float> browRightY;
    std::vector<float> browLeftY;
    std::vector<float> faceRectW;
    std::vector<float> faceRotateX;
    std::vector<float> faceRotateY;
    std::vector<float> faceRotateZ;
    std::vector<float> faceRotateAngle;
    std::vector<float> faceScale;
    
    ofPoint hatTranslation;
    
    ofImage beard;
    ofFbo beardFbo;
    
    ofFbo glitchFbo;
    ofxPostGlitch myGlitch;
};
