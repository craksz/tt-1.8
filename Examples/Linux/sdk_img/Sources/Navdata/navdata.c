#include <ardrone_tool/Navdata/ardrone_navdata_client.h>

#include <Navdata/navdata.h>
#include <stdio.h>
#include "UI/gui.h"
#include "UI/gamepad.h"

#define upTo 0


static char theString[500];
static int theCounter=0;

/* Initialization local variables before event loop  */
inline C_RESULT demo_navdata_client_init( void* data )
{
  return C_OK;
}

/* Receving navdata during the event loop */
inline C_RESULT demo_navdata_client_process( const navdata_unpacked_t* const navdata )
{
/*	const navdata_demo_t*nd = &navdata->navdata_demo;	
	
	if (theCounter>=upTo){
  gdk_threads_enter();
  
  gui_t *gui = get_gui();
	if (gui){
		if(nd->ctrl_state!=0)
			sprintf(theString,"Status: OK! \nBattery:\t%imV\nAltitude\t%i\n",nd->vbat_flying_percentage,nd->altitude);
		else
			sprintf(theString,"Status: EMERGENCY! \nBattery:\t%i\nAltitude\t%i\n",nd->vbat_flying_percentage,nd->altitude);
			
		gtk_label_set_text((GtkLabel*)gui->labelL,theString);
		sprintf(theString,
				"\n\nOrientation:\n\t\t\t[Theta]%4.1f\n\t\t\t[Phi]%4.1f\n\t\t\t[Psi] %4.1f\nSpeed:\n\t\t\t[vX]%4.3f\n\t\t\t[vY] %4.3f\n\t\t\t[vZPsi] %4.3f\n",
				nd->theta/100.0,nd->phi/100.0,nd->psi/100.0,nd->vx,nd->vy,nd->vz);
		gtk_label_set_text((GtkLabel*)gui->labelR,theString);
		
	}
  gdk_threads_leave();
  theCounter=0;
  }
  else theCounter++;
  Control*chingao=getSomeData(4);
  //printf("convive! %0.2f\n",chingao->h[1]);
	FuzzyControl( chingao , nd->altitude);
	//dispControl( chingao , 'Z' );
//*/
  return C_OK;
}

/* Relinquish the local resources after the event loop exit */
inline C_RESULT demo_navdata_client_release( void )
{
  return C_OK;
}

/* Registering to navdata client */
BEGIN_NAVDATA_HANDLER_TABLE
  NAVDATA_HANDLER_TABLE_ENTRY(demo_navdata_client_init, demo_navdata_client_process, demo_navdata_client_release, NULL)
END_NAVDATA_HANDLER_TABLE

