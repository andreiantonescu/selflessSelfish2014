#include "ofApp.h"

using namespace ofxCv;
using namespace std;

void ofApp::setup() {
#ifdef TARGET_OSX
	ofSetDataPathRoot("../../../data/");
#endif
	ofSetVerticalSync(true);
	cam.initGrabber(camWidth, camHeight);

	camTracker.setup();
    
    santaHat.loadModel("/Users/andreiantonescu/Desktop/blender-santa/santa-cut3.3ds", true);
    santaHat.setScale(hatScale, hatScale, hatScale);
    
    beardModel.loadModel("/Users/andreiantonescu/Desktop/blender-santa/beard5.3ds", true);
    beardModel.setScale(hatScale, hatScale, hatScale);

    glShadeModel(GL_SMOOTH);
    
    light.setAmbientColor(ofFloatColor(0.2,0.2,0.2));
    light.setDiffuseColor(ofFloatColor(0.5,0.5,0.5));
    light.setSpecularColor(ofFloatColor(0.15, 0.05, 0.05));
    
    light.setPosition(0, 300, 0);
//    
    glitchFbo.allocate(camWidth,camHeight);
    myGlitch.setup(&glitchFbo);
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
        
		camTracker.update(initialFramePreproc(toCv(cam)));
        
		if(camTracker.getFound()) {
            
            // get rotation
            ofMatrix4x4 rot = camTracker.getRotationMatrix();
            float angle,x,y,z;
            rot.getRotate().getRotate(angle, x, y, z);
            
            // push into smooth vecs
            faceRotateX.push_back(x); faceRotateY.push_back(y); faceRotateZ.push_back(z); faceRotateAngle.push_back(angle);
            camTrackerX.push_back(camTracker.getPosition().x);
            camTrackerY.push_back(camTracker.getPosition().y);
            browLeftY.push_back(camTracker.getImageFeature(ofxFaceTracker::LEFT_EYEBROW).getBoundingBox().position.y);
            browRightY.push_back(camTracker.getImageFeature(ofxFaceTracker::RIGHT_EYEBROW).getBoundingBox().position.y);
            faceRectW.push_back(camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getBoundingBox().width);
            faceScale.push_back(camTracker.getScale());
            
            //smooth
            gaussianiir1d(&camTrackerX[0], camTrackerX.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&camTrackerY[0], camTrackerY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&browLeftY[0], browLeftY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&browRightY[0], browRightY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRectW[0], faceRectW.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateX[0], faceRotateX.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateY[0], faceRotateY.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateZ[0], faceRotateZ.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceRotateAngle[0], faceRotateAngle.size(), smoothAlpha, smoothSteps);
            gaussianiir1d(&faceScale[0], faceScale.size(), smoothAlpha, smoothSteps);
		}
	}
}

void ofApp::draw() {

    glitchFbo.begin();
    ofClear(255);
    cam.draw(0,0);
    
    glDisable(GL_LIGHTING);
	ofSetColor(255);
    
    if(camTrackerX.size() && camTrackerY.size()){
    
        ofPushMatrix();
        ofTranslate(ofVec3f(camTrackerX[camTrackerX.size()-1], camTrackerY[camTrackerY.size() - 1], - faceScale[faceScale.size()-1] * zCorrection));
        ofRotate(faceRotateAngle[faceRotateAngle.size() - 1],
                 -faceRotateX[faceRotateX.size() - 1], -faceRotateY[faceRotateY.size() - 1], faceRotateZ[faceRotateZ.size() - 1]);
        ofPoint transFirst = ofPoint(camTrackerX[camTrackerX.size() - 1] - faceScale[faceScale.size()-1] * xCorrection ,
                               (browLeftY[browLeftY.size() - 1] + browRightY[browRightY.size() - 1])/2 - faceScale[faceScale.size()-1] * foreheadCorrection)
                        - ofPoint(camTrackerX[camTrackerX.size()-1], camTrackerY[camTrackerY.size() - 1]);
        ofTranslate(transFirst);
        ofTranslate(0, -faceRotateX[faceRotateX.size() - 1] * faceScale[faceScale.size()-1]*5);
        
        ofScale(faceScale[faceScale.size()-1] * santaHat.getScale().x, faceScale[faceScale.size()-1] * santaHat.getScale().y,
                faceScale[faceScale.size()-1] * santaHat.getScale().z);
        
        light.enable();
        light.setPosition(0,cos(ofGetElapsedTimef()) * RAD_TO_DEG, 200);
        ofEnableDepthTest();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        santaHat.drawFaces();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofDisableDepthTest();
        light.disable();
        glDisable(GL_LIGHTING);
        ofPopMatrix();
        
        ofPushMatrix();
        ofTranslate(camTracker.getPosition());
        
        ofRotate(faceRotateAngle[faceRotateAngle.size() - 1],
                 -faceRotateX[faceRotateX.size() - 1], -faceRotateY[faceRotateY.size() - 1], faceRotateZ[faceRotateZ.size() - 1]);
        ofPoint transSecond = camTracker.getImageFeature(ofxFaceTracker::OUTER_MOUTH).getCentroid2D();
        transSecond.y+= - camTracker.getImageFeature(ofxFaceTracker::OUTER_MOUTH).getCentroid2D().y + camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE).getCentroid2D().y;
        transSecond-=camTracker.getPosition();
        if(faceRotateX[faceRotateX.size() - 1]<0)
            ofTranslate(0, -faceRotateX[faceRotateX.size() - 1] * faceScale[faceScale.size()-1]*10);
        ofScale(faceScale[faceScale.size()-1] * 0.2, faceScale[faceScale.size()-1] * 0.22,
                faceScale[faceScale.size()-1] * 0.22);
        
        light.enable();
        light.setOrientation( ofVec3f( 0, cos(ofGetElapsedTimef()) * RAD_TO_DEG, 0) );
        light.setPosition(0,0, 200);
        ofEnableDepthTest();
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        beardModel.drawFaces();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofDisableDepthTest();
        light.disable();
        glDisable(GL_LIGHTING);
        ofPopMatrix();
    }

    glitchFbo.end();

    myGlitch.setFx(OFXPOSTGLITCH_GLOW,true);
    myGlitch.setFx(OFXPOSTGLITCH_CR_REDRAISE,true);
    myGlitch.setFx(OFXPOSTGLITCH_SLITSCAN, true);
    myGlitch.setFx(OFXPOSTGLITCH_CUTSLIDER, true);
    myGlitch.generateFx();
    glitchFbo.draw(0, 0);

}

void ofApp::dragEvent(ofDragInfo dragInfo) {

}

void ofApp::mouseMoved(int x, int y ) {
	
}

void ofApp::mouseDragged(int x, int y, int button) {

}

void ofApp::mousePressed(int x, int y, int button) {
	
}

void ofApp::mouseReleased(int x, int y, int button) {

}

void ofApp::keyPressed(int key) {

}

void ofApp::keyReleased(int key) {
}

void ofApp::exit(){
    camTracker.stopThread();
}