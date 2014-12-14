//
//  helpers.h
//  SS-2014
//
//  Created by Andrei Antonescu on 14/12/14.
//
//

#ifndef __SS_2014__helpers__
#define __SS_2014__helpers__

#include <stdio.h>
#include "ofxCv.h"
#include "ofMain.h"
#include "constants.h"

cv::Mat correctGamma(const cv::Mat& image, const double& gamma);
cv::Mat initialFramePreproc(const cv::Mat& image);
ofImage cvToOF(const cv::Mat&cvImage);
double calcualteMeanIntesity(const cv::Mat& image);

#endif /* defined(__SS_2014__helpers__) */
