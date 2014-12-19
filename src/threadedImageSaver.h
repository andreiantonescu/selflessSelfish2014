//
//  threadedImageSaver.h
//  SS-2014
//
//  Created by Andrei Antonescu on 19/12/14.
//
//

#ifndef __SS_2014__threadedImageSaver__
#define __SS_2014__threadedImageSaver__

#include <stdio.h>
#include <iostream>
#include "ofMain.h"
#include "ofThread.h"

class threadedImageSaver : public ofThread, public ofImage{
public:
    
    string fileName;
    void threadedFunction();
    void saveThreaded(string fileName);
};

#endif /* defined(__SS_2014__threadedImageSaver__) */
