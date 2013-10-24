#include "gtk/gtk.h"
#include "stdio.h"
#include "cv.h"
#include "highgui.h"
#include "ttcontourstest.h"


#define use_webcam 0
#if use_webcam==1

#define justWidth 320
#define justHeight 240
#endif
//tester for opencv+ttcontours+webcam+gtk on calibration mode!

static CvPoint thePixel;
static int changeFlag=0;
static int theColor=0;
static hsvRanger theRanger;

#if use_webcam==0
static int justWidth;
static int justHeight;
#endif
//static int segValues[6];


	GtkWidget *image,*hval;
	GtkWidget *gseg;
	//bool cvDefined=FALSE;
	
static void ocvThread(void){

	//if(cvDefined==FALSE){
		ttModels theModels;
		ttInit(&theModels);
		
	static GdkPixbuf *pixbuf;
	
	IplImage *theFrame, *segmented;
	static int theH=0;
	static char theStr[12];
  thePixel=cvPoint(0,0);
  //globalFrame=cvCreateImage(size,IPL_DEPTH_8U,3);
	//char theChar;
	
	#if use_webcam==1
	CvCapture* theCamera;
	CvSize size=cvSize(justWidth,justHeight);
	theCamera=cvCaptureFromCAM(-1);
	cvSetCaptureProperty( theCamera,CV_CAP_PROP_FRAME_WIDTH,justWidth );
	cvSetCaptureProperty( theCamera,CV_CAP_PROP_FRAME_HEIGHT,justHeight );
  theFrame=cvCreateImage(size,IPL_DEPTH_8U,3);
	#else
	theFrame=cvLoadImage("testImage09.jpg",1);
	assert(theFrame!=NULL);
	justWidth=theFrame->width;
	justHeight=theFrame->height;
	CvSize size=cvSize(justWidth,justHeight);
		cvConvertImage(theFrame,theFrame,CV_CVTIMG_SWAP_RB);
	#endif
  segmented=cvCreateImage(size,IPL_DEPTH_8U,3);
		
	while (1){
	
	#if use_webcam==1
		theFrame=cvQueryFrame(theCamera);
		
		assert(theFrame!=NULL);
		cvConvertImage(theFrame,theFrame,CV_CVTIMG_SWAP_RB);
	#endif
	
		if(changeFlag==1){
			theRanger.hue=-1;
			theH=ttCalibration(theFrame,&thePixel,&theRanger,NULL);
			theRanger.hue=theH;
			changeFlag=0;
		}
		ttCalibration(theFrame,&thePixel,&theRanger,segmented);
		sprintf(theStr,"Hue=%d",theH);
		//cvShowImage("window",theImage);
		//theFrame=theImage;
		
		//cvWaitKey(5000);
		
 		gdk_threads_enter();
		pixbuf = gdk_pixbuf_new_from_data ((guchar*) theFrame->imageData,
																		GDK_COLORSPACE_RGB,
																		FALSE,
																		theFrame->depth,
																		theFrame->width,
																		theFrame->height,
																		(theFrame->widthStep),
																		NULL,
																		NULL); 
	
		                 
	//printf("\n\nchingadamadre!\n");CV_CVTIMG_SWAP_RB
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), pixbuf);
    
    pixbuf = gdk_pixbuf_new_from_data ((guchar*) segmented->imageData,
																		GDK_COLORSPACE_RGB,
																		FALSE,
																		theFrame->depth,
																		theFrame->width,
																		theFrame->height,
																		(theFrame->widthStep),
																		NULL,
																		NULL); 
																		
    gtk_image_set_from_pixbuf(GTK_IMAGE(gseg), pixbuf);
    gtk_label_set_text((GtkLabel *)hval,theStr);
  	gdk_threads_leave();
    //cvWaitKey();
    #if use_webcam==0
	  g_usleep(50000);
	  #endif
	}
}

static gboolean kill_window(GtkWidget *widget,GdkEvent *event, gpointer data){
	gtk_main_quit();
	printf("\n%d\t%d\t%d\t%d\t%d\n",theRanger.sHigh,theRanger.sLow,theRanger.vHigh,theRanger.vLow,theRanger.hue);
	exit(0);
	return FALSE;
}
static void pressed(GtkWidget *widget, GdkEventButton *event, gpointer data){
	//gtk_main_quit();
	CvPoint pixel=cvPoint(event->x,event->y);
	thePixel=pixel;
	changeFlag=1;
}
static void updateValue(GtkWidget *widget, gpointer data){
	switch((int)data){
		case 1:
			theRanger.sHigh=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			break;
		case 2:
			if(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget))>theRanger.sHigh)
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),theRanger.sHigh);
				
				theRanger.sLow=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			break;
		case 3:
			theRanger.vHigh=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			break;
		case 4:
			if(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget))>theRanger.vHigh)
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget),theRanger.vHigh);
				
			theRanger.vLow=gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
			break;
		default:
			break;
	}
	//printf("\n%d\t%d\t%d\t%d\n",theRanger.sHigh,theRanger.sLow,theRanger.vHigh,theRanger.vLow);
}


int main(void){
	
	GThread *theThread;
	GtkWidget *window;
	GtkWidget *hbox;
	GtkWidget *theTable;
	GtkWidget *eBox;
	GtkWidget *spinvh,*spinvl,*spinsh,*spinsl;
	GtkWidget *lsh,*lsl,*lvh,*lvl;
	
	gtk_init(0,NULL);
	
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	hbox=gtk_hbox_new(FALSE,2);
	image=gtk_image_new();
	gseg=gtk_image_new();
	eBox=gtk_event_box_new();
	theTable=gtk_table_new(5,2,TRUE);
	spinvh=gtk_spin_button_new_with_range(0,255,1);
	spinvl=gtk_spin_button_new_with_range(0,255,1);
	spinsh=gtk_spin_button_new_with_range(0,255,1);
	spinsl=gtk_spin_button_new_with_range(0,255,1);
	lsh=gtk_label_new_with_mnemonic("sh");
	lsl=gtk_label_new_with_mnemonic("sl");
	lvh=gtk_label_new_with_mnemonic("vh");
	lvl=gtk_label_new_with_mnemonic("vl");
	hval=gtk_label_new_with_mnemonic("Hue=0");
	
	gtk_window_set_title(GTK_WINDOW(window),"gtk application");
	
	g_signal_connect(window,"delete_event",G_CALLBACK(kill_window),NULL);
	g_signal_connect(eBox,"button_press_event",G_CALLBACK(pressed),image);
	g_signal_connect(spinsh,"changed",G_CALLBACK(updateValue), (gpointer)1);
	g_signal_connect(spinvh,"changed",G_CALLBACK(updateValue),(gpointer)3);
	g_signal_connect(spinvl,"changed",G_CALLBACK(updateValue),(gpointer)4);
	g_signal_connect(spinsl,"changed",G_CALLBACK(updateValue),(gpointer)2);//*/
	
	gtk_table_attach_defaults(GTK_TABLE(theTable),spinsh,0,1,0,1);
	gtk_table_attach_defaults(GTK_TABLE(theTable),spinsl,0,1,1,2);
	gtk_table_attach_defaults(GTK_TABLE(theTable),spinvh,0,1,2,3);
	gtk_table_attach_defaults(GTK_TABLE(theTable),spinvl,0,1,3,4);
	gtk_table_attach_defaults(GTK_TABLE(theTable),lsh,1,2,0,1);
	gtk_table_attach_defaults(GTK_TABLE(theTable),lsl,1,2,1,2);
	gtk_table_attach_defaults(GTK_TABLE(theTable),lvh,1,2,2,3);
	gtk_table_attach_defaults(GTK_TABLE(theTable),lvl,1,2,3,4);
	gtk_table_attach_defaults(GTK_TABLE(theTable),hval,0,2,4,5);
	
	
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinsh),255);
				gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinvh),255);
	
	gtk_container_add(GTK_CONTAINER(eBox),image);
	gtk_box_pack_start(GTK_BOX(hbox),gseg,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),eBox,FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),theTable,FALSE,FALSE,2);
	gtk_container_add(GTK_CONTAINER(window),hbox);
	
	  if( g_thread_supported() )
  {
     g_thread_init(NULL);
     gdk_threads_init();                   // Called to initialize internal mutex "gdk_threads_mutex".
     printf("g_thread supported\n");
  }
  else
  {
     printf("g_thread NOT supported\n");
     exit(0);
  }
  theThread = g_thread_create((GThreadFunc)ocvThread, NULL, TRUE, NULL);
  assert(theThread!=NULL);
  
	gtk_widget_show_all(window);
	
	gtk_main();
	
	return 0;
}	
