/**
 *  \brief    gamepad handling implementation
 *  \author   Sylvain Gaeremynck <sylvain.gaeremynck@parrot.fr>
 *  \version  1.0
 *  \date     04/06/2007
 *  \warning  Subject to completion
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <linux/joystick.h>

#include <ardrone_api.h>
#include <VP_Os/vp_os_print.h>
#include "gamepad.h"

typedef struct {
  int32_t bus;
  int32_t vendor;
  int32_t product;
  int32_t version;
  char    name[MAX_NAME_LENGTH];
  char    handlers[MAX_NAME_LENGTH];
} device_t;

extern int32_t MiscVar[];

static C_RESULT add_device(device_t* device, const int32_t id);

static C_RESULT parse_proc_input_devices(FILE* f, const int32_t id);

input_device_t gamepad = {
  "Gamepad",
  open_gamepad,
  update_gamepad,
  close_gamepad
};

static int32_t joy_dev = 0;

input_device_t radioGP = {
  "GreatPlanes",
  open_radioGP,
  update_radioGP,
  close_radioGP
};


input_device_t ps3pad = {
  "PS3Gamepad",
  open_ps3pad,
  update_ps3pad,
  close_ps3pad
};

input_device_t fpad = {
  "fuzzyPad",
  open_fpad,
  update_fpad,
  close_fpad
};

///////////////////////////////
//  FuzzyGP input functions  //
//////////////////////////////


int sets[4];	//	Para la Fusificación
float fuzzy_E[7];
float fuzzy_R[7];



float H[11][9];		//	Inferencia Difusa
float inferencia[11];

	Control X;
	Control Y;
	Control Z;
	Control Yaw;
	
//float h1[] = = {-1500.0,-1000.0,-700.0,-400.0,-100.0,0.0,100.0,400.0,700.0,1000.0,1500.0}
/*float h3[] ;	//	Conjunto de Salida 3
float h4[] = {-1500,-1000,-700,-400,-100,0,100,400,700,1000,1500};	//	Conjunto de Salida 4//*/
void startArr(float*dst,float* src){
	int i;
	for(i=0;i<11;i++){
		dst[i]=src[i];
		//printf("//%0.2f//\n\n",dst[i]);
	}
}
	
Control* getSomeData(int theVar){
	switch (theVar){
		case 1:
			return &X; 
		break;
		case 2:
			return &Y;
		break;
		case 3:
			return &Yaw;
		break;
		case 4:
			return &Z;
		break;
		default:
		break;
	}
	return NULL;
}
	
C_RESULT open_fpad(void){
	float h[]= {-1500,-1000,-700,-400,-100,0,100,400,700,1000,1500};
	startArr(X.h,h);
	startArr(Y.h,h);
	startArr(Z.h,h);
	startArr(Yaw.h,h);
	
	X.name='X';
	Y.name='Y';
	Yaw.name='W';
	Z.name='Z';
	
	//printf("arrays initialized %0.2f\n\n",X.h[0]);

	Control *ptrX = &X;
	Control *ptrY = &Y;
	Control *ptrZ = &Z;
	Control *ptrYaw = &Yaw;

//	Se establecen las condiciones iniciales (variables de entrada a 0)
	condi(ptrX);
	condi(ptrY);
	condi(ptrZ);
	condi(ptrYaw);
	
//	Se establecen las ganancias; en este orden: Ke, kr, Ku.
	gains(ptrX, 100.0 , 150.0 , 1.0/1500 );
	gains(ptrY, 80.0 , 160.0 , 1.0/1500 );
	gains(ptrZ, 50.0 , 1.0 , 1.0/800.0);
	gains(ptrYaw, 4500.0 , 9000.0 , 1.0/1500 );

//	Se establecen las Referencias
	ptrX->Ref = 88.0;	//	Referencias fijas: Ref X y Ref Y 
	ptrY->Ref = 72.0;
	ptrZ->Ref = 800.0;	//	Referencias variables: Ref Z y Ref Yaw	
	ptrYaw->Ref = 0;
	return C_OK;
}

C_RESULT update_fpad(void){
	/*FuzzyControl( &X , 0.0 , h1 );
	FuzzyControl( &Y , 0.0 , h2 );
	FuzzyControl( &Yaw , 0.0 , h3 );
	FuzzyControl( &Z , 0.0 , h4 );//*/
	//printf("%d\n",fcx);
	dispControl(&Z);
	/*ardrone_at_set_progress_cmd( 1,
                                    //roll/(float)(stick1LR-center_x1)/32767.0f,
                                    //pitch/(float)(stick1UD-center_y1)/32767.0f,
                                    //gaz/-(float)(stick2UD-center_x2)/32767.0f,
                                    //yaw/(float)(stick2LR-center_y2)/32767.0f );*/
                                    //printf("++%0.2f++\n",Z.vout);
	ardrone_at_set_progress_cmd( 1,0,0,Z.vout,0);
	return C_OK;
}

C_RESULT close_fpad(void){
	return C_OK;
}

void condi(Control *theControl){
		theControl->Ref = 0.0;
		theControl->x0 = 0.0;
		theControl->vout = 0.0; 
	};

	void gains( Control *theControl, float a , float b , float c ){
		theControl->ke = a;
		theControl->kr = b;
		theControl->ku = c;
	};	

	void varin( Control *theControl, float *x1 ) {
		theControl->error = theControl->ke * ( theControl->Ref - *x1 );
		theControl->rate = theControl->kr * ( theControl->x0 - *x1 );
		theControl->xin = theControl->x0;
		//printf("\n\n**%0.2f**\n**%0.2f**\n**%0.2f**\n**%0.2f**\n\n",theControl->kr,theControl->x0,*x1,theControl->kr * ( theControl->x0 - *x1 ));
		theControl->x0 = *x1;
	};
	
void Fuzzy_Mem( float *var , float *fuzzy_var ){
	clean_vec( fuzzy_var, 7);
//	Selección de cunjuntos positivos o negativos
	if ( *var < 0) {
		*var =	-*var;
		sets[0] = 3;
		sets[1] = 2;
		sets[2] = 1;
		sets[3] = 0;
	}else {
		sets[0] = 3;
		sets[1] = 4;
		sets[2] = 5;
		sets[3] = 6;
	};
//	Conjunto A0
	if ( ( *var >= 0 ) && ( *var < 500) ){	
		fuzzy_var[ sets[0] ] = 1;
	}else if ( ( *var >= 500 ) && ( *var <= 1500) ){ 
		rect_ec( 500, 1, 1500, 0, var , &fuzzy_var[ sets[0] ] );
	}else 	fuzzy_var[ sets[0] ] = 0;
//	Conjunto A1||A-1
	if ( ( *var >= 500 ) && ( *var < 1500 ) ){
		rect_ec( 500, 0, 1500, 1, var , &fuzzy_var[ sets[1] ] );
	}else if ( ( *var >= 1500 ) && ( *var < 2500 ) ){
		fuzzy_var[ sets[1] ] = 1;
	}else if ( ( *var >= 2500 ) && ( *var <= 3500 ) ){
		rect_ec( 2500, 1, 3500, 0, var , &fuzzy_var[ sets[1] ] );
	}else	fuzzy_var[ sets[1] ] = 0;
//	Conjunto A2||A-2
	if ( ( *var >= 2500 ) && ( *var < 3500 ) ){
		rect_ec( 2500, 0, 3500, 1, var , &fuzzy_var[ sets[2] ] );
	}else if ( ( *var >= 3500 ) && ( *var < 4500 ) ){
		fuzzy_var[ sets[2] ] = 1;
	}else if ( ( *var >= 4500 ) && ( *var <= 5500 ) ){
		rect_ec( 4500, 1, 5500, 0, var , &fuzzy_var[ sets[2] ] );
	}else	fuzzy_var[ sets[2] ] = 0;
//	Conjunto A3||A-3
	if ( ( *var < 4500 ) ){
		fuzzy_var[ sets[3] ] = 0;
	}else if ( ( *var >= 4500 ) && ( *var < 5500 ) ){	
		rect_ec( 4500, 0, 5500, 1, var , &fuzzy_var[ sets[3] ] );
	}else 	fuzzy_var[ sets[3] ] = 1;
}

//	Inferencia Difusa
void Fuzzy_inf( float *fuzzy_E, float *fuzzy_R, float *inf_var){

//	Limpia variables
	clean_vec( inf_var, 11);
	clean_matH();

//	Reglas Difusas
//	fuzzy logic AND
//	1.-	Si E(n) es A[3] AND R(n) es A[3] entonces u(n) = C[10]
	minimo( &fuzzy_E[6] , &fuzzy_R[6] , &H[10][0] );
//	2.-	Si E(n) es A[3] AND R(n) es A[2] entonces u(n) = C[10]
	minimo( &fuzzy_E[6] , &fuzzy_R[5] , &H[10][1] );
//	3.-	Si E(n) es A[3] AND R(n) es A[1] entonces u(n) = C[9]
	minimo( &fuzzy_E[6] , &fuzzy_R[4] , &H[9][0] );
//	4.- 	Si E(n) es A[3] AND R(n) es A[0] entonces u(n) = C[7]
	minimo( &fuzzy_E[6] , &fuzzy_R[3] , &H[7][0] );
//	5.-	Si E(n) es A[3] AND R(n) es A[-1] entonces u(n) = C[9]
	minimo( &fuzzy_E[6] , &fuzzy_R[2] , &H[9][1] );
//	6.-	Si E(n) es A[3] AND R(n) es A[-2] entonces u(n) = C[10]
	minimo( &fuzzy_E[6] , &fuzzy_R[1] , &H[10][2] );
//	7.-	Si E(n) es A[3] AND R(n) es A[-3] entonces u(n) = C[10]
	minimo( &fuzzy_E[6] , &fuzzy_R[0] , &H[10][3] );

//	8.-	Si E(n) es A[2] AND R(n) es A[3] entonces u(n) = C[9]
	minimo( &fuzzy_E[5] , &fuzzy_R[6] , &H[9][2] );
//	9.-	Si E(n) es A[2] AND R(n) es A[2] entonces u(n) = C[8]
	minimo( &fuzzy_E[5] , &fuzzy_R[5] , &H[8][0] );
//	10.-	Si E(n) es A[2] AND R(n) es A[1] entonces u(n) = C[7]
	minimo( &fuzzy_E[5] , &fuzzy_R[4] , &H[7][1] );
//	11.-	Si E(n) es A[2] AND R(n) es A[0] entonces u(n) = C[6]
	minimo( &fuzzy_E[5] , &fuzzy_R[3] , &H[6][0] );
//	12.-	Si E(n) es A[2] AND R(n) es A[-3] entonces u(n) = C[7]
	minimo( &fuzzy_E[5] , &fuzzy_R[2] , &H[7][2] );
//	13.-	Si E(n) es A[2] AND R(n) es A[-2] entonces u(n) = C[8]
	minimo( &fuzzy_E[5] , &fuzzy_R[1] , &H[8][1] );
//	14.-	Si E(n) es A[2] AND R(n) es A[-1] entonces u(n) = C[9]
	minimo( &fuzzy_E[5] , &fuzzy_R[0] , &H[9][3] );

//	15.-	Si E(n) es A[1] AND R(n) es A[3] entonces u(n) = C[7]
	minimo( &fuzzy_E[4] , &fuzzy_R[6] , &H[7][2] );
//	16.-	Si E(n) es A[1] AND R(n) es A[2] entonces u(n) = C[6]
	minimo( &fuzzy_E[4] , &fuzzy_R[5] , &H[6][1] );
//	17.-	Si E(n) es A[1] AND R(n) es A[1] entonces u(n) = C[6]
	minimo( &fuzzy_E[4] , &fuzzy_R[4] , &H[6][2] );
//	18.-	Si E(n) es A[1] AND R(n) es A[0] entonces u(n) = C[5]
	minimo( &fuzzy_E[4] , &fuzzy_R[3] , &H[5][0] );
//	19.-	Si E(n) es A[1] AND R(n) es A[-1] entonces u(n) = C[6]
	minimo( &fuzzy_E[4] , &fuzzy_R[2] , &H[6][3] );
//	20.-	Si E(n) es A[1] AND R(n) es A[-2] entonces u(n) = C[6]
	minimo( &fuzzy_E[4] , &fuzzy_R[1] , &H[6][4] );
//	21.-	Si E(n) es A[1] AND R(n) es A[-3] entonces u(n) = C[7]
	minimo( &fuzzy_E[4] , &fuzzy_R[0] , &H[6][4] );

//	22.-	Si E(n) es A[0] AND R(n) es A[3] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[6] , &H[5][1] );
//	23.-	Si E(n) es A[0] AND R(n) es A[2] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[5] , &H[5][2] );
//	24.-	Si E(n) es A[0] AND R(n) es A[1] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[4] , &H[5][3] );
//	25.-	Si E(n) es A[0] AND R(n) es A[0] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[3] , &H[5][4] );
//	26.-	Si E(n) es A[0] AND R(n) es A[-1] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[2] , &H[5][5] );
//	27.-	Si E(n) es A[0] AND R(n) es A[-2] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[1] , &H[5][6] );
//	28.-	Si E(n) es A[0] AND R(n) es A[-3] entonces u(n) = C[5]
	minimo( &fuzzy_E[3] , &fuzzy_R[0] , &H[5][7] );

//	29.-	Si E(n) es A[-1] AND R(n) es A[3] entonces u(n) = C[3]
	minimo( &fuzzy_E[2] , &fuzzy_R[1] , &H[3][0] );
//	30.-	Si E(n) es A[-1] AND R(n) es A[2] entonces u(n) = C[4]
	minimo( &fuzzy_E[2] , &fuzzy_R[1] , &H[4][0] );
//	31.-	Si E(n) es A[-1] AND R(n) es A[1] entonces u(n) = C[4]
	minimo( &fuzzy_E[2] , &fuzzy_R[1] , &H[4][1] );
//	32.-	Si E(n) es A[-1] AND R(n) es A[0] entonces u(n) = C[5]
	minimo( &fuzzy_E[2] , &fuzzy_R[1] , &H[5][8] );
//	33.-	Si E(n) es A[-1] AND R(n) es A[-1] entonces u(n) = C[4]
	minimo( &fuzzy_E[2] , &fuzzy_R[1] , &H[4][2] );
//	34.-	Si E(n) es A[-1] AND R(n) es A[-2] entonces u(n) = C[4]
	minimo( &fuzzy_E[2] , &fuzzy_R[1] , &H[4][3] );
//	35.-	Si E(n) es A[-1] AND R(n) es A[-3] entonces u(n) = C[3]
	minimo( &fuzzy_E[2] , &fuzzy_R[0] , &H[3][1] );

//	36.-	Si E(n) es A[-2] AND R(n) es A[3] entonces u(n) = C[1]
	minimo( &fuzzy_E[1] , &fuzzy_R[6] , &H[1][0] );
//	37.-	Si E(n) es A[-2] AND R(n) es A[2] entonces u(n) = C[2]
	minimo( &fuzzy_E[1] , &fuzzy_R[5] , &H[2][0] );
//	38.-	Si E(n) es A[-2] AND R(n) es A[1] entonces u(n) = C[3]
	minimo( &fuzzy_E[1] , &fuzzy_R[4] , &H[3][2] );
//	39.-	Si E(n) es A[-2] AND R(n) es A[0] entonces u(n) = C[4]
	minimo( &fuzzy_E[1] , &fuzzy_R[3] , &H[4][5] );
//	40.-	Si E(n) es A[-2] AND R(n) es A[-1] entonces u(n) = C[3]
	minimo( &fuzzy_E[1] , &fuzzy_R[2] , &H[3][3] );
//	41.-	Si E(n) es A[-2] AND R(n) es A[-2] entonces u(n) = C[2]
	minimo( &fuzzy_E[1] , &fuzzy_R[1] , &H[2][1] );
//	42.-	Si E(n) es A[-2] AND R(n) es A[-3] entonces u(n) = C[1]
	minimo( &fuzzy_E[1] , &fuzzy_R[0] , &H[1][1] );

//	43.-	Si E(n) es A[-3] AND R(n) es A[3] entonces u(n) = C[0]
	minimo( &fuzzy_E[0] , &fuzzy_R[6] , &H[0][0] );
//	44.-	Si E(n) es A[-3] AND R(n) es A[2] entonces u(n) = C[0]
	minimo( &fuzzy_E[0] , &fuzzy_R[5] , &H[0][1] );
//	45.-	Si E(n) es A[-3] AND R(n) es A[1] entonces u(n) = C[1]
	minimo( &fuzzy_E[0] , &fuzzy_R[4] , &H[1][2] );
//	46.-	Si E(n) es A[-3] AND R(n) es A[0] entonces u(n) = C[3]
	minimo( &fuzzy_E[0] , &fuzzy_R[3] , &H[3][4] );
//	47.-	Si E(n) es A[-3] AND R(n) es A[-1] entonces u(n) = C[1]
	minimo( &fuzzy_E[0] , &fuzzy_R[2] , &H[1][3] );
//	48.-	Si E(n) es A[-3] AND R(n) es A[-2] entonces u(n) = C[0]
	minimo( &fuzzy_E[0] , &fuzzy_R[1] , &H[0][2] );
//	49.-	Si E(n) es A[-3] AND R(n) es A[-3] entonces u(n) = C[0]
	minimo( &fuzzy_E[0] , &fuzzy_R[0] , &H[0][3] );

//	fuzzy logic OR
	maximo( inf_var );
}
//	Defuzzificación.
void Defuzz( float *u , float *h , float *ku , float *Un ){
	//printf("\n\n**%0.2f**\n\n",h[1]);
	float aux = 0.0;
	*Un = 0.0;
	int i;
	for(i = 0; i < 11; i++ ){
		aux += u[i];
		*Un += h[i] * u[i];
		//printf("++%0.2f++\n++%0.2f++\n\n",*Un,h[i]);
	};

	*Un = *ku * *Un;
	if( *Un != 0.0 ) *Un = *Un/aux;
}
//	Proceso del Control Difuso
void FuzzyControl( Control *Difuso , float vin){

	varin(Difuso, &vin );
	Fuzzy_Mem( &Difuso->error , fuzzy_E );
	Fuzzy_Mem( &Difuso->rate , fuzzy_R);
	Fuzzy_inf( fuzzy_E , fuzzy_R , inferencia );
	Defuzz( inferencia , Difuso->h , &Difuso->ku , &Difuso->vout );
	//printf("Convivid! %0.2F\n",Difuso->h[0]);
}
//	Impresión de los datos de Control
void dispControl( Control *Difuso){

	//cout<<"ccccccccccccccccccccccccccccccccccccccccccccccccccc"<<endl;
	//cout<<"Referencia "<<tipo<<":\t"<<Difuso.Ref<<endl;
	//cout<<"Posición Inicial:\t"<<Difuso.xin<<endl;
	//cout<<"Posición Actual:\t"<<Difuso.x0<<endl;
	//cout<<"Velocidad de Salida:\t"<<Difuso.vout<<endl;
	//printf("Ref %c:\t%0.2f\nInicial\t%0.2f\nActual\t%0.2f\nVout\t%0.10f\nerror:\t%0.2f\nrate:\t%0.10f\n\n",Difuso->name,Difuso->Ref,Difuso->xin,Difuso->x0,Difuso->vout,Difuso->error,Difuso->ku);
	//	exit(0);
}//*/
//	Ecuación de la recta
void rect_ec( int A, int B, int C, int D, float *x, float *y ){

	*y = (B-D)*(*x-A)/(A-C) + B;
}
//	Mínimo entre dos valores
void minimo( float *var1 , float *var2 , float *var3 ){
	if( *var1 < *var2 ){
		*var3 = *var1;
	}else
		*var3 = *var2;
}
//	Máximo entre variables
void maximo( float *y ){
	int i;
	for(i = 0; i < 11; i++ ){
		y[i] = H[i][0];
		int j;
		for(j = 1; j < 9; j++)
			if( y[i] < H[i][j] ) y[i] = H[i][j];
	};
}
//	Limpia Matriz H
void clean_matH( ){
	int i,j;
	for(i = 0; i < 11; i++)	
		for(j = 0; j < 9; j++)
			H[i][j] = 0;
}
//	Limpia Vectores
void clean_vec( float *A , int k){
	int i;
	for(i = 0; i < k; i++)
		A[i] = 0.0;
}
//	Imprime Matriz H
/*void disp_matH( ){
	int i,j;
	for(i = 0; i < 11; i++){	
		for(j = 0; j < 9; j++)
			cout<<"\t"<<H[i][j];
		cout<<endl;
	};
}
//	Imprime Vectores
void disp_vec( float *A , int k){
	int i;
	for(i = 0; i < k; i++)
		cout<<"\t"<<A[i];
	cout<<endl;
}//*/


////////////////////////////////
//  RadioGP input functions  //
///////////////////////////////
C_RESULT open_radioGP(void)
{
  C_RESULT res = C_FAIL;

  FILE* f = fopen("/proc/bus/input/devices", "r");

  if( f != NULL )
  {
    res = parse_proc_input_devices( f,  RADIO_GP_ID);

    fclose( f );

    if( SUCCEED( res ) && strcmp(radioGP.name, "GreatPlanes")!=0)
    {
			char dev_path[20]="/dev/input/";
			strcat(dev_path, radioGP.name);
      joy_dev = open(dev_path, O_NONBLOCK | O_RDONLY);

			return res;
    }
		else
		{
			return C_FAIL;
		}
  }

  return res;
}

C_RESULT update_radioGP(void)
{
  static float32_t roll = 0, pitch = 0, gaz=0, yaw=0;
  static bool_t refresh_values = FALSE;
  ssize_t res;
  static struct js_event js_e_buffer[64];

  res = read(joy_dev, js_e_buffer, sizeof(struct js_event) * 64);

  if( !res || (res < 0 && errno == EAGAIN) )
    return C_OK;

  if( res < 0 )
    return C_FAIL;

  if (res < (int) sizeof(struct js_event))// If non-complete bloc: ignored
    return C_OK;

  // Buffer decomposition in blocs (if the last is incomplete, it's ignored)
  int32_t idx = 0;
  refresh_values = FALSE;
  for (idx = 0; idx < res / sizeof(struct js_event); idx++)
  {
    if(js_e_buffer[idx].type & JS_EVENT_INIT )// If Init, the first values are ignored
    {
      break;
    }
    else if(js_e_buffer[idx].type & JS_EVENT_BUTTON )// Event Button detected
    {
      switch( js_e_buffer[idx].number )
      {
        case GP_BOARD_LEFT :
					ardrone_tool_set_ui_pad_start(js_e_buffer[idx].value);
					break;
        case GP_SIDE_RIGHT :
					ardrone_tool_set_ui_pad_r2(js_e_buffer[idx].value);
					break;
        case GP_IMPULSE :
					ardrone_tool_set_ui_pad_select(js_e_buffer[idx].value);
					break;
        case GP_SIDE_LEFT_DOWN :
					ardrone_tool_set_ui_pad_ad(js_e_buffer[idx].value);
					break;
        case GP_SIDE_LEFT_UP :
					ardrone_tool_set_ui_pad_ad(js_e_buffer[idx].value);
					break;
        default: break;
      }
    }
    else if(js_e_buffer[idx].type & JS_EVENT_AXIS )// Event Axis detected
    {
      refresh_values = TRUE;
      switch( js_e_buffer[idx].number )
      {
        case GP_PITCH:
          pitch = js_e_buffer[idx].value;
          break;
        case GP_GAZ:
          gaz = js_e_buffer[idx].value;
          break;
        case GP_ROLL:
          roll = js_e_buffer[idx].value;
          break;
        case GP_PID:
          break;
        case GP_YAW:
          yaw = js_e_buffer[idx].value;
          break;
        default:
          break;
      }
    }
    else
    {// TODO: default: ERROR (non-supported)
    }
  }

  if(refresh_values)// Axis values to refresh
    {
       ardrone_at_set_progress_cmd( 1,
                                               roll/25000.0,
                                               pitch/25000.0,
                                               -gaz/25000.0,
                                               yaw/25000.0);
    }

  return C_OK;
}

C_RESULT close_radioGP(void)
{
  close( joy_dev );

  return C_OK;
}


///////////////////////////////
//  GamePad input functions  //
///////////////////////////////

C_RESULT open_gamepad(void)
{
  C_RESULT res = C_FAIL;

  FILE* f = fopen("/proc/bus/input/devices", "r");

  if( f != NULL )
  {
    res = parse_proc_input_devices( f, GAMEPAD_LOGICTECH_ID);

    fclose( f );

    if( SUCCEED( res ) && strcmp(gamepad.name, "Gamepad")!=0)
    {
			char dev_path[20]="/dev/input/";
			strcat(dev_path, gamepad.name);
            joy_dev = open(dev_path, O_NONBLOCK | O_RDONLY);
    }
		else
		{
			return C_FAIL;
		}
  }

  return res;
}

C_RESULT update_gamepad(void)
{
  static int32_t x = 0, y = 0;
  static bool_t refresh_values = FALSE;
  ssize_t res;
  static struct js_event js_e_buffer[64];
  static int32_t start = 0;
  input_state_t* input_state;

  static int32_t theta_trim = 0;
  static int32_t phi_trim   = 0;
  static int32_t yaw_trim   = 0;


  res = read(joy_dev, js_e_buffer, sizeof(struct js_event) * 64);

  if( !res || (res < 0 && errno == EAGAIN) )
    return C_OK;

  if( res < 0 )
    return C_FAIL;

  if (res < (int) sizeof(struct js_event))// If non-complete bloc: ignored
    return C_OK;

  // Buffer decomposition in blocs (if the last is incomplete, it's ignored)
  int32_t idx = 0;
  refresh_values = FALSE;
  input_state = ardrone_tool_get_input_state();
  for (idx = 0; idx < res / sizeof(struct js_event); idx++)
  {
    if(js_e_buffer[idx].type & JS_EVENT_INIT )// If Init, the first values are ignored
    {
      break;
    }
    else if(js_e_buffer[idx].type & JS_EVENT_BUTTON )// Event Button detected
    {
      switch( js_e_buffer[idx].number )
      {
        case PAD_AG :
		ardrone_tool_set_ui_pad_ag(js_e_buffer[idx].value);
		break;
        case PAD_AB :
		ardrone_tool_set_ui_pad_ab(js_e_buffer[idx].value);
		break;
        case PAD_AD :
		ardrone_tool_set_ui_pad_ad(js_e_buffer[idx].value);
		break;
        case PAD_AH :
		ardrone_tool_set_ui_pad_ah(js_e_buffer[idx].value);
		break;
        case PAD_L1 :
		if( js_e_buffer[idx].value )
		{
			yaw_trim = 0;

			if( input_state->r2 )
			{
				yaw_trim = -1;
			}
			else
			{
				ardrone_tool_set_ui_pad_l1(1);
			}
        // ui_pad_yaw_trim( yaw_trim );
		}
      else
      {
         yaw_trim = 0;
         ardrone_tool_set_ui_pad_l1(0);
       //  ui_pad_yaw_trim( yaw_trim );
      }
		break;
        case PAD_R1 :
		if( js_e_buffer[idx].value )
		{
			yaw_trim = 0;

			if( input_state->r2 )
			{
				yaw_trim = 1;
			}
			else
			{
				ardrone_tool_set_ui_pad_r1(1);
			}
        // ui_pad_yaw_trim( yaw_trim );
		}
      else
      {
         yaw_trim = 0;
         ardrone_tool_set_ui_pad_r1(0);
        // ui_pad_yaw_trim( yaw_trim );
      }
		break;
        case PAD_L2 :
		ardrone_tool_set_ui_pad_l2(js_e_buffer[idx].value);
		if( !js_e_buffer[idx].value )
		{
			ardrone_at_set_pmode( MiscVar[0] );
			ardrone_at_set_ui_misc( MiscVar[0], MiscVar[1], MiscVar[2], MiscVar[3] );
		}
		break;
        case PAD_R2 :
		ardrone_tool_set_ui_pad_r2(js_e_buffer[idx].value);
      if( !js_e_buffer[idx].value )
         ardrone_at_set_flat_trim();
		break;
        case PAD_SELECT :
		ardrone_tool_set_ui_pad_select(js_e_buffer[idx].value);
		break;
        case PAD_START :
      if( js_e_buffer[idx].value )
      {
         start ^= 1;
		   ardrone_tool_set_ui_pad_start( start );
		}
      break;
        default:
		break;
      }

    }
    else if(js_e_buffer[idx].type & JS_EVENT_AXIS )// Event Axis detected
    {
      refresh_values = TRUE;
      switch( js_e_buffer[idx].number )
      {
        case PAD_X:
          x = ( js_e_buffer[idx].value + 1 ) >> 15;
          break;
        case PAD_Y:
          y = ( js_e_buffer[idx].value + 1 ) >> 15;
          break;
        default:
          break;
      }
    }
    else
    {// TODO: default: ERROR (non-supported)
    }
  }

  if(refresh_values)// Axis values to refresh
    {

		 phi_trim    = 0;
		 theta_trim  = 0;

		 if( input_state->r2 )
		 {
			 switch(x + 1)
			 {
				 case 0:
					 phi_trim = -1;
					 break;

				 case 2:
					 phi_trim = 1;
					 break;
			 }

			 switch(y + 1)
			 {
				 case 0:
					 theta_trim = -1;
					 break;

				 case 2:
					 theta_trim = 1;
					 break;
			 }
			 // We are triming so we don't want to update ardrone position
			 x = 0;
			 y = 0;
		 }
		 ardrone_tool_set_ui_pad_xy( x, y );
       //ui_pad_phi_trim( phi_trim );
       //ui_pad_theta_trim( theta_trim );
    }
  return C_OK;
}

C_RESULT close_gamepad(void)
{
  close( joy_dev );

  return C_OK;
}






///////////////////////////////
//  Playstation 3 Gamepad input functions  //
///////////////////////////////

C_RESULT open_ps3pad(void)
{
    PRINT("Searching PS3 Pad device ...\n");

  C_RESULT res = C_FAIL;

  FILE* f = fopen("/proc/bus/input/devices", "r");

  if( f != NULL )
  {
    res = parse_proc_input_devices( f, GAMEPAD_PLAYSTATION3_ID );

    fclose( f );

    if( SUCCEED( res ) && strcmp(ps3pad.name, "PS3Gamepad")!=0)
    {
			char dev_path[20]="/dev/input/";
			strcat(dev_path, ps3pad.name);
            joy_dev = open(dev_path, O_NONBLOCK | O_RDONLY);

            if (joy_dev)
            {
                printf("Joydev %s ouvert\n",dev_path);
            }
            else{
                printf("Joydev %s pas ouvert\n",dev_path);
            }
    }
		else
		{
		    PRINT("PS3 Pad device not found.\n");
			return C_FAIL;

		}
  }

  return res;
}

C_RESULT update_ps3pad(void)
{

  static int32_t stick1LR = 0, stick1UD = 0 , stick2LR = 0 , stick2UD = 0;
  static bool_t refresh_values = FALSE;
  ssize_t res;
  static struct js_event js_e_buffer[64];
  static int32_t start = 0;
  input_state_t* input_state;

  static int center_x1=0;
  static int center_y1=0;
  static int center_x2=0;
  static int center_y2=0;

  res = read(joy_dev, js_e_buffer, sizeof(struct js_event) * 64);


  if( !res || (res < 0 && errno == EAGAIN) )
    return C_OK;


  if( res < 0 )
    return C_FAIL;


  if (res < (int) sizeof(struct js_event))// If non-complete bloc: ignored
    return C_OK;


  int32_t idx = 0;
  refresh_values = FALSE;
  input_state = ardrone_tool_get_input_state();
  for (idx = 0; idx < res / sizeof(struct js_event); idx++)
  {
    if(js_e_buffer[idx].type & JS_EVENT_INIT )// If Init, the first values are ignored
    {
      break;
    }
    else if(js_e_buffer[idx].type & JS_EVENT_BUTTON )// Event Button detected
    {
      switch( js_e_buffer[idx].number )
      {
        case PS3BTN_LEFTARROW :
		ardrone_tool_set_ui_pad_ag(js_e_buffer[idx].value);
		break;
        case PS3BTN_DOWNARROW :
		ardrone_tool_set_ui_pad_ab(js_e_buffer[idx].value);
		break;
        case PS3BTN_RIGHTARROW :
		ardrone_tool_set_ui_pad_ad(js_e_buffer[idx].value);
		break;
        case PS3BTN_UPARROW :
		ardrone_tool_set_ui_pad_ah(js_e_buffer[idx].value);
		break;
        case PS3BTN_L1 :
		ardrone_tool_set_ui_pad_l1(js_e_buffer[idx].value);
		break;
        case PS3BTN_R1 :
		ardrone_tool_set_ui_pad_r1(js_e_buffer[idx].value);
		break;
        case PS3BTN_L2 :
		ardrone_tool_set_ui_pad_l2(js_e_buffer[idx].value);
		break;
        case PS3BTN_R2 :
		ardrone_tool_set_ui_pad_r2(js_e_buffer[idx].value);
		break;
        case PS3BTN_SELECT :
		ardrone_tool_set_ui_pad_select(js_e_buffer[idx].value);
		break;
        case PS3BTN_START :
            if( js_e_buffer[idx].value )   { start ^= 1;  ardrone_tool_set_ui_pad_start( start );   }
		break;
        case PS3BTN_PS3:
            /* Calibrate joystick */
           /* center_x1 = stick1LR;
            center_y1 = stick1UD;
            center_x2 = stick2UD;
            center_y2 = stick2LR;*/
        break;
        default:
		break;
      }
    }
    else if(js_e_buffer[idx].type & JS_EVENT_AXIS )// Event Axis detected
    {
      refresh_values = TRUE;
      switch( js_e_buffer[idx].number )
      {
        case PS3AXIS_STICK1_LR:
          stick1LR = ( js_e_buffer[idx].value ) ;
          break;
        case PS3AXIS_STICK1_UD:
          stick1UD = ( js_e_buffer[idx].value ) ;
          break;

        case PS3AXIS_STICK2_LR:
          stick2LR = ( js_e_buffer[idx].value ) ;
          break;
        case PS3AXIS_STICK2_UD:
          stick2UD = ( js_e_buffer[idx].value ) ;
          break;

        default:
          break;
      }
    }
    else
    {// TODO: default: ERROR (non-supported)
    }
  }

  if(refresh_values)// Axis values to refresh
    {
      ardrone_at_set_progress_cmd( 1,
                                    /*roll*/(float)(stick1LR-center_x1)/32767.0f,
                                    /*pitch*/(float)(stick1UD-center_y1)/32767.0f,
                                    /*gaz*/-(float)(stick2UD-center_x2)/32767.0f,
                                    /*yaw*/(float)(stick2LR-center_y2)/32767.0f );
    }
  return C_OK;
}





C_RESULT close_ps3pad(void)
{
  close( joy_dev );

  return C_OK;
}





static int32_t make_id(device_t* device)
{
  return ( (device->vendor << 16) & 0xffff0000) | (device->product & 0xffff);
}

static C_RESULT add_device(device_t* device, const int32_t id_wanted)
{
  int32_t id = make_id(device);

  if( id_wanted == GAMEPAD_LOGICTECH_ID && id == id_wanted)
  {
		PRINT("Input device %s found\n", device->name);
    strncpy(gamepad.name, device->handlers, MAX_NAME_LENGTH);
    return C_OK;
	}

	if(id_wanted == RADIO_GP_ID && id==id_wanted)
	{
		PRINT("Input device %s found\n", device->name);
    strncpy(radioGP.name, device->handlers, MAX_NAME_LENGTH);
    return C_OK;
  }

    if(id_wanted == GAMEPAD_PLAYSTATION3_ID && id==id_wanted)
	{
		PRINT("PS3 : Input device %s found\n", device->name);
        strncpy(ps3pad.name, device->handlers, MAX_NAME_LENGTH);
        return C_OK;
  }

  return C_FAIL;
}





/** simple /proc/bus/input/devices generic LL(1) parser **/

#define KW_MAX_LEN 64

typedef enum {
  KW_BUS,
  KW_VENDOR,
  KW_PRODUCT,
  KW_VERSION,
  KW_NAME,
  KW_HANDLERS,
  KW_MAX
} keyword_t;

typedef enum {
  INT,
  STRING,
} value_type_t;

typedef struct {
  const char*   name;
  value_type_t  value_type;
  int32_t       value_offset;
} kw_tab_entry_t;

static int current_c;
static int next_c; // look ahead buffer

static device_t current_device;

static const int separators[] = { ' ',  ':', '=', '\"', '\n' };
static const int quote = '\"';
static const int eol = '\n';

static kw_tab_entry_t kw_tab[] = {
  [KW_BUS]      = {  "Bus",      INT,    offsetof(device_t, bus)       },
  [KW_VENDOR]   = {  "Vendor",   INT,    offsetof(device_t, vendor)    },
  [KW_PRODUCT]  = {  "Product",  INT,    offsetof(device_t, product)   },
  [KW_VERSION]  = {  "Version",  INT,    offsetof(device_t, version)   },
  [KW_NAME]     = {  "Name",     STRING, offsetof(device_t, name)      },
  [KW_HANDLERS] = {  "Handlers", STRING, offsetof(device_t, handlers)  }
};

static const char* handler_names[] = {
  "js0",
  "js1",
  "js2",
  "js3",
  0
};

static bool_t is_separator(int c)
{
  int32_t i;
  bool_t found = FALSE;

  for( i = 0; i < sizeof separators && !found; i++ )
  {
    found = ( c == separators[i] );
  }

  return found;
}

static bool_t is_quote(int c)
{
  return c == quote;
}

static bool_t is_eol(int c)
{
  return c == eol;
}

static C_RESULT fetch_char(FILE* f)
{
  C_RESULT res = C_FAIL;

  current_c = next_c;

  if( !feof(f) )
  {
    next_c = fgetc(f);
    res = C_OK;
  }

  // PRINT("current_c = %c, next_c = %c\n", current_c, next_c );

  return res;
}

static C_RESULT parse_string(FILE* f, char* str, int32_t maxlen)
{
  int32_t i = 0;
  bool_t is_quoted = is_quote(current_c);

  if( is_quoted )
  {
    while( SUCCEED(fetch_char(f)) && ! ( is_separator(current_c) && is_quote(current_c) ) )  {
      str[i] = current_c;
      i++;
    }
  }
  else
  {
    while( SUCCEED(fetch_char(f)) && !is_separator(current_c) )  {
      str[i] = current_c;
      i++;
    }
  }

  str[i] = '\0';
  // PRINT("parse_string: %s\n", str);

  return is_eol( current_c ) ? C_FAIL : C_OK;
}

static C_RESULT parse_int(FILE* f, int32_t* i)
{
  C_RESULT res = C_OK;
  int value;

  *i = 0;

  while( !is_separator(next_c) && SUCCEED(fetch_char(f)) && res == C_OK )  {
    value = current_c - '0';

    if (value > 9 || value < 0)
    {
      value = current_c - 'a' + 10;
      res = (value > 0xF || value < 0xa) ? C_FAIL : C_OK;
    }

    *i *= 16;
    *i += value;
  }

  return res;
}

static C_RESULT skip_line(FILE* f)
{
  while( !is_eol(next_c) && SUCCEED(fetch_char(f)) );

  return C_OK;
}

static C_RESULT match_keyword( const char* keyword, keyword_t* kw )
{
  int32_t i;
  C_RESULT res = C_FAIL;

  for( i = 0; i < KW_MAX && res != C_OK; i++ )
  {
    res = ( strcmp( keyword, kw_tab[i].name ) == 0 ) ? C_OK : C_FAIL;
  }

  *kw = i-1;

  return res;
}

static C_RESULT match_handler( void )
{
  int32_t i = 0;
  bool_t found = FALSE;

  while( !found && handler_names[i] != 0 )
  {
    found = strcmp( (char*)((char*)&current_device + kw_tab[KW_HANDLERS].value_offset), handler_names[i] ) == 0;

    i ++;
  }

	if(found)
	{
		strcpy(current_device.handlers, handler_names[i-1]);
	}

  return found ? C_OK : C_FAIL;
}

static C_RESULT parse_keyword( FILE* f, keyword_t kw )
{
  C_RESULT res = C_OK;

  while( is_separator(next_c) && SUCCEED(fetch_char(f)) );

  switch( kw_tab[kw].value_type ) {
    case INT:
      parse_int( f, (int32_t*)((char*)&current_device + kw_tab[kw].value_offset) );
      //PRINT("%s = %x\n", kw_tab[kw].name, *(int32_t*)((char*)&current_device + kw_tab[kw].value_offset) );
      break;

    case STRING:
      parse_string( f, (char*)((char*)&current_device + kw_tab[kw].value_offset), KW_MAX_LEN );
      //PRINT("%s = %s\n", kw_tab[kw].name, (char*)((char*)&current_device + kw_tab[kw].value_offset) );
      break;

    default:
      res = C_FAIL;
      break;
  }

  return res;
}

static C_RESULT parse_I(FILE* f)
{
  char keyword[KW_MAX_LEN];

  while( SUCCEED(fetch_char(f)) && is_separator(next_c) );

  while( !is_eol(next_c) ) {
    keyword_t kw;

    parse_string( f, keyword, KW_MAX_LEN );
    if( SUCCEED( match_keyword( keyword, &kw ) ) )
    {
      parse_keyword( f, kw );
    }
  }

  return C_OK;
}

static C_RESULT parse_N(FILE* f)
{
  char keyword[KW_MAX_LEN];

  while( SUCCEED(fetch_char(f)) && is_separator(next_c) );

  while( !is_eol(next_c) ) {
    keyword_t kw;

    parse_string( f, keyword, KW_MAX_LEN );
    if( SUCCEED( match_keyword( keyword, &kw ) ) )
    {
      parse_keyword( f, kw );
    }
  }


  return C_OK;
}

static C_RESULT parse_H(FILE* f)
{
  C_RESULT res = C_FAIL;
  char keyword[KW_MAX_LEN];

  while( SUCCEED(fetch_char(f)) && is_separator(next_c) );

  while( !is_eol(next_c) ) {
    parse_string( f, keyword, KW_MAX_LEN );
    if( strcmp( keyword, kw_tab[KW_HANDLERS].name ) == 0 )
    {
      while( FAILED(res) && SUCCEED( parse_string(f,
                                                  (char*)((char*)&current_device + kw_tab[KW_HANDLERS].value_offset),
                                                  KW_MAX_LEN ) ) )
      {
        res = match_handler();
      }
    }
  }

  return res;
}

static C_RESULT end_device(const int32_t id)
{
  C_RESULT res = C_FAIL;
  res=add_device(&current_device, id);
  vp_os_memset( &current_device, 0, sizeof(device_t) );

  return res;
}

static C_RESULT parse_proc_input_devices(FILE* f, const int32_t id)
{
  C_RESULT res = C_FAIL;

  next_c = '\0';
  vp_os_memset( &current_device, 0, sizeof(device_t) );

  while( res != C_OK && SUCCEED( fetch_char(f) ) )
  {
    switch( next_c )
    {
      case 'I': parse_I(f); break;
      case 'N': parse_N(f); break;
      case 'H': if( SUCCEED( parse_H(f) ) ) res = end_device(id); break;
      case 'P':
      case 'S':
      case 'B':
      default: skip_line(f); break;
    }
  }

  return res;
}
