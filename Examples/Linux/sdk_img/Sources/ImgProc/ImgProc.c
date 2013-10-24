//#include "ttcontours.h"
#include "ImgProc.h"

#include "cv.h"
#include "highgui.h"
#include "stdio.h"


#if useiofiles == 1
#include "ttiofiles.h"
#endif

static int segValues[18];
CvMemStorage* storage;

CvSeq* ttContours(IplImage *src, int mode, ttModels *theModels, double *theErr){
	//mode0:build models
	//modex:modelx
	CvSeq* firstContour = NULL,*lastContour=NULL;
	double lastMatch=10000.00, theMatch=0,area,theE;
	
	int n=cvFindContours(src,storage,&firstContour,sizeof(CvContour),CV_RETR_TREE,CV_CHAIN_APPROX_NONE,cvPoint(0,0));//*/
	#if TESTING==1
		IplImage *out=cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
		cvNamedWindow("chie",0);
	#endif
	//CvRect theRect;
	//if (TESTING==1)	printf("numberOfContours:%d\n",n);
	if(n==0)
		return NULL;
	
	if (mode==0) 
		return firstContour;
	
	CvSeq* i=firstContour,*sContour=NULL;
	while(1){	
		area=cvContourArea(i,CV_WHOLE_SEQ,0);
		//printf("ECOLE!\n");
		//if (TESTING==1) printf("area : %2.2f\n",area);
		if(area>MINAREA){
		/*if(lastArea<area){
			lastArea=area;
			lastContour=i;
		}//*/
		//if (TESTING==1) theRect=cvBoundingRect(i,0);
		//if (TESTING==1) cvRectangle(out,cvPoint(theRect.x,theRect.y),cvPoint(theRect.x+theRect.width,theRect.y+theRect.height),CV_RGB(255,255,255));
		
			if(mode>0){
				switch(mode){
					case 1:					
						theMatch=cvMatchShapes(theModels->c1,i,1,0);
						theE=theModels->e1/ERR_RESOLUTION/10;
						//printf("match:%0.2f\n",theMatch);
					break;
					case 2:
						theMatch=cvMatchShapes(theModels->c2,i,1,0);
						theE=theModels->e2/ERR_RESOLUTION/10;
					break;
					case 3:
						theMatch=cvMatchShapes(theModels->c3,i,1,0);
						theE=theModels->e3/ERR_RESOLUTION/10;
					break;
					default:
					break;
				}
				if(lastMatch>theMatch){
					lastMatch=theMatch;
					
			
					lastContour=i;
					if(theErr!=NULL){
						*theErr=theMatch;
					}
				}	//*/
			}
			#if TESTING==1
				 if(abs(theMatch*ERR_RESOLUTION-theE*ERR_RESOLUTION)<=ERR_TOLERANCE){
				  printf("CHIDO  real:%f ideal:%f\n",theMatch*ERR_RESOLUTION,theE*ERR_RESOLUTION);
				
				cvZero(out);
				cvDrawContours(out,i,CV_RGB(100,100,100),CV_RGB(100,100,100),100,1,8,cvPoint(0,0));
				cvShowImage("chie",out);
				cvWaitKey(0);
			#endif//*/
				
		}
		//printf("file");
		if(i->h_next){
			if(i->v_next){
				//SAVECURRENT
				sContour=i->h_next;
				i=i->v_next;
				//printf("saveCurrent\n");
			}	
			else {
				//GOHNEXT
				i=i->h_next;
				//printf("goHnext1\n");
			}
		}
		else {
			if(i->v_next){
				//GOVNEXT
				i=i->v_next;
				//printf("goVnext1\n");
			}
			else{
				if (sContour!=NULL){
					//GOSAVED
					i=sContour;
					sContour=NULL;
					//printf("goSaved\n");
				}
				else break;
			}
		}
		
		//printf("Continuing\n");
		if(i==NULL) printf("i=NULL\n");
		//FIXME RETURN THE EXPECTED CONTOUR BASED ON STATISTIC VALUES
	}
	if(lastContour==NULL)
		lastContour=firstContour;
	/*cvZero(out);
	cvDrawContours(out,lastContour,CV_RGB(255,255,255),CV_RGB(255,255,255),100,10,8);
	cvShowImage("chie",out);
	cvWaitKey();//*/
	return lastContour;
}

#if useiofiles == 1
void ttInit(ttModels *theModels){
	storage  =cvCreateMemStorage(0);
		theCell theModelsLoader, theSegLoader; 
		loadVars("segVals.dat",theSegLoader);	
		for(int i=0;i<18;i++){
			segValues[i]=theSegLoader[i].theInt;
		}
		
		loadVars("models.dat",theModelsLoader);
		/*for(int i=0;i<total;i++)
			printf("%s\n",theLoader[i].theString);*/
		theModels->model1=cvLoadImage(theModelsLoader[0].theString,0);
		theModels->model2=cvLoadImage(theModelsLoader[1].theString,0);
		theModels->model3=cvLoadImage(theModelsLoader[2].theString,0);
		cvThreshold(theModels->model1,theModels->model1,180,1,CV_THRESH_BINARY);
		cvThreshold(theModels->model2,theModels->model2,180,1,CV_THRESH_BINARY);
		cvThreshold(theModels->model3,theModels->model3,180,1,CV_THRESH_BINARY);
		theModels->c1=ttContours(theModels->model1,0,theModels,NULL);
		theModels->c2=ttContours(theModels->model2,0,theModels,NULL);
		theModels->c3=ttContours(theModels->model3,0,theModels,NULL);//*/
		/*theModels->c1=ttContours(theModels->model1,0,theModels,NULL);
		theModels->c2=ttContours(theModels->model2,0,theModels,NULL);
		theModels->c3=ttContours(theModels->model3,0,theModels,NULL);//*/
		
		loadVars("errors.dat",theModelsLoader);
		theModels->e1=theModelsLoader[0].theInt;
		theModels->e2=theModelsLoader[1].theInt;
		theModels->e3=theModelsLoader[2].theInt;
}

#else

void ttInit(ttModels *theModels){
	storage  =cvCreateMemStorage(0);
	//red 151	19	180	123	176
		segValues[0]=0;
		segValues[1]=10;
		segValues[2]=72;
		segValues[3]=82;
		segValues[4]=90;
		segValues[5]=115;
		
		segValues[6]=39;
		segValues[7]=131;
		segValues[8]=37;
		segValues[9]=137;
		segValues[10]=30;
		segValues[11]=115;
		
		segValues[12]=115;
		segValues[13]=240;
		segValues[14]=128;
		segValues[15]=181;
		segValues[16]=140;
		segValues[17]=255;

		theModels->model1=cvLoadImage("model1.jpg",0);
		theModels->model2=cvLoadImage("model2.jpg",0);
		theModels->model3=cvLoadImage("model3.jpg",0);
		cvThreshold(theModels->model1,theModels->model1,180,1,CV_THRESH_BINARY);
		cvThreshold(theModels->model2,theModels->model2,180,1,CV_THRESH_BINARY);
		cvThreshold(theModels->model3,theModels->model3,180,1,CV_THRESH_BINARY);
		theModels->c1=ttContours(theModels->model1,0,theModels,NULL);
		theModels->c2=ttContours(theModels->model2,0,theModels,NULL);
		theModels->c3=ttContours(theModels->model3,0,theModels,NULL);//*/
	
		theModels->e1=5670215;
		theModels->e2=6925461;
		theModels->e3=6605288;
}
#endif

void ttSegmenter(IplImage *src, IplImage *dst, int color){
	IplImage *c1,*c2,*c3;
	int cRange[6];
	c1=cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	c2=cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	c3=cvCreateImage(cvGetSize(src),IPL_DEPTH_8U,1);
	cvCvtColor(src,src,CV_RGB2HSV);
	
	//printf("\n\t\tEEEEEECOLE!\n\n");
	
	cvSplit(src,c1,c2,c3,NULL);
	/*rojo 105:135
	azul	
	verde
	*/
	//assert(segValues[0]!=NULL);
	/*switch (color){
		case 0://red
			break;
		case 1://green
			cRange[0]=segValues[2];
			cRange[1]=segValues[3];
			break;
		case 2://blue
			cRange[0]=segValues[4];
			cRange[1]=segValues[5];
			break;
		default:
			break;
	}*/
	cRange[0]=segValues[color*2+0];
	cRange[1]=segValues[color*2+1];
	cRange[2]=segValues[color*2+6];
	cRange[3]=segValues[color*2+7];
	cRange[4]=segValues[color*2+12];
	cRange[5]=segValues[color*2+13];
	//printf("%d\n",cRange[0]);
	cvThreshold(c1,c1,cRange[1],255,CV_THRESH_TOZERO_INV);
	cvThreshold(c1,c1,cRange[0],255,CV_THRESH_BINARY);
	cvThreshold(c2,c2,cRange[3],255,CV_THRESH_TOZERO_INV);
	cvThreshold(c2,c2,cRange[2],255,CV_THRESH_BINARY);
	cvThreshold(c3,c3,cRange[5],255,CV_THRESH_TOZERO_INV);
	cvThreshold(c3,c3,cRange[4],255,CV_THRESH_BINARY);
	/*cvShowImage("H",c1);
	cvShowImage("S",c2);
	cvShowImage("V",c3);//*/
	
	//cvCvtColor(src,src,CV_HSV2RGB);
	//printf("chinga");	
	cvAnd(c1,c2,dst,c3);
	//cvCopy(c1,dst,NULL);
	//cvShowImage("win2",dst);
	//cvShowImage("win2",c3);
	//cvMerge(c1,c2,c3,NULL,src);
}

void ttImprover(IplImage *src,IplImage *dst){
	cvDilate(src,dst,NULL,2);
	cvErode(src,dst,NULL,2);
	cvSmooth(src,dst,CV_GAUSSIAN,3,3,0,0);
	//cvCvtColor(dst,dst,CV_RGB2GRAY);
}

CvPoint ttFindCentroid(CvSeq* theContour){
	CvPoint theCentroid;
	CvRect theRect=cvBoundingRect(theContour,0);
	theCentroid=cvPoint(theRect.x+theRect.width/2,theRect.y+theRect.height/2);
	return theCentroid;
}

int theC=0;

void ttCalibration(IplImage *src){
	if(theC)
	
	theC++;
}

