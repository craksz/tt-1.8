/*
 * @video_stage.c
 * @author marc-olivier.dzeukou@parrot.com
 * @date 2007/07/27
 *
 * ihm vision thread implementation
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>
#include <VP_Stages/vp_stages_io_file.h>
#include <VP_Stages/vp_stages_i_camif.h>

#include <config.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_delay.h>
#include <VP_Stages/vp_stages_yuv2rgb.h>
#include <VP_Stages/vp_stages_buffer_to_picture.h>
#include <VLIB/Stages/vlib_stage_decode.h>


#include <ardrone_tool/ardrone_tool_configuration.h>
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Com/config_com.h>
#include <gtk/gtk.h>
#include "UI/gui.h"
#include "ImgProc/ImgProc.h"
#include "cv.h"
#include "highgui.h"



#ifndef RECORD_VIDEO
#define RECORD_VIDEO
#endif
#ifdef RECORD_VIDEO
#include <ardrone_tool/Video/video_stage_recorder.h>
#endif

#include <ardrone_tool/Video/video_com_stage.h>

#include "Video/video_stage.h"

#define NB_STAGES 10
#define use_frontal 0
#define calib_mode 0
#define upTo 0

PIPELINE_HANDLE pipeline_handle;
int justWidth=176, justHeight=144;
static uint8_t*  pixbuf_data       = NULL;
static vp_os_mutex_t  video_update_lock = PTHREAD_MUTEX_INITIALIZER;
static int theCounter=0;
	ttModels theModels;
 	IplImage* theFrame,*theGFrame;
 	IplImage* theBFrame;
	CvPoint theCentroid;
	CvSeq* theContour;

C_RESULT output_gtk_stage_open( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
	ttInit(&theModels);
					
	theGFrame=cvCreateImage(cvSize(justWidth,justHeight),IPL_DEPTH_8U,1);
  theBFrame=cvCreateImage(cvSize(justWidth,justHeight),IPL_DEPTH_8U,3);
  return (SUCCESS);
}

C_RESULT output_gtk_stage_transform( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  vp_os_mutex_lock(&video_update_lock);
  // Get a reference to the last decoded picture
  pixbuf_data      = (uint8_t*)in->buffers[0];
  vp_os_mutex_unlock(&video_update_lock);
 
  
  // GdkPixbuf structure to store the displayed picture
  
  //OPENCV
 	theFrame=gtkToOcv(pixbuf_data,use_frontal);
 		cvCvtColor(theFrame,theFrame,CV_RGB2BGR);
  /*if (calib_mode==1){
  	ttCalibration(theFrame);
  }
  else{
 		cvZero(theBFrame);
 		cvCopy(theFrame,theBFrame,NULL);
 		//if (theFrame!=NULL)
		//cvMerge(theGFrame,theGFrame,theGFrame,NULL,theFrame);
		
		ttSegmenter(theFrame,theGFrame,1);
		//ttImprover(theFrame2,theFrame2);
		theContour=ttContours(theGFrame,3,&theModels,NULL);
		if (theContour!=NULL){
			//theCentroid=ttFindCentroid(theContour);
			cvCircle(theBFrame,theCentroid,1,CV_RGB(255,255,255),1,8,0);
			cvCircle(theBFrame,theCentroid,6,CV_RGB(255,0,0),1,8,0);
			//cvCircle(theFrame3,theCentroid,11,CV_RGB(255,255,255),1,8,0);
			//cvCircle(theFrame3,theCentroid,16,CV_RGB(255,0,0),1,8,0);
			//cvCircle(theFrame3,ttFindCentroid(theContour),15,CV_RGB(255,255,255),1,8,0);
			cvDrawContours(theBFrame,theContour,CV_RGB(255,255,255),CV_RGB(255,255,255),1,2,8,cvPoint(0,0));
			theCounter=0;
		}
	}*/
  ///OPENCV
  gdk_threads_enter();
  static GdkPixbuf *pixbuf = NULL;
  if(pixbuf!=NULL)
    {
      g_object_unref(pixbuf);
      pixbuf=NULL;
    }
  //OPENCV
 	
	
 	///OPENCV//*/
  // Creating the GdkPixbuf from the transmited data
   pixbuf = gdk_pixbuf_new_from_data((const guchar*)theFrame->imageData,
                    GDK_COLORSPACE_RGB,
                    FALSE,   // No alpha channel
                    8,       // 8 bits per pixel
                    theFrame->width,     // Image width
                    theFrame->height,     // Image height
                    theFrame->widthStep, // New pixel every 3 bytes (3channel per pixel)
                    NULL,    // Function pointers
                    NULL);
 
  gui_t *gui = get_gui();
  if (gui && gui->cam) // Displaying the image
    gtk_image_set_from_pixbuf(GTK_IMAGE(gui->cam), pixbuf);
  gdk_threads_leave();
  return (SUCCESS);
}

C_RESULT output_gtk_stage_close( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return (SUCCESS);
}


const vp_api_stage_funcs_t vp_stages_output_gtk_funcs =
{
  NULL,
  (vp_api_stage_open_t)output_gtk_stage_open,
  (vp_api_stage_transform_t)output_gtk_stage_transform,
  (vp_api_stage_close_t)output_gtk_stage_close
};

DEFINE_THREAD_ROUTINE(video_stage, data)
{
  C_RESULT res;

  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES];

  vp_api_picture_t picture;

  video_com_config_t              icc;
  vlib_stage_decoding_config_t    vec;
  vp_stages_yuv2rgb_config_t      yuv2rgbconf;
#ifdef RECORD_VIDEO
  video_stage_recorder_config_t   vrc;
#endif
  /// Picture configuration

	int32_t nextChannel;

  if(use_frontal==0){
		justWidth=176;
		justHeight=144;
		nextChannel = ZAP_CHANNEL_VERT;
	}
	else {
		justWidth=QVGA_WIDTH;
		justHeight=QVGA_HEIGHT;
		nextChannel = ZAP_CHANNEL_HORI;
	}

	ARDRONE_TOOL_CONFIGURATION_ADDEVENT (video_channel, &nextChannel, NULL);	


    picture.format        = PIX_FMT_YUV420P;

  picture.width         = justWidth;
  picture.height        = justHeight;
  picture.framerate     = 30;

  picture.y_buf   = vp_os_malloc( justWidth * justHeight     );
  picture.cr_buf  = vp_os_malloc( justWidth * justHeight / 4 );
  picture.cb_buf  = vp_os_malloc( justWidth * justHeight / 4 );

  picture.y_line_size   = justWidth;
  picture.cb_line_size  = justWidth / 2;
  picture.cr_line_size  = justWidth / 2;

  vp_os_memset(&icc,          0, sizeof( icc ));
  vp_os_memset(&vec,          0, sizeof( vec ));
  vp_os_memset(&yuv2rgbconf,  0, sizeof( yuv2rgbconf ));

  icc.com                 = COM_VIDEO();
  icc.buffer_size         = 100000;
  icc.protocol            = VP_COM_UDP;
  COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);

  vec.width               = justWidth;
  vec.height              = justHeight;
  vec.picture             = &picture;
  vec.block_mode_enable   = TRUE;
  vec.luma_only           = FALSE;

  yuv2rgbconf.rgb_format = VP_STAGES_RGB_FORMAT_RGB24;
#ifdef RECORD_VIDEO
  vrc.fp = NULL;
#endif
  pipeline.nb_stages = 0;
  

  stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  stages[pipeline.nb_stages].cfg     = (void *)&icc;
  stages[pipeline.nb_stages].funcs   = video_com_funcs;

  pipeline.nb_stages++;

#ifdef RECORD_VIDEO
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vrc;
  stages[pipeline.nb_stages].funcs   = video_recorder_funcs;

  pipeline.nb_stages++;
#endif // RECORD_VIDEO
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vec;
  stages[pipeline.nb_stages].funcs   = vlib_decoding_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_FILTER_YUV2RGB;
  stages[pipeline.nb_stages].cfg     = (void*)&yuv2rgbconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_yuv2rgb_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_OUTPUT_SDL;
  stages[pipeline.nb_stages].cfg     = NULL;
  stages[pipeline.nb_stages].funcs   = vp_stages_output_gtk_funcs;

  pipeline.nb_stages++;

  pipeline.stages = &stages[0];
 
  /* Processing of a pipeline */
  if( !ardrone_tool_exit() )
  {
    PRINT("\n   Video stage thread initialisation\n\n");

    res = vp_api_open(&pipeline, &pipeline_handle);

    if( SUCCEED(res) )
    {
      int loop = SUCCESS;
      out.status = VP_API_STATUS_PROCESSING;

      while( !ardrone_tool_exit() && (loop == SUCCESS) )
      {
          if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
            if( (out.status == VP_API_STATUS_PROCESSING || out.status == VP_API_STATUS_STILL_RUNNING) ) {
              loop = SUCCESS;
            }
          }
          else loop = -1; // Finish this thread
      }

      vp_api_close(&pipeline, &pipeline_handle);
    }
  }

  PRINT("   Video stage thread ended\n\n");

  return (THREAD_RET)0;
}

