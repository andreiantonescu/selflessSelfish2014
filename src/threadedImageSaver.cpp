//
//  threadedImageSaver.cpp
//  SS-2014
//
//  Created by Andrei Antonescu on 19/12/14.
//
//

#include "threadedImageSaver.h"

void threadedImageSaver::threadedFunction(){
    if(lock()) {
        saveImage(fileName);
        unlock();
    } else {
        printf("ofxThreadedImageSaver - cannot save %s cos I'm locked", fileName.c_str());
    }
    stopThread();
}

void threadedImageSaver::saveThreaded(string fileName){
    this->fileName = fileName;
    startThread(false, false);   // blocking, verbose
}