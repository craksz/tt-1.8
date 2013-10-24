#include <stdlib.h>
#include "gui.h"
#include <ardrone_tool/UI/ardrone_input.h>
#include "ImgProc/ImgProc.h"
//#define theStep 1
 
gui_t *gui = NULL;
 
gui_t *get_gui()
{
  return gui;
}
 
/* If the drone is landed, only start is clickable,
   if the drone is in the air, only stop is clickable
*/
static void keyPressed(GtkWidget *widget,GdkEventKey *kevent, gpointer data){
		static float theStep=1.0;
    static int value = 1;
	   switch(kevent->keyval){
		case ' ': 
			if(kevent->type == GDK_KEY_PRESS)  
  			ardrone_tool_set_ui_pad_select(1);
  		else ardrone_tool_set_ui_pad_select(0);
			break;
		case 'q': 
			if(kevent->type == GDK_KEY_PRESS)  {
    		ardrone_tool_set_ui_pad_start(value);
    		value = (value + 1) % 2;
    		printf("++%d++\n",value);
    	}
  		break;
		case 'w'://up 
			if(kevent->type == GDK_KEY_PRESS){
				ardrone_at_set_progress_cmd( 1,0,0,theStep,0);
				}
  		break;
		case 's': //down
			if(kevent->type == GDK_KEY_PRESS) { 
				ardrone_at_set_progress_cmd( 1,0,0,-theStep,0);
				}
			break;
		case 'a': //ccw
			if(kevent->type == GDK_KEY_PRESS) {  
				ardrone_at_set_progress_cmd( 1,0,0,0,theStep);
				}
			break;
		case 'd': //cw
			if(kevent->type == GDK_KEY_PRESS) {  
				ardrone_at_set_progress_cmd( 1,0,0,0,-theStep);
				}
			break;
		case 'i': //front
			if(kevent->type == GDK_KEY_PRESS) {  
				ardrone_at_set_progress_cmd( 1,0,theStep,0,0);
				}
			break;
		case 'k': //back
			if(kevent->type == GDK_KEY_PRESS) {  
				ardrone_at_set_progress_cmd( 1,0,-theStep,0,0);
				}
			break;
		case 'j': //left
			if(kevent->type == GDK_KEY_PRESS) {  
				ardrone_at_set_progress_cmd( 1,theStep,0,0,0);
				}
			break;
		case 'l': //right
			if(kevent->type == GDK_KEY_PRESS) {  
				ardrone_at_set_progress_cmd( 1,-theStep,0,0,0);
				}
			break;
		default:
			break;
	}
	if(kevent->type == GDK_KEY_PRESS) {  
		printf("++%c++\n",kevent->keyval);
	}
	
}

static void buttons_callback(GtkWidget *widget,gpointer   data ){
	static int counter=0;
	char string[19];
	guchar * imgdata=gdk_pixbuf_get_pixels(gtk_image_get_pixbuf(GTK_IMAGE(gui->cam)));
	IplImage * theFrame=gtkToOcv((uint8_t*)imgdata,0);
	
	assert(theFrame!=NULL);
	if(counter<10)
		sprintf(string,"images/image0%d.jpg",counter);
	else
		sprintf(string,"images/image%d.jpg",counter);
		
	counter++;	
	cvSaveImage(string,theFrame,0);
}
 
static void on_destroy(GtkWidget *widget, gpointer data)
{
  vp_os_free(gui);
  exit(0);
  gtk_main_quit();
}
 
IplImage *gtkToOcv(uint8_t* data, int useFrontal){
  //IplImage *currframe;
  IplImage *dst;
 	if (useFrontal==1){
	  //currframe = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U, 3);
 		dst = cvCreateImage(cvSize(320,240), IPL_DEPTH_8U, 3);
  }
  else {
  	//currframe = cvCreateImage(cvSize(176,144), IPL_DEPTH_8U, 3);
  	dst = cvCreateImage(cvSize(176,144), IPL_DEPTH_8U, 3);
  }
 
  dst->imageData = (char*)data;
  cvCvtColor(dst, dst, CV_BGR2RGB);
  //cvReleaseImage(&currframe);
  return dst;
}

void init_gui(int argc, char **argv)
{
  gui = vp_os_malloc(sizeof (gui_t));
 
 	//OPENCV
	//ttModels theModels;
	//ttInit(&theModels);
	
  ///OPENCV//*/
  g_thread_init(NULL);
  gdk_threads_init();
  gtk_init(&argc, &argv);
 
  gui->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gui->hbox = gtk_hbox_new(FALSE, 10);
  gui->cam = gtk_image_new();
  gui->button = gtk_button_new_with_label("Snapshot!");
  
  g_signal_connect(G_OBJECT(gui->window),
           											"destroy",
           											G_CALLBACK(on_destroy),
           											NULL);
           											
  
  
  g_signal_connect (gui->button, "clicked",
              			G_CALLBACK (buttons_callback), NULL);
              			
   g_signal_connect(G_OBJECT(gui->window), "key_press_event", G_CALLBACK(keyPressed),gui->window);
   g_signal_connect(G_OBJECT(gui->window), "key_release_event", G_CALLBACK(keyPressed),gui->window);
  
	
  gtk_box_pack_start(GTK_BOX(gui->hbox), gui->cam, FALSE, FALSE, 10);
  gtk_box_pack_start(GTK_BOX(gui->hbox), gui->button, FALSE, TRUE, 10);
  
  gtk_container_add(GTK_CONTAINER(gui->window),gui->hbox);
 
  gtk_widget_show_all(gui->window);
}
