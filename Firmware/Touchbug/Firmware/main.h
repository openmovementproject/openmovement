// main.h 
// KL 09-06-2011

// Super Essential Defines / Pre-compiler stuff
#define USE_AND_OR /* To enable AND_OR mask setting */

// Includes
#include <Compiler.h>
//#include <p24Fxxxx.h>
#include <TimeDelay.h>
#include <Graphics/Graphics.h>
#include <Graphics/Primitive.h>
#include "USB/USB.h"
#include "USB/usb_function_msd.h"
#include "HardwareProfile.h"
#include "NandFlash.h"
#include "USB_CDC_MSD.h"
#include "FSconfig.h"
#include "util.h"
#include "FSIO.h"
#include "FSDefs.h"
#include "Data.h"
#include "display.h"
#include "SSD1308.h"
#include "LCDBasicFont.h"
#include "myGraphics.h"
#include "accel.h"
#include "gyro.h"
#include "Analogue.h"
#include <string.h>
#include <Rtcc.h>
#include "myRTC.h"
#include "ConfigFile.h"

// Globals

// Defines


// Config settings
/*24 bit config 1 0b<unimp>0000<unimp>0000<res>0<jtag off>0<code prot off>1<prog writes on>1<debug off>1<res>1<EMUD1>11<wdtoff>0<stdrd wdt>1<unimp>0<wdt pres 32>1<wdt posc see table>****>*/ 
/* 
WDT bits
1111 = 1:32,768
1110 = 1:16,384
1101 = 1:8,192
1100 = 1:4,096
1011 = 1:2,048
1010 = 1:1,024
1001 = 1:512
1000 = 1:256
0111 = 1:128
0110 = 1:64
0101 = 1:32
0100 = 1:16
0011 = 1:8
0010 = 1:4
0001 = 1:2
0000 = 1:1
*/
_CONFIG1(0b000000000011111101001001) 
/*24 bit config 2 0b<unimp>0000<unimp>0000<IESO off>0<plldiv2>001<plldis off>0<init osc = frc>000<clock switch en>01<no clk op>0<iolock on-off en>0<usb vreg off>1<res>1<prim osc off>11>*/
_CONFIG2(0b000000000001000001001111)
//    _CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_OFF & ICS_PGx2) 
//    _CONFIG2( 0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_ON & POSCMOD_HS & FNOSC_FRCPLL & PLLDIV_DIV2 & IOL1WAY_ON)





