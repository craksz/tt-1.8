#ifndef TT_CONTOURS_H

#define TT_CONTOURS_H
#define useiofiles 0

#include "cv.h"
#include "highgui.h"


#define ERR_RESOLUTION 1000000
#define ERR_TOLERANCE 400000
#define MINAREA 1000
#define TESTING 0

typedef struct theTTModels{
	IplImage *model1;
	IplImage *model2;
	IplImage *model3;
	CvSeq* c1;
	CvSeq* c2;
	CvSeq* c3;
	double e1;
	double e2;
	double e3;
} ttModels;

CvSeq* ttContours(IplImage *src, int mode, ttModels *theModels, double *theErr);

void ttInit(ttModels *theModels);

void ttSegmenter(IplImage *src, IplImage *dst, int color);

void ttImprover(IplImage *src,IplImage *dst);

CvPoint ttFindCentroid(CvSeq* theContour);

void ttCalibration(IplImage *src);
#endif
