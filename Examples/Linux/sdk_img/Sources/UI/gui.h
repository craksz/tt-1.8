#ifndef GUI_H_
#define GUI_H_

//#include "ttiofiles.h"
#include "ImgProc/ImgProc.h"
#include "cv.h"
#include <gtk/gtk.h>
typedef struct gui
{
  GtkWidget *window;
  GtkWidget *cam;
  GtkWidget *button;
  GtkWidget *hbox;
} gui_t;
 
gui_t *get_gui();
 
void init_gui(int argc, char **argv);

IplImage *gtkToOcv(uint8_t* data, int useFrontal); 
#endif
