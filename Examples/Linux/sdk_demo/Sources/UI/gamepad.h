#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "UI/ui.h"

#define GAMEPAD_PLAYSTATION3_ID 0x054C0268

typedef enum
{
    PS3BTN_SELECT=0,
    PS3BTN_L3,
    PS3BTN_R3,
    PS3BTN_START,
    PS3BTN_UPARROW,
    PS3BTN_RIGHTARROW,
    PS3BTN_DOWNARROW,
    PS3BTN_LEFTARROW,
    PS3BTN_L2,
    PS3BTN_R2,
    PS3BTN_L1,
    PS3BTN_R1,
    PS3BTN_TRIANGLE,
    PS3BTN_CIRCLE,
    PS3BTN_CROSS,
    PS3BTN_SQUARE,
    PS3BTN_PS3
}PS3PAD_BUTTONS;

/* On Playstation3 Gamepad, buttons can return analogic values (though non-precise) */
typedef enum
{
    PS3AXIS_STICK1_LR=0,
    PS3AXIS_STICK1_UD,
    PS3AXIS_STICK2_LR,
    PS3AXIS_STICK2_UD,
    PS3AXIS_UPARROW=8,
    PS3AXIS_RIGHTARROW,
    PS3AXIS_DOWNARROW,
    PS3AXIS_LEFTARROW,
    PS3AXIS_L2,
    PS3AXIS_R2,
    PS3AXIS_L1,
    PS3AXIS_R1,
    PS3AXIS_TRIANGLE,
    PS3AXIS_CIRCLE,
    PS3AXIS_CROSS,
    PS3AXIS_SQUARE
}PS3PAD_AXIS;


extern input_device_t ps3pad;
extern input_device_t kbPad;


C_RESULT open_ps3pad(void);
C_RESULT update_ps3pad(void);
C_RESULT close_ps3pad(void);

C_RESULT openKbp(void);
C_RESULT updateKbp(void);
C_RESULT closeKbp(void);

void reset_terminal_mode();
void set_conio_terminal_mode();
int kbhit();
int getch();





typedef enum _TYPE
{
  TYPE_MIN      =  0,
  TYPE_BUTTON   =  1,
  TYPE_ANALOGIC =  2,
  TYPE_MAX      =  3
} TYPE;

#endif // _GAMEPAD_H_
