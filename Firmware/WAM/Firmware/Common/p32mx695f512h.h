/*-------------------------------------------------------------------------
 * MPLAB-Cxx  PIC32MX695F512H processor header
 *
 * (c) Copyright 1999-2011 Microchip Technology, All rights reserved
 *-------------------------------------------------------------------------*/
#ifndef __32MX695F512H_H
#define __32MX695F512H_H

#if defined (__LANGUAGE_C__)
extern volatile unsigned int        WDTCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned WDTCLR:1;
    unsigned :1;
    unsigned SWDTPS:5;
    unsigned :8;
    unsigned ON:1;
  };
  struct {
    unsigned :2;
    unsigned SWDTPS0:1;
    unsigned SWDTPS1:1;
    unsigned SWDTPS2:1;
    unsigned SWDTPS3:1;
    unsigned SWDTPS4:1;
  };
  struct {
    unsigned :2;
    unsigned WDTPSTA:5;
  };
  struct {
    unsigned :2;
    unsigned WDTPS:5;
  };
  struct {
    unsigned w:32;
  };
} __WDTCONbits_t;
extern volatile __WDTCONbits_t WDTCONbits __asm__ ("WDTCON") __attribute__((section("sfrs")));
extern volatile unsigned int        WDTCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        WDTCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        WDTCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        RTCCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RTCOE:1;
    unsigned HALFSEC:1;
    unsigned RTCSYNC:1;
    unsigned RTCWREN:1;
    unsigned :2;
    unsigned RTCCLKON:1;
    unsigned RTSECSEL:1;
    unsigned :5;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned CAL:10;
  };
  struct {
    unsigned w:32;
  };
} __RTCCONbits_t;
extern volatile __RTCCONbits_t RTCCONbits __asm__ ("RTCCON") __attribute__((section("sfrs")));
extern volatile unsigned int        RTCCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        RTCCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        RTCCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        RTCALRM __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ARPT:8;
    unsigned AMASK:4;
    unsigned ALRMSYNC:1;
    unsigned PIV:1;
    unsigned CHIME:1;
    unsigned ALRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __RTCALRMbits_t;
extern volatile __RTCALRMbits_t RTCALRMbits __asm__ ("RTCALRM") __attribute__((section("sfrs")));
extern volatile unsigned int        RTCALRMCLR __attribute__((section("sfrs")));
extern volatile unsigned int        RTCALRMSET __attribute__((section("sfrs")));
extern volatile unsigned int        RTCALRMINV __attribute__((section("sfrs")));
extern volatile unsigned int        RTCTIME __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :8;
    unsigned SEC01:4;
    unsigned SEC10:4;
    unsigned MIN01:4;
    unsigned MIN10:4;
    unsigned HR01:4;
    unsigned HR10:4;
  };
  struct {
    unsigned w:32;
  };
} __RTCTIMEbits_t;
extern volatile __RTCTIMEbits_t RTCTIMEbits __asm__ ("RTCTIME") __attribute__((section("sfrs")));
extern volatile unsigned int        RTCTIMECLR __attribute__((section("sfrs")));
extern volatile unsigned int        RTCTIMESET __attribute__((section("sfrs")));
extern volatile unsigned int        RTCTIMEINV __attribute__((section("sfrs")));
extern volatile unsigned int        RTCDATE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned WDAY01:4;
    unsigned :4;
    unsigned DAY01:4;
    unsigned DAY10:4;
    unsigned MONTH01:4;
    unsigned MONTH10:4;
    unsigned YEAR01:4;
    unsigned YEAR10:4;
  };
  struct {
    unsigned w:32;
  };
} __RTCDATEbits_t;
extern volatile __RTCDATEbits_t RTCDATEbits __asm__ ("RTCDATE") __attribute__((section("sfrs")));
extern volatile unsigned int        RTCDATECLR __attribute__((section("sfrs")));
extern volatile unsigned int        RTCDATESET __attribute__((section("sfrs")));
extern volatile unsigned int        RTCDATEINV __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMTIME __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :8;
    unsigned SEC01:4;
    unsigned SEC10:4;
    unsigned MIN01:4;
    unsigned MIN10:4;
    unsigned HR01:4;
    unsigned HR10:4;
  };
  struct {
    unsigned w:32;
  };
} __ALRMTIMEbits_t;
extern volatile __ALRMTIMEbits_t ALRMTIMEbits __asm__ ("ALRMTIME") __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMTIMECLR __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMTIMESET __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMTIMEINV __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMDATE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned WDAY01:4;
    unsigned :4;
    unsigned DAY01:4;
    unsigned DAY10:4;
    unsigned MONTH01:4;
    unsigned MONTH10:4;
  };
  struct {
    unsigned w:32;
  };
} __ALRMDATEbits_t;
extern volatile __ALRMDATEbits_t ALRMDATEbits __asm__ ("ALRMDATE") __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMDATECLR __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMDATESET __attribute__((section("sfrs")));
extern volatile unsigned int        ALRMDATEINV __attribute__((section("sfrs")));
extern volatile unsigned int        T1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :1;
    unsigned TCS:1;
    unsigned TSYNC:1;
    unsigned :1;
    unsigned TCKPS:2;
    unsigned :1;
    unsigned TGATE:1;
    unsigned :3;
    unsigned TWIP:1;
    unsigned TWDIS:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :4;
    unsigned TCKPS0:1;
    unsigned TCKPS1:1;
  };
  struct {
    unsigned :13;
    unsigned TSIDL:1;
    unsigned :1;
    unsigned TON:1;
  };
  struct {
    unsigned w:32;
  };
} __T1CONbits_t;
extern volatile __T1CONbits_t T1CONbits __asm__ ("T1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        T1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        T1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        T1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        TMR1 __attribute__((section("sfrs")));
extern volatile unsigned int        TMR1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        TMR1SET __attribute__((section("sfrs")));
extern volatile unsigned int        TMR1INV __attribute__((section("sfrs")));
extern volatile unsigned int        PR1 __attribute__((section("sfrs")));
extern volatile unsigned int        PR1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        PR1SET __attribute__((section("sfrs")));
extern volatile unsigned int        PR1INV __attribute__((section("sfrs")));
extern volatile unsigned int        T2CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned T32:1;
    unsigned TCKPS:3;
    unsigned TGATE:1;
    unsigned :5;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :4;
    unsigned TCKPS0:1;
    unsigned TCKPS1:1;
    unsigned TCKPS2:1;
  };
  struct {
    unsigned :13;
    unsigned TSIDL:1;
    unsigned :1;
    unsigned TON:1;
  };
  struct {
    unsigned w:32;
  };
} __T2CONbits_t;
extern volatile __T2CONbits_t T2CONbits __asm__ ("T2CON") __attribute__((section("sfrs")));
extern volatile unsigned int        T2CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        T2CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        T2CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        TMR2 __attribute__((section("sfrs")));
extern volatile unsigned int        TMR2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        TMR2SET __attribute__((section("sfrs")));
extern volatile unsigned int        TMR2INV __attribute__((section("sfrs")));
extern volatile unsigned int        PR2 __attribute__((section("sfrs")));
extern volatile unsigned int        PR2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        PR2SET __attribute__((section("sfrs")));
extern volatile unsigned int        PR2INV __attribute__((section("sfrs")));
extern volatile unsigned int        T3CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :4;
    unsigned TCKPS:3;
    unsigned TGATE:1;
    unsigned :5;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :4;
    unsigned TCKPS0:1;
    unsigned TCKPS1:1;
    unsigned TCKPS2:1;
  };
  struct {
    unsigned :13;
    unsigned TSIDL:1;
    unsigned :1;
    unsigned TON:1;
  };
  struct {
    unsigned w:32;
  };
} __T3CONbits_t;
extern volatile __T3CONbits_t T3CONbits __asm__ ("T3CON") __attribute__((section("sfrs")));
extern volatile unsigned int        T3CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        T3CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        T3CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        TMR3 __attribute__((section("sfrs")));
extern volatile unsigned int        TMR3CLR __attribute__((section("sfrs")));
extern volatile unsigned int        TMR3SET __attribute__((section("sfrs")));
extern volatile unsigned int        TMR3INV __attribute__((section("sfrs")));
extern volatile unsigned int        PR3 __attribute__((section("sfrs")));
extern volatile unsigned int        PR3CLR __attribute__((section("sfrs")));
extern volatile unsigned int        PR3SET __attribute__((section("sfrs")));
extern volatile unsigned int        PR3INV __attribute__((section("sfrs")));
extern volatile unsigned int        T4CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned T32:1;
    unsigned TCKPS:3;
    unsigned TGATE:1;
    unsigned :5;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :4;
    unsigned TCKPS0:1;
    unsigned TCKPS1:1;
    unsigned TCKPS2:1;
  };
  struct {
    unsigned :13;
    unsigned TSIDL:1;
    unsigned :1;
    unsigned TON:1;
  };
  struct {
    unsigned w:32;
  };
} __T4CONbits_t;
extern volatile __T4CONbits_t T4CONbits __asm__ ("T4CON") __attribute__((section("sfrs")));
extern volatile unsigned int        T4CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        T4CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        T4CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        TMR4 __attribute__((section("sfrs")));
extern volatile unsigned int        TMR4CLR __attribute__((section("sfrs")));
extern volatile unsigned int        TMR4SET __attribute__((section("sfrs")));
extern volatile unsigned int        TMR4INV __attribute__((section("sfrs")));
extern volatile unsigned int        PR4 __attribute__((section("sfrs")));
extern volatile unsigned int        PR4CLR __attribute__((section("sfrs")));
extern volatile unsigned int        PR4SET __attribute__((section("sfrs")));
extern volatile unsigned int        PR4INV __attribute__((section("sfrs")));
extern volatile unsigned int        T5CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :4;
    unsigned TCKPS:3;
    unsigned TGATE:1;
    unsigned :5;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :4;
    unsigned TCKPS0:1;
    unsigned TCKPS1:1;
    unsigned TCKPS2:1;
  };
  struct {
    unsigned :13;
    unsigned TSIDL:1;
    unsigned :1;
    unsigned TON:1;
  };
  struct {
    unsigned w:32;
  };
} __T5CONbits_t;
extern volatile __T5CONbits_t T5CONbits __asm__ ("T5CON") __attribute__((section("sfrs")));
extern volatile unsigned int        T5CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        T5CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        T5CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        TMR5 __attribute__((section("sfrs")));
extern volatile unsigned int        TMR5CLR __attribute__((section("sfrs")));
extern volatile unsigned int        TMR5SET __attribute__((section("sfrs")));
extern volatile unsigned int        TMR5INV __attribute__((section("sfrs")));
extern volatile unsigned int        PR5 __attribute__((section("sfrs")));
extern volatile unsigned int        PR5CLR __attribute__((section("sfrs")));
extern volatile unsigned int        PR5SET __attribute__((section("sfrs")));
extern volatile unsigned int        PR5INV __attribute__((section("sfrs")));
extern volatile unsigned int        IC1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ICM:3;
    unsigned ICBNE:1;
    unsigned ICOV:1;
    unsigned ICI:2;
    unsigned ICTMR:1;
    unsigned C32:1;
    unsigned FEDGE:1;
    unsigned :3;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned ICM0:1;
    unsigned ICM1:1;
    unsigned ICM2:1;
    unsigned :2;
    unsigned ICI0:1;
    unsigned ICI1:1;
  };
  struct {
    unsigned :13;
    unsigned ICSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __IC1CONbits_t;
extern volatile __IC1CONbits_t IC1CONbits __asm__ ("IC1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        IC1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        IC1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        IC1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        IC1BUF __attribute__((section("sfrs")));
extern volatile unsigned int        IC2CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ICM:3;
    unsigned ICBNE:1;
    unsigned ICOV:1;
    unsigned ICI:2;
    unsigned ICTMR:1;
    unsigned C32:1;
    unsigned FEDGE:1;
    unsigned :3;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned ICM0:1;
    unsigned ICM1:1;
    unsigned ICM2:1;
    unsigned :2;
    unsigned ICI0:1;
    unsigned ICI1:1;
  };
  struct {
    unsigned :13;
    unsigned ICSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __IC2CONbits_t;
extern volatile __IC2CONbits_t IC2CONbits __asm__ ("IC2CON") __attribute__((section("sfrs")));
extern volatile unsigned int        IC2CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        IC2CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        IC2CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        IC2BUF __attribute__((section("sfrs")));
extern volatile unsigned int        IC3CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ICM:3;
    unsigned ICBNE:1;
    unsigned ICOV:1;
    unsigned ICI:2;
    unsigned ICTMR:1;
    unsigned C32:1;
    unsigned FEDGE:1;
    unsigned :3;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned ICM0:1;
    unsigned ICM1:1;
    unsigned ICM2:1;
    unsigned :2;
    unsigned ICI0:1;
    unsigned ICI1:1;
  };
  struct {
    unsigned :13;
    unsigned ICSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __IC3CONbits_t;
extern volatile __IC3CONbits_t IC3CONbits __asm__ ("IC3CON") __attribute__((section("sfrs")));
extern volatile unsigned int        IC3CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        IC3CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        IC3CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        IC3BUF __attribute__((section("sfrs")));
extern volatile unsigned int        IC4CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ICM:3;
    unsigned ICBNE:1;
    unsigned ICOV:1;
    unsigned ICI:2;
    unsigned ICTMR:1;
    unsigned C32:1;
    unsigned FEDGE:1;
    unsigned :3;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned ICM0:1;
    unsigned ICM1:1;
    unsigned ICM2:1;
    unsigned :2;
    unsigned ICI0:1;
    unsigned ICI1:1;
  };
  struct {
    unsigned :13;
    unsigned ICSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __IC4CONbits_t;
extern volatile __IC4CONbits_t IC4CONbits __asm__ ("IC4CON") __attribute__((section("sfrs")));
extern volatile unsigned int        IC4CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        IC4CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        IC4CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        IC4BUF __attribute__((section("sfrs")));
extern volatile unsigned int        IC5CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ICM:3;
    unsigned ICBNE:1;
    unsigned ICOV:1;
    unsigned ICI:2;
    unsigned ICTMR:1;
    unsigned C32:1;
    unsigned FEDGE:1;
    unsigned :3;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned ICM0:1;
    unsigned ICM1:1;
    unsigned ICM2:1;
    unsigned :2;
    unsigned ICI0:1;
    unsigned ICI1:1;
  };
  struct {
    unsigned :13;
    unsigned ICSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __IC5CONbits_t;
extern volatile __IC5CONbits_t IC5CONbits __asm__ ("IC5CON") __attribute__((section("sfrs")));
extern volatile unsigned int        IC5CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        IC5CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        IC5CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        IC5BUF __attribute__((section("sfrs")));
extern volatile unsigned int        OC1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OCM:3;
    unsigned OCTSEL:1;
    unsigned OCFLT:1;
    unsigned OC32:1;
    unsigned :7;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned OCM0:1;
    unsigned OCM1:1;
    unsigned OCM2:1;
  };
  struct {
    unsigned :13;
    unsigned OCSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __OC1CONbits_t;
extern volatile __OC1CONbits_t OC1CONbits __asm__ ("OC1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        OC1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC1R __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RS __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RSSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC1RSINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC2CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OCM:3;
    unsigned OCTSEL:1;
    unsigned OCFLT:1;
    unsigned OC32:1;
    unsigned :7;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned OCM0:1;
    unsigned OCM1:1;
    unsigned OCM2:1;
  };
  struct {
    unsigned :13;
    unsigned OCSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __OC2CONbits_t;
extern volatile __OC2CONbits_t OC2CONbits __asm__ ("OC2CON") __attribute__((section("sfrs")));
extern volatile unsigned int        OC2CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC2CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC2CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC2R __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RS __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RSSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC2RSINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC3CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OCM:3;
    unsigned OCTSEL:1;
    unsigned OCFLT:1;
    unsigned OC32:1;
    unsigned :7;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned OCM0:1;
    unsigned OCM1:1;
    unsigned OCM2:1;
  };
  struct {
    unsigned :13;
    unsigned OCSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __OC3CONbits_t;
extern volatile __OC3CONbits_t OC3CONbits __asm__ ("OC3CON") __attribute__((section("sfrs")));
extern volatile unsigned int        OC3CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC3CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC3CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC3R __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RS __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RSSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC3RSINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC4CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OCM:3;
    unsigned OCTSEL:1;
    unsigned OCFLT:1;
    unsigned OC32:1;
    unsigned :7;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned OCM0:1;
    unsigned OCM1:1;
    unsigned OCM2:1;
  };
  struct {
    unsigned :13;
    unsigned OCSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __OC4CONbits_t;
extern volatile __OC4CONbits_t OC4CONbits __asm__ ("OC4CON") __attribute__((section("sfrs")));
extern volatile unsigned int        OC4CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC4CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC4CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC4R __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RS __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RSSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC4RSINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC5CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OCM:3;
    unsigned OCTSEL:1;
    unsigned OCFLT:1;
    unsigned OC32:1;
    unsigned :7;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned OCM0:1;
    unsigned OCM1:1;
    unsigned OCM2:1;
  };
  struct {
    unsigned :13;
    unsigned OCSIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __OC5CONbits_t;
extern volatile __OC5CONbits_t OC5CONbits __asm__ ("OC5CON") __attribute__((section("sfrs")));
extern volatile unsigned int        OC5CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC5CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC5CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC5R __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RINV __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RS __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RSSET __attribute__((section("sfrs")));
extern volatile unsigned int        OC5RSINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C1ACONbits_t;
extern volatile __I2C1ACONbits_t I2C1ACONbits __asm__ ("I2C1ACON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C3CONbits_t;
extern volatile __I2C3CONbits_t I2C3CONbits __asm__ ("I2C3CON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C1ASTATbits_t;
extern volatile __I2C1ASTATbits_t I2C1ASTATbits __asm__ ("I2C1ASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C3STATbits_t;
extern volatile __I2C3STATbits_t I2C3STATbits __asm__ ("I2C3STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AMSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3MSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AMSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3MSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AMSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3MSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1AMSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3MSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3BRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ATRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3TRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ATRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3TRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ATRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3TRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ATRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3TRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ARCV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3RCV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C2ACONbits_t;
extern volatile __I2C2ACONbits_t I2C2ACONbits __asm__ ("I2C2ACON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C4CONbits_t;
extern volatile __I2C4CONbits_t I2C4CONbits __asm__ ("I2C4CON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C2ASTATbits_t;
extern volatile __I2C2ASTATbits_t I2C2ASTATbits __asm__ ("I2C2ASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C4STATbits_t;
extern volatile __I2C4STATbits_t I2C4STATbits __asm__ ("I2C4STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4ADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4ADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4ADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4ADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AMSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4MSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AMSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4MSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AMSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4MSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2AMSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4MSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4BRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ATRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4TRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ATRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4TRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ATRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4TRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ATRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4TRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C2ARCV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C4RCV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C3ACONbits_t;
extern volatile __I2C3ACONbits_t I2C3ACONbits __asm__ ("I2C3ACON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C5CONbits_t;
extern volatile __I2C5CONbits_t I2C5CONbits __asm__ ("I2C5CON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C3ASTATbits_t;
extern volatile __I2C3ASTATbits_t I2C3ASTATbits __asm__ ("I2C3ASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C5STATbits_t;
extern volatile __I2C5STATbits_t I2C5STATbits __asm__ ("I2C5STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5ADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5ADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5ADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5ADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AMSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5MSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AMSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5MSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AMSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5MSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3AMSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5MSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5BRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ATRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5TRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ATRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5TRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ATRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5TRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ATRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5TRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C3ARCV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C5RCV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SEN:1;
    unsigned RSEN:1;
    unsigned PEN:1;
    unsigned RCEN:1;
    unsigned ACKEN:1;
    unsigned ACKDT:1;
    unsigned STREN:1;
    unsigned GCEN:1;
    unsigned SMEN:1;
    unsigned DISSLW:1;
    unsigned A10M:1;
    unsigned STRICT:1;
    unsigned SCLREL:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :11;
    unsigned IPMIEN:1;
    unsigned :1;
    unsigned I2CSIDL:1;
    unsigned :1;
    unsigned I2CEN:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C1CONbits_t;
extern volatile __I2C1CONbits_t I2C1CONbits __asm__ ("I2C1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TBF:1;
    unsigned RBF:1;
    unsigned R_W:1;
    unsigned S:1;
    unsigned P:1;
    unsigned D_A:1;
    unsigned I2COV:1;
    unsigned IWCOL:1;
    unsigned ADD10:1;
    unsigned GCSTAT:1;
    unsigned BCL:1;
    unsigned :3;
    unsigned TRSTAT:1;
    unsigned ACKSTAT:1;
  };
  struct {
    unsigned :6;
    unsigned I2CPOV:1;
  };
  struct {
    unsigned w:32;
  };
} __I2C1STATbits_t;
extern volatile __I2C1STATbits_t I2C1STATbits __asm__ ("I2C1STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ADD __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ADDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ADDSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1ADDINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1MSK __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1MSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1MSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1MSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1BRG __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1TRN __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1TRNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1TRNSET __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1TRNINV __attribute__((section("sfrs")));
extern volatile unsigned int        I2C1RCV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SRXISEL:2;
    unsigned STXISEL:2;
    unsigned :1;
    unsigned MSTEN:1;
    unsigned CKP:1;
    unsigned SSEN:1;
    unsigned CKE:1;
    unsigned SMP:1;
    unsigned MODE16:1;
    unsigned MODE32:1;
    unsigned DISSDO:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned ENHBUF:1;
    unsigned SPIFE:1;
    unsigned :6;
    unsigned FRMCNT:3;
    unsigned FRMSYPW:1;
    unsigned MSSEN:1;
    unsigned FRMPOL:1;
    unsigned FRMSYNC:1;
    unsigned FRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __SPI1ACONbits_t;
extern volatile __SPI1ACONbits_t SPI1ACONbits __asm__ ("SPI1ACON") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SRXISEL:2;
    unsigned STXISEL:2;
    unsigned :1;
    unsigned MSTEN:1;
    unsigned CKP:1;
    unsigned SSEN:1;
    unsigned CKE:1;
    unsigned SMP:1;
    unsigned MODE16:1;
    unsigned MODE32:1;
    unsigned DISSDO:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned ENHBUF:1;
    unsigned SPIFE:1;
    unsigned :6;
    unsigned FRMCNT:3;
    unsigned FRMSYPW:1;
    unsigned MSSEN:1;
    unsigned FRMPOL:1;
    unsigned FRMSYNC:1;
    unsigned FRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __SPI3CONbits_t;
extern volatile __SPI3CONbits_t SPI3CONbits __asm__ ("SPI3CON") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SPIRBF:1;
    unsigned SPITBF:1;
    unsigned :1;
    unsigned SPITBE:1;
    unsigned :1;
    unsigned SPIRBE:1;
    unsigned SPIROV:1;
    unsigned SRMT:1;
    unsigned SPITUR:1;
    unsigned :2;
    unsigned SPIBUSY:1;
    unsigned :4;
    unsigned TXBUFELM:5;
    unsigned :3;
    unsigned RXBUFELM:5;
  };
  struct {
    unsigned w:32;
  };
} __SPI1ASTATbits_t;
extern volatile __SPI1ASTATbits_t SPI1ASTATbits __asm__ ("SPI1ASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SPIRBF:1;
    unsigned SPITBF:1;
    unsigned :1;
    unsigned SPITBE:1;
    unsigned :1;
    unsigned SPIRBE:1;
    unsigned SPIROV:1;
    unsigned SRMT:1;
    unsigned SPITUR:1;
    unsigned :2;
    unsigned SPIBUSY:1;
    unsigned :4;
    unsigned TXBUFELM:5;
    unsigned :3;
    unsigned RXBUFELM:5;
  };
  struct {
    unsigned w:32;
  };
} __SPI3STATbits_t;
extern volatile __SPI3STATbits_t SPI3STATbits __asm__ ("SPI3STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ABUF __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3BUF __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3BRG __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI1ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SRXISEL:2;
    unsigned STXISEL:2;
    unsigned :1;
    unsigned MSTEN:1;
    unsigned CKP:1;
    unsigned SSEN:1;
    unsigned CKE:1;
    unsigned SMP:1;
    unsigned MODE16:1;
    unsigned MODE32:1;
    unsigned DISSDO:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned ENHBUF:1;
    unsigned SPIFE:1;
    unsigned :6;
    unsigned FRMCNT:3;
    unsigned FRMSYPW:1;
    unsigned MSSEN:1;
    unsigned FRMPOL:1;
    unsigned FRMSYNC:1;
    unsigned FRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __SPI2ACONbits_t;
extern volatile __SPI2ACONbits_t SPI2ACONbits __asm__ ("SPI2ACON") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SRXISEL:2;
    unsigned STXISEL:2;
    unsigned :1;
    unsigned MSTEN:1;
    unsigned CKP:1;
    unsigned SSEN:1;
    unsigned CKE:1;
    unsigned SMP:1;
    unsigned MODE16:1;
    unsigned MODE32:1;
    unsigned DISSDO:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned ENHBUF:1;
    unsigned SPIFE:1;
    unsigned :6;
    unsigned FRMCNT:3;
    unsigned FRMSYPW:1;
    unsigned MSSEN:1;
    unsigned FRMPOL:1;
    unsigned FRMSYNC:1;
    unsigned FRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __SPI2CONbits_t;
extern volatile __SPI2CONbits_t SPI2CONbits __asm__ ("SPI2CON") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SPIRBF:1;
    unsigned SPITBF:1;
    unsigned :1;
    unsigned SPITBE:1;
    unsigned :1;
    unsigned SPIRBE:1;
    unsigned SPIROV:1;
    unsigned SRMT:1;
    unsigned SPITUR:1;
    unsigned :2;
    unsigned SPIBUSY:1;
    unsigned :4;
    unsigned TXBUFELM:5;
    unsigned :3;
    unsigned RXBUFELM:5;
  };
  struct {
    unsigned w:32;
  };
} __SPI2ASTATbits_t;
extern volatile __SPI2ASTATbits_t SPI2ASTATbits __asm__ ("SPI2ASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SPIRBF:1;
    unsigned SPITBF:1;
    unsigned :1;
    unsigned SPITBE:1;
    unsigned :1;
    unsigned SPIRBE:1;
    unsigned SPIROV:1;
    unsigned SRMT:1;
    unsigned SPITUR:1;
    unsigned :2;
    unsigned SPIBUSY:1;
    unsigned :4;
    unsigned TXBUFELM:5;
    unsigned :3;
    unsigned RXBUFELM:5;
  };
  struct {
    unsigned w:32;
  };
} __SPI2STATbits_t;
extern volatile __SPI2STATbits_t SPI2STATbits __asm__ ("SPI2STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ABUF __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2BUF __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2BRG __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI2BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SRXISEL:2;
    unsigned STXISEL:2;
    unsigned :1;
    unsigned MSTEN:1;
    unsigned CKP:1;
    unsigned SSEN:1;
    unsigned CKE:1;
    unsigned SMP:1;
    unsigned MODE16:1;
    unsigned MODE32:1;
    unsigned DISSDO:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned ENHBUF:1;
    unsigned SPIFE:1;
    unsigned :6;
    unsigned FRMCNT:3;
    unsigned FRMSYPW:1;
    unsigned MSSEN:1;
    unsigned FRMPOL:1;
    unsigned FRMSYNC:1;
    unsigned FRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __SPI3ACONbits_t;
extern volatile __SPI3ACONbits_t SPI3ACONbits __asm__ ("SPI3ACON") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SRXISEL:2;
    unsigned STXISEL:2;
    unsigned :1;
    unsigned MSTEN:1;
    unsigned CKP:1;
    unsigned SSEN:1;
    unsigned CKE:1;
    unsigned SMP:1;
    unsigned MODE16:1;
    unsigned MODE32:1;
    unsigned DISSDO:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned ENHBUF:1;
    unsigned SPIFE:1;
    unsigned :6;
    unsigned FRMCNT:3;
    unsigned FRMSYPW:1;
    unsigned MSSEN:1;
    unsigned FRMPOL:1;
    unsigned FRMSYNC:1;
    unsigned FRMEN:1;
  };
  struct {
    unsigned w:32;
  };
} __SPI4CONbits_t;
extern volatile __SPI4CONbits_t SPI4CONbits __asm__ ("SPI4CON") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SPIRBF:1;
    unsigned SPITBF:1;
    unsigned :1;
    unsigned SPITBE:1;
    unsigned :1;
    unsigned SPIRBE:1;
    unsigned SPIROV:1;
    unsigned SRMT:1;
    unsigned SPITUR:1;
    unsigned :2;
    unsigned SPIBUSY:1;
    unsigned :4;
    unsigned TXBUFELM:5;
    unsigned :3;
    unsigned RXBUFELM:5;
  };
  struct {
    unsigned w:32;
  };
} __SPI3ASTATbits_t;
extern volatile __SPI3ASTATbits_t SPI3ASTATbits __asm__ ("SPI3ASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SPIRBF:1;
    unsigned SPITBF:1;
    unsigned :1;
    unsigned SPITBE:1;
    unsigned :1;
    unsigned SPIRBE:1;
    unsigned SPIROV:1;
    unsigned SRMT:1;
    unsigned SPITUR:1;
    unsigned :2;
    unsigned SPIBUSY:1;
    unsigned :4;
    unsigned TXBUFELM:5;
    unsigned :3;
    unsigned RXBUFELM:5;
  };
  struct {
    unsigned w:32;
  };
} __SPI4STATbits_t;
extern volatile __SPI4STATbits_t SPI4STATbits __asm__ ("SPI4STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ABUF __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4BUF __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4BRG __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        SPI3ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        SPI4BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1AMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned UEN:2;
    unsigned :1;
    unsigned RTSMD:1;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
    unsigned :5;
    unsigned UEN0:1;
    unsigned UEN1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U1AMODEbits_t;
extern volatile __U1AMODEbits_t U1AMODEbits __asm__ ("U1AMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U1MODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned UEN:2;
    unsigned :1;
    unsigned RTSMD:1;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
    unsigned :5;
    unsigned UEN0:1;
    unsigned UEN1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U1MODEbits_t;
extern volatile __U1MODEbits_t U1MODEbits __asm__ ("U1MODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U1AMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1MODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1AMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U1MODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U1AMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1MODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1ASTA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U1ASTAbits_t;
extern volatile __U1ASTAbits_t U1ASTAbits __asm__ ("U1ASTA") __attribute__((section("sfrs")));
extern volatile unsigned int        U1STA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U1STAbits_t;
extern volatile __U1STAbits_t U1STAbits __asm__ ("U1STA") __attribute__((section("sfrs")));
extern volatile unsigned int        U1ASTACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1STACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1ASTASET __attribute__((section("sfrs")));
extern volatile unsigned int        U1STASET __attribute__((section("sfrs")));
extern volatile unsigned int        U1ASTAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1STAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1ATXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U1TXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U1ARXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U1RXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U1ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        U1BRG __attribute__((section("sfrs")));
extern volatile unsigned int        U1ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned :4;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U1BMODEbits_t;
extern volatile __U1BMODEbits_t U1BMODEbits __asm__ ("U1BMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U4MODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned :4;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U4MODEbits_t;
extern volatile __U4MODEbits_t U4MODEbits __asm__ ("U4MODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U1BMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U4MODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U4MODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U4MODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BSTA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U1BSTAbits_t;
extern volatile __U1BSTAbits_t U1BSTAbits __asm__ ("U1BSTA") __attribute__((section("sfrs")));
extern volatile unsigned int        U4STA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U4STAbits_t;
extern volatile __U4STAbits_t U4STAbits __asm__ ("U4STA") __attribute__((section("sfrs")));
extern volatile unsigned int        U1BSTACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U4STACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BSTASET __attribute__((section("sfrs")));
extern volatile unsigned int        U4STASET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BSTAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U4STAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BTXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U4TXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U1BRXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U4RXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U1BBRG __attribute__((section("sfrs")));
extern volatile unsigned int        U4BRG __attribute__((section("sfrs")));
extern volatile unsigned int        U1BBRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U4BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BBRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U4BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BBRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U4BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2AMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned UEN:2;
    unsigned :1;
    unsigned RTSMD:1;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
    unsigned :5;
    unsigned UEN0:1;
    unsigned UEN1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U2AMODEbits_t;
extern volatile __U2AMODEbits_t U2AMODEbits __asm__ ("U2AMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U3MODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned UEN:2;
    unsigned :1;
    unsigned RTSMD:1;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
    unsigned :5;
    unsigned UEN0:1;
    unsigned UEN1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U3MODEbits_t;
extern volatile __U3MODEbits_t U3MODEbits __asm__ ("U3MODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U2AMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3MODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2AMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U3MODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U2AMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3MODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2ASTA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U2ASTAbits_t;
extern volatile __U2ASTAbits_t U2ASTAbits __asm__ ("U2ASTA") __attribute__((section("sfrs")));
extern volatile unsigned int        U3STA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U3STAbits_t;
extern volatile __U3STAbits_t U3STAbits __asm__ ("U3STA") __attribute__((section("sfrs")));
extern volatile unsigned int        U2ASTACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3STACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2ASTASET __attribute__((section("sfrs")));
extern volatile unsigned int        U3STASET __attribute__((section("sfrs")));
extern volatile unsigned int        U2ASTAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3STAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2ATXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U3TXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U2ARXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U3RXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U2ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        U3BRG __attribute__((section("sfrs")));
extern volatile unsigned int        U2ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U3BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U2ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2BMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned :4;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U2BMODEbits_t;
extern volatile __U2BMODEbits_t U2BMODEbits __asm__ ("U2BMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U6MODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned :4;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U6MODEbits_t;
extern volatile __U6MODEbits_t U6MODEbits __asm__ ("U6MODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U2BMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U6MODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2BMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U6MODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U2BMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U6MODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2BSTA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U2BSTAbits_t;
extern volatile __U2BSTAbits_t U2BSTAbits __asm__ ("U2BSTA") __attribute__((section("sfrs")));
extern volatile unsigned int        U6STA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U6STAbits_t;
extern volatile __U6STAbits_t U6STAbits __asm__ ("U6STA") __attribute__((section("sfrs")));
extern volatile unsigned int        U2BSTACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U6STACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2BSTASET __attribute__((section("sfrs")));
extern volatile unsigned int        U6STASET __attribute__((section("sfrs")));
extern volatile unsigned int        U2BSTAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U6STAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2BTXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U6TXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U2BRXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U6RXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U2BBRG __attribute__((section("sfrs")));
extern volatile unsigned int        U6BRG __attribute__((section("sfrs")));
extern volatile unsigned int        U2BBRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U6BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2BBRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U6BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U2BBRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U6BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2MODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned UEN:2;
    unsigned :1;
    unsigned RTSMD:1;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
    unsigned :5;
    unsigned UEN0:1;
    unsigned UEN1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U2MODEbits_t;
extern volatile __U2MODEbits_t U2MODEbits __asm__ ("U2MODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U3AMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned UEN:2;
    unsigned :1;
    unsigned RTSMD:1;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
    unsigned :5;
    unsigned UEN0:1;
    unsigned UEN1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U3AMODEbits_t;
extern volatile __U3AMODEbits_t U3AMODEbits __asm__ ("U3AMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U2MODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3AMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2MODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U3AMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U2MODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3AMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2STA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U2STAbits_t;
extern volatile __U2STAbits_t U2STAbits __asm__ ("U2STA") __attribute__((section("sfrs")));
extern volatile unsigned int        U3ASTA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U3ASTAbits_t;
extern volatile __U3ASTAbits_t U3ASTAbits __asm__ ("U3ASTA") __attribute__((section("sfrs")));
extern volatile unsigned int        U2STACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3ASTACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2STASET __attribute__((section("sfrs")));
extern volatile unsigned int        U3ASTASET __attribute__((section("sfrs")));
extern volatile unsigned int        U2STAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3ASTAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U2TXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U3ATXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U2RXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U3ARXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U2BRG __attribute__((section("sfrs")));
extern volatile unsigned int        U3ABRG __attribute__((section("sfrs")));
extern volatile unsigned int        U2BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3ABRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U2BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U3ABRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U2BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3ABRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3BMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned :4;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U3BMODEbits_t;
extern volatile __U3BMODEbits_t U3BMODEbits __asm__ ("U3BMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U5MODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STSEL:1;
    unsigned PDSEL:2;
    unsigned BRGH:1;
    unsigned RXINV:1;
    unsigned ABAUD:1;
    unsigned LPBACK:1;
    unsigned WAKE:1;
    unsigned :4;
    unsigned IREN:1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :1;
    unsigned PDSEL0:1;
    unsigned PDSEL1:1;
  };
  struct {
    unsigned :13;
    unsigned USIDL:1;
    unsigned :1;
    unsigned UARTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __U5MODEbits_t;
extern volatile __U5MODEbits_t U5MODEbits __asm__ ("U5MODE") __attribute__((section("sfrs")));
extern volatile unsigned int        U3BMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U5MODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3BMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U5MODESET __attribute__((section("sfrs")));
extern volatile unsigned int        U3BMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U5MODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3BSTA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U3BSTAbits_t;
extern volatile __U3BSTAbits_t U3BSTAbits __asm__ ("U3BSTA") __attribute__((section("sfrs")));
extern volatile unsigned int        U5STA __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URXDA:1;
    unsigned OERR:1;
    unsigned FERR:1;
    unsigned PERR:1;
    unsigned RIDLE:1;
    unsigned ADDEN:1;
    unsigned URXISEL:2;
    unsigned TRMT:1;
    unsigned UTXBF:1;
    unsigned UTXEN:1;
    unsigned UTXBRK:1;
    unsigned URXEN:1;
    unsigned UTXINV:1;
    unsigned UTXISEL:2;
    unsigned ADDR:8;
    unsigned ADM_EN:1;
  };
  struct {
    unsigned :6;
    unsigned URXISEL0:1;
    unsigned URXISEL1:1;
    unsigned :6;
    unsigned UTXISEL0:1;
    unsigned UTXISEL1:1;
  };
  struct {
    unsigned :14;
    unsigned UTXSEL:2;
  };
  struct {
    unsigned w:32;
  };
} __U5STAbits_t;
extern volatile __U5STAbits_t U5STAbits __asm__ ("U5STA") __attribute__((section("sfrs")));
extern volatile unsigned int        U3BSTACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U5STACLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3BSTASET __attribute__((section("sfrs")));
extern volatile unsigned int        U5STASET __attribute__((section("sfrs")));
extern volatile unsigned int        U3BSTAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U5STAINV __attribute__((section("sfrs")));
extern volatile unsigned int        U3BTXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U5TXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U3BRXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U5RXREG __attribute__((section("sfrs")));
extern volatile unsigned int        U3BBRG __attribute__((section("sfrs")));
extern volatile unsigned int        U5BRG __attribute__((section("sfrs")));
extern volatile unsigned int        U3BBRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U5BRGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U3BBRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U5BRGSET __attribute__((section("sfrs")));
extern volatile unsigned int        U3BBRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        U5BRGINV __attribute__((section("sfrs")));
extern volatile unsigned int        PMCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RDSP:1;
    unsigned WRSP:1;
    unsigned :1;
    unsigned CS1P:1;
    unsigned CS2P:1;
    unsigned ALP:1;
    unsigned CSF:2;
    unsigned PTRDEN:1;
    unsigned PTWREN:1;
    unsigned PMPTTL:1;
    unsigned ADRMUX:2;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :6;
    unsigned CSF0:1;
    unsigned CSF1:1;
    unsigned :3;
    unsigned ADRMUX0:1;
    unsigned ADRMUX1:1;
  };
  struct {
    unsigned :13;
    unsigned PSIDL:1;
    unsigned :1;
    unsigned PMPEN:1;
  };
  struct {
    unsigned w:32;
  };
} __PMCONbits_t;
extern volatile __PMCONbits_t PMCONbits __asm__ ("PMCON") __attribute__((section("sfrs")));
extern volatile unsigned int        PMCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        PMCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        PMMODE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned WAITE:2;
    unsigned WAITM:4;
    unsigned WAITB:2;
    unsigned MODE:2;
    unsigned MODE16:1;
    unsigned INCM:2;
    unsigned IRQM:2;
    unsigned BUSY:1;
  };
  struct {
    unsigned WAITE0:1;
    unsigned WAITE1:1;
    unsigned WAITM0:1;
    unsigned WAITM1:1;
    unsigned WAITM2:1;
    unsigned WAITM3:1;
    unsigned WAITB0:1;
    unsigned WAITB1:1;
    unsigned MODE0:1;
    unsigned MODE1:1;
    unsigned :1;
    unsigned INCM0:1;
    unsigned INCM1:1;
    unsigned IRQM0:1;
    unsigned IRQM1:1;
  };
  struct {
    unsigned w:32;
  };
} __PMMODEbits_t;
extern volatile __PMMODEbits_t PMMODEbits __asm__ ("PMMODE") __attribute__((section("sfrs")));
extern volatile unsigned int        PMMODECLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMMODESET __attribute__((section("sfrs")));
extern volatile unsigned int        PMMODEINV __attribute__((section("sfrs")));
extern volatile unsigned int        PMADDR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ADDR:14;
    unsigned CS1:1;
    unsigned CS2:1;
  };
  struct {
    unsigned PADDR:14;
  };
  struct {
    unsigned :14;
    unsigned CS:2;
  };
  struct {
    unsigned w:32;
  };
} __PMADDRbits_t;
extern volatile __PMADDRbits_t PMADDRbits __asm__ ("PMADDR") __attribute__((section("sfrs")));
extern volatile unsigned int        PMADDRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMADDRSET __attribute__((section("sfrs")));
extern volatile unsigned int        PMADDRINV __attribute__((section("sfrs")));
extern volatile unsigned int        PMDOUT __attribute__((section("sfrs")));
extern volatile unsigned int        PMDOUTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMDOUTSET __attribute__((section("sfrs")));
extern volatile unsigned int        PMDOUTINV __attribute__((section("sfrs")));
extern volatile unsigned int        PMDIN __attribute__((section("sfrs")));
extern volatile unsigned int        PMDINCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMDINSET __attribute__((section("sfrs")));
extern volatile unsigned int        PMDININV __attribute__((section("sfrs")));
extern volatile unsigned int        PMAEN __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PTEN:16;
  };
  struct {
    unsigned PTEN0:1;
    unsigned PTEN1:1;
    unsigned PTEN2:1;
    unsigned PTEN3:1;
    unsigned PTEN4:1;
    unsigned PTEN5:1;
    unsigned PTEN6:1;
    unsigned PTEN7:1;
    unsigned PTEN8:1;
    unsigned PTEN9:1;
    unsigned PTEN10:1;
    unsigned PTEN11:1;
    unsigned PTEN12:1;
    unsigned PTEN13:1;
    unsigned PTEN14:1;
    unsigned PTEN15:1;
  };
  struct {
    unsigned w:32;
  };
} __PMAENbits_t;
extern volatile __PMAENbits_t PMAENbits __asm__ ("PMAEN") __attribute__((section("sfrs")));
extern volatile unsigned int        PMAENCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMAENSET __attribute__((section("sfrs")));
extern volatile unsigned int        PMAENINV __attribute__((section("sfrs")));
extern volatile unsigned int        PMSTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OB0E:1;
    unsigned OB1E:1;
    unsigned OB2E:1;
    unsigned OB3E:1;
    unsigned :2;
    unsigned OBUF:1;
    unsigned OBE:1;
    unsigned IB0F:1;
    unsigned IB1F:1;
    unsigned IB2F:1;
    unsigned IB3F:1;
    unsigned :2;
    unsigned IBOV:1;
    unsigned IBF:1;
  };
  struct {
    unsigned w:32;
  };
} __PMSTATbits_t;
extern volatile __PMSTATbits_t PMSTATbits __asm__ ("PMSTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        PMSTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PMSTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        PMSTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned DONE:1;
    unsigned SAMP:1;
    unsigned ASAM:1;
    unsigned :1;
    unsigned CLRASAM:1;
    unsigned SSRC:3;
    unsigned FORM:3;
    unsigned :2;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned :5;
    unsigned SSRC0:1;
    unsigned SSRC1:1;
    unsigned SSRC2:1;
    unsigned FORM0:1;
    unsigned FORM1:1;
    unsigned FORM2:1;
  };
  struct {
    unsigned :13;
    unsigned ADSIDL:1;
    unsigned :1;
    unsigned ADON:1;
  };
  struct {
    unsigned w:32;
  };
} __AD1CON1bits_t;
extern volatile __AD1CON1bits_t AD1CON1bits __asm__ ("AD1CON1") __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON1SET __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON1INV __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ALTS:1;
    unsigned BUFM:1;
    unsigned SMPI:4;
    unsigned :1;
    unsigned BUFS:1;
    unsigned :2;
    unsigned CSCNA:1;
    unsigned :1;
    unsigned OFFCAL:1;
    unsigned VCFG:3;
  };
  struct {
    unsigned :2;
    unsigned SMPI0:1;
    unsigned SMPI1:1;
    unsigned SMPI2:1;
    unsigned SMPI3:1;
    unsigned :7;
    unsigned VCFG0:1;
    unsigned VCFG1:1;
    unsigned VCFG2:1;
  };
  struct {
    unsigned w:32;
  };
} __AD1CON2bits_t;
extern volatile __AD1CON2bits_t AD1CON2bits __asm__ ("AD1CON2") __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON2SET __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON2INV __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON3 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ADCS:8;
    unsigned SAMC:5;
    unsigned :2;
    unsigned ADRC:1;
  };
  struct {
    unsigned ADCS0:1;
    unsigned ADCS1:1;
    unsigned ADCS2:1;
    unsigned ADCS3:1;
    unsigned ADCS4:1;
    unsigned ADCS5:1;
    unsigned ADCS6:1;
    unsigned ADCS7:1;
    unsigned SAMC0:1;
    unsigned SAMC1:1;
    unsigned SAMC2:1;
    unsigned SAMC3:1;
    unsigned SAMC4:1;
  };
  struct {
    unsigned w:32;
  };
} __AD1CON3bits_t;
extern volatile __AD1CON3bits_t AD1CON3bits __asm__ ("AD1CON3") __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON3CLR __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON3SET __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CON3INV __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CHS __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :16;
    unsigned CH0SA:4;
    unsigned :3;
    unsigned CH0NA:1;
    unsigned CH0SB:4;
    unsigned :3;
    unsigned CH0NB:1;
  };
  struct {
    unsigned :16;
    unsigned CH0SA0:1;
    unsigned CH0SA1:1;
    unsigned CH0SA2:1;
    unsigned CH0SA3:1;
    unsigned :4;
    unsigned CH0SB0:1;
    unsigned CH0SB1:1;
    unsigned CH0SB2:1;
    unsigned CH0SB3:1;
  };
  struct {
    unsigned w:32;
  };
} __AD1CHSbits_t;
extern volatile __AD1CHSbits_t AD1CHSbits __asm__ ("AD1CHS") __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CHSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CHSSET __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CHSINV __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CSSL __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CSSL:16;
  };
  struct {
    unsigned CSSL0:1;
    unsigned CSSL1:1;
    unsigned CSSL2:1;
    unsigned CSSL3:1;
    unsigned CSSL4:1;
    unsigned CSSL5:1;
    unsigned CSSL6:1;
    unsigned CSSL7:1;
    unsigned CSSL8:1;
    unsigned CSSL9:1;
    unsigned CSSL10:1;
    unsigned CSSL11:1;
    unsigned CSSL12:1;
    unsigned CSSL13:1;
    unsigned CSSL14:1;
    unsigned CSSL15:1;
  };
  struct {
    unsigned w:32;
  };
} __AD1CSSLbits_t;
extern volatile __AD1CSSLbits_t AD1CSSLbits __asm__ ("AD1CSSL") __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CSSLCLR __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CSSLSET __attribute__((section("sfrs")));
extern volatile unsigned int        AD1CSSLINV __attribute__((section("sfrs")));
extern volatile unsigned int        AD1PCFG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PCFG:16;
  };
  struct {
    unsigned PCFG0:1;
    unsigned PCFG1:1;
    unsigned PCFG2:1;
    unsigned PCFG3:1;
    unsigned PCFG4:1;
    unsigned PCFG5:1;
    unsigned PCFG6:1;
    unsigned PCFG7:1;
    unsigned PCFG8:1;
    unsigned PCFG9:1;
    unsigned PCFG10:1;
    unsigned PCFG11:1;
    unsigned PCFG12:1;
    unsigned PCFG13:1;
    unsigned PCFG14:1;
    unsigned PCFG15:1;
  };
  struct {
    unsigned w:32;
  };
} __AD1PCFGbits_t;
extern volatile __AD1PCFGbits_t AD1PCFGbits __asm__ ("AD1PCFG") __attribute__((section("sfrs")));
extern volatile unsigned int        AD1PCFGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        AD1PCFGSET __attribute__((section("sfrs")));
extern volatile unsigned int        AD1PCFGINV __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF0 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF1 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF2 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF3 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF4 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF5 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF6 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF7 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF8 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUF9 __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUFA __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUFB __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUFC __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUFD __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUFE __attribute__((section("sfrs")));
extern volatile unsigned int        ADC1BUFF __attribute__((section("sfrs")));
extern volatile unsigned int        CVRCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CVR:4;
    unsigned CVRSS:1;
    unsigned CVRR:1;
    unsigned CVROE:1;
    unsigned :8;
    unsigned ON:1;
  };
  struct {
    unsigned CVR0:1;
    unsigned CVR1:1;
    unsigned CVR2:1;
    unsigned CVR3:1;
  };
  struct {
    unsigned w:32;
  };
} __CVRCONbits_t;
extern volatile __CVRCONbits_t CVRCONbits __asm__ ("CVRCON") __attribute__((section("sfrs")));
extern volatile unsigned int        CVRCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CVRCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        CVRCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        CM1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CCH:2;
    unsigned :2;
    unsigned CREF:1;
    unsigned :1;
    unsigned EVPOL:2;
    unsigned COUT:1;
    unsigned :4;
    unsigned CPOL:1;
    unsigned COE:1;
    unsigned ON:1;
  };
  struct {
    unsigned CCH0:1;
    unsigned CCH1:1;
    unsigned :4;
    unsigned EVPOL0:1;
    unsigned EVPOL1:1;
  };
  struct {
    unsigned w:32;
  };
} __CM1CONbits_t;
extern volatile __CM1CONbits_t CM1CONbits __asm__ ("CM1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        CM1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CM1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        CM1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        CM2CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CCH:2;
    unsigned :2;
    unsigned CREF:1;
    unsigned :1;
    unsigned EVPOL:2;
    unsigned COUT:1;
    unsigned :4;
    unsigned CPOL:1;
    unsigned COE:1;
    unsigned ON:1;
  };
  struct {
    unsigned CCH0:1;
    unsigned CCH1:1;
    unsigned :4;
    unsigned EVPOL0:1;
    unsigned EVPOL1:1;
  };
  struct {
    unsigned w:32;
  };
} __CM2CONbits_t;
extern volatile __CM2CONbits_t CM2CONbits __asm__ ("CM2CON") __attribute__((section("sfrs")));
extern volatile unsigned int        CM2CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CM2CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        CM2CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        CMSTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned C1OUT:1;
    unsigned C2OUT:1;
    unsigned :11;
    unsigned SIDL:1;
  };
  struct {
    unsigned w:32;
  };
} __CMSTATbits_t;
extern volatile __CMSTATbits_t CMSTATbits __asm__ ("CMSTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        CMSTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CMSTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        CMSTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        OSCCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned OSWEN:1;
    unsigned SOSCEN:1;
    unsigned UFRCEN:1;
    unsigned CF:1;
    unsigned SLPEN:1;
    unsigned SLOCK:1;
    unsigned ULOCK:1;
    unsigned CLKLOCK:1;
    unsigned NOSC:3;
    unsigned :1;
    unsigned COSC:3;
    unsigned :1;
    unsigned PLLMULT:3;
    unsigned PBDIV:2;
    unsigned :1;
    unsigned SOSCRDY:1;
    unsigned :1;
    unsigned FRCDIV:3;
    unsigned PLLODIV:3;
  };
  struct {
    unsigned :8;
    unsigned NOSC0:1;
    unsigned NOSC1:1;
    unsigned NOSC2:1;
    unsigned :1;
    unsigned COSC0:1;
    unsigned COSC1:1;
    unsigned COSC2:1;
    unsigned :1;
    unsigned PLLMULT0:1;
    unsigned PLLMULT1:1;
    unsigned PLLMULT2:1;
    unsigned PBDIV0:1;
    unsigned PBDIV1:1;
    unsigned :3;
    unsigned FRCDIV0:1;
    unsigned FRCDIV1:1;
    unsigned FRCDIV2:1;
    unsigned PLLODIV0:1;
    unsigned PLLODIV1:1;
    unsigned PLLODIV2:1;
  };
  struct {
    unsigned w:32;
  };
} __OSCCONbits_t;
extern volatile __OSCCONbits_t OSCCONbits __asm__ ("OSCCON") __attribute__((section("sfrs")));
extern volatile unsigned int        OSCCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OSCCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        OSCCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        OSCTUN __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TUN:6;
  };
  struct {
    unsigned TUN0:1;
    unsigned TUN1:1;
    unsigned TUN2:1;
    unsigned TUN3:1;
    unsigned TUN4:1;
    unsigned TUN5:1;
  };
  struct {
    unsigned w:32;
  };
} __OSCTUNbits_t;
extern volatile __OSCTUNbits_t OSCTUNbits __asm__ ("OSCTUN") __attribute__((section("sfrs")));
extern volatile unsigned int        OSCTUNCLR __attribute__((section("sfrs")));
extern volatile unsigned int        OSCTUNSET __attribute__((section("sfrs")));
extern volatile unsigned int        OSCTUNINV __attribute__((section("sfrs")));
extern volatile unsigned int        DDPCON __attribute__((section("sfrs")));
typedef struct {
  unsigned TDOEN:1;
  unsigned :1;
  unsigned TROEN:1;
  unsigned JTAGEN:1;
} __DDPCONbits_t;
extern volatile __DDPCONbits_t DDPCONbits __asm__ ("DDPCON") __attribute__((section("sfrs")));
extern volatile unsigned int        DEVID __attribute__((section("sfrs")));
typedef struct {
  unsigned DEVID:28;
  unsigned VER:4;
} __DEVIDbits_t;
extern volatile __DEVIDbits_t DEVIDbits __asm__ ("DEVID") __attribute__((section("sfrs")));
extern volatile unsigned int        SYSKEY __attribute__((section("sfrs")));
extern volatile unsigned int        SYSKEYCLR __attribute__((section("sfrs")));
extern volatile unsigned int        SYSKEYSET __attribute__((section("sfrs")));
extern volatile unsigned int        SYSKEYINV __attribute__((section("sfrs")));
extern volatile unsigned int        NVMCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned NVMOP:4;
    unsigned :7;
    unsigned LVDSTAT:1;
    unsigned LVDERR:1;
    unsigned WRERR:1;
    unsigned WREN:1;
    unsigned WR:1;
  };
  struct {
    unsigned NVMOP0:1;
    unsigned NVMOP1:1;
    unsigned NVMOP2:1;
    unsigned NVMOP3:1;
  };
  struct {
    unsigned PROGOP:4;
  };
  struct {
    unsigned PROGOP0:1;
    unsigned PROGOP1:1;
    unsigned PROGOP2:1;
    unsigned PROGOP3:1;
  };
  struct {
    unsigned w:32;
  };
} __NVMCONbits_t;
extern volatile __NVMCONbits_t NVMCONbits __asm__ ("NVMCON") __attribute__((section("sfrs")));
extern volatile unsigned int        NVMCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        NVMCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        NVMCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        NVMKEY __attribute__((section("sfrs")));
extern volatile unsigned int        NVMADDR __attribute__((section("sfrs")));
extern volatile unsigned int        NVMADDRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        NVMADDRSET __attribute__((section("sfrs")));
extern volatile unsigned int        NVMADDRINV __attribute__((section("sfrs")));
extern volatile unsigned int        NVMDATA __attribute__((section("sfrs")));
extern volatile unsigned int        NVMSRCADDR __attribute__((section("sfrs")));
extern volatile unsigned int        RCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned POR:1;
    unsigned BOR:1;
    unsigned IDLE:1;
    unsigned SLEEP:1;
    unsigned WDTO:1;
    unsigned :1;
    unsigned SWR:1;
    unsigned EXTR:1;
    unsigned VREGS:1;
    unsigned CMR:1;
  };
  struct {
    unsigned w:32;
  };
} __RCONbits_t;
extern volatile __RCONbits_t RCONbits __asm__ ("RCON") __attribute__((section("sfrs")));
extern volatile unsigned int        RCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        RCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        RCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        RSWRST __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SWRST:1;
  };
  struct {
    unsigned w:32;
  };
} __RSWRSTbits_t;
extern volatile __RSWRSTbits_t RSWRSTbits __asm__ ("RSWRST") __attribute__((section("sfrs")));
extern volatile unsigned int        RSWRSTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        RSWRSTSET __attribute__((section("sfrs")));
extern volatile unsigned int        RSWRSTINV __attribute__((section("sfrs")));
extern volatile unsigned int        _DDPSTAT __attribute__((section("sfrs")));
typedef struct {
  unsigned :1;
  unsigned APIFUL:1;
  unsigned APOFUL:1;
  unsigned STRFUL:1;
  unsigned :5;
  unsigned APIOV:1;
  unsigned APOOV:1;
  unsigned :5;
  unsigned STOV:16;
} ___DDPSTATbits_t;
extern volatile ___DDPSTATbits_t _DDPSTATbits __asm__ ("_DDPSTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        _STRO __attribute__((section("sfrs")));
extern volatile unsigned int        _STROCLR __attribute__((section("sfrs")));
extern volatile unsigned int        _STROSET __attribute__((section("sfrs")));
extern volatile unsigned int        _STROINV __attribute__((section("sfrs")));
extern volatile unsigned int        _APPO __attribute__((section("sfrs")));
extern volatile unsigned int        _APPOCLR __attribute__((section("sfrs")));
extern volatile unsigned int        _APPOSET __attribute__((section("sfrs")));
extern volatile unsigned int        _APPOINV __attribute__((section("sfrs")));
extern volatile unsigned int        _APPI __attribute__((section("sfrs")));
extern volatile unsigned int        INTCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned INT0EP:1;
    unsigned INT1EP:1;
    unsigned INT2EP:1;
    unsigned INT3EP:1;
    unsigned INT4EP:1;
    unsigned :3;
    unsigned TPC:3;
    unsigned :1;
    unsigned MVEC:1;
    unsigned :1;
    unsigned FRZ:1;
    unsigned :1;
    unsigned SS0:1;
  };
  struct {
    unsigned w:32;
  };
} __INTCONbits_t;
extern volatile __INTCONbits_t INTCONbits __asm__ ("INTCON") __attribute__((section("sfrs")));
extern volatile unsigned int        INTCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        INTCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        INTCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        INTSTAT __attribute__((section("sfrs")));
typedef struct {
  unsigned VEC:6;
  unsigned :2;
  unsigned SRIPL:3;
} __INTSTATbits_t;
extern volatile __INTSTATbits_t INTSTATbits __asm__ ("INTSTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        IPTMR __attribute__((section("sfrs")));
extern volatile unsigned int        IPTMRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPTMRSET __attribute__((section("sfrs")));
extern volatile unsigned int        IPTMRINV __attribute__((section("sfrs")));
extern volatile unsigned int        IFS0 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CTIF:1;
    unsigned CS0IF:1;
    unsigned CS1IF:1;
    unsigned INT0IF:1;
    unsigned T1IF:1;
    unsigned IC1IF:1;
    unsigned OC1IF:1;
    unsigned INT1IF:1;
    unsigned T2IF:1;
    unsigned IC2IF:1;
    unsigned OC2IF:1;
    unsigned INT2IF:1;
    unsigned T3IF:1;
    unsigned IC3IF:1;
    unsigned OC3IF:1;
    unsigned INT3IF:1;
    unsigned T4IF:1;
    unsigned IC4IF:1;
    unsigned OC4IF:1;
    unsigned INT4IF:1;
    unsigned T5IF:1;
    unsigned IC5IF:1;
    unsigned OC5IF:1;
    unsigned :3;
    unsigned U1EIF:1;
    unsigned U1RXIF:1;
    unsigned U1TXIF:1;
    unsigned I2C1BIF:1;
    unsigned I2C1SIF:1;
    unsigned I2C1MIF:1;
  };
  struct {
    unsigned :26;
    unsigned U1AEIF:1;
    unsigned U1ARXIF:1;
    unsigned U1ATXIF:1;
  };
  struct {
    unsigned :26;
    unsigned SPI3EIF:1;
    unsigned SPI3RXIF:1;
    unsigned SPI3TXIF:1;
  };
  struct {
    unsigned :26;
    unsigned SPI1AEIF:1;
    unsigned SPI1ARXIF:1;
    unsigned SPI1ATXIF:1;
  };
  struct {
    unsigned :26;
    unsigned I2C3BIF:1;
    unsigned I2C3SIF:1;
    unsigned I2C3MIF:1;
  };
  struct {
    unsigned :26;
    unsigned I2C1ABIF:1;
    unsigned I2C1ASIF:1;
    unsigned I2C1AMIF:1;
  };
  struct {
    unsigned w:32;
  };
} __IFS0bits_t;
extern volatile __IFS0bits_t IFS0bits __asm__ ("IFS0") __attribute__((section("sfrs")));
extern volatile unsigned int        IFS0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IFS0SET __attribute__((section("sfrs")));
extern volatile unsigned int        IFS0INV __attribute__((section("sfrs")));
extern volatile unsigned int        IFS1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CNIF:1;
    unsigned AD1IF:1;
    unsigned PMPIF:1;
    unsigned CMP1IF:1;
    unsigned CMP2IF:1;
    unsigned U3EIF:1;
    unsigned U3RXIF:1;
    unsigned U3TXIF:1;
    unsigned U2EIF:1;
    unsigned U2RXIF:1;
    unsigned U2TXIF:1;
    unsigned I2C2BIF:1;
    unsigned I2C2SIF:1;
    unsigned I2C2MIF:1;
    unsigned FSCMIF:1;
    unsigned RTCCIF:1;
    unsigned DMA0IF:1;
    unsigned DMA1IF:1;
    unsigned DMA2IF:1;
    unsigned DMA3IF:1;
    unsigned DMA4IF:1;
    unsigned DMA5IF:1;
    unsigned DMA6IF:1;
    unsigned DMA7IF:1;
    unsigned FCEIF:1;
    unsigned USBIF:1;
    unsigned CAN1IF:1;
    unsigned CAN2IF:1;
    unsigned ETHIF:1;
    unsigned IC1EIF:1;
    unsigned IC2EIF:1;
    unsigned IC3EIF:1;
  };
  struct {
    unsigned :5;
    unsigned U2AEIF:1;
    unsigned U2ARXIF:1;
    unsigned U2ATXIF:1;
    unsigned U3AEIF:1;
    unsigned U3ARXIF:1;
    unsigned U3ATXIF:1;
  };
  struct {
    unsigned :5;
    unsigned SPI2EIF:1;
    unsigned SPI2RXIF:1;
    unsigned SPI2TXIF:1;
    unsigned SPI4EIF:1;
    unsigned SPI4RXIF:1;
    unsigned SPI4TXIF:1;
  };
  struct {
    unsigned :5;
    unsigned SPI2AEIF:1;
    unsigned SPI2ARXIF:1;
    unsigned SPI2ATXIF:1;
    unsigned SPI3AEIF:1;
    unsigned SPI3ARXIF:1;
    unsigned SPI3ATXIF:1;
  };
  struct {
    unsigned :5;
    unsigned I2C4BIF:1;
    unsigned I2C4SIF:1;
    unsigned I2C4MIF:1;
    unsigned I2C5BIF:1;
    unsigned I2C5SIF:1;
    unsigned I2C5MIF:1;
  };
  struct {
    unsigned :5;
    unsigned I2C2ABIF:1;
    unsigned I2C2ASIF:1;
    unsigned I2C2AMIF:1;
    unsigned I2C3ABIF:1;
    unsigned I2C3ASIF:1;
    unsigned I2C3AMIF:1;
  };
  struct {
    unsigned w:32;
  };
} __IFS1bits_t;
extern volatile __IFS1bits_t IFS1bits __asm__ ("IFS1") __attribute__((section("sfrs")));
extern volatile unsigned int        IFS1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IFS1SET __attribute__((section("sfrs")));
extern volatile unsigned int        IFS1INV __attribute__((section("sfrs")));
extern volatile unsigned int        IFS2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned IC4EIF:1;
    unsigned IC5EIF:1;
    unsigned PMPEIF:1;
    unsigned U1BEIF:1;
    unsigned U1BRXIF:1;
    unsigned U1BTXIF:1;
    unsigned U2BEIF:1;
    unsigned U2BRXIF:1;
    unsigned U2BTXIF:1;
    unsigned U3BEIF:1;
    unsigned U3BRXIF:1;
    unsigned U3BTXIF:1;
  };
  struct {
    unsigned :3;
    unsigned U4EIF:1;
    unsigned U4RXIF:1;
    unsigned U4TXIF:1;
    unsigned U6EIF:1;
    unsigned U6RXIF:1;
    unsigned U6TXIF:1;
    unsigned U5EIF:1;
    unsigned U5RXIF:1;
    unsigned U5TXIF:1;
  };
  struct {
    unsigned w:32;
  };
} __IFS2bits_t;
extern volatile __IFS2bits_t IFS2bits __asm__ ("IFS2") __attribute__((section("sfrs")));
extern volatile unsigned int        IFS2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IFS2SET __attribute__((section("sfrs")));
extern volatile unsigned int        IFS2INV __attribute__((section("sfrs")));
extern volatile unsigned int        IEC0 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CTIE:1;
    unsigned CS0IE:1;
    unsigned CS1IE:1;
    unsigned INT0IE:1;
    unsigned T1IE:1;
    unsigned IC1IE:1;
    unsigned OC1IE:1;
    unsigned INT1IE:1;
    unsigned T2IE:1;
    unsigned IC2IE:1;
    unsigned OC2IE:1;
    unsigned INT2IE:1;
    unsigned T3IE:1;
    unsigned IC3IE:1;
    unsigned OC3IE:1;
    unsigned INT3IE:1;
    unsigned T4IE:1;
    unsigned IC4IE:1;
    unsigned OC4IE:1;
    unsigned INT4IE:1;
    unsigned T5IE:1;
    unsigned IC5IE:1;
    unsigned OC5IE:1;
    unsigned :3;
    unsigned U1EIE:1;
    unsigned U1RXIE:1;
    unsigned U1TXIE:1;
    unsigned I2C1BIE:1;
    unsigned I2C1SIE:1;
    unsigned I2C1MIE:1;
  };
  struct {
    unsigned :26;
    unsigned U1AEIE:1;
    unsigned U1ARXIE:1;
    unsigned U1ATXIE:1;
  };
  struct {
    unsigned :26;
    unsigned SPI3EIE:1;
    unsigned SPI3RXIE:1;
    unsigned SPI3TXIE:1;
  };
  struct {
    unsigned :26;
    unsigned SPI1AEIE:1;
    unsigned SPI1ARXIE:1;
    unsigned SPI1ATXIE:1;
  };
  struct {
    unsigned :26;
    unsigned I2C3BIE:1;
    unsigned I2C3SIE:1;
    unsigned I2C3MIE:1;
  };
  struct {
    unsigned :26;
    unsigned I2C1ABIE:1;
    unsigned I2C1ASIE:1;
    unsigned I2C1AMIE:1;
  };
  struct {
    unsigned w:32;
  };
} __IEC0bits_t;
extern volatile __IEC0bits_t IEC0bits __asm__ ("IEC0") __attribute__((section("sfrs")));
extern volatile unsigned int        IEC0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IEC0SET __attribute__((section("sfrs")));
extern volatile unsigned int        IEC0INV __attribute__((section("sfrs")));
extern volatile unsigned int        IEC1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CNIE:1;
    unsigned AD1IE:1;
    unsigned PMPIE:1;
    unsigned CMP1IE:1;
    unsigned CMP2IE:1;
    unsigned U3EIE:1;
    unsigned U3RXIE:1;
    unsigned U3TXIE:1;
    unsigned U2EIE:1;
    unsigned U2RXIE:1;
    unsigned U2TXIE:1;
    unsigned I2C2BIE:1;
    unsigned I2C2SIE:1;
    unsigned I2C2MIE:1;
    unsigned FSCMIE:1;
    unsigned RTCCIE:1;
    unsigned DMA0IE:1;
    unsigned DMA1IE:1;
    unsigned DMA2IE:1;
    unsigned DMA3IE:1;
    unsigned DMA4IE:1;
    unsigned DMA5IE:1;
    unsigned DMA6IE:1;
    unsigned DMA7IE:1;
    unsigned FCEIE:1;
    unsigned USBIE:1;
    unsigned CAN1IE:1;
    unsigned CAN2IE:1;
    unsigned ETHIE:1;
    unsigned IC1EIE:1;
    unsigned IC2EIE:1;
    unsigned IC3EIE:1;
  };
  struct {
    unsigned :5;
    unsigned U2AEIE:1;
    unsigned U2ARXIE:1;
    unsigned U2ATXIE:1;
    unsigned U3AEIE:1;
    unsigned U3ARXIE:1;
    unsigned U3ATXIE:1;
  };
  struct {
    unsigned :5;
    unsigned SPI2EIE:1;
    unsigned SPI2RXIE:1;
    unsigned SPI2TXIE:1;
    unsigned SPI4EIE:1;
    unsigned SPI4RXIE:1;
    unsigned SPI4TXIE:1;
  };
  struct {
    unsigned :5;
    unsigned SPI2AEIE:1;
    unsigned SPI2ARXIE:1;
    unsigned SPI2ATXIE:1;
    unsigned SPI3AEIE:1;
    unsigned SPI3ARXIE:1;
    unsigned SPI3ATXIE:1;
  };
  struct {
    unsigned :5;
    unsigned I2C4BIE:1;
    unsigned I2C4SIE:1;
    unsigned I2C4MIE:1;
    unsigned I2C5BIE:1;
    unsigned I2C5SIE:1;
    unsigned I2C5MIE:1;
  };
  struct {
    unsigned :5;
    unsigned I2C2ABIE:1;
    unsigned I2C2ASIE:1;
    unsigned I2C2AMIE:1;
    unsigned I2C3ABIE:1;
    unsigned I2C3ASIE:1;
    unsigned I2C3AMIE:1;
  };
  struct {
    unsigned w:32;
  };
} __IEC1bits_t;
extern volatile __IEC1bits_t IEC1bits __asm__ ("IEC1") __attribute__((section("sfrs")));
extern volatile unsigned int        IEC1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IEC1SET __attribute__((section("sfrs")));
extern volatile unsigned int        IEC1INV __attribute__((section("sfrs")));
extern volatile unsigned int        IEC2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned IC4EIE:1;
    unsigned IC5EIE:1;
    unsigned PMPEIE:1;
    unsigned U1BEIE:1;
    unsigned U1BRXIE:1;
    unsigned U1BTXIE:1;
    unsigned U2BEIE:1;
    unsigned U2BRXIE:1;
    unsigned U2BTXIE:1;
    unsigned U3BEIE:1;
    unsigned U3BRXIE:1;
    unsigned U3BTXIE:1;
  };
  struct {
    unsigned :3;
    unsigned U4EIE:1;
    unsigned U4RXIE:1;
    unsigned U4TXIE:1;
    unsigned U6EIE:1;
    unsigned U6RXIE:1;
    unsigned U6TXIE:1;
    unsigned U5EIE:1;
    unsigned U5RXIE:1;
    unsigned U5TXIE:1;
  };
  struct {
    unsigned w:32;
  };
} __IEC2bits_t;
extern volatile __IEC2bits_t IEC2bits __asm__ ("IEC2") __attribute__((section("sfrs")));
extern volatile unsigned int        IEC2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IEC2SET __attribute__((section("sfrs")));
extern volatile unsigned int        IEC2INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC0 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CTIS:2;
    unsigned CTIP:3;
    unsigned :3;
    unsigned CS0IS:2;
    unsigned CS0IP:3;
    unsigned :3;
    unsigned CS1IS:2;
    unsigned CS1IP:3;
    unsigned :3;
    unsigned INT0IS:2;
    unsigned INT0IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC0bits_t;
extern volatile __IPC0bits_t IPC0bits __asm__ ("IPC0") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC0SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC0INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned T1IS:2;
    unsigned T1IP:3;
    unsigned :3;
    unsigned IC1IS:2;
    unsigned IC1IP:3;
    unsigned :3;
    unsigned OC1IS:2;
    unsigned OC1IP:3;
    unsigned :3;
    unsigned INT1IS:2;
    unsigned INT1IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC1bits_t;
extern volatile __IPC1bits_t IPC1bits __asm__ ("IPC1") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC1SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC1INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned T2IS:2;
    unsigned T2IP:3;
    unsigned :3;
    unsigned IC2IS:2;
    unsigned IC2IP:3;
    unsigned :3;
    unsigned OC2IS:2;
    unsigned OC2IP:3;
    unsigned :3;
    unsigned INT2IS:2;
    unsigned INT2IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC2bits_t;
extern volatile __IPC2bits_t IPC2bits __asm__ ("IPC2") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC2SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC2INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC3 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned T3IS:2;
    unsigned T3IP:3;
    unsigned :3;
    unsigned IC3IS:2;
    unsigned IC3IP:3;
    unsigned :3;
    unsigned OC3IS:2;
    unsigned OC3IP:3;
    unsigned :3;
    unsigned INT3IS:2;
    unsigned INT3IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC3bits_t;
extern volatile __IPC3bits_t IPC3bits __asm__ ("IPC3") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC3CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC3SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC3INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC4 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned T4IS:2;
    unsigned T4IP:3;
    unsigned :3;
    unsigned IC4IS:2;
    unsigned IC4IP:3;
    unsigned :3;
    unsigned OC4IS:2;
    unsigned OC4IP:3;
    unsigned :3;
    unsigned INT4IS:2;
    unsigned INT4IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC4bits_t;
extern volatile __IPC4bits_t IPC4bits __asm__ ("IPC4") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC4CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC4SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC4INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC5 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned T5IS:2;
    unsigned T5IP:3;
    unsigned :3;
    unsigned IC5IS:2;
    unsigned IC5IP:3;
    unsigned :3;
    unsigned OC5IS:2;
    unsigned OC5IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC5bits_t;
extern volatile __IPC5bits_t IPC5bits __asm__ ("IPC5") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC5CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC5SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC5INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC6 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned U1IS:2;
    unsigned U1IP:3;
    unsigned :3;
    unsigned I2C1IS:2;
    unsigned I2C1IP:3;
    unsigned :3;
    unsigned CNIS:2;
    unsigned CNIP:3;
    unsigned :3;
    unsigned AD1IS:2;
    unsigned AD1IP:3;
  };
  struct {
    unsigned U1AIS:2;
    unsigned U1AIP:3;
  };
  struct {
    unsigned SPI3IS:2;
    unsigned SPI3IP:3;
  };
  struct {
    unsigned SPI1AIS:2;
    unsigned SPI1AIP:3;
  };
  struct {
    unsigned I2C3IS:2;
    unsigned I2C3IP:3;
  };
  struct {
    unsigned I2C1AIS:2;
    unsigned I2C1AIP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC6bits_t;
extern volatile __IPC6bits_t IPC6bits __asm__ ("IPC6") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC6CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC6SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC6INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC7 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PMPIS:2;
    unsigned PMPIP:3;
    unsigned :3;
    unsigned CMP1IS:2;
    unsigned CMP1IP:3;
    unsigned :3;
    unsigned CMP2IS:2;
    unsigned CMP2IP:3;
    unsigned :3;
    unsigned U3IS:2;
    unsigned U3IP:3;
  };
  struct {
    unsigned :24;
    unsigned U2AIS:2;
    unsigned U2AIP:3;
  };
  struct {
    unsigned :24;
    unsigned SPI2IS:2;
    unsigned SPI2IP:3;
  };
  struct {
    unsigned :24;
    unsigned SPI2AIS:2;
    unsigned SPI2AIP:3;
  };
  struct {
    unsigned :24;
    unsigned I2C4IS:2;
    unsigned I2C4IP:3;
  };
  struct {
    unsigned :24;
    unsigned I2C2AIS:2;
    unsigned I2C2AIP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC7bits_t;
extern volatile __IPC7bits_t IPC7bits __asm__ ("IPC7") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC7CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC7SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC7INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC8 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned U2IS:2;
    unsigned U2IP:3;
    unsigned :3;
    unsigned I2C2IS:2;
    unsigned I2C2IP:3;
    unsigned :3;
    unsigned FSCMIS:2;
    unsigned FSCMIP:3;
    unsigned :3;
    unsigned RTCCIS:2;
    unsigned RTCCIP:3;
  };
  struct {
    unsigned U3AIS:2;
    unsigned U3AIP:3;
  };
  struct {
    unsigned SPI4IS:2;
    unsigned SPI4IP:3;
  };
  struct {
    unsigned SPI3AIS:2;
    unsigned SPI3AIP:3;
  };
  struct {
    unsigned I2C5IS:2;
    unsigned I2C5IP:3;
  };
  struct {
    unsigned I2C3AIS:2;
    unsigned I2C3AIP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC8bits_t;
extern volatile __IPC8bits_t IPC8bits __asm__ ("IPC8") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC8CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC8SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC8INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC9 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned DMA0IS:2;
    unsigned DMA0IP:3;
    unsigned :3;
    unsigned DMA1IS:2;
    unsigned DMA1IP:3;
    unsigned :3;
    unsigned DMA2IS:2;
    unsigned DMA2IP:3;
    unsigned :3;
    unsigned DMA3IS:2;
    unsigned DMA3IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC9bits_t;
extern volatile __IPC9bits_t IPC9bits __asm__ ("IPC9") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC9CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC9SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC9INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC10 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned DMA4IS:2;
    unsigned DMA4IP:3;
    unsigned :3;
    unsigned DMA5IS:2;
    unsigned DMA5IP:3;
    unsigned :3;
    unsigned DMA6IS:2;
    unsigned DMA6IP:3;
    unsigned :3;
    unsigned DMA7IS:2;
    unsigned DMA7IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC10bits_t;
extern volatile __IPC10bits_t IPC10bits __asm__ ("IPC10") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC10CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC10SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC10INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC11 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FCEIS:2;
    unsigned FCEIP:3;
    unsigned :3;
    unsigned USBIS:2;
    unsigned USBIP:3;
    unsigned :3;
    unsigned CAN1IS:2;
    unsigned CAN1IP:3;
    unsigned :3;
    unsigned CAN2IS:2;
    unsigned CAN2IP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC11bits_t;
extern volatile __IPC11bits_t IPC11bits __asm__ ("IPC11") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC11CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC11SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC11INV __attribute__((section("sfrs")));
extern volatile unsigned int        IPC12 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ETHIS:2;
    unsigned ETHIP:3;
    unsigned :3;
    unsigned U4IS:2;
    unsigned U4IP:3;
    unsigned :3;
    unsigned U6IS:2;
    unsigned U6IP:3;
    unsigned :3;
    unsigned U5IS:2;
    unsigned U5IP:3;
  };
  struct {
    unsigned :8;
    unsigned U1BIS:2;
    unsigned U1BIP:3;
    unsigned :3;
    unsigned U2BIS:2;
    unsigned U2BIP:3;
    unsigned :3;
    unsigned U3BIS:2;
    unsigned U3BIP:3;
  };
  struct {
    unsigned w:32;
  };
} __IPC12bits_t;
extern volatile __IPC12bits_t IPC12bits __asm__ ("IPC12") __attribute__((section("sfrs")));
extern volatile unsigned int        IPC12CLR __attribute__((section("sfrs")));
extern volatile unsigned int        IPC12SET __attribute__((section("sfrs")));
extern volatile unsigned int        IPC12INV __attribute__((section("sfrs")));
extern volatile unsigned int        BMXCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned BMXARB:3;
    unsigned :3;
    unsigned BMXWSDRM:1;
    unsigned :9;
    unsigned BMXERRIS:1;
    unsigned BMXERRDS:1;
    unsigned BMXERRDMA:1;
    unsigned BMXERRICD:1;
    unsigned BMXERRIXI:1;
    unsigned :5;
    unsigned BMXCHEDMA:1;
  };
  struct {
    unsigned w:32;
  };
} __BMXCONbits_t;
extern volatile __BMXCONbits_t BMXCONbits __asm__ ("BMXCON") __attribute__((section("sfrs")));
extern volatile unsigned int        BMXCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        BMXCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        BMXCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDKPBA __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDKPBACLR __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDKPBASET __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDKPBAINV __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUDBA __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUDBACLR __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUDBASET __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUDBAINV __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUPBA __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUPBACLR __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUPBASET __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDUPBAINV __attribute__((section("sfrs")));
extern volatile unsigned int        BMXDRMSZ __attribute__((section("sfrs")));
extern volatile unsigned int        BMXPUPBA __attribute__((section("sfrs")));
extern volatile unsigned int        BMXPUPBACLR __attribute__((section("sfrs")));
extern volatile unsigned int        BMXPUPBASET __attribute__((section("sfrs")));
extern volatile unsigned int        BMXPUPBAINV __attribute__((section("sfrs")));
extern volatile unsigned int        BMXPFMSZ __attribute__((section("sfrs")));
extern volatile unsigned int        BMXBOOTSZ __attribute__((section("sfrs")));
extern volatile unsigned int        DMACON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :11;
    unsigned DMABUSY:1;
    unsigned SUSPEND:1;
    unsigned :2;
    unsigned ON:1;
  };
  struct {
    unsigned w:32;
  };
} __DMACONbits_t;
extern volatile __DMACONbits_t DMACONbits __asm__ ("DMACON") __attribute__((section("sfrs")));
extern volatile unsigned int        DMACONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DMACONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DMACONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DMASTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned DMACH:3;
    unsigned RDWR:1;
  };
  struct {
    unsigned w:32;
  };
} __DMASTATbits_t;
extern volatile __DMASTATbits_t DMASTATbits __asm__ ("DMASTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        DMASTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DMASTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DMASTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DMAADDR __attribute__((section("sfrs")));
extern volatile unsigned int        DMAADDRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DMAADDRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DMAADDRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CRCCH:3;
    unsigned :2;
    unsigned CRCTYP:1;
    unsigned CRCAPP:1;
    unsigned CRCEN:1;
    unsigned PLEN:5;
    unsigned :11;
    unsigned BITO:1;
    unsigned :2;
    unsigned WBO:1;
    unsigned BYTO:2;
  };
  struct {
    unsigned w:32;
  };
} __DCRCCONbits_t;
extern volatile __DCRCCONbits_t DCRCCONbits __asm__ ("DCRCCON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCDATA __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCDATACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCDATASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCDATAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCXOR __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCXORCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCXORSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCRCXORINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH0CONbits_t;
extern volatile __DCH0CONbits_t DCH0CONbits __asm__ ("DCH0CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH0ECONbits_t;
extern volatile __DCH0ECONbits_t DCH0ECONbits __asm__ ("DCH0ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH0INTbits_t;
extern volatile __DCH0INTbits_t DCH0INTbits __asm__ ("DCH0INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH0DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH1CONbits_t;
extern volatile __DCH1CONbits_t DCH1CONbits __asm__ ("DCH1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH1ECONbits_t;
extern volatile __DCH1ECONbits_t DCH1ECONbits __asm__ ("DCH1ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH1INTbits_t;
extern volatile __DCH1INTbits_t DCH1INTbits __asm__ ("DCH1INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH1DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH2CONbits_t;
extern volatile __DCH2CONbits_t DCH2CONbits __asm__ ("DCH2CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH2ECONbits_t;
extern volatile __DCH2ECONbits_t DCH2ECONbits __asm__ ("DCH2ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH2INTbits_t;
extern volatile __DCH2INTbits_t DCH2INTbits __asm__ ("DCH2INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH2DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH3CONbits_t;
extern volatile __DCH3CONbits_t DCH3CONbits __asm__ ("DCH3CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH3ECONbits_t;
extern volatile __DCH3ECONbits_t DCH3ECONbits __asm__ ("DCH3ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH3INTbits_t;
extern volatile __DCH3INTbits_t DCH3INTbits __asm__ ("DCH3INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH3DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH4CONbits_t;
extern volatile __DCH4CONbits_t DCH4CONbits __asm__ ("DCH4CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH4ECONbits_t;
extern volatile __DCH4ECONbits_t DCH4ECONbits __asm__ ("DCH4ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH4INTbits_t;
extern volatile __DCH4INTbits_t DCH4INTbits __asm__ ("DCH4INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH4DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH5CONbits_t;
extern volatile __DCH5CONbits_t DCH5CONbits __asm__ ("DCH5CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH5ECONbits_t;
extern volatile __DCH5ECONbits_t DCH5ECONbits __asm__ ("DCH5ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH5INTbits_t;
extern volatile __DCH5INTbits_t DCH5INTbits __asm__ ("DCH5INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH5DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH6CONbits_t;
extern volatile __DCH6CONbits_t DCH6CONbits __asm__ ("DCH6CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH6ECONbits_t;
extern volatile __DCH6ECONbits_t DCH6ECONbits __asm__ ("DCH6ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH6INTbits_t;
extern volatile __DCH6INTbits_t DCH6INTbits __asm__ ("DCH6INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH6DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHPRI:2;
    unsigned CHEDET:1;
    unsigned :1;
    unsigned CHAEN:1;
    unsigned CHCHN:1;
    unsigned CHAED:1;
    unsigned CHEN:1;
    unsigned CHCHNS:1;
    unsigned :6;
    unsigned CHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH7CONbits_t;
extern volatile __DCH7CONbits_t DCH7CONbits __asm__ ("DCH7CON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7ECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :3;
    unsigned AIRQEN:1;
    unsigned SIRQEN:1;
    unsigned PATEN:1;
    unsigned CABORT:1;
    unsigned CFORCE:1;
    unsigned CHSIRQ:8;
    unsigned CHAIRQ:8;
  };
  struct {
    unsigned w:32;
  };
} __DCH7ECONbits_t;
extern volatile __DCH7ECONbits_t DCH7ECONbits __asm__ ("DCH7ECON") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7ECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7ECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7ECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7INT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CHERIF:1;
    unsigned CHTAIF:1;
    unsigned CHCCIF:1;
    unsigned CHBCIF:1;
    unsigned CHDHIF:1;
    unsigned CHDDIF:1;
    unsigned CHSHIF:1;
    unsigned CHSDIF:1;
    unsigned :8;
    unsigned CHERIE:1;
    unsigned CHTAIE:1;
    unsigned CHCCIE:1;
    unsigned CHBCIE:1;
    unsigned CHDHIE:1;
    unsigned CHDDIE:1;
    unsigned CHSHIE:1;
    unsigned CHSDIE:1;
  };
  struct {
    unsigned w:32;
  };
} __DCH7INTbits_t;
extern volatile __DCH7INTbits_t DCH7INTbits __asm__ ("DCH7INT") __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7INTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7INTSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7INTINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSA __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSACLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSASET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSAINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7SPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CSIZ __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CSIZCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CSIZSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CSIZINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CPTR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CPTRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CPTRSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7CPTRINV __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DAT __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DATSET __attribute__((section("sfrs")));
extern volatile unsigned int        DCH7DATINV __attribute__((section("sfrs")));
extern volatile unsigned int        CHECON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PFMWS:3;
    unsigned :1;
    unsigned PREFEN:2;
    unsigned :2;
    unsigned DCSZ:2;
    unsigned :6;
    unsigned CHECOH:1;
  };
  struct {
    unsigned w:32;
  };
} __CHECONbits_t;
extern volatile __CHECONbits_t CHECONbits __asm__ ("CHECON") __attribute__((section("sfrs")));
extern volatile unsigned int        CHECONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CHECONSET __attribute__((section("sfrs")));
extern volatile unsigned int        CHECONINV __attribute__((section("sfrs")));
extern volatile unsigned int        CHEACC __attribute__((section("sfrs")));
extern volatile unsigned int        CHEACCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CHEACCSET __attribute__((section("sfrs")));
extern volatile unsigned int        CHEACCINV __attribute__((section("sfrs")));
extern volatile unsigned int        CHETAG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :1;
    unsigned LTYPE:1;
    unsigned LLOCK:1;
    unsigned LVALID:1;
    unsigned LTAG:20;
    unsigned :7;
    unsigned LTAGBOOT:1;
  };
  struct {
    unsigned w:32;
  };
} __CHETAGbits_t;
extern volatile __CHETAGbits_t CHETAGbits __asm__ ("CHETAG") __attribute__((section("sfrs")));
extern volatile unsigned int        CHETAGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CHETAGSET __attribute__((section("sfrs")));
extern volatile unsigned int        CHETAGINV __attribute__((section("sfrs")));
extern volatile unsigned int        CHEMSK __attribute__((section("sfrs")));
extern volatile unsigned int        CHEMSKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CHEMSKSET __attribute__((section("sfrs")));
extern volatile unsigned int        CHEMSKINV __attribute__((section("sfrs")));
extern volatile unsigned int        CHEW0 __attribute__((section("sfrs")));
extern volatile unsigned int        CHEW1 __attribute__((section("sfrs")));
extern volatile unsigned int        CHEW2 __attribute__((section("sfrs")));
extern volatile unsigned int        CHEW3 __attribute__((section("sfrs")));
extern volatile unsigned int        CHELRU __attribute__((section("sfrs")));
extern volatile unsigned int        CHEHIT __attribute__((section("sfrs")));
extern volatile unsigned int        CHEMIS __attribute__((section("sfrs")));
extern volatile unsigned int        CHEPFABT __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGIR __attribute__((section("sfrs")));
typedef struct {
  unsigned VBUSVDIF:1;
  unsigned :1;
  unsigned SESENDIF:1;
  unsigned SESVDIF:1;
  unsigned ACTVIF:1;
  unsigned LSTATEIF:1;
  unsigned T1MSECIF:1;
  unsigned IDIF:1;
} __U1OTGIRbits_t;
extern volatile __U1OTGIRbits_t U1OTGIRbits __asm__ ("U1OTGIR") __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGIRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGIE __attribute__((section("sfrs")));
typedef struct {
  unsigned VBUSVDIE:1;
  unsigned :1;
  unsigned SESENDIE:1;
  unsigned SESVDIE:1;
  unsigned ACTVIE:1;
  unsigned LSTATEIE:1;
  unsigned T1MSECIE:1;
  unsigned IDIE:1;
} __U1OTGIEbits_t;
extern volatile __U1OTGIEbits_t U1OTGIEbits __asm__ ("U1OTGIE") __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGIECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGIESET __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGIEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGSTAT __attribute__((section("sfrs")));
typedef struct {
  unsigned VBUSVD:1;
  unsigned :1;
  unsigned SESEND:1;
  unsigned SESVD:1;
  unsigned :1;
  unsigned LSTATE:1;
  unsigned :1;
  unsigned ID:1;
} __U1OTGSTATbits_t;
extern volatile __U1OTGSTATbits_t U1OTGSTATbits __asm__ ("U1OTGSTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGCON __attribute__((section("sfrs")));
typedef struct {
  unsigned VBUSDIS:1;
  unsigned VBUSCHG:1;
  unsigned OTGEN:1;
  unsigned VBUSON:1;
  unsigned DMPULDWN:1;
  unsigned DPPULDWN:1;
  unsigned DMPULUP:1;
  unsigned DPPULUP:1;
} __U1OTGCONbits_t;
extern volatile __U1OTGCONbits_t U1OTGCONbits __asm__ ("U1OTGCON") __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1OTGCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1PWRC __attribute__((section("sfrs")));
typedef struct {
  unsigned USBPWR:1;
  unsigned USUSPEND:1;
  unsigned :1;
  unsigned USBBUSY:1;
  unsigned USLPGRD:1;
  unsigned :2;
  unsigned UACTPND:1;
} __U1PWRCbits_t;
extern volatile __U1PWRCbits_t U1PWRCbits __asm__ ("U1PWRC") __attribute__((section("sfrs")));
extern volatile unsigned int        U1PWRCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1PWRCSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1PWRCINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1IR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URSTIF_DETACHIF:1;
    unsigned UERRIF:1;
    unsigned SOFIF:1;
    unsigned TRNIF:1;
    unsigned IDLEIF:1;
    unsigned RESUMEIF:1;
    unsigned ATTACHIF:1;
    unsigned STALLIF:1;
  };
  struct {
    unsigned DETACHIF:1;
  };
  struct {
    unsigned URSTIF:1;
  };
} __U1IRbits_t;
extern volatile __U1IRbits_t U1IRbits __asm__ ("U1IR") __attribute__((section("sfrs")));
extern volatile unsigned int        U1IRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1IE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned URSTIE_DETACHIE:1;
    unsigned UERRIE:1;
    unsigned SOFIE:1;
    unsigned TRNIE:1;
    unsigned IDLEIE:1;
    unsigned RESUMEIE:1;
    unsigned ATTACHIE:1;
    unsigned STALLIE:1;
  };
  struct {
    unsigned DETACHIE:1;
  };
  struct {
    unsigned URSTIE:1;
  };
} __U1IEbits_t;
extern volatile __U1IEbits_t U1IEbits __asm__ ("U1IE") __attribute__((section("sfrs")));
extern volatile unsigned int        U1IECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1IESET __attribute__((section("sfrs")));
extern volatile unsigned int        U1IEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EIR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PIDEF:1;
    unsigned CRC5EF_EOFEF:1;
    unsigned CRC16EF:1;
    unsigned DFN8EF:1;
    unsigned BTOEF:1;
    unsigned DMAEF:1;
    unsigned BMXEF:1;
    unsigned BTSEF:1;
  };
  struct {
    unsigned :1;
    unsigned CRC5EF:1;
  };
  struct {
    unsigned :1;
    unsigned EOFEF:1;
  };
} __U1EIRbits_t;
extern volatile __U1EIRbits_t U1EIRbits __asm__ ("U1EIR") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EIRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EIE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PIDEE:1;
    unsigned CRC5EE_EOFEE:1;
    unsigned CRC16EE:1;
    unsigned DFN8EE:1;
    unsigned BTOEE:1;
    unsigned DMAEE:1;
    unsigned BMXEE:1;
    unsigned BTSEE:1;
  };
  struct {
    unsigned :1;
    unsigned CRC5EE:1;
  };
  struct {
    unsigned :1;
    unsigned EOFEE:1;
  };
} __U1EIEbits_t;
extern volatile __U1EIEbits_t U1EIEbits __asm__ ("U1EIE") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EIECLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EIESET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EIEINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1STAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned PPBI:1;
    unsigned DIR:1;
    unsigned ENDPT0:1;
  };
  struct {
    unsigned :4;
    unsigned ENDPT:4;
  };
  struct {
    unsigned :5;
    unsigned ENDPT1:1;
    unsigned ENDPT2:1;
    unsigned ENDPT3:1;
  };
} __U1STATbits_t;
extern volatile __U1STATbits_t U1STATbits __asm__ ("U1STAT") __attribute__((section("sfrs")));
extern volatile unsigned int        U1STATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1STATSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1STATINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1CON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned USBEN_SOFEN:1;
    unsigned PPBRST:1;
    unsigned RESUME:1;
    unsigned HOSTEN:1;
    unsigned USBRST:1;
    unsigned PKTDIS_TOKBUSY:1;
    unsigned SE0:1;
    unsigned JSTATE:1;
  };
  struct {
    unsigned USBEN:1;
  };
  struct {
    unsigned SOFEN:1;
    unsigned :4;
    unsigned PKTDIS:1;
  };
  struct {
    unsigned :5;
    unsigned TOKBUSY:1;
  };
} __U1CONbits_t;
extern volatile __U1CONbits_t U1CONbits __asm__ ("U1CON") __attribute__((section("sfrs")));
extern volatile unsigned int        U1CONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1CONSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1CONINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1ADDR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned DEVADDR:7;
    unsigned LSPDEN:1;
  };
  struct {
    unsigned DEVADDR0:1;
    unsigned DEVADDR1:1;
    unsigned DEVADDR2:1;
    unsigned DEVADDR3:1;
    unsigned DEVADDR4:1;
    unsigned DEVADDR5:1;
    unsigned DEVADDR6:1;
  };
} __U1ADDRbits_t;
extern volatile __U1ADDRbits_t U1ADDRbits __asm__ ("U1ADDR") __attribute__((section("sfrs")));
extern volatile unsigned int        U1ADDRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1ADDRSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1ADDRINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP1 __attribute__((section("sfrs")));
typedef struct {
  unsigned :1;
  unsigned BDTPTRL:7;
} __U1BDTP1bits_t;
extern volatile __U1BDTP1bits_t U1BDTP1bits __asm__ ("U1BDTP1") __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP1SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP1INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRML __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FRML:8;
  };
  struct {
    unsigned FRM0:1;
    unsigned FRM1:1;
    unsigned FRM2:1;
    unsigned FRM3:1;
    unsigned FRM4:1;
    unsigned FRM5:1;
    unsigned FRM6:1;
    unsigned FRM7:1;
  };
} __U1FRMLbits_t;
extern volatile __U1FRMLbits_t U1FRMLbits __asm__ ("U1FRML") __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMLCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMLSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMLINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMH __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FRMH:3;
  };
  struct {
    unsigned FRM8:1;
    unsigned FRM9:1;
    unsigned FRM10:1;
  };
} __U1FRMHbits_t;
extern volatile __U1FRMHbits_t U1FRMHbits __asm__ ("U1FRMH") __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMHCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMHSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1FRMHINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1TOK __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned EP:4;
    unsigned PID:4;
  };
  struct {
    unsigned EP0:1;
  };
  struct {
    unsigned :1;
    unsigned EP1:1;
    unsigned EP2:1;
    unsigned EP3:1;
    unsigned PID0:1;
  };
  struct {
    unsigned :5;
    unsigned PID1:1;
    unsigned PID2:1;
    unsigned PID3:1;
  };
} __U1TOKbits_t;
extern volatile __U1TOKbits_t U1TOKbits __asm__ ("U1TOK") __attribute__((section("sfrs")));
extern volatile unsigned int        U1TOKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1TOKSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1TOKINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1SOF __attribute__((section("sfrs")));
typedef struct {
  unsigned CNT:8;
} __U1SOFbits_t;
extern volatile __U1SOFbits_t U1SOFbits __asm__ ("U1SOF") __attribute__((section("sfrs")));
extern volatile unsigned int        U1SOFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1SOFSET __attribute__((section("sfrs")));
extern volatile unsigned int        U1SOFINV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP2 __attribute__((section("sfrs")));
typedef struct {
  unsigned BDTPTRH:8;
} __U1BDTP2bits_t;
extern volatile __U1BDTP2bits_t U1BDTP2bits __asm__ ("U1BDTP2") __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP2SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP2INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP3 __attribute__((section("sfrs")));
typedef struct {
  unsigned BDTPTRU:8;
} __U1BDTP3bits_t;
extern volatile __U1BDTP3bits_t U1BDTP3bits __asm__ ("U1BDTP3") __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP3CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP3SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1BDTP3INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1CNFG1 __attribute__((section("sfrs")));
typedef struct {
  unsigned UASUSPND:1;
  unsigned :3;
  unsigned USBSIDL:1;
  unsigned :1;
  unsigned UOEMON:1;
  unsigned UTEYE:1;
} __U1CNFG1bits_t;
extern volatile __U1CNFG1bits_t U1CNFG1bits __asm__ ("U1CNFG1") __attribute__((section("sfrs")));
extern volatile unsigned int        U1CNFG1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1CNFG1SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1CNFG1INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP0 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
  unsigned :1;
  unsigned RETRYDIS:1;
  unsigned LSPD:1;
} __U1EP0bits_t;
extern volatile __U1EP0bits_t U1EP0bits __asm__ ("U1EP0") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP0SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP0INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP1 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP1bits_t;
extern volatile __U1EP1bits_t U1EP1bits __asm__ ("U1EP1") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP1SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP1INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP2 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP2bits_t;
extern volatile __U1EP2bits_t U1EP2bits __asm__ ("U1EP2") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP2SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP2INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP3 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP3bits_t;
extern volatile __U1EP3bits_t U1EP3bits __asm__ ("U1EP3") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP3CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP3SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP3INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP4 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP4bits_t;
extern volatile __U1EP4bits_t U1EP4bits __asm__ ("U1EP4") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP4CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP4SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP4INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP5 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP5bits_t;
extern volatile __U1EP5bits_t U1EP5bits __asm__ ("U1EP5") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP5CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP5SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP5INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP6 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP6bits_t;
extern volatile __U1EP6bits_t U1EP6bits __asm__ ("U1EP6") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP6CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP6SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP6INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP7 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP7bits_t;
extern volatile __U1EP7bits_t U1EP7bits __asm__ ("U1EP7") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP7CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP7SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP7INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP8 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP8bits_t;
extern volatile __U1EP8bits_t U1EP8bits __asm__ ("U1EP8") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP8CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP8SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP8INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP9 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP9bits_t;
extern volatile __U1EP9bits_t U1EP9bits __asm__ ("U1EP9") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP9CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP9SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP9INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP10 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP10bits_t;
extern volatile __U1EP10bits_t U1EP10bits __asm__ ("U1EP10") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP10CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP10SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP10INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP11 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP11bits_t;
extern volatile __U1EP11bits_t U1EP11bits __asm__ ("U1EP11") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP11CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP11SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP11INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP12 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP12bits_t;
extern volatile __U1EP12bits_t U1EP12bits __asm__ ("U1EP12") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP12CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP12SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP12INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP13 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP13bits_t;
extern volatile __U1EP13bits_t U1EP13bits __asm__ ("U1EP13") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP13CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP13SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP13INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP14 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP14bits_t;
extern volatile __U1EP14bits_t U1EP14bits __asm__ ("U1EP14") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP14CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP14SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP14INV __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP15 __attribute__((section("sfrs")));
typedef struct {
  unsigned EPHSHK:1;
  unsigned EPSTALL:1;
  unsigned EPTXEN:1;
  unsigned EPRXEN:1;
  unsigned EPCONDIS:1;
} __U1EP15bits_t;
extern volatile __U1EP15bits_t U1EP15bits __asm__ ("U1EP15") __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP15CLR __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP15SET __attribute__((section("sfrs")));
extern volatile unsigned int        U1EP15INV __attribute__((section("sfrs")));
extern volatile unsigned int        TRISB __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TRISB0:1;
    unsigned TRISB1:1;
    unsigned TRISB2:1;
    unsigned TRISB3:1;
    unsigned TRISB4:1;
    unsigned TRISB5:1;
    unsigned TRISB6:1;
    unsigned TRISB7:1;
    unsigned TRISB8:1;
    unsigned TRISB9:1;
    unsigned TRISB10:1;
    unsigned TRISB11:1;
    unsigned TRISB12:1;
    unsigned TRISB13:1;
    unsigned TRISB14:1;
    unsigned TRISB15:1;
  };
  struct {
    unsigned w:32;
  };
} __TRISBbits_t;
extern volatile __TRISBbits_t TRISBbits __asm__ ("TRISB") __attribute__((section("sfrs")));
extern volatile unsigned int        TRISBCLR __attribute__((section("sfrs")));
extern volatile unsigned int        TRISBSET __attribute__((section("sfrs")));
extern volatile unsigned int        TRISBINV __attribute__((section("sfrs")));
extern volatile unsigned int        PORTB __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RB0:1;
    unsigned RB1:1;
    unsigned RB2:1;
    unsigned RB3:1;
    unsigned RB4:1;
    unsigned RB5:1;
    unsigned RB6:1;
    unsigned RB7:1;
    unsigned RB8:1;
    unsigned RB9:1;
    unsigned RB10:1;
    unsigned RB11:1;
    unsigned RB12:1;
    unsigned RB13:1;
    unsigned RB14:1;
    unsigned RB15:1;
  };
  struct {
    unsigned w:32;
  };
} __PORTBbits_t;
extern volatile __PORTBbits_t PORTBbits __asm__ ("PORTB") __attribute__((section("sfrs")));
extern volatile unsigned int        PORTBCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PORTBSET __attribute__((section("sfrs")));
extern volatile unsigned int        PORTBINV __attribute__((section("sfrs")));
extern volatile unsigned int        LATB __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned LATB0:1;
    unsigned LATB1:1;
    unsigned LATB2:1;
    unsigned LATB3:1;
    unsigned LATB4:1;
    unsigned LATB5:1;
    unsigned LATB6:1;
    unsigned LATB7:1;
    unsigned LATB8:1;
    unsigned LATB9:1;
    unsigned LATB10:1;
    unsigned LATB11:1;
    unsigned LATB12:1;
    unsigned LATB13:1;
    unsigned LATB14:1;
    unsigned LATB15:1;
  };
  struct {
    unsigned w:32;
  };
} __LATBbits_t;
extern volatile __LATBbits_t LATBbits __asm__ ("LATB") __attribute__((section("sfrs")));
extern volatile unsigned int        LATBCLR __attribute__((section("sfrs")));
extern volatile unsigned int        LATBSET __attribute__((section("sfrs")));
extern volatile unsigned int        LATBINV __attribute__((section("sfrs")));
extern volatile unsigned int        ODCB __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ODCB0:1;
    unsigned ODCB1:1;
    unsigned ODCB2:1;
    unsigned ODCB3:1;
    unsigned ODCB4:1;
    unsigned ODCB5:1;
    unsigned ODCB6:1;
    unsigned ODCB7:1;
    unsigned ODCB8:1;
    unsigned ODCB9:1;
    unsigned ODCB10:1;
    unsigned ODCB11:1;
    unsigned ODCB12:1;
    unsigned ODCB13:1;
    unsigned ODCB14:1;
    unsigned ODCB15:1;
  };
  struct {
    unsigned w:32;
  };
} __ODCBbits_t;
extern volatile __ODCBbits_t ODCBbits __asm__ ("ODCB") __attribute__((section("sfrs")));
extern volatile unsigned int        ODCBCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ODCBSET __attribute__((section("sfrs")));
extern volatile unsigned int        ODCBINV __attribute__((section("sfrs")));
extern volatile unsigned int        TRISC __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :12;
    unsigned TRISC12:1;
    unsigned TRISC13:1;
    unsigned TRISC14:1;
    unsigned TRISC15:1;
  };
  struct {
    unsigned w:32;
  };
} __TRISCbits_t;
extern volatile __TRISCbits_t TRISCbits __asm__ ("TRISC") __attribute__((section("sfrs")));
extern volatile unsigned int        TRISCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        TRISCSET __attribute__((section("sfrs")));
extern volatile unsigned int        TRISCINV __attribute__((section("sfrs")));
extern volatile unsigned int        PORTC __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :12;
    unsigned RC12:1;
    unsigned RC13:1;
    unsigned RC14:1;
    unsigned RC15:1;
  };
  struct {
    unsigned w:32;
  };
} __PORTCbits_t;
extern volatile __PORTCbits_t PORTCbits __asm__ ("PORTC") __attribute__((section("sfrs")));
extern volatile unsigned int        PORTCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PORTCSET __attribute__((section("sfrs")));
extern volatile unsigned int        PORTCINV __attribute__((section("sfrs")));
extern volatile unsigned int        LATC __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :12;
    unsigned LATC12:1;
    unsigned LATC13:1;
    unsigned LATC14:1;
    unsigned LATC15:1;
  };
  struct {
    unsigned w:32;
  };
} __LATCbits_t;
extern volatile __LATCbits_t LATCbits __asm__ ("LATC") __attribute__((section("sfrs")));
extern volatile unsigned int        LATCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        LATCSET __attribute__((section("sfrs")));
extern volatile unsigned int        LATCINV __attribute__((section("sfrs")));
extern volatile unsigned int        ODCC __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :12;
    unsigned ODCC12:1;
    unsigned ODCC13:1;
    unsigned ODCC14:1;
    unsigned ODCC15:1;
  };
  struct {
    unsigned w:32;
  };
} __ODCCbits_t;
extern volatile __ODCCbits_t ODCCbits __asm__ ("ODCC") __attribute__((section("sfrs")));
extern volatile unsigned int        ODCCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ODCCSET __attribute__((section("sfrs")));
extern volatile unsigned int        ODCCINV __attribute__((section("sfrs")));
extern volatile unsigned int        TRISD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TRISD0:1;
    unsigned TRISD1:1;
    unsigned TRISD2:1;
    unsigned TRISD3:1;
    unsigned TRISD4:1;
    unsigned TRISD5:1;
    unsigned TRISD6:1;
    unsigned TRISD7:1;
    unsigned TRISD8:1;
    unsigned TRISD9:1;
    unsigned TRISD10:1;
    unsigned TRISD11:1;
  };
  struct {
    unsigned w:32;
  };
} __TRISDbits_t;
extern volatile __TRISDbits_t TRISDbits __asm__ ("TRISD") __attribute__((section("sfrs")));
extern volatile unsigned int        TRISDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        TRISDSET __attribute__((section("sfrs")));
extern volatile unsigned int        TRISDINV __attribute__((section("sfrs")));
extern volatile unsigned int        PORTD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RD0:1;
    unsigned RD1:1;
    unsigned RD2:1;
    unsigned RD3:1;
    unsigned RD4:1;
    unsigned RD5:1;
    unsigned RD6:1;
    unsigned RD7:1;
    unsigned RD8:1;
    unsigned RD9:1;
    unsigned RD10:1;
    unsigned RD11:1;
  };
  struct {
    unsigned w:32;
  };
} __PORTDbits_t;
extern volatile __PORTDbits_t PORTDbits __asm__ ("PORTD") __attribute__((section("sfrs")));
extern volatile unsigned int        PORTDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PORTDSET __attribute__((section("sfrs")));
extern volatile unsigned int        PORTDINV __attribute__((section("sfrs")));
extern volatile unsigned int        LATD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned LATD0:1;
    unsigned LATD1:1;
    unsigned LATD2:1;
    unsigned LATD3:1;
    unsigned LATD4:1;
    unsigned LATD5:1;
    unsigned LATD6:1;
    unsigned LATD7:1;
    unsigned LATD8:1;
    unsigned LATD9:1;
    unsigned LATD10:1;
    unsigned LATD11:1;
  };
  struct {
    unsigned w:32;
  };
} __LATDbits_t;
extern volatile __LATDbits_t LATDbits __asm__ ("LATD") __attribute__((section("sfrs")));
extern volatile unsigned int        LATDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        LATDSET __attribute__((section("sfrs")));
extern volatile unsigned int        LATDINV __attribute__((section("sfrs")));
extern volatile unsigned int        ODCD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ODCD0:1;
    unsigned ODCD1:1;
    unsigned ODCD2:1;
    unsigned ODCD3:1;
    unsigned ODCD4:1;
    unsigned ODCD5:1;
    unsigned ODCD6:1;
    unsigned ODCD7:1;
    unsigned ODCD8:1;
    unsigned ODCD9:1;
    unsigned ODCD10:1;
    unsigned ODCD11:1;
  };
  struct {
    unsigned w:32;
  };
} __ODCDbits_t;
extern volatile __ODCDbits_t ODCDbits __asm__ ("ODCD") __attribute__((section("sfrs")));
extern volatile unsigned int        ODCDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ODCDSET __attribute__((section("sfrs")));
extern volatile unsigned int        ODCDINV __attribute__((section("sfrs")));
extern volatile unsigned int        TRISE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TRISE0:1;
    unsigned TRISE1:1;
    unsigned TRISE2:1;
    unsigned TRISE3:1;
    unsigned TRISE4:1;
    unsigned TRISE5:1;
    unsigned TRISE6:1;
    unsigned TRISE7:1;
  };
  struct {
    unsigned w:32;
  };
} __TRISEbits_t;
extern volatile __TRISEbits_t TRISEbits __asm__ ("TRISE") __attribute__((section("sfrs")));
extern volatile unsigned int        TRISECLR __attribute__((section("sfrs")));
extern volatile unsigned int        TRISESET __attribute__((section("sfrs")));
extern volatile unsigned int        TRISEINV __attribute__((section("sfrs")));
extern volatile unsigned int        PORTE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RE0:1;
    unsigned RE1:1;
    unsigned RE2:1;
    unsigned RE3:1;
    unsigned RE4:1;
    unsigned RE5:1;
    unsigned RE6:1;
    unsigned RE7:1;
  };
  struct {
    unsigned w:32;
  };
} __PORTEbits_t;
extern volatile __PORTEbits_t PORTEbits __asm__ ("PORTE") __attribute__((section("sfrs")));
extern volatile unsigned int        PORTECLR __attribute__((section("sfrs")));
extern volatile unsigned int        PORTESET __attribute__((section("sfrs")));
extern volatile unsigned int        PORTEINV __attribute__((section("sfrs")));
extern volatile unsigned int        LATE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned LATE0:1;
    unsigned LATE1:1;
    unsigned LATE2:1;
    unsigned LATE3:1;
    unsigned LATE4:1;
    unsigned LATE5:1;
    unsigned LATE6:1;
    unsigned LATE7:1;
  };
  struct {
    unsigned w:32;
  };
} __LATEbits_t;
extern volatile __LATEbits_t LATEbits __asm__ ("LATE") __attribute__((section("sfrs")));
extern volatile unsigned int        LATECLR __attribute__((section("sfrs")));
extern volatile unsigned int        LATESET __attribute__((section("sfrs")));
extern volatile unsigned int        LATEINV __attribute__((section("sfrs")));
extern volatile unsigned int        ODCE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ODCE0:1;
    unsigned ODCE1:1;
    unsigned ODCE2:1;
    unsigned ODCE3:1;
    unsigned ODCE4:1;
    unsigned ODCE5:1;
    unsigned ODCE6:1;
    unsigned ODCE7:1;
  };
  struct {
    unsigned w:32;
  };
} __ODCEbits_t;
extern volatile __ODCEbits_t ODCEbits __asm__ ("ODCE") __attribute__((section("sfrs")));
extern volatile unsigned int        ODCECLR __attribute__((section("sfrs")));
extern volatile unsigned int        ODCESET __attribute__((section("sfrs")));
extern volatile unsigned int        ODCEINV __attribute__((section("sfrs")));
extern volatile unsigned int        TRISF __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned TRISF0:1;
    unsigned TRISF1:1;
    unsigned TRISF2:1;
    unsigned TRISF3:1;
    unsigned TRISF4:1;
    unsigned TRISF5:1;
  };
  struct {
    unsigned w:32;
  };
} __TRISFbits_t;
extern volatile __TRISFbits_t TRISFbits __asm__ ("TRISF") __attribute__((section("sfrs")));
extern volatile unsigned int        TRISFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        TRISFSET __attribute__((section("sfrs")));
extern volatile unsigned int        TRISFINV __attribute__((section("sfrs")));
extern volatile unsigned int        PORTF __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RF0:1;
    unsigned RF1:1;
    unsigned RF2:1;
    unsigned RF3:1;
    unsigned RF4:1;
    unsigned RF5:1;
  };
  struct {
    unsigned w:32;
  };
} __PORTFbits_t;
extern volatile __PORTFbits_t PORTFbits __asm__ ("PORTF") __attribute__((section("sfrs")));
extern volatile unsigned int        PORTFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PORTFSET __attribute__((section("sfrs")));
extern volatile unsigned int        PORTFINV __attribute__((section("sfrs")));
extern volatile unsigned int        LATF __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned LATF0:1;
    unsigned LATF1:1;
    unsigned LATF2:1;
    unsigned LATF3:1;
    unsigned LATF4:1;
    unsigned LATF5:1;
  };
  struct {
    unsigned w:32;
  };
} __LATFbits_t;
extern volatile __LATFbits_t LATFbits __asm__ ("LATF") __attribute__((section("sfrs")));
extern volatile unsigned int        LATFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        LATFSET __attribute__((section("sfrs")));
extern volatile unsigned int        LATFINV __attribute__((section("sfrs")));
extern volatile unsigned int        ODCF __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ODCF0:1;
    unsigned ODCF1:1;
    unsigned ODCF2:1;
    unsigned ODCF3:1;
    unsigned ODCF4:1;
    unsigned ODCF5:1;
  };
  struct {
    unsigned w:32;
  };
} __ODCFbits_t;
extern volatile __ODCFbits_t ODCFbits __asm__ ("ODCF") __attribute__((section("sfrs")));
extern volatile unsigned int        ODCFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ODCFSET __attribute__((section("sfrs")));
extern volatile unsigned int        ODCFINV __attribute__((section("sfrs")));
extern volatile unsigned int        TRISG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned TRISG2:1;
    unsigned TRISG3:1;
    unsigned :2;
    unsigned TRISG6:1;
    unsigned TRISG7:1;
    unsigned TRISG8:1;
    unsigned TRISG9:1;
  };
  struct {
    unsigned w:32;
  };
} __TRISGbits_t;
extern volatile __TRISGbits_t TRISGbits __asm__ ("TRISG") __attribute__((section("sfrs")));
extern volatile unsigned int        TRISGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        TRISGSET __attribute__((section("sfrs")));
extern volatile unsigned int        TRISGINV __attribute__((section("sfrs")));
extern volatile unsigned int        PORTG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned RG2:1;
    unsigned RG3:1;
    unsigned :2;
    unsigned RG6:1;
    unsigned RG7:1;
    unsigned RG8:1;
    unsigned RG9:1;
  };
  struct {
    unsigned w:32;
  };
} __PORTGbits_t;
extern volatile __PORTGbits_t PORTGbits __asm__ ("PORTG") __attribute__((section("sfrs")));
extern volatile unsigned int        PORTGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        PORTGSET __attribute__((section("sfrs")));
extern volatile unsigned int        PORTGINV __attribute__((section("sfrs")));
extern volatile unsigned int        LATG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned LATG2:1;
    unsigned LATG3:1;
    unsigned :2;
    unsigned LATG6:1;
    unsigned LATG7:1;
    unsigned LATG8:1;
    unsigned LATG9:1;
  };
  struct {
    unsigned w:32;
  };
} __LATGbits_t;
extern volatile __LATGbits_t LATGbits __asm__ ("LATG") __attribute__((section("sfrs")));
extern volatile unsigned int        LATGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        LATGSET __attribute__((section("sfrs")));
extern volatile unsigned int        LATGINV __attribute__((section("sfrs")));
extern volatile unsigned int        ODCG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned ODCG2:1;
    unsigned ODCG3:1;
    unsigned :2;
    unsigned ODCG6:1;
    unsigned ODCG7:1;
    unsigned ODCG8:1;
    unsigned ODCG9:1;
  };
  struct {
    unsigned w:32;
  };
} __ODCGbits_t;
extern volatile __ODCGbits_t ODCGbits __asm__ ("ODCG") __attribute__((section("sfrs")));
extern volatile unsigned int        ODCGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ODCGSET __attribute__((section("sfrs")));
extern volatile unsigned int        ODCGINV __attribute__((section("sfrs")));
extern volatile unsigned int        CNCON __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :13;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
  };
  struct {
    unsigned w:32;
  };
} __CNCONbits_t;
extern volatile __CNCONbits_t CNCONbits __asm__ ("CNCON") __attribute__((section("sfrs")));
extern volatile unsigned int        CNCONCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CNCONSET __attribute__((section("sfrs")));
extern volatile unsigned int        CNCONINV __attribute__((section("sfrs")));
extern volatile unsigned int        CNEN __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CNEN0:1;
    unsigned CNEN1:1;
    unsigned CNEN2:1;
    unsigned CNEN3:1;
    unsigned CNEN4:1;
    unsigned CNEN5:1;
    unsigned CNEN6:1;
    unsigned CNEN7:1;
    unsigned CNEN8:1;
    unsigned CNEN9:1;
    unsigned CNEN10:1;
    unsigned CNEN11:1;
    unsigned CNEN12:1;
    unsigned CNEN13:1;
    unsigned CNEN14:1;
    unsigned CNEN15:1;
    unsigned CNEN16:1;
    unsigned CNEN17:1;
    unsigned CNEN18:1;
  };
  struct {
    unsigned w:32;
  };
} __CNENbits_t;
extern volatile __CNENbits_t CNENbits __asm__ ("CNEN") __attribute__((section("sfrs")));
extern volatile unsigned int        CNENCLR __attribute__((section("sfrs")));
extern volatile unsigned int        CNENSET __attribute__((section("sfrs")));
extern volatile unsigned int        CNENINV __attribute__((section("sfrs")));
extern volatile unsigned int        CNPUE __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned CNPUE0:1;
    unsigned CNPUE1:1;
    unsigned CNPUE2:1;
    unsigned CNPUE3:1;
    unsigned CNPUE4:1;
    unsigned CNPUE5:1;
    unsigned CNPUE6:1;
    unsigned CNPUE7:1;
    unsigned CNPUE8:1;
    unsigned CNPUE9:1;
    unsigned CNPUE10:1;
    unsigned CNPUE11:1;
    unsigned CNPUE12:1;
    unsigned CNPUE13:1;
    unsigned CNPUE14:1;
    unsigned CNPUE15:1;
    unsigned CNPUE16:1;
    unsigned CNPUE17:1;
    unsigned CNPUE18:1;
  };
  struct {
    unsigned w:32;
  };
} __CNPUEbits_t;
extern volatile __CNPUEbits_t CNPUEbits __asm__ ("CNPUE") __attribute__((section("sfrs")));
extern volatile unsigned int        CNPUECLR __attribute__((section("sfrs")));
extern volatile unsigned int        CNPUESET __attribute__((section("sfrs")));
extern volatile unsigned int        CNPUEINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned BUFCDEC:1;
    unsigned :3;
    unsigned MANFC:1;
    unsigned :2;
    unsigned AUTOFC:1;
    unsigned RXEN:1;
    unsigned TXRTS:1;
    unsigned :3;
    unsigned SIDL:1;
    unsigned :1;
    unsigned ON:1;
    unsigned PTV:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHCON1bits_t;
extern volatile __ETHCON1bits_t ETHCON1bits __asm__ ("ETHCON1") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON1SET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON1INV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :4;
    unsigned RXBUF_SZ:7;
  };
  struct {
    unsigned w:32;
  };
} __ETHCON2bits_t;
extern volatile __ETHCON2bits_t ETHCON2bits __asm__ ("ETHCON2") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON2SET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHCON2INV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHTXST __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned TXSTADDR:30;
  };
  struct {
    unsigned w:32;
  };
} __ETHTXSTbits_t;
extern volatile __ETHTXSTbits_t ETHTXSTbits __asm__ ("ETHTXST") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHTXSTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHTXSTSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHTXSTINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXST __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :2;
    unsigned RXSTADDR:30;
  };
  struct {
    unsigned w:32;
  };
} __ETHRXSTbits_t;
extern volatile __ETHRXSTbits_t ETHRXSTbits __asm__ ("ETHRXST") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXSTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXSTSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXSTINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT0 __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT0SET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT0INV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT1 __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT1SET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHHT1INV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM0 __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM0SET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM0INV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM1 __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM1SET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMM1INV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMCS __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PMCS:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHPMCSbits_t;
extern volatile __ETHPMCSbits_t ETHPMCSbits __asm__ ("ETHPMCS") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMCSCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMCSSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMCSINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMO __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned PMO:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHPMObits_t;
extern volatile __ETHPMObits_t ETHPMObits __asm__ ("ETHPMO") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMOCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMOSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHPMOINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXFC __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned BCEN:1;
    unsigned MCEN:1;
    unsigned NOTMEEN:1;
    unsigned UCEN:1;
    unsigned RUNTEN:1;
    unsigned RUNTERREN:1;
    unsigned CRCOKEN:1;
    unsigned CRCERREN:1;
    unsigned PMMODE:4;
    unsigned NOTPM:1;
    unsigned :1;
    unsigned MPEN:1;
    unsigned HTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __ETHRXFCbits_t;
extern volatile __ETHRXFCbits_t ETHRXFCbits __asm__ ("ETHRXFC") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXFCCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXFCSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXFCINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXWM __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RXEWM:8;
    unsigned :8;
    unsigned RXFWM:8;
  };
  struct {
    unsigned w:32;
  };
} __ETHRXWMbits_t;
extern volatile __ETHRXWMbits_t ETHRXWMbits __asm__ ("ETHRXWM") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXWMCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXWMSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXWMINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIEN __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RXOVFLWIE:1;
    unsigned RXBUFNAIE:1;
    unsigned TXABORTIE:1;
    unsigned TXDONEIE:1;
    unsigned :1;
    unsigned RXACTIE:1;
    unsigned PKTPENDIE:1;
    unsigned RXDONEIE:1;
    unsigned FWMARKIE:1;
    unsigned EWMARKIE:1;
    unsigned :3;
    unsigned RXBUSEIE:1;
    unsigned TXBUSEIE:1;
  };
  struct {
    unsigned w:32;
  };
} __ETHIENbits_t;
extern volatile __ETHIENbits_t ETHIENbits __asm__ ("ETHIEN") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIENCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIENSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIENINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIRQ __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RXOVFLW:1;
    unsigned RXBUFNA:1;
    unsigned TXABORT:1;
    unsigned TXDONE:1;
    unsigned :1;
    unsigned RXACT:1;
    unsigned PKTPEND:1;
    unsigned RXDONE:1;
    unsigned FWMARK:1;
    unsigned EWMARK:1;
    unsigned :3;
    unsigned RXBUSE:1;
    unsigned TXBUSE:1;
  };
  struct {
    unsigned w:32;
  };
} __ETHIRQbits_t;
extern volatile __ETHIRQbits_t ETHIRQbits __asm__ ("ETHIRQ") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIRQCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIRQSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHIRQINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSTAT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :5;
    unsigned RXBUSY:1;
    unsigned TXBUSY:1;
    unsigned BUSY:1;
    unsigned :8;
    unsigned BUFCNT:8;
  };
  struct {
    unsigned :7;
    unsigned ETHBUSY:1;
  };
  struct {
    unsigned w:32;
  };
} __ETHSTATbits_t;
extern volatile __ETHSTATbits_t ETHSTATbits __asm__ ("ETHSTAT") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSTATCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSTATSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSTATINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXOVFLOW __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RXOVFLWCNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHRXOVFLOWbits_t;
extern volatile __ETHRXOVFLOWbits_t ETHRXOVFLOWbits __asm__ ("ETHRXOVFLOW") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXOVFLOWCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXOVFLOWSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHRXOVFLOWINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMTXOK __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FRMTXOKCNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHFRMTXOKbits_t;
extern volatile __ETHFRMTXOKbits_t ETHFRMTXOKbits __asm__ ("ETHFRMTXOK") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMTXOKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMTXOKSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMTXOKINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSCOLFRM __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SCOLFRMCNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHSCOLFRMbits_t;
extern volatile __ETHSCOLFRMbits_t ETHSCOLFRMbits __asm__ ("ETHSCOLFRM") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSCOLFRMCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSCOLFRMSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHSCOLFRMINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHMCOLFRM __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MCOLFRMCNT:16;
  };
  struct {
    unsigned MCOLFRM_CNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHMCOLFRMbits_t;
extern volatile __ETHMCOLFRMbits_t ETHMCOLFRMbits __asm__ ("ETHMCOLFRM") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHMCOLFRMCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHMCOLFRMSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHMCOLFRMINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMRXOK __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FRMRXOKCNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHFRMRXOKbits_t;
extern volatile __ETHFRMRXOKbits_t ETHFRMRXOKbits __asm__ ("ETHFRMRXOK") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMRXOKCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMRXOKSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFRMRXOKINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFCSERR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FCSERRCNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHFCSERRbits_t;
extern volatile __ETHFCSERRbits_t ETHFCSERRbits __asm__ ("ETHFCSERR") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFCSERRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFCSERRSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHFCSERRINV __attribute__((section("sfrs")));
extern volatile unsigned int        ETHALGNERR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned ALGNERRCNT:16;
  };
  struct {
    unsigned w:32;
  };
} __ETHALGNERRbits_t;
extern volatile __ETHALGNERRbits_t ETHALGNERRbits __asm__ ("ETHALGNERR") __attribute__((section("sfrs")));
extern volatile unsigned int        ETHALGNERRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        ETHALGNERRSET __attribute__((section("sfrs")));
extern volatile unsigned int        ETHALGNERRINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RXENABLE:1;
    unsigned PASSALL:1;
    unsigned RXPAUSE:1;
    unsigned TXPAUSE:1;
    unsigned LOOPBACK:1;
    unsigned :3;
    unsigned RESETTFUN:1;
    unsigned RESETTMCS:1;
    unsigned RESETRFUN:1;
    unsigned RESETRMCS:1;
    unsigned :2;
    unsigned SIMRESET:1;
    unsigned SOFTRESET:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1CFG1bits_t;
extern volatile __EMAC1CFG1bits_t EMAC1CFG1bits __asm__ ("EMAC1CFG1") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RXENABLE:1;
    unsigned PASSALL:1;
    unsigned RXPAUSE:1;
    unsigned TXPAUSE:1;
    unsigned LOOPBACK:1;
    unsigned :3;
    unsigned RESETTFUN:1;
    unsigned RESETTMCS:1;
    unsigned RESETRFUN:1;
    unsigned RESETRMCS:1;
    unsigned :2;
    unsigned SIMRESET:1;
    unsigned SOFTRESET:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxCFG1bits_t;
extern volatile __EMACxCFG1bits_t EMACxCFG1bits __asm__ ("EMACxCFG1") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG1SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG1SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG1INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG1INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FULLDPLX:1;
    unsigned LENGTHCK:1;
    unsigned HUGEFRM:1;
    unsigned DELAYCRC:1;
    unsigned CRCENABLE:1;
    unsigned PADENABLE:1;
    unsigned VLANPAD:1;
    unsigned AUTOPAD:1;
    unsigned PUREPRE:1;
    unsigned LONGPRE:1;
    unsigned :2;
    unsigned NOBKOFF:1;
    unsigned BPNOBKOFF:1;
    unsigned EXCESSDFR:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1CFG2bits_t;
extern volatile __EMAC1CFG2bits_t EMAC1CFG2bits __asm__ ("EMAC1CFG2") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FULLDPLX:1;
    unsigned LENGTHCK:1;
    unsigned HUGEFRM:1;
    unsigned DELAYCRC:1;
    unsigned CRCENABLE:1;
    unsigned PADENABLE:1;
    unsigned VLANPAD:1;
    unsigned AUTOPAD:1;
    unsigned PUREPRE:1;
    unsigned LONGPRE:1;
    unsigned :2;
    unsigned NOBKOFF:1;
    unsigned BPNOBKOFF:1;
    unsigned EXCESSDFR:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxCFG2bits_t;
extern volatile __EMACxCFG2bits_t EMACxCFG2bits __asm__ ("EMACxCFG2") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG2SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG2SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CFG2INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCFG2INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned B2BIPKTGP:7;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1IPGTbits_t;
extern volatile __EMAC1IPGTbits_t EMAC1IPGTbits __asm__ ("EMAC1IPGT") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned B2BIPKTGP:7;
  };
  struct {
    unsigned w:32;
  };
} __EMACxIPGTbits_t;
extern volatile __EMACxIPGTbits_t EMACxIPGTbits __asm__ ("EMACxIPGT") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGTSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGTSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGTINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGTINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned NB2BIPKTGP2:7;
    unsigned :1;
    unsigned NB2BIPKTGP1:7;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1IPGRbits_t;
extern volatile __EMAC1IPGRbits_t EMAC1IPGRbits __asm__ ("EMAC1IPGR") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned NB2BIPKTGP2:7;
    unsigned :1;
    unsigned NB2BIPKTGP1:7;
  };
  struct {
    unsigned w:32;
  };
} __EMACxIPGRbits_t;
extern volatile __EMACxIPGRbits_t EMACxIPGRbits __asm__ ("EMACxIPGR") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGRSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGRSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1IPGRINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxIPGRINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CLRT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RETX:4;
    unsigned :4;
    unsigned CWINDOW:6;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1CLRTbits_t;
extern volatile __EMAC1CLRTbits_t EMAC1CLRTbits __asm__ ("EMAC1CLRT") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCLRT __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned RETX:4;
    unsigned :4;
    unsigned CWINDOW:6;
  };
  struct {
    unsigned w:32;
  };
} __EMACxCLRTbits_t;
extern volatile __EMACxCLRTbits_t EMACxCLRTbits __asm__ ("EMACxCLRT") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CLRTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCLRTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CLRTSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCLRTSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1CLRTINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxCLRTINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MAXF __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MACMAXF:16;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MAXFbits_t;
extern volatile __EMAC1MAXFbits_t EMAC1MAXFbits __asm__ ("EMAC1MAXF") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMAXF __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MACMAXF:16;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMAXFbits_t;
extern volatile __EMACxMAXFbits_t EMACxMAXFbits __asm__ ("EMACxMAXF") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MAXFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMAXFCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MAXFSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMAXFSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MAXFINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMAXFINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SUPP __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :8;
    unsigned SPEEDRMII:1;
    unsigned :2;
    unsigned RESETRMII:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1SUPPbits_t;
extern volatile __EMAC1SUPPbits_t EMAC1SUPPbits __asm__ ("EMAC1SUPP") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSUPP __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned :8;
    unsigned SPEEDRMII:1;
    unsigned :2;
    unsigned RESETRMII:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxSUPPbits_t;
extern volatile __EMACxSUPPbits_t EMACxSUPPbits __asm__ ("EMACxSUPP") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SUPPCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSUPPCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SUPPSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSUPPSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SUPPINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSUPPINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1TEST __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SHRTQNTA:1;
    unsigned TESTPAUSE:1;
    unsigned TESTBP:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1TESTbits_t;
extern volatile __EMAC1TESTbits_t EMAC1TESTbits __asm__ ("EMAC1TEST") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxTEST __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SHRTQNTA:1;
    unsigned TESTPAUSE:1;
    unsigned TESTBP:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxTESTbits_t;
extern volatile __EMACxTESTbits_t EMACxTESTbits __asm__ ("EMACxTEST") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1TESTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxTESTCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1TESTSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxTESTSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1TESTINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxTESTINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCFG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SCANINC:1;
    unsigned NOPRE:1;
    unsigned CLKSEL:4;
    unsigned :9;
    unsigned RESETMGMT:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MCFGbits_t;
extern volatile __EMAC1MCFGbits_t EMAC1MCFGbits __asm__ ("EMAC1MCFG") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCFG __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned SCANINC:1;
    unsigned NOPRE:1;
    unsigned CLKSEL:4;
    unsigned :9;
    unsigned RESETMGMT:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMCFGbits_t;
extern volatile __EMACxMCFGbits_t EMACxMCFGbits __asm__ ("EMACxMCFG") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCFGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCFGCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCFGSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCFGSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCFGINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCFGINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCMD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned READ:1;
    unsigned SCAN:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MCMDbits_t;
extern volatile __EMAC1MCMDbits_t EMAC1MCMDbits __asm__ ("EMAC1MCMD") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCMD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned READ:1;
    unsigned SCAN:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMCMDbits_t;
extern volatile __EMACxMCMDbits_t EMACxMCMDbits __asm__ ("EMACxMCMD") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCMDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCMDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCMDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCMDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MCMDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMCMDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MADR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned REGADDR:5;
    unsigned :3;
    unsigned PHYADDR:5;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MADRbits_t;
extern volatile __EMAC1MADRbits_t EMAC1MADRbits __asm__ ("EMAC1MADR") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMADR __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned REGADDR:5;
    unsigned :3;
    unsigned PHYADDR:5;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMADRbits_t;
extern volatile __EMACxMADRbits_t EMACxMADRbits __asm__ ("EMACxMADR") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MADRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMADRCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MADRSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMADRSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MADRINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMADRINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MWTD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MWTD:16;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MWTDbits_t;
extern volatile __EMAC1MWTDbits_t EMAC1MWTDbits __asm__ ("EMAC1MWTD") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMWTD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MWTD:16;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMWTDbits_t;
extern volatile __EMACxMWTDbits_t EMACxMWTDbits __asm__ ("EMACxMWTD") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MWTDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMWTDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MWTDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMWTDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MWTDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMWTDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MRDD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MRDD:16;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MRDDbits_t;
extern volatile __EMAC1MRDDbits_t EMAC1MRDDbits __asm__ ("EMAC1MRDD") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMRDD __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MRDD:16;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMRDDbits_t;
extern volatile __EMACxMRDDbits_t EMACxMRDDbits __asm__ ("EMACxMRDD") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MRDDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMRDDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MRDDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMRDDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MRDDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMRDDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MIND __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MIIMBUSY:1;
    unsigned SCAN:1;
    unsigned NOTVALID:1;
    unsigned LINKFAIL:1;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1MINDbits_t;
extern volatile __EMAC1MINDbits_t EMAC1MINDbits __asm__ ("EMAC1MIND") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMIND __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned MIIMBUSY:1;
    unsigned SCAN:1;
    unsigned NOTVALID:1;
    unsigned LINKFAIL:1;
  };
  struct {
    unsigned w:32;
  };
} __EMACxMINDbits_t;
extern volatile __EMACxMINDbits_t EMACxMINDbits __asm__ ("EMACxMIND") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MINDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMINDCLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MINDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMINDSET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1MINDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxMINDINV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA0 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STNADDR5:8;
    unsigned STNADDR6:8;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1SA0bits_t;
extern volatile __EMAC1SA0bits_t EMAC1SA0bits __asm__ ("EMAC1SA0") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA0 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STNADDR5:8;
    unsigned STNADDR6:8;
  };
  struct {
    unsigned w:32;
  };
} __EMACxSA0bits_t;
extern volatile __EMACxSA0bits_t EMACxSA0bits __asm__ ("EMACxSA0") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA0CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA0SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA0SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA0INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA0INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STNADDR3:8;
    unsigned STNADDR4:8;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1SA1bits_t;
extern volatile __EMAC1SA1bits_t EMAC1SA1bits __asm__ ("EMAC1SA1") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STNADDR3:8;
    unsigned STNADDR4:8;
  };
  struct {
    unsigned w:32;
  };
} __EMACxSA1bits_t;
extern volatile __EMACxSA1bits_t EMACxSA1bits __asm__ ("EMACxSA1") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA1CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA1SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA1SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA1INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA1INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STNADDR1:8;
    unsigned STNADDR2:8;
  };
  struct {
    unsigned w:32;
  };
} __EMAC1SA2bits_t;
extern volatile __EMAC1SA2bits_t EMAC1SA2bits __asm__ ("EMAC1SA2") __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned STNADDR1:8;
    unsigned STNADDR2:8;
  };
  struct {
    unsigned w:32;
  };
} __EMACxSA2bits_t;
extern volatile __EMACxSA2bits_t EMACxSA2bits __asm__ ("EMACxSA2") __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA2CLR __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA2SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA2SET __attribute__((section("sfrs")));
extern volatile unsigned int        EMAC1SA2INV __attribute__((section("sfrs")));
extern volatile unsigned int        EMACxSA2INV __attribute__((section("sfrs")));
extern volatile unsigned int        DEVCFG3 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned USERID:16;
    unsigned FSRSSEL:3;
    unsigned :5;
    unsigned FMIIEN:1;
    unsigned FETHIO:1;
    unsigned :4;
    unsigned FUSBIDIO:1;
    unsigned FVBUSONIO:1;
  };
  struct {
    unsigned w:32;
  };
} __DEVCFG3bits_t;
extern volatile __DEVCFG3bits_t DEVCFG3bits __asm__ ("DEVCFG3") __attribute__((section("sfrs")));
extern volatile unsigned int        DEVCFG2 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FPLLIDIV:3;
    unsigned :1;
    unsigned FPLLMUL:3;
    unsigned :1;
    unsigned UPLLIDIV:3;
    unsigned :4;
    unsigned UPLLEN:1;
    unsigned FPLLODIV:3;
  };
  struct {
    unsigned w:32;
  };
} __DEVCFG2bits_t;
extern volatile __DEVCFG2bits_t DEVCFG2bits __asm__ ("DEVCFG2") __attribute__((section("sfrs")));
extern volatile unsigned int        DEVCFG1 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned FNOSC:3;
    unsigned :2;
    unsigned FSOSCEN:1;
    unsigned :1;
    unsigned IESO:1;
    unsigned POSCMOD:2;
    unsigned OSCIOFNC:1;
    unsigned :1;
    unsigned FPBDIV:2;
    unsigned FCKSM:2;
    unsigned WDTPS:5;
    unsigned :2;
    unsigned FWDTEN:1;
  };
  struct {
    unsigned w:32;
  };
} __DEVCFG1bits_t;
extern volatile __DEVCFG1bits_t DEVCFG1bits __asm__ ("DEVCFG1") __attribute__((section("sfrs")));
extern volatile unsigned int        DEVCFG0 __attribute__((section("sfrs")));
typedef union {
  struct {
    unsigned DEBUG:2;
    unsigned RESERVED1:1;
    unsigned ICESEL:1;
    unsigned :8;
    unsigned PWP:8;
    unsigned :4;
    unsigned BWP:1;
    unsigned :3;
    unsigned CP:1;
    unsigned :2;
    unsigned RESERVED:1;
  };
  struct {
    unsigned FDEBUG:2;
  };
  struct {
    unsigned w:32;
  };
} __DEVCFG0bits_t;
extern volatile __DEVCFG0bits_t DEVCFG0bits __asm__ ("DEVCFG0") __attribute__((section("sfrs")));
#elif defined (__LANGUAGE_ASSEMBLY__)
  .extern WDTCON           /* 0xBF800000 */
  .extern WDTCONCLR        /* 0xBF800004 */
  .extern WDTCONSET        /* 0xBF800008 */
  .extern WDTCONINV        /* 0xBF80000C */
  .extern RTCCON           /* 0xBF800200 */
  .extern RTCCONCLR        /* 0xBF800204 */
  .extern RTCCONSET        /* 0xBF800208 */
  .extern RTCCONINV        /* 0xBF80020C */
  .extern RTCALRM          /* 0xBF800210 */
  .extern RTCALRMCLR       /* 0xBF800214 */
  .extern RTCALRMSET       /* 0xBF800218 */
  .extern RTCALRMINV       /* 0xBF80021C */
  .extern RTCTIME          /* 0xBF800220 */
  .extern RTCTIMECLR       /* 0xBF800224 */
  .extern RTCTIMESET       /* 0xBF800228 */
  .extern RTCTIMEINV       /* 0xBF80022C */
  .extern RTCDATE          /* 0xBF800230 */
  .extern RTCDATECLR       /* 0xBF800234 */
  .extern RTCDATESET       /* 0xBF800238 */
  .extern RTCDATEINV       /* 0xBF80023C */
  .extern ALRMTIME         /* 0xBF800240 */
  .extern ALRMTIMECLR      /* 0xBF800244 */
  .extern ALRMTIMESET      /* 0xBF800248 */
  .extern ALRMTIMEINV      /* 0xBF80024C */
  .extern ALRMDATE         /* 0xBF800250 */
  .extern ALRMDATECLR      /* 0xBF800254 */
  .extern ALRMDATESET      /* 0xBF800258 */
  .extern ALRMDATEINV      /* 0xBF80025C */
  .extern T1CON            /* 0xBF800600 */
  .extern T1CONCLR         /* 0xBF800604 */
  .extern T1CONSET         /* 0xBF800608 */
  .extern T1CONINV         /* 0xBF80060C */
  .extern TMR1             /* 0xBF800610 */
  .extern TMR1CLR          /* 0xBF800614 */
  .extern TMR1SET          /* 0xBF800618 */
  .extern TMR1INV          /* 0xBF80061C */
  .extern PR1              /* 0xBF800620 */
  .extern PR1CLR           /* 0xBF800624 */
  .extern PR1SET           /* 0xBF800628 */
  .extern PR1INV           /* 0xBF80062C */
  .extern T2CON            /* 0xBF800800 */
  .extern T2CONCLR         /* 0xBF800804 */
  .extern T2CONSET         /* 0xBF800808 */
  .extern T2CONINV         /* 0xBF80080C */
  .extern TMR2             /* 0xBF800810 */
  .extern TMR2CLR          /* 0xBF800814 */
  .extern TMR2SET          /* 0xBF800818 */
  .extern TMR2INV          /* 0xBF80081C */
  .extern PR2              /* 0xBF800820 */
  .extern PR2CLR           /* 0xBF800824 */
  .extern PR2SET           /* 0xBF800828 */
  .extern PR2INV           /* 0xBF80082C */
  .extern T3CON            /* 0xBF800A00 */
  .extern T3CONCLR         /* 0xBF800A04 */
  .extern T3CONSET         /* 0xBF800A08 */
  .extern T3CONINV         /* 0xBF800A0C */
  .extern TMR3             /* 0xBF800A10 */
  .extern TMR3CLR          /* 0xBF800A14 */
  .extern TMR3SET          /* 0xBF800A18 */
  .extern TMR3INV          /* 0xBF800A1C */
  .extern PR3              /* 0xBF800A20 */
  .extern PR3CLR           /* 0xBF800A24 */
  .extern PR3SET           /* 0xBF800A28 */
  .extern PR3INV           /* 0xBF800A2C */
  .extern T4CON            /* 0xBF800C00 */
  .extern T4CONCLR         /* 0xBF800C04 */
  .extern T4CONSET         /* 0xBF800C08 */
  .extern T4CONINV         /* 0xBF800C0C */
  .extern TMR4             /* 0xBF800C10 */
  .extern TMR4CLR          /* 0xBF800C14 */
  .extern TMR4SET          /* 0xBF800C18 */
  .extern TMR4INV          /* 0xBF800C1C */
  .extern PR4              /* 0xBF800C20 */
  .extern PR4CLR           /* 0xBF800C24 */
  .extern PR4SET           /* 0xBF800C28 */
  .extern PR4INV           /* 0xBF800C2C */
  .extern T5CON            /* 0xBF800E00 */
  .extern T5CONCLR         /* 0xBF800E04 */
  .extern T5CONSET         /* 0xBF800E08 */
  .extern T5CONINV         /* 0xBF800E0C */
  .extern TMR5             /* 0xBF800E10 */
  .extern TMR5CLR          /* 0xBF800E14 */
  .extern TMR5SET          /* 0xBF800E18 */
  .extern TMR5INV          /* 0xBF800E1C */
  .extern PR5              /* 0xBF800E20 */
  .extern PR5CLR           /* 0xBF800E24 */
  .extern PR5SET           /* 0xBF800E28 */
  .extern PR5INV           /* 0xBF800E2C */
  .extern IC1CON           /* 0xBF802000 */
  .extern IC1CONCLR        /* 0xBF802004 */
  .extern IC1CONSET        /* 0xBF802008 */
  .extern IC1CONINV        /* 0xBF80200C */
  .extern IC1BUF           /* 0xBF802010 */
  .extern IC2CON           /* 0xBF802200 */
  .extern IC2CONCLR        /* 0xBF802204 */
  .extern IC2CONSET        /* 0xBF802208 */
  .extern IC2CONINV        /* 0xBF80220C */
  .extern IC2BUF           /* 0xBF802210 */
  .extern IC3CON           /* 0xBF802400 */
  .extern IC3CONCLR        /* 0xBF802404 */
  .extern IC3CONSET        /* 0xBF802408 */
  .extern IC3CONINV        /* 0xBF80240C */
  .extern IC3BUF           /* 0xBF802410 */
  .extern IC4CON           /* 0xBF802600 */
  .extern IC4CONCLR        /* 0xBF802604 */
  .extern IC4CONSET        /* 0xBF802608 */
  .extern IC4CONINV        /* 0xBF80260C */
  .extern IC4BUF           /* 0xBF802610 */
  .extern IC5CON           /* 0xBF802800 */
  .extern IC5CONCLR        /* 0xBF802804 */
  .extern IC5CONSET        /* 0xBF802808 */
  .extern IC5CONINV        /* 0xBF80280C */
  .extern IC5BUF           /* 0xBF802810 */
  .extern OC1CON           /* 0xBF803000 */
  .extern OC1CONCLR        /* 0xBF803004 */
  .extern OC1CONSET        /* 0xBF803008 */
  .extern OC1CONINV        /* 0xBF80300C */
  .extern OC1R             /* 0xBF803010 */
  .extern OC1RCLR          /* 0xBF803014 */
  .extern OC1RSET          /* 0xBF803018 */
  .extern OC1RINV          /* 0xBF80301C */
  .extern OC1RS            /* 0xBF803020 */
  .extern OC1RSCLR         /* 0xBF803024 */
  .extern OC1RSSET         /* 0xBF803028 */
  .extern OC1RSINV         /* 0xBF80302C */
  .extern OC2CON           /* 0xBF803200 */
  .extern OC2CONCLR        /* 0xBF803204 */
  .extern OC2CONSET        /* 0xBF803208 */
  .extern OC2CONINV        /* 0xBF80320C */
  .extern OC2R             /* 0xBF803210 */
  .extern OC2RCLR          /* 0xBF803214 */
  .extern OC2RSET          /* 0xBF803218 */
  .extern OC2RINV          /* 0xBF80321C */
  .extern OC2RS            /* 0xBF803220 */
  .extern OC2RSCLR         /* 0xBF803224 */
  .extern OC2RSSET         /* 0xBF803228 */
  .extern OC2RSINV         /* 0xBF80322C */
  .extern OC3CON           /* 0xBF803400 */
  .extern OC3CONCLR        /* 0xBF803404 */
  .extern OC3CONSET        /* 0xBF803408 */
  .extern OC3CONINV        /* 0xBF80340C */
  .extern OC3R             /* 0xBF803410 */
  .extern OC3RCLR          /* 0xBF803414 */
  .extern OC3RSET          /* 0xBF803418 */
  .extern OC3RINV          /* 0xBF80341C */
  .extern OC3RS            /* 0xBF803420 */
  .extern OC3RSCLR         /* 0xBF803424 */
  .extern OC3RSSET         /* 0xBF803428 */
  .extern OC3RSINV         /* 0xBF80342C */
  .extern OC4CON           /* 0xBF803600 */
  .extern OC4CONCLR        /* 0xBF803604 */
  .extern OC4CONSET        /* 0xBF803608 */
  .extern OC4CONINV        /* 0xBF80360C */
  .extern OC4R             /* 0xBF803610 */
  .extern OC4RCLR          /* 0xBF803614 */
  .extern OC4RSET          /* 0xBF803618 */
  .extern OC4RINV          /* 0xBF80361C */
  .extern OC4RS            /* 0xBF803620 */
  .extern OC4RSCLR         /* 0xBF803624 */
  .extern OC4RSSET         /* 0xBF803628 */
  .extern OC4RSINV         /* 0xBF80362C */
  .extern OC5CON           /* 0xBF803800 */
  .extern OC5CONCLR        /* 0xBF803804 */
  .extern OC5CONSET        /* 0xBF803808 */
  .extern OC5CONINV        /* 0xBF80380C */
  .extern OC5R             /* 0xBF803810 */
  .extern OC5RCLR          /* 0xBF803814 */
  .extern OC5RSET          /* 0xBF803818 */
  .extern OC5RINV          /* 0xBF80381C */
  .extern OC5RS            /* 0xBF803820 */
  .extern OC5RSCLR         /* 0xBF803824 */
  .extern OC5RSSET         /* 0xBF803828 */
  .extern OC5RSINV         /* 0xBF80382C */
  .extern I2C1ACON         /* 0xBF805000 */
  .extern I2C3CON          /* 0xBF805000 */
  .extern I2C1ACONCLR      /* 0xBF805004 */
  .extern I2C3CONCLR       /* 0xBF805004 */
  .extern I2C1ACONSET      /* 0xBF805008 */
  .extern I2C3CONSET       /* 0xBF805008 */
  .extern I2C1ACONINV      /* 0xBF80500C */
  .extern I2C3CONINV       /* 0xBF80500C */
  .extern I2C1ASTAT        /* 0xBF805010 */
  .extern I2C3STAT         /* 0xBF805010 */
  .extern I2C1ASTATCLR     /* 0xBF805014 */
  .extern I2C3STATCLR      /* 0xBF805014 */
  .extern I2C1ASTATSET     /* 0xBF805018 */
  .extern I2C3STATSET      /* 0xBF805018 */
  .extern I2C1ASTATINV     /* 0xBF80501C */
  .extern I2C3STATINV      /* 0xBF80501C */
  .extern I2C1AADD         /* 0xBF805020 */
  .extern I2C3ADD          /* 0xBF805020 */
  .extern I2C1AADDCLR      /* 0xBF805024 */
  .extern I2C3ADDCLR       /* 0xBF805024 */
  .extern I2C1AADDSET      /* 0xBF805028 */
  .extern I2C3ADDSET       /* 0xBF805028 */
  .extern I2C1AADDINV      /* 0xBF80502C */
  .extern I2C3ADDINV       /* 0xBF80502C */
  .extern I2C1AMSK         /* 0xBF805030 */
  .extern I2C3MSK          /* 0xBF805030 */
  .extern I2C1AMSKCLR      /* 0xBF805034 */
  .extern I2C3MSKCLR       /* 0xBF805034 */
  .extern I2C1AMSKSET      /* 0xBF805038 */
  .extern I2C3MSKSET       /* 0xBF805038 */
  .extern I2C1AMSKINV      /* 0xBF80503C */
  .extern I2C3MSKINV       /* 0xBF80503C */
  .extern I2C1ABRG         /* 0xBF805040 */
  .extern I2C3BRG          /* 0xBF805040 */
  .extern I2C1ABRGCLR      /* 0xBF805044 */
  .extern I2C3BRGCLR       /* 0xBF805044 */
  .extern I2C1ABRGSET      /* 0xBF805048 */
  .extern I2C3BRGSET       /* 0xBF805048 */
  .extern I2C1ABRGINV      /* 0xBF80504C */
  .extern I2C3BRGINV       /* 0xBF80504C */
  .extern I2C1ATRN         /* 0xBF805050 */
  .extern I2C3TRN          /* 0xBF805050 */
  .extern I2C1ATRNCLR      /* 0xBF805054 */
  .extern I2C3TRNCLR       /* 0xBF805054 */
  .extern I2C1ATRNSET      /* 0xBF805058 */
  .extern I2C3TRNSET       /* 0xBF805058 */
  .extern I2C1ATRNINV      /* 0xBF80505C */
  .extern I2C3TRNINV       /* 0xBF80505C */
  .extern I2C1ARCV         /* 0xBF805060 */
  .extern I2C3RCV          /* 0xBF805060 */
  .extern I2C2ACON         /* 0xBF805100 */
  .extern I2C4CON          /* 0xBF805100 */
  .extern I2C2ACONCLR      /* 0xBF805104 */
  .extern I2C4CONCLR       /* 0xBF805104 */
  .extern I2C2ACONSET      /* 0xBF805108 */
  .extern I2C4CONSET       /* 0xBF805108 */
  .extern I2C2ACONINV      /* 0xBF80510C */
  .extern I2C4CONINV       /* 0xBF80510C */
  .extern I2C2ASTAT        /* 0xBF805110 */
  .extern I2C4STAT         /* 0xBF805110 */
  .extern I2C2ASTATCLR     /* 0xBF805114 */
  .extern I2C4STATCLR      /* 0xBF805114 */
  .extern I2C2ASTATSET     /* 0xBF805118 */
  .extern I2C4STATSET      /* 0xBF805118 */
  .extern I2C2ASTATINV     /* 0xBF80511C */
  .extern I2C4STATINV      /* 0xBF80511C */
  .extern I2C2AADD         /* 0xBF805120 */
  .extern I2C4ADD          /* 0xBF805120 */
  .extern I2C2AADDCLR      /* 0xBF805124 */
  .extern I2C4ADDCLR       /* 0xBF805124 */
  .extern I2C2AADDSET      /* 0xBF805128 */
  .extern I2C4ADDSET       /* 0xBF805128 */
  .extern I2C2AADDINV      /* 0xBF80512C */
  .extern I2C4ADDINV       /* 0xBF80512C */
  .extern I2C2AMSK         /* 0xBF805130 */
  .extern I2C4MSK          /* 0xBF805130 */
  .extern I2C2AMSKCLR      /* 0xBF805134 */
  .extern I2C4MSKCLR       /* 0xBF805134 */
  .extern I2C2AMSKSET      /* 0xBF805138 */
  .extern I2C4MSKSET       /* 0xBF805138 */
  .extern I2C2AMSKINV      /* 0xBF80513C */
  .extern I2C4MSKINV       /* 0xBF80513C */
  .extern I2C2ABRG         /* 0xBF805140 */
  .extern I2C4BRG          /* 0xBF805140 */
  .extern I2C2ABRGCLR      /* 0xBF805144 */
  .extern I2C4BRGCLR       /* 0xBF805144 */
  .extern I2C2ABRGSET      /* 0xBF805148 */
  .extern I2C4BRGSET       /* 0xBF805148 */
  .extern I2C2ABRGINV      /* 0xBF80514C */
  .extern I2C4BRGINV       /* 0xBF80514C */
  .extern I2C2ATRN         /* 0xBF805150 */
  .extern I2C4TRN          /* 0xBF805150 */
  .extern I2C2ATRNCLR      /* 0xBF805154 */
  .extern I2C4TRNCLR       /* 0xBF805154 */
  .extern I2C2ATRNSET      /* 0xBF805158 */
  .extern I2C4TRNSET       /* 0xBF805158 */
  .extern I2C2ATRNINV      /* 0xBF80515C */
  .extern I2C4TRNINV       /* 0xBF80515C */
  .extern I2C2ARCV         /* 0xBF805160 */
  .extern I2C4RCV          /* 0xBF805160 */
  .extern I2C3ACON         /* 0xBF805200 */
  .extern I2C5CON          /* 0xBF805200 */
  .extern I2C3ACONCLR      /* 0xBF805204 */
  .extern I2C5CONCLR       /* 0xBF805204 */
  .extern I2C3ACONSET      /* 0xBF805208 */
  .extern I2C5CONSET       /* 0xBF805208 */
  .extern I2C3ACONINV      /* 0xBF80520C */
  .extern I2C5CONINV       /* 0xBF80520C */
  .extern I2C3ASTAT        /* 0xBF805210 */
  .extern I2C5STAT         /* 0xBF805210 */
  .extern I2C3ASTATCLR     /* 0xBF805214 */
  .extern I2C5STATCLR      /* 0xBF805214 */
  .extern I2C3ASTATSET     /* 0xBF805218 */
  .extern I2C5STATSET      /* 0xBF805218 */
  .extern I2C3ASTATINV     /* 0xBF80521C */
  .extern I2C5STATINV      /* 0xBF80521C */
  .extern I2C3AADD         /* 0xBF805220 */
  .extern I2C5ADD          /* 0xBF805220 */
  .extern I2C3AADDCLR      /* 0xBF805224 */
  .extern I2C5ADDCLR       /* 0xBF805224 */
  .extern I2C3AADDSET      /* 0xBF805228 */
  .extern I2C5ADDSET       /* 0xBF805228 */
  .extern I2C3AADDINV      /* 0xBF80522C */
  .extern I2C5ADDINV       /* 0xBF80522C */
  .extern I2C3AMSK         /* 0xBF805230 */
  .extern I2C5MSK          /* 0xBF805230 */
  .extern I2C3AMSKCLR      /* 0xBF805234 */
  .extern I2C5MSKCLR       /* 0xBF805234 */
  .extern I2C3AMSKSET      /* 0xBF805238 */
  .extern I2C5MSKSET       /* 0xBF805238 */
  .extern I2C3AMSKINV      /* 0xBF80523C */
  .extern I2C5MSKINV       /* 0xBF80523C */
  .extern I2C3ABRG         /* 0xBF805240 */
  .extern I2C5BRG          /* 0xBF805240 */
  .extern I2C3ABRGCLR      /* 0xBF805244 */
  .extern I2C5BRGCLR       /* 0xBF805244 */
  .extern I2C3ABRGSET      /* 0xBF805248 */
  .extern I2C5BRGSET       /* 0xBF805248 */
  .extern I2C3ABRGINV      /* 0xBF80524C */
  .extern I2C5BRGINV       /* 0xBF80524C */
  .extern I2C3ATRN         /* 0xBF805250 */
  .extern I2C5TRN          /* 0xBF805250 */
  .extern I2C3ATRNCLR      /* 0xBF805254 */
  .extern I2C5TRNCLR       /* 0xBF805254 */
  .extern I2C3ATRNSET      /* 0xBF805258 */
  .extern I2C5TRNSET       /* 0xBF805258 */
  .extern I2C3ATRNINV      /* 0xBF80525C */
  .extern I2C5TRNINV       /* 0xBF80525C */
  .extern I2C3ARCV         /* 0xBF805260 */
  .extern I2C5RCV          /* 0xBF805260 */
  .extern I2C1CON          /* 0xBF805300 */
  .extern I2C1CONCLR       /* 0xBF805304 */
  .extern I2C1CONSET       /* 0xBF805308 */
  .extern I2C1CONINV       /* 0xBF80530C */
  .extern I2C1STAT         /* 0xBF805310 */
  .extern I2C1STATCLR      /* 0xBF805314 */
  .extern I2C1STATSET      /* 0xBF805318 */
  .extern I2C1STATINV      /* 0xBF80531C */
  .extern I2C1ADD          /* 0xBF805320 */
  .extern I2C1ADDCLR       /* 0xBF805324 */
  .extern I2C1ADDSET       /* 0xBF805328 */
  .extern I2C1ADDINV       /* 0xBF80532C */
  .extern I2C1MSK          /* 0xBF805330 */
  .extern I2C1MSKCLR       /* 0xBF805334 */
  .extern I2C1MSKSET       /* 0xBF805338 */
  .extern I2C1MSKINV       /* 0xBF80533C */
  .extern I2C1BRG          /* 0xBF805340 */
  .extern I2C1BRGCLR       /* 0xBF805344 */
  .extern I2C1BRGSET       /* 0xBF805348 */
  .extern I2C1BRGINV       /* 0xBF80534C */
  .extern I2C1TRN          /* 0xBF805350 */
  .extern I2C1TRNCLR       /* 0xBF805354 */
  .extern I2C1TRNSET       /* 0xBF805358 */
  .extern I2C1TRNINV       /* 0xBF80535C */
  .extern I2C1RCV          /* 0xBF805360 */
  .extern SPI1ACON         /* 0xBF805800 */
  .extern SPI3CON          /* 0xBF805800 */
  .extern SPI1ACONCLR      /* 0xBF805804 */
  .extern SPI3CONCLR       /* 0xBF805804 */
  .extern SPI1ACONSET      /* 0xBF805808 */
  .extern SPI3CONSET       /* 0xBF805808 */
  .extern SPI1ACONINV      /* 0xBF80580C */
  .extern SPI3CONINV       /* 0xBF80580C */
  .extern SPI1ASTAT        /* 0xBF805810 */
  .extern SPI3STAT         /* 0xBF805810 */
  .extern SPI1ASTATCLR     /* 0xBF805814 */
  .extern SPI3STATCLR      /* 0xBF805814 */
  .extern SPI1ASTATSET     /* 0xBF805818 */
  .extern SPI3STATSET      /* 0xBF805818 */
  .extern SPI1ASTATINV     /* 0xBF80581C */
  .extern SPI3STATINV      /* 0xBF80581C */
  .extern SPI1ABUF         /* 0xBF805820 */
  .extern SPI3BUF          /* 0xBF805820 */
  .extern SPI1ABRG         /* 0xBF805830 */
  .extern SPI3BRG          /* 0xBF805830 */
  .extern SPI1ABRGCLR      /* 0xBF805834 */
  .extern SPI3BRGCLR       /* 0xBF805834 */
  .extern SPI1ABRGSET      /* 0xBF805838 */
  .extern SPI3BRGSET       /* 0xBF805838 */
  .extern SPI1ABRGINV      /* 0xBF80583C */
  .extern SPI3BRGINV       /* 0xBF80583C */
  .extern SPI2ACON         /* 0xBF805A00 */
  .extern SPI2CON          /* 0xBF805A00 */
  .extern SPI2ACONCLR      /* 0xBF805A04 */
  .extern SPI2CONCLR       /* 0xBF805A04 */
  .extern SPI2ACONSET      /* 0xBF805A08 */
  .extern SPI2CONSET       /* 0xBF805A08 */
  .extern SPI2ACONINV      /* 0xBF805A0C */
  .extern SPI2CONINV       /* 0xBF805A0C */
  .extern SPI2ASTAT        /* 0xBF805A10 */
  .extern SPI2STAT         /* 0xBF805A10 */
  .extern SPI2ASTATCLR     /* 0xBF805A14 */
  .extern SPI2STATCLR      /* 0xBF805A14 */
  .extern SPI2ASTATSET     /* 0xBF805A18 */
  .extern SPI2STATSET      /* 0xBF805A18 */
  .extern SPI2ASTATINV     /* 0xBF805A1C */
  .extern SPI2STATINV      /* 0xBF805A1C */
  .extern SPI2ABUF         /* 0xBF805A20 */
  .extern SPI2BUF          /* 0xBF805A20 */
  .extern SPI2ABRG         /* 0xBF805A30 */
  .extern SPI2BRG          /* 0xBF805A30 */
  .extern SPI2ABRGCLR      /* 0xBF805A34 */
  .extern SPI2BRGCLR       /* 0xBF805A34 */
  .extern SPI2ABRGSET      /* 0xBF805A38 */
  .extern SPI2BRGSET       /* 0xBF805A38 */
  .extern SPI2ABRGINV      /* 0xBF805A3C */
  .extern SPI2BRGINV       /* 0xBF805A3C */
  .extern SPI3ACON         /* 0xBF805C00 */
  .extern SPI4CON          /* 0xBF805C00 */
  .extern SPI3ACONCLR      /* 0xBF805C04 */
  .extern SPI4CONCLR       /* 0xBF805C04 */
  .extern SPI3ACONSET      /* 0xBF805C08 */
  .extern SPI4CONSET       /* 0xBF805C08 */
  .extern SPI3ACONINV      /* 0xBF805C0C */
  .extern SPI4CONINV       /* 0xBF805C0C */
  .extern SPI3ASTAT        /* 0xBF805C10 */
  .extern SPI4STAT         /* 0xBF805C10 */
  .extern SPI3ASTATCLR     /* 0xBF805C14 */
  .extern SPI4STATCLR      /* 0xBF805C14 */
  .extern SPI3ASTATSET     /* 0xBF805C18 */
  .extern SPI4STATSET      /* 0xBF805C18 */
  .extern SPI3ASTATINV     /* 0xBF805C1C */
  .extern SPI4STATINV      /* 0xBF805C1C */
  .extern SPI3ABUF         /* 0xBF805C20 */
  .extern SPI4BUF          /* 0xBF805C20 */
  .extern SPI3ABRG         /* 0xBF805C30 */
  .extern SPI4BRG          /* 0xBF805C30 */
  .extern SPI3ABRGCLR      /* 0xBF805C34 */
  .extern SPI4BRGCLR       /* 0xBF805C34 */
  .extern SPI3ABRGSET      /* 0xBF805C38 */
  .extern SPI4BRGSET       /* 0xBF805C38 */
  .extern SPI3ABRGINV      /* 0xBF805C3C */
  .extern SPI4BRGINV       /* 0xBF805C3C */
  .extern U1AMODE          /* 0xBF806000 */
  .extern U1MODE           /* 0xBF806000 */
  .extern U1AMODECLR       /* 0xBF806004 */
  .extern U1MODECLR        /* 0xBF806004 */
  .extern U1AMODESET       /* 0xBF806008 */
  .extern U1MODESET        /* 0xBF806008 */
  .extern U1AMODEINV       /* 0xBF80600C */
  .extern U1MODEINV        /* 0xBF80600C */
  .extern U1ASTA           /* 0xBF806010 */
  .extern U1STA            /* 0xBF806010 */
  .extern U1ASTACLR        /* 0xBF806014 */
  .extern U1STACLR         /* 0xBF806014 */
  .extern U1ASTASET        /* 0xBF806018 */
  .extern U1STASET         /* 0xBF806018 */
  .extern U1ASTAINV        /* 0xBF80601C */
  .extern U1STAINV         /* 0xBF80601C */
  .extern U1ATXREG         /* 0xBF806020 */
  .extern U1TXREG          /* 0xBF806020 */
  .extern U1ARXREG         /* 0xBF806030 */
  .extern U1RXREG          /* 0xBF806030 */
  .extern U1ABRG           /* 0xBF806040 */
  .extern U1BRG            /* 0xBF806040 */
  .extern U1ABRGCLR        /* 0xBF806044 */
  .extern U1BRGCLR         /* 0xBF806044 */
  .extern U1ABRGSET        /* 0xBF806048 */
  .extern U1BRGSET         /* 0xBF806048 */
  .extern U1ABRGINV        /* 0xBF80604C */
  .extern U1BRGINV         /* 0xBF80604C */
  .extern U1BMODE          /* 0xBF806200 */
  .extern U4MODE           /* 0xBF806200 */
  .extern U1BMODECLR       /* 0xBF806204 */
  .extern U4MODECLR        /* 0xBF806204 */
  .extern U1BMODESET       /* 0xBF806208 */
  .extern U4MODESET        /* 0xBF806208 */
  .extern U1BMODEINV       /* 0xBF80620C */
  .extern U4MODEINV        /* 0xBF80620C */
  .extern U1BSTA           /* 0xBF806210 */
  .extern U4STA            /* 0xBF806210 */
  .extern U1BSTACLR        /* 0xBF806214 */
  .extern U4STACLR         /* 0xBF806214 */
  .extern U1BSTASET        /* 0xBF806218 */
  .extern U4STASET         /* 0xBF806218 */
  .extern U1BSTAINV        /* 0xBF80621C */
  .extern U4STAINV         /* 0xBF80621C */
  .extern U1BTXREG         /* 0xBF806220 */
  .extern U4TXREG          /* 0xBF806220 */
  .extern U1BRXREG         /* 0xBF806230 */
  .extern U4RXREG          /* 0xBF806230 */
  .extern U1BBRG           /* 0xBF806240 */
  .extern U4BRG            /* 0xBF806240 */
  .extern U1BBRGCLR        /* 0xBF806244 */
  .extern U4BRGCLR         /* 0xBF806244 */
  .extern U1BBRGSET        /* 0xBF806248 */
  .extern U4BRGSET         /* 0xBF806248 */
  .extern U1BBRGINV        /* 0xBF80624C */
  .extern U4BRGINV         /* 0xBF80624C */
  .extern U2AMODE          /* 0xBF806400 */
  .extern U3MODE           /* 0xBF806400 */
  .extern U2AMODECLR       /* 0xBF806404 */
  .extern U3MODECLR        /* 0xBF806404 */
  .extern U2AMODESET       /* 0xBF806408 */
  .extern U3MODESET        /* 0xBF806408 */
  .extern U2AMODEINV       /* 0xBF80640C */
  .extern U3MODEINV        /* 0xBF80640C */
  .extern U2ASTA           /* 0xBF806410 */
  .extern U3STA            /* 0xBF806410 */
  .extern U2ASTACLR        /* 0xBF806414 */
  .extern U3STACLR         /* 0xBF806414 */
  .extern U2ASTASET        /* 0xBF806418 */
  .extern U3STASET         /* 0xBF806418 */
  .extern U2ASTAINV        /* 0xBF80641C */
  .extern U3STAINV         /* 0xBF80641C */
  .extern U2ATXREG         /* 0xBF806420 */
  .extern U3TXREG          /* 0xBF806420 */
  .extern U2ARXREG         /* 0xBF806430 */
  .extern U3RXREG          /* 0xBF806430 */
  .extern U2ABRG           /* 0xBF806440 */
  .extern U3BRG            /* 0xBF806440 */
  .extern U2ABRGCLR        /* 0xBF806444 */
  .extern U3BRGCLR         /* 0xBF806444 */
  .extern U2ABRGSET        /* 0xBF806448 */
  .extern U3BRGSET         /* 0xBF806448 */
  .extern U2ABRGINV        /* 0xBF80644C */
  .extern U3BRGINV         /* 0xBF80644C */
  .extern U2BMODE          /* 0xBF806600 */
  .extern U6MODE           /* 0xBF806600 */
  .extern U2BMODECLR       /* 0xBF806604 */
  .extern U6MODECLR        /* 0xBF806604 */
  .extern U2BMODESET       /* 0xBF806608 */
  .extern U6MODESET        /* 0xBF806608 */
  .extern U2BMODEINV       /* 0xBF80660C */
  .extern U6MODEINV        /* 0xBF80660C */
  .extern U2BSTA           /* 0xBF806610 */
  .extern U6STA            /* 0xBF806610 */
  .extern U2BSTACLR        /* 0xBF806614 */
  .extern U6STACLR         /* 0xBF806614 */
  .extern U2BSTASET        /* 0xBF806618 */
  .extern U6STASET         /* 0xBF806618 */
  .extern U2BSTAINV        /* 0xBF80661C */
  .extern U6STAINV         /* 0xBF80661C */
  .extern U2BTXREG         /* 0xBF806620 */
  .extern U6TXREG          /* 0xBF806620 */
  .extern U2BRXREG         /* 0xBF806630 */
  .extern U6RXREG          /* 0xBF806630 */
  .extern U2BBRG           /* 0xBF806640 */
  .extern U6BRG            /* 0xBF806640 */
  .extern U2BBRGCLR        /* 0xBF806644 */
  .extern U6BRGCLR         /* 0xBF806644 */
  .extern U2BBRGSET        /* 0xBF806648 */
  .extern U6BRGSET         /* 0xBF806648 */
  .extern U2BBRGINV        /* 0xBF80664C */
  .extern U6BRGINV         /* 0xBF80664C */
  .extern U2MODE           /* 0xBF806800 */
  .extern U3AMODE          /* 0xBF806800 */
  .extern U2MODECLR        /* 0xBF806804 */
  .extern U3AMODECLR       /* 0xBF806804 */
  .extern U2MODESET        /* 0xBF806808 */
  .extern U3AMODESET       /* 0xBF806808 */
  .extern U2MODEINV        /* 0xBF80680C */
  .extern U3AMODEINV       /* 0xBF80680C */
  .extern U2STA            /* 0xBF806810 */
  .extern U3ASTA           /* 0xBF806810 */
  .extern U2STACLR         /* 0xBF806814 */
  .extern U3ASTACLR        /* 0xBF806814 */
  .extern U2STASET         /* 0xBF806818 */
  .extern U3ASTASET        /* 0xBF806818 */
  .extern U2STAINV         /* 0xBF80681C */
  .extern U3ASTAINV        /* 0xBF80681C */
  .extern U2TXREG          /* 0xBF806820 */
  .extern U3ATXREG         /* 0xBF806820 */
  .extern U2RXREG          /* 0xBF806830 */
  .extern U3ARXREG         /* 0xBF806830 */
  .extern U2BRG            /* 0xBF806840 */
  .extern U3ABRG           /* 0xBF806840 */
  .extern U2BRGCLR         /* 0xBF806844 */
  .extern U3ABRGCLR        /* 0xBF806844 */
  .extern U2BRGSET         /* 0xBF806848 */
  .extern U3ABRGSET        /* 0xBF806848 */
  .extern U2BRGINV         /* 0xBF80684C */
  .extern U3ABRGINV        /* 0xBF80684C */
  .extern U3BMODE          /* 0xBF806A00 */
  .extern U5MODE           /* 0xBF806A00 */
  .extern U3BMODECLR       /* 0xBF806A04 */
  .extern U5MODECLR        /* 0xBF806A04 */
  .extern U3BMODESET       /* 0xBF806A08 */
  .extern U5MODESET        /* 0xBF806A08 */
  .extern U3BMODEINV       /* 0xBF806A0C */
  .extern U5MODEINV        /* 0xBF806A0C */
  .extern U3BSTA           /* 0xBF806A10 */
  .extern U5STA            /* 0xBF806A10 */
  .extern U3BSTACLR        /* 0xBF806A14 */
  .extern U5STACLR         /* 0xBF806A14 */
  .extern U3BSTASET        /* 0xBF806A18 */
  .extern U5STASET         /* 0xBF806A18 */
  .extern U3BSTAINV        /* 0xBF806A1C */
  .extern U5STAINV         /* 0xBF806A1C */
  .extern U3BTXREG         /* 0xBF806A20 */
  .extern U5TXREG          /* 0xBF806A20 */
  .extern U3BRXREG         /* 0xBF806A30 */
  .extern U5RXREG          /* 0xBF806A30 */
  .extern U3BBRG           /* 0xBF806A40 */
  .extern U5BRG            /* 0xBF806A40 */
  .extern U3BBRGCLR        /* 0xBF806A44 */
  .extern U5BRGCLR         /* 0xBF806A44 */
  .extern U3BBRGSET        /* 0xBF806A48 */
  .extern U5BRGSET         /* 0xBF806A48 */
  .extern U3BBRGINV        /* 0xBF806A4C */
  .extern U5BRGINV         /* 0xBF806A4C */
  .extern PMCON            /* 0xBF807000 */
  .extern PMCONCLR         /* 0xBF807004 */
  .extern PMCONSET         /* 0xBF807008 */
  .extern PMCONINV         /* 0xBF80700C */
  .extern PMMODE           /* 0xBF807010 */
  .extern PMMODECLR        /* 0xBF807014 */
  .extern PMMODESET        /* 0xBF807018 */
  .extern PMMODEINV        /* 0xBF80701C */
  .extern PMADDR           /* 0xBF807020 */
  .extern PMADDRCLR        /* 0xBF807024 */
  .extern PMADDRSET        /* 0xBF807028 */
  .extern PMADDRINV        /* 0xBF80702C */
  .extern PMDOUT           /* 0xBF807030 */
  .extern PMDOUTCLR        /* 0xBF807034 */
  .extern PMDOUTSET        /* 0xBF807038 */
  .extern PMDOUTINV        /* 0xBF80703C */
  .extern PMDIN            /* 0xBF807040 */
  .extern PMDINCLR         /* 0xBF807044 */
  .extern PMDINSET         /* 0xBF807048 */
  .extern PMDININV         /* 0xBF80704C */
  .extern PMAEN            /* 0xBF807050 */
  .extern PMAENCLR         /* 0xBF807054 */
  .extern PMAENSET         /* 0xBF807058 */
  .extern PMAENINV         /* 0xBF80705C */
  .extern PMSTAT           /* 0xBF807060 */
  .extern PMSTATCLR        /* 0xBF807064 */
  .extern PMSTATSET        /* 0xBF807068 */
  .extern PMSTATINV        /* 0xBF80706C */
  .extern AD1CON1          /* 0xBF809000 */
  .extern AD1CON1CLR       /* 0xBF809004 */
  .extern AD1CON1SET       /* 0xBF809008 */
  .extern AD1CON1INV       /* 0xBF80900C */
  .extern AD1CON2          /* 0xBF809010 */
  .extern AD1CON2CLR       /* 0xBF809014 */
  .extern AD1CON2SET       /* 0xBF809018 */
  .extern AD1CON2INV       /* 0xBF80901C */
  .extern AD1CON3          /* 0xBF809020 */
  .extern AD1CON3CLR       /* 0xBF809024 */
  .extern AD1CON3SET       /* 0xBF809028 */
  .extern AD1CON3INV       /* 0xBF80902C */
  .extern AD1CHS           /* 0xBF809040 */
  .extern AD1CHSCLR        /* 0xBF809044 */
  .extern AD1CHSSET        /* 0xBF809048 */
  .extern AD1CHSINV        /* 0xBF80904C */
  .extern AD1CSSL          /* 0xBF809050 */
  .extern AD1CSSLCLR       /* 0xBF809054 */
  .extern AD1CSSLSET       /* 0xBF809058 */
  .extern AD1CSSLINV       /* 0xBF80905C */
  .extern AD1PCFG          /* 0xBF809060 */
  .extern AD1PCFGCLR       /* 0xBF809064 */
  .extern AD1PCFGSET       /* 0xBF809068 */
  .extern AD1PCFGINV       /* 0xBF80906C */
  .extern ADC1BUF0         /* 0xBF809070 */
  .extern ADC1BUF1         /* 0xBF809080 */
  .extern ADC1BUF2         /* 0xBF809090 */
  .extern ADC1BUF3         /* 0xBF8090A0 */
  .extern ADC1BUF4         /* 0xBF8090B0 */
  .extern ADC1BUF5         /* 0xBF8090C0 */
  .extern ADC1BUF6         /* 0xBF8090D0 */
  .extern ADC1BUF7         /* 0xBF8090E0 */
  .extern ADC1BUF8         /* 0xBF8090F0 */
  .extern ADC1BUF9         /* 0xBF809100 */
  .extern ADC1BUFA         /* 0xBF809110 */
  .extern ADC1BUFB         /* 0xBF809120 */
  .extern ADC1BUFC         /* 0xBF809130 */
  .extern ADC1BUFD         /* 0xBF809140 */
  .extern ADC1BUFE         /* 0xBF809150 */
  .extern ADC1BUFF         /* 0xBF809160 */
  .extern CVRCON           /* 0xBF809800 */
  .extern CVRCONCLR        /* 0xBF809804 */
  .extern CVRCONSET        /* 0xBF809808 */
  .extern CVRCONINV        /* 0xBF80980C */
  .extern CM1CON           /* 0xBF80A000 */
  .extern CM1CONCLR        /* 0xBF80A004 */
  .extern CM1CONSET        /* 0xBF80A008 */
  .extern CM1CONINV        /* 0xBF80A00C */
  .extern CM2CON           /* 0xBF80A010 */
  .extern CM2CONCLR        /* 0xBF80A014 */
  .extern CM2CONSET        /* 0xBF80A018 */
  .extern CM2CONINV        /* 0xBF80A01C */
  .extern CMSTAT           /* 0xBF80A060 */
  .extern CMSTATCLR        /* 0xBF80A064 */
  .extern CMSTATSET        /* 0xBF80A068 */
  .extern CMSTATINV        /* 0xBF80A06C */
  .extern OSCCON           /* 0xBF80F000 */
  .extern OSCCONCLR        /* 0xBF80F004 */
  .extern OSCCONSET        /* 0xBF80F008 */
  .extern OSCCONINV        /* 0xBF80F00C */
  .extern OSCTUN           /* 0xBF80F010 */
  .extern OSCTUNCLR        /* 0xBF80F014 */
  .extern OSCTUNSET        /* 0xBF80F018 */
  .extern OSCTUNINV        /* 0xBF80F01C */
  .extern DDPCON           /* 0xBF80F200 */
  .extern DEVID            /* 0xBF80F220 */
  .extern SYSKEY           /* 0xBF80F230 */
  .extern SYSKEYCLR        /* 0xBF80F234 */
  .extern SYSKEYSET        /* 0xBF80F238 */
  .extern SYSKEYINV        /* 0xBF80F23C */
  .extern NVMCON           /* 0xBF80F400 */
  .extern NVMCONCLR        /* 0xBF80F404 */
  .extern NVMCONSET        /* 0xBF80F408 */
  .extern NVMCONINV        /* 0xBF80F40C */
  .extern NVMKEY           /* 0xBF80F410 */
  .extern NVMADDR          /* 0xBF80F420 */
  .extern NVMADDRCLR       /* 0xBF80F424 */
  .extern NVMADDRSET       /* 0xBF80F428 */
  .extern NVMADDRINV       /* 0xBF80F42C */
  .extern NVMDATA          /* 0xBF80F430 */
  .extern NVMSRCADDR       /* 0xBF80F440 */
  .extern RCON             /* 0xBF80F600 */
  .extern RCONCLR          /* 0xBF80F604 */
  .extern RCONSET          /* 0xBF80F608 */
  .extern RCONINV          /* 0xBF80F60C */
  .extern RSWRST           /* 0xBF80F610 */
  .extern RSWRSTCLR        /* 0xBF80F614 */
  .extern RSWRSTSET        /* 0xBF80F618 */
  .extern RSWRSTINV        /* 0xBF80F61C */
  .extern _DDPSTAT         /* 0xBF880140 */
  .extern _STRO            /* 0xBF880170 */
  .extern _STROCLR         /* 0xBF880174 */
  .extern _STROSET         /* 0xBF880178 */
  .extern _STROINV         /* 0xBF88017C */
  .extern _APPO            /* 0xBF880180 */
  .extern _APPOCLR         /* 0xBF880184 */
  .extern _APPOSET         /* 0xBF880188 */
  .extern _APPOINV         /* 0xBF88018C */
  .extern _APPI            /* 0xBF880190 */
  .extern INTCON           /* 0xBF881000 */
  .extern INTCONCLR        /* 0xBF881004 */
  .extern INTCONSET        /* 0xBF881008 */
  .extern INTCONINV        /* 0xBF88100C */
  .extern INTSTAT          /* 0xBF881010 */
  .extern IPTMR            /* 0xBF881020 */
  .extern IPTMRCLR         /* 0xBF881024 */
  .extern IPTMRSET         /* 0xBF881028 */
  .extern IPTMRINV         /* 0xBF88102C */
  .extern IFS0             /* 0xBF881030 */
  .extern IFS0CLR          /* 0xBF881034 */
  .extern IFS0SET          /* 0xBF881038 */
  .extern IFS0INV          /* 0xBF88103C */
  .extern IFS1             /* 0xBF881040 */
  .extern IFS1CLR          /* 0xBF881044 */
  .extern IFS1SET          /* 0xBF881048 */
  .extern IFS1INV          /* 0xBF88104C */
  .extern IFS2             /* 0xBF881050 */
  .extern IFS2CLR          /* 0xBF881054 */
  .extern IFS2SET          /* 0xBF881058 */
  .extern IFS2INV          /* 0xBF88105C */
  .extern IEC0             /* 0xBF881060 */
  .extern IEC0CLR          /* 0xBF881064 */
  .extern IEC0SET          /* 0xBF881068 */
  .extern IEC0INV          /* 0xBF88106C */
  .extern IEC1             /* 0xBF881070 */
  .extern IEC1CLR          /* 0xBF881074 */
  .extern IEC1SET          /* 0xBF881078 */
  .extern IEC1INV          /* 0xBF88107C */
  .extern IEC2             /* 0xBF881080 */
  .extern IEC2CLR          /* 0xBF881084 */
  .extern IEC2SET          /* 0xBF881088 */
  .extern IEC2INV          /* 0xBF88108C */
  .extern IPC0             /* 0xBF881090 */
  .extern IPC0CLR          /* 0xBF881094 */
  .extern IPC0SET          /* 0xBF881098 */
  .extern IPC0INV          /* 0xBF88109C */
  .extern IPC1             /* 0xBF8810A0 */
  .extern IPC1CLR          /* 0xBF8810A4 */
  .extern IPC1SET          /* 0xBF8810A8 */
  .extern IPC1INV          /* 0xBF8810AC */
  .extern IPC2             /* 0xBF8810B0 */
  .extern IPC2CLR          /* 0xBF8810B4 */
  .extern IPC2SET          /* 0xBF8810B8 */
  .extern IPC2INV          /* 0xBF8810BC */
  .extern IPC3             /* 0xBF8810C0 */
  .extern IPC3CLR          /* 0xBF8810C4 */
  .extern IPC3SET          /* 0xBF8810C8 */
  .extern IPC3INV          /* 0xBF8810CC */
  .extern IPC4             /* 0xBF8810D0 */
  .extern IPC4CLR          /* 0xBF8810D4 */
  .extern IPC4SET          /* 0xBF8810D8 */
  .extern IPC4INV          /* 0xBF8810DC */
  .extern IPC5             /* 0xBF8810E0 */
  .extern IPC5CLR          /* 0xBF8810E4 */
  .extern IPC5SET          /* 0xBF8810E8 */
  .extern IPC5INV          /* 0xBF8810EC */
  .extern IPC6             /* 0xBF8810F0 */
  .extern IPC6CLR          /* 0xBF8810F4 */
  .extern IPC6SET          /* 0xBF8810F8 */
  .extern IPC6INV          /* 0xBF8810FC */
  .extern IPC7             /* 0xBF881100 */
  .extern IPC7CLR          /* 0xBF881104 */
  .extern IPC7SET          /* 0xBF881108 */
  .extern IPC7INV          /* 0xBF88110C */
  .extern IPC8             /* 0xBF881110 */
  .extern IPC8CLR          /* 0xBF881114 */
  .extern IPC8SET          /* 0xBF881118 */
  .extern IPC8INV          /* 0xBF88111C */
  .extern IPC9             /* 0xBF881120 */
  .extern IPC9CLR          /* 0xBF881124 */
  .extern IPC9SET          /* 0xBF881128 */
  .extern IPC9INV          /* 0xBF88112C */
  .extern IPC10            /* 0xBF881130 */
  .extern IPC10CLR         /* 0xBF881134 */
  .extern IPC10SET         /* 0xBF881138 */
  .extern IPC10INV         /* 0xBF88113C */
  .extern IPC11            /* 0xBF881140 */
  .extern IPC11CLR         /* 0xBF881144 */
  .extern IPC11SET         /* 0xBF881148 */
  .extern IPC11INV         /* 0xBF88114C */
  .extern IPC12            /* 0xBF881150 */
  .extern IPC12CLR         /* 0xBF881154 */
  .extern IPC12SET         /* 0xBF881158 */
  .extern IPC12INV         /* 0xBF88115C */
  .extern BMXCON           /* 0xBF882000 */
  .extern BMXCONCLR        /* 0xBF882004 */
  .extern BMXCONSET        /* 0xBF882008 */
  .extern BMXCONINV        /* 0xBF88200C */
  .extern BMXDKPBA         /* 0xBF882010 */
  .extern BMXDKPBACLR      /* 0xBF882014 */
  .extern BMXDKPBASET      /* 0xBF882018 */
  .extern BMXDKPBAINV      /* 0xBF88201C */
  .extern BMXDUDBA         /* 0xBF882020 */
  .extern BMXDUDBACLR      /* 0xBF882024 */
  .extern BMXDUDBASET      /* 0xBF882028 */
  .extern BMXDUDBAINV      /* 0xBF88202C */
  .extern BMXDUPBA         /* 0xBF882030 */
  .extern BMXDUPBACLR      /* 0xBF882034 */
  .extern BMXDUPBASET      /* 0xBF882038 */
  .extern BMXDUPBAINV      /* 0xBF88203C */
  .extern BMXDRMSZ         /* 0xBF882040 */
  .extern BMXPUPBA         /* 0xBF882050 */
  .extern BMXPUPBACLR      /* 0xBF882054 */
  .extern BMXPUPBASET      /* 0xBF882058 */
  .extern BMXPUPBAINV      /* 0xBF88205C */
  .extern BMXPFMSZ         /* 0xBF882060 */
  .extern BMXBOOTSZ        /* 0xBF882070 */
  .extern DMACON           /* 0xBF883000 */
  .extern DMACONCLR        /* 0xBF883004 */
  .extern DMACONSET        /* 0xBF883008 */
  .extern DMACONINV        /* 0xBF88300C */
  .extern DMASTAT          /* 0xBF883010 */
  .extern DMASTATCLR       /* 0xBF883014 */
  .extern DMASTATSET       /* 0xBF883018 */
  .extern DMASTATINV       /* 0xBF88301C */
  .extern DMAADDR          /* 0xBF883020 */
  .extern DMAADDRCLR       /* 0xBF883024 */
  .extern DMAADDRSET       /* 0xBF883028 */
  .extern DMAADDRINV       /* 0xBF88302C */
  .extern DCRCCON          /* 0xBF883030 */
  .extern DCRCCONCLR       /* 0xBF883034 */
  .extern DCRCCONSET       /* 0xBF883038 */
  .extern DCRCCONINV       /* 0xBF88303C */
  .extern DCRCDATA         /* 0xBF883040 */
  .extern DCRCDATACLR      /* 0xBF883044 */
  .extern DCRCDATASET      /* 0xBF883048 */
  .extern DCRCDATAINV      /* 0xBF88304C */
  .extern DCRCXOR          /* 0xBF883050 */
  .extern DCRCXORCLR       /* 0xBF883054 */
  .extern DCRCXORSET       /* 0xBF883058 */
  .extern DCRCXORINV       /* 0xBF88305C */
  .extern DCH0CON          /* 0xBF883060 */
  .extern DCH0CONCLR       /* 0xBF883064 */
  .extern DCH0CONSET       /* 0xBF883068 */
  .extern DCH0CONINV       /* 0xBF88306C */
  .extern DCH0ECON         /* 0xBF883070 */
  .extern DCH0ECONCLR      /* 0xBF883074 */
  .extern DCH0ECONSET      /* 0xBF883078 */
  .extern DCH0ECONINV      /* 0xBF88307C */
  .extern DCH0INT          /* 0xBF883080 */
  .extern DCH0INTCLR       /* 0xBF883084 */
  .extern DCH0INTSET       /* 0xBF883088 */
  .extern DCH0INTINV       /* 0xBF88308C */
  .extern DCH0SSA          /* 0xBF883090 */
  .extern DCH0SSACLR       /* 0xBF883094 */
  .extern DCH0SSASET       /* 0xBF883098 */
  .extern DCH0SSAINV       /* 0xBF88309C */
  .extern DCH0DSA          /* 0xBF8830A0 */
  .extern DCH0DSACLR       /* 0xBF8830A4 */
  .extern DCH0DSASET       /* 0xBF8830A8 */
  .extern DCH0DSAINV       /* 0xBF8830AC */
  .extern DCH0SSIZ         /* 0xBF8830B0 */
  .extern DCH0SSIZCLR      /* 0xBF8830B4 */
  .extern DCH0SSIZSET      /* 0xBF8830B8 */
  .extern DCH0SSIZINV      /* 0xBF8830BC */
  .extern DCH0DSIZ         /* 0xBF8830C0 */
  .extern DCH0DSIZCLR      /* 0xBF8830C4 */
  .extern DCH0DSIZSET      /* 0xBF8830C8 */
  .extern DCH0DSIZINV      /* 0xBF8830CC */
  .extern DCH0SPTR         /* 0xBF8830D0 */
  .extern DCH0SPTRCLR      /* 0xBF8830D4 */
  .extern DCH0SPTRSET      /* 0xBF8830D8 */
  .extern DCH0SPTRINV      /* 0xBF8830DC */
  .extern DCH0DPTR         /* 0xBF8830E0 */
  .extern DCH0DPTRCLR      /* 0xBF8830E4 */
  .extern DCH0DPTRSET      /* 0xBF8830E8 */
  .extern DCH0DPTRINV      /* 0xBF8830EC */
  .extern DCH0CSIZ         /* 0xBF8830F0 */
  .extern DCH0CSIZCLR      /* 0xBF8830F4 */
  .extern DCH0CSIZSET      /* 0xBF8830F8 */
  .extern DCH0CSIZINV      /* 0xBF8830FC */
  .extern DCH0CPTR         /* 0xBF883100 */
  .extern DCH0CPTRCLR      /* 0xBF883104 */
  .extern DCH0CPTRSET      /* 0xBF883108 */
  .extern DCH0CPTRINV      /* 0xBF88310C */
  .extern DCH0DAT          /* 0xBF883110 */
  .extern DCH0DATCLR       /* 0xBF883114 */
  .extern DCH0DATSET       /* 0xBF883118 */
  .extern DCH0DATINV       /* 0xBF88311C */
  .extern DCH1CON          /* 0xBF883120 */
  .extern DCH1CONCLR       /* 0xBF883124 */
  .extern DCH1CONSET       /* 0xBF883128 */
  .extern DCH1CONINV       /* 0xBF88312C */
  .extern DCH1ECON         /* 0xBF883130 */
  .extern DCH1ECONCLR      /* 0xBF883134 */
  .extern DCH1ECONSET      /* 0xBF883138 */
  .extern DCH1ECONINV      /* 0xBF88313C */
  .extern DCH1INT          /* 0xBF883140 */
  .extern DCH1INTCLR       /* 0xBF883144 */
  .extern DCH1INTSET       /* 0xBF883148 */
  .extern DCH1INTINV       /* 0xBF88314C */
  .extern DCH1SSA          /* 0xBF883150 */
  .extern DCH1SSACLR       /* 0xBF883154 */
  .extern DCH1SSASET       /* 0xBF883158 */
  .extern DCH1SSAINV       /* 0xBF88315C */
  .extern DCH1DSA          /* 0xBF883160 */
  .extern DCH1DSACLR       /* 0xBF883164 */
  .extern DCH1DSASET       /* 0xBF883168 */
  .extern DCH1DSAINV       /* 0xBF88316C */
  .extern DCH1SSIZ         /* 0xBF883170 */
  .extern DCH1SSIZCLR      /* 0xBF883174 */
  .extern DCH1SSIZSET      /* 0xBF883178 */
  .extern DCH1SSIZINV      /* 0xBF88317C */
  .extern DCH1DSIZ         /* 0xBF883180 */
  .extern DCH1DSIZCLR      /* 0xBF883184 */
  .extern DCH1DSIZSET      /* 0xBF883188 */
  .extern DCH1DSIZINV      /* 0xBF88318C */
  .extern DCH1SPTR         /* 0xBF883190 */
  .extern DCH1SPTRCLR      /* 0xBF883194 */
  .extern DCH1SPTRSET      /* 0xBF883198 */
  .extern DCH1SPTRINV      /* 0xBF88319C */
  .extern DCH1DPTR         /* 0xBF8831A0 */
  .extern DCH1DPTRCLR      /* 0xBF8831A4 */
  .extern DCH1DPTRSET      /* 0xBF8831A8 */
  .extern DCH1DPTRINV      /* 0xBF8831AC */
  .extern DCH1CSIZ         /* 0xBF8831B0 */
  .extern DCH1CSIZCLR      /* 0xBF8831B4 */
  .extern DCH1CSIZSET      /* 0xBF8831B8 */
  .extern DCH1CSIZINV      /* 0xBF8831BC */
  .extern DCH1CPTR         /* 0xBF8831C0 */
  .extern DCH1CPTRCLR      /* 0xBF8831C4 */
  .extern DCH1CPTRSET      /* 0xBF8831C8 */
  .extern DCH1CPTRINV      /* 0xBF8831CC */
  .extern DCH1DAT          /* 0xBF8831D0 */
  .extern DCH1DATCLR       /* 0xBF8831D4 */
  .extern DCH1DATSET       /* 0xBF8831D8 */
  .extern DCH1DATINV       /* 0xBF8831DC */
  .extern DCH2CON          /* 0xBF8831E0 */
  .extern DCH2CONCLR       /* 0xBF8831E4 */
  .extern DCH2CONSET       /* 0xBF8831E8 */
  .extern DCH2CONINV       /* 0xBF8831EC */
  .extern DCH2ECON         /* 0xBF8831F0 */
  .extern DCH2ECONCLR      /* 0xBF8831F4 */
  .extern DCH2ECONSET      /* 0xBF8831F8 */
  .extern DCH2ECONINV      /* 0xBF8831FC */
  .extern DCH2INT          /* 0xBF883200 */
  .extern DCH2INTCLR       /* 0xBF883204 */
  .extern DCH2INTSET       /* 0xBF883208 */
  .extern DCH2INTINV       /* 0xBF88320C */
  .extern DCH2SSA          /* 0xBF883210 */
  .extern DCH2SSACLR       /* 0xBF883214 */
  .extern DCH2SSASET       /* 0xBF883218 */
  .extern DCH2SSAINV       /* 0xBF88321C */
  .extern DCH2DSA          /* 0xBF883220 */
  .extern DCH2DSACLR       /* 0xBF883224 */
  .extern DCH2DSASET       /* 0xBF883228 */
  .extern DCH2DSAINV       /* 0xBF88322C */
  .extern DCH2SSIZ         /* 0xBF883230 */
  .extern DCH2SSIZCLR      /* 0xBF883234 */
  .extern DCH2SSIZSET      /* 0xBF883238 */
  .extern DCH2SSIZINV      /* 0xBF88323C */
  .extern DCH2DSIZ         /* 0xBF883240 */
  .extern DCH2DSIZCLR      /* 0xBF883244 */
  .extern DCH2DSIZSET      /* 0xBF883248 */
  .extern DCH2DSIZINV      /* 0xBF88324C */
  .extern DCH2SPTR         /* 0xBF883250 */
  .extern DCH2SPTRCLR      /* 0xBF883254 */
  .extern DCH2SPTRSET      /* 0xBF883258 */
  .extern DCH2SPTRINV      /* 0xBF88325C */
  .extern DCH2DPTR         /* 0xBF883260 */
  .extern DCH2DPTRCLR      /* 0xBF883264 */
  .extern DCH2DPTRSET      /* 0xBF883268 */
  .extern DCH2DPTRINV      /* 0xBF88326C */
  .extern DCH2CSIZ         /* 0xBF883270 */
  .extern DCH2CSIZCLR      /* 0xBF883274 */
  .extern DCH2CSIZSET      /* 0xBF883278 */
  .extern DCH2CSIZINV      /* 0xBF88327C */
  .extern DCH2CPTR         /* 0xBF883280 */
  .extern DCH2CPTRCLR      /* 0xBF883284 */
  .extern DCH2CPTRSET      /* 0xBF883288 */
  .extern DCH2CPTRINV      /* 0xBF88328C */
  .extern DCH2DAT          /* 0xBF883290 */
  .extern DCH2DATCLR       /* 0xBF883294 */
  .extern DCH2DATSET       /* 0xBF883298 */
  .extern DCH2DATINV       /* 0xBF88329C */
  .extern DCH3CON          /* 0xBF8832A0 */
  .extern DCH3CONCLR       /* 0xBF8832A4 */
  .extern DCH3CONSET       /* 0xBF8832A8 */
  .extern DCH3CONINV       /* 0xBF8832AC */
  .extern DCH3ECON         /* 0xBF8832B0 */
  .extern DCH3ECONCLR      /* 0xBF8832B4 */
  .extern DCH3ECONSET      /* 0xBF8832B8 */
  .extern DCH3ECONINV      /* 0xBF8832BC */
  .extern DCH3INT          /* 0xBF8832C0 */
  .extern DCH3INTCLR       /* 0xBF8832C4 */
  .extern DCH3INTSET       /* 0xBF8832C8 */
  .extern DCH3INTINV       /* 0xBF8832CC */
  .extern DCH3SSA          /* 0xBF8832D0 */
  .extern DCH3SSACLR       /* 0xBF8832D4 */
  .extern DCH3SSASET       /* 0xBF8832D8 */
  .extern DCH3SSAINV       /* 0xBF8832DC */
  .extern DCH3DSA          /* 0xBF8832E0 */
  .extern DCH3DSACLR       /* 0xBF8832E4 */
  .extern DCH3DSASET       /* 0xBF8832E8 */
  .extern DCH3DSAINV       /* 0xBF8832EC */
  .extern DCH3SSIZ         /* 0xBF8832F0 */
  .extern DCH3SSIZCLR      /* 0xBF8832F4 */
  .extern DCH3SSIZSET      /* 0xBF8832F8 */
  .extern DCH3SSIZINV      /* 0xBF8832FC */
  .extern DCH3DSIZ         /* 0xBF883300 */
  .extern DCH3DSIZCLR      /* 0xBF883304 */
  .extern DCH3DSIZSET      /* 0xBF883308 */
  .extern DCH3DSIZINV      /* 0xBF88330C */
  .extern DCH3SPTR         /* 0xBF883310 */
  .extern DCH3SPTRCLR      /* 0xBF883314 */
  .extern DCH3SPTRSET      /* 0xBF883318 */
  .extern DCH3SPTRINV      /* 0xBF88331C */
  .extern DCH3DPTR         /* 0xBF883320 */
  .extern DCH3DPTRCLR      /* 0xBF883324 */
  .extern DCH3DPTRSET      /* 0xBF883328 */
  .extern DCH3DPTRINV      /* 0xBF88332C */
  .extern DCH3CSIZ         /* 0xBF883330 */
  .extern DCH3CSIZCLR      /* 0xBF883334 */
  .extern DCH3CSIZSET      /* 0xBF883338 */
  .extern DCH3CSIZINV      /* 0xBF88333C */
  .extern DCH3CPTR         /* 0xBF883340 */
  .extern DCH3CPTRCLR      /* 0xBF883344 */
  .extern DCH3CPTRSET      /* 0xBF883348 */
  .extern DCH3CPTRINV      /* 0xBF88334C */
  .extern DCH3DAT          /* 0xBF883350 */
  .extern DCH3DATCLR       /* 0xBF883354 */
  .extern DCH3DATSET       /* 0xBF883358 */
  .extern DCH3DATINV       /* 0xBF88335C */
  .extern DCH4CON          /* 0xBF883360 */
  .extern DCH4CONCLR       /* 0xBF883364 */
  .extern DCH4CONSET       /* 0xBF883368 */
  .extern DCH4CONINV       /* 0xBF88336C */
  .extern DCH4ECON         /* 0xBF883370 */
  .extern DCH4ECONCLR      /* 0xBF883374 */
  .extern DCH4ECONSET      /* 0xBF883378 */
  .extern DCH4ECONINV      /* 0xBF88337C */
  .extern DCH4INT          /* 0xBF883380 */
  .extern DCH4INTCLR       /* 0xBF883384 */
  .extern DCH4INTSET       /* 0xBF883388 */
  .extern DCH4INTINV       /* 0xBF88338C */
  .extern DCH4SSA          /* 0xBF883390 */
  .extern DCH4SSACLR       /* 0xBF883394 */
  .extern DCH4SSASET       /* 0xBF883398 */
  .extern DCH4SSAINV       /* 0xBF88339C */
  .extern DCH4DSA          /* 0xBF8833A0 */
  .extern DCH4DSACLR       /* 0xBF8833A4 */
  .extern DCH4DSASET       /* 0xBF8833A8 */
  .extern DCH4DSAINV       /* 0xBF8833AC */
  .extern DCH4SSIZ         /* 0xBF8833B0 */
  .extern DCH4SSIZCLR      /* 0xBF8833B4 */
  .extern DCH4SSIZSET      /* 0xBF8833B8 */
  .extern DCH4SSIZINV      /* 0xBF8833BC */
  .extern DCH4DSIZ         /* 0xBF8833C0 */
  .extern DCH4DSIZCLR      /* 0xBF8833C4 */
  .extern DCH4DSIZSET      /* 0xBF8833C8 */
  .extern DCH4DSIZINV      /* 0xBF8833CC */
  .extern DCH4SPTR         /* 0xBF8833D0 */
  .extern DCH4SPTRCLR      /* 0xBF8833D4 */
  .extern DCH4SPTRSET      /* 0xBF8833D8 */
  .extern DCH4SPTRINV      /* 0xBF8833DC */
  .extern DCH4DPTR         /* 0xBF8833E0 */
  .extern DCH4DPTRCLR      /* 0xBF8833E4 */
  .extern DCH4DPTRSET      /* 0xBF8833E8 */
  .extern DCH4DPTRINV      /* 0xBF8833EC */
  .extern DCH4CSIZ         /* 0xBF8833F0 */
  .extern DCH4CSIZCLR      /* 0xBF8833F4 */
  .extern DCH4CSIZSET      /* 0xBF8833F8 */
  .extern DCH4CSIZINV      /* 0xBF8833FC */
  .extern DCH4CPTR         /* 0xBF883400 */
  .extern DCH4CPTRCLR      /* 0xBF883404 */
  .extern DCH4CPTRSET      /* 0xBF883408 */
  .extern DCH4CPTRINV      /* 0xBF88340C */
  .extern DCH4DAT          /* 0xBF883410 */
  .extern DCH4DATCLR       /* 0xBF883414 */
  .extern DCH4DATSET       /* 0xBF883418 */
  .extern DCH4DATINV       /* 0xBF88341C */
  .extern DCH5CON          /* 0xBF883420 */
  .extern DCH5CONCLR       /* 0xBF883424 */
  .extern DCH5CONSET       /* 0xBF883428 */
  .extern DCH5CONINV       /* 0xBF88342C */
  .extern DCH5ECON         /* 0xBF883430 */
  .extern DCH5ECONCLR      /* 0xBF883434 */
  .extern DCH5ECONSET      /* 0xBF883438 */
  .extern DCH5ECONINV      /* 0xBF88343C */
  .extern DCH5INT          /* 0xBF883440 */
  .extern DCH5INTCLR       /* 0xBF883444 */
  .extern DCH5INTSET       /* 0xBF883448 */
  .extern DCH5INTINV       /* 0xBF88344C */
  .extern DCH5SSA          /* 0xBF883450 */
  .extern DCH5SSACLR       /* 0xBF883454 */
  .extern DCH5SSASET       /* 0xBF883458 */
  .extern DCH5SSAINV       /* 0xBF88345C */
  .extern DCH5DSA          /* 0xBF883460 */
  .extern DCH5DSACLR       /* 0xBF883464 */
  .extern DCH5DSASET       /* 0xBF883468 */
  .extern DCH5DSAINV       /* 0xBF88346C */
  .extern DCH5SSIZ         /* 0xBF883470 */
  .extern DCH5SSIZCLR      /* 0xBF883474 */
  .extern DCH5SSIZSET      /* 0xBF883478 */
  .extern DCH5SSIZINV      /* 0xBF88347C */
  .extern DCH5DSIZ         /* 0xBF883480 */
  .extern DCH5DSIZCLR      /* 0xBF883484 */
  .extern DCH5DSIZSET      /* 0xBF883488 */
  .extern DCH5DSIZINV      /* 0xBF88348C */
  .extern DCH5SPTR         /* 0xBF883490 */
  .extern DCH5SPTRCLR      /* 0xBF883494 */
  .extern DCH5SPTRSET      /* 0xBF883498 */
  .extern DCH5SPTRINV      /* 0xBF88349C */
  .extern DCH5DPTR         /* 0xBF8834A0 */
  .extern DCH5DPTRCLR      /* 0xBF8834A4 */
  .extern DCH5DPTRSET      /* 0xBF8834A8 */
  .extern DCH5DPTRINV      /* 0xBF8834AC */
  .extern DCH5CSIZ         /* 0xBF8834B0 */
  .extern DCH5CSIZCLR      /* 0xBF8834B4 */
  .extern DCH5CSIZSET      /* 0xBF8834B8 */
  .extern DCH5CSIZINV      /* 0xBF8834BC */
  .extern DCH5CPTR         /* 0xBF8834C0 */
  .extern DCH5CPTRCLR      /* 0xBF8834C4 */
  .extern DCH5CPTRSET      /* 0xBF8834C8 */
  .extern DCH5CPTRINV      /* 0xBF8834CC */
  .extern DCH5DAT          /* 0xBF8834D0 */
  .extern DCH5DATCLR       /* 0xBF8834D4 */
  .extern DCH5DATSET       /* 0xBF8834D8 */
  .extern DCH5DATINV       /* 0xBF8834DC */
  .extern DCH6CON          /* 0xBF8834E0 */
  .extern DCH6CONCLR       /* 0xBF8834E4 */
  .extern DCH6CONSET       /* 0xBF8834E8 */
  .extern DCH6CONINV       /* 0xBF8834EC */
  .extern DCH6ECON         /* 0xBF8834F0 */
  .extern DCH6ECONCLR      /* 0xBF8834F4 */
  .extern DCH6ECONSET      /* 0xBF8834F8 */
  .extern DCH6ECONINV      /* 0xBF8834FC */
  .extern DCH6INT          /* 0xBF883500 */
  .extern DCH6INTCLR       /* 0xBF883504 */
  .extern DCH6INTSET       /* 0xBF883508 */
  .extern DCH6INTINV       /* 0xBF88350C */
  .extern DCH6SSA          /* 0xBF883510 */
  .extern DCH6SSACLR       /* 0xBF883514 */
  .extern DCH6SSASET       /* 0xBF883518 */
  .extern DCH6SSAINV       /* 0xBF88351C */
  .extern DCH6DSA          /* 0xBF883520 */
  .extern DCH6DSACLR       /* 0xBF883524 */
  .extern DCH6DSASET       /* 0xBF883528 */
  .extern DCH6DSAINV       /* 0xBF88352C */
  .extern DCH6SSIZ         /* 0xBF883530 */
  .extern DCH6SSIZCLR      /* 0xBF883534 */
  .extern DCH6SSIZSET      /* 0xBF883538 */
  .extern DCH6SSIZINV      /* 0xBF88353C */
  .extern DCH6DSIZ         /* 0xBF883540 */
  .extern DCH6DSIZCLR      /* 0xBF883544 */
  .extern DCH6DSIZSET      /* 0xBF883548 */
  .extern DCH6DSIZINV      /* 0xBF88354C */
  .extern DCH6SPTR         /* 0xBF883550 */
  .extern DCH6SPTRCLR      /* 0xBF883554 */
  .extern DCH6SPTRSET      /* 0xBF883558 */
  .extern DCH6SPTRINV      /* 0xBF88355C */
  .extern DCH6DPTR         /* 0xBF883560 */
  .extern DCH6DPTRCLR      /* 0xBF883564 */
  .extern DCH6DPTRSET      /* 0xBF883568 */
  .extern DCH6DPTRINV      /* 0xBF88356C */
  .extern DCH6CSIZ         /* 0xBF883570 */
  .extern DCH6CSIZCLR      /* 0xBF883574 */
  .extern DCH6CSIZSET      /* 0xBF883578 */
  .extern DCH6CSIZINV      /* 0xBF88357C */
  .extern DCH6CPTR         /* 0xBF883580 */
  .extern DCH6CPTRCLR      /* 0xBF883584 */
  .extern DCH6CPTRSET      /* 0xBF883588 */
  .extern DCH6CPTRINV      /* 0xBF88358C */
  .extern DCH6DAT          /* 0xBF883590 */
  .extern DCH6DATCLR       /* 0xBF883594 */
  .extern DCH6DATSET       /* 0xBF883598 */
  .extern DCH6DATINV       /* 0xBF88359C */
  .extern DCH7CON          /* 0xBF8835A0 */
  .extern DCH7CONCLR       /* 0xBF8835A4 */
  .extern DCH7CONSET       /* 0xBF8835A8 */
  .extern DCH7CONINV       /* 0xBF8835AC */
  .extern DCH7ECON         /* 0xBF8835B0 */
  .extern DCH7ECONCLR      /* 0xBF8835B4 */
  .extern DCH7ECONSET      /* 0xBF8835B8 */
  .extern DCH7ECONINV      /* 0xBF8835BC */
  .extern DCH7INT          /* 0xBF8835C0 */
  .extern DCH7INTCLR       /* 0xBF8835C4 */
  .extern DCH7INTSET       /* 0xBF8835C8 */
  .extern DCH7INTINV       /* 0xBF8835CC */
  .extern DCH7SSA          /* 0xBF8835D0 */
  .extern DCH7SSACLR       /* 0xBF8835D4 */
  .extern DCH7SSASET       /* 0xBF8835D8 */
  .extern DCH7SSAINV       /* 0xBF8835DC */
  .extern DCH7DSA          /* 0xBF8835E0 */
  .extern DCH7DSACLR       /* 0xBF8835E4 */
  .extern DCH7DSASET       /* 0xBF8835E8 */
  .extern DCH7DSAINV       /* 0xBF8835EC */
  .extern DCH7SSIZ         /* 0xBF8835F0 */
  .extern DCH7SSIZCLR      /* 0xBF8835F4 */
  .extern DCH7SSIZSET      /* 0xBF8835F8 */
  .extern DCH7SSIZINV      /* 0xBF8835FC */
  .extern DCH7DSIZ         /* 0xBF883600 */
  .extern DCH7DSIZCLR      /* 0xBF883604 */
  .extern DCH7DSIZSET      /* 0xBF883608 */
  .extern DCH7DSIZINV      /* 0xBF88360C */
  .extern DCH7SPTR         /* 0xBF883610 */
  .extern DCH7SPTRCLR      /* 0xBF883614 */
  .extern DCH7SPTRSET      /* 0xBF883618 */
  .extern DCH7SPTRINV      /* 0xBF88361C */
  .extern DCH7DPTR         /* 0xBF883620 */
  .extern DCH7DPTRCLR      /* 0xBF883624 */
  .extern DCH7DPTRSET      /* 0xBF883628 */
  .extern DCH7DPTRINV      /* 0xBF88362C */
  .extern DCH7CSIZ         /* 0xBF883630 */
  .extern DCH7CSIZCLR      /* 0xBF883634 */
  .extern DCH7CSIZSET      /* 0xBF883638 */
  .extern DCH7CSIZINV      /* 0xBF88363C */
  .extern DCH7CPTR         /* 0xBF883640 */
  .extern DCH7CPTRCLR      /* 0xBF883644 */
  .extern DCH7CPTRSET      /* 0xBF883648 */
  .extern DCH7CPTRINV      /* 0xBF88364C */
  .extern DCH7DAT          /* 0xBF883650 */
  .extern DCH7DATCLR       /* 0xBF883654 */
  .extern DCH7DATSET       /* 0xBF883658 */
  .extern DCH7DATINV       /* 0xBF88365C */
  .extern CHECON           /* 0xBF884000 */
  .extern CHECONCLR        /* 0xBF884004 */
  .extern CHECONSET        /* 0xBF884008 */
  .extern CHECONINV        /* 0xBF88400C */
  .extern CHEACC           /* 0xBF884010 */
  .extern CHEACCCLR        /* 0xBF884014 */
  .extern CHEACCSET        /* 0xBF884018 */
  .extern CHEACCINV        /* 0xBF88401C */
  .extern CHETAG           /* 0xBF884020 */
  .extern CHETAGCLR        /* 0xBF884024 */
  .extern CHETAGSET        /* 0xBF884028 */
  .extern CHETAGINV        /* 0xBF88402C */
  .extern CHEMSK           /* 0xBF884030 */
  .extern CHEMSKCLR        /* 0xBF884034 */
  .extern CHEMSKSET        /* 0xBF884038 */
  .extern CHEMSKINV        /* 0xBF88403C */
  .extern CHEW0            /* 0xBF884040 */
  .extern CHEW1            /* 0xBF884050 */
  .extern CHEW2            /* 0xBF884060 */
  .extern CHEW3            /* 0xBF884070 */
  .extern CHELRU           /* 0xBF884080 */
  .extern CHEHIT           /* 0xBF884090 */
  .extern CHEMIS           /* 0xBF8840A0 */
  .extern CHEPFABT         /* 0xBF8840C0 */
  .extern U1OTGIR          /* 0xBF885040 */
  .extern U1OTGIRCLR       /* 0xBF885044 */
  .extern U1OTGIE          /* 0xBF885050 */
  .extern U1OTGIECLR       /* 0xBF885054 */
  .extern U1OTGIESET       /* 0xBF885058 */
  .extern U1OTGIEINV       /* 0xBF88505C */
  .extern U1OTGSTAT        /* 0xBF885060 */
  .extern U1OTGCON         /* 0xBF885070 */
  .extern U1OTGCONCLR      /* 0xBF885074 */
  .extern U1OTGCONSET      /* 0xBF885078 */
  .extern U1OTGCONINV      /* 0xBF88507C */
  .extern U1PWRC           /* 0xBF885080 */
  .extern U1PWRCCLR        /* 0xBF885084 */
  .extern U1PWRCSET        /* 0xBF885088 */
  .extern U1PWRCINV        /* 0xBF88508C */
  .extern U1IR             /* 0xBF885200 */
  .extern U1IRCLR          /* 0xBF885204 */
  .extern U1IE             /* 0xBF885210 */
  .extern U1IECLR          /* 0xBF885214 */
  .extern U1IESET          /* 0xBF885218 */
  .extern U1IEINV          /* 0xBF88521C */
  .extern U1EIR            /* 0xBF885220 */
  .extern U1EIRCLR         /* 0xBF885224 */
  .extern U1EIE            /* 0xBF885230 */
  .extern U1EIECLR         /* 0xBF885234 */
  .extern U1EIESET         /* 0xBF885238 */
  .extern U1EIEINV         /* 0xBF88523C */
  .extern U1STAT           /* 0xBF885240 */
  .extern U1STATCLR        /* 0xBF885244 */
  .extern U1STATSET        /* 0xBF885248 */
  .extern U1STATINV        /* 0xBF88524C */
  .extern U1CON            /* 0xBF885250 */
  .extern U1CONCLR         /* 0xBF885254 */
  .extern U1CONSET         /* 0xBF885258 */
  .extern U1CONINV         /* 0xBF88525C */
  .extern U1ADDR           /* 0xBF885260 */
  .extern U1ADDRCLR        /* 0xBF885264 */
  .extern U1ADDRSET        /* 0xBF885268 */
  .extern U1ADDRINV        /* 0xBF88526C */
  .extern U1BDTP1          /* 0xBF885270 */
  .extern U1BDTP1CLR       /* 0xBF885274 */
  .extern U1BDTP1SET       /* 0xBF885278 */
  .extern U1BDTP1INV       /* 0xBF88527C */
  .extern U1FRML           /* 0xBF885280 */
  .extern U1FRMLCLR        /* 0xBF885284 */
  .extern U1FRMLSET        /* 0xBF885288 */
  .extern U1FRMLINV        /* 0xBF88528C */
  .extern U1FRMH           /* 0xBF885290 */
  .extern U1FRMHCLR        /* 0xBF885294 */
  .extern U1FRMHSET        /* 0xBF885298 */
  .extern U1FRMHINV        /* 0xBF88529C */
  .extern U1TOK            /* 0xBF8852A0 */
  .extern U1TOKCLR         /* 0xBF8852A4 */
  .extern U1TOKSET         /* 0xBF8852A8 */
  .extern U1TOKINV         /* 0xBF8852AC */
  .extern U1SOF            /* 0xBF8852B0 */
  .extern U1SOFCLR         /* 0xBF8852B4 */
  .extern U1SOFSET         /* 0xBF8852B8 */
  .extern U1SOFINV         /* 0xBF8852BC */
  .extern U1BDTP2          /* 0xBF8852C0 */
  .extern U1BDTP2CLR       /* 0xBF8852C4 */
  .extern U1BDTP2SET       /* 0xBF8852C8 */
  .extern U1BDTP2INV       /* 0xBF8852CC */
  .extern U1BDTP3          /* 0xBF8852D0 */
  .extern U1BDTP3CLR       /* 0xBF8852D4 */
  .extern U1BDTP3SET       /* 0xBF8852D8 */
  .extern U1BDTP3INV       /* 0xBF8852DC */
  .extern U1CNFG1          /* 0xBF8852E0 */
  .extern U1CNFG1CLR       /* 0xBF8852E4 */
  .extern U1CNFG1SET       /* 0xBF8852E8 */
  .extern U1CNFG1INV       /* 0xBF8852EC */
  .extern U1EP0            /* 0xBF885300 */
  .extern U1EP0CLR         /* 0xBF885304 */
  .extern U1EP0SET         /* 0xBF885308 */
  .extern U1EP0INV         /* 0xBF88530C */
  .extern U1EP1            /* 0xBF885310 */
  .extern U1EP1CLR         /* 0xBF885314 */
  .extern U1EP1SET         /* 0xBF885318 */
  .extern U1EP1INV         /* 0xBF88531C */
  .extern U1EP2            /* 0xBF885320 */
  .extern U1EP2CLR         /* 0xBF885324 */
  .extern U1EP2SET         /* 0xBF885328 */
  .extern U1EP2INV         /* 0xBF88532C */
  .extern U1EP3            /* 0xBF885330 */
  .extern U1EP3CLR         /* 0xBF885334 */
  .extern U1EP3SET         /* 0xBF885338 */
  .extern U1EP3INV         /* 0xBF88533C */
  .extern U1EP4            /* 0xBF885340 */
  .extern U1EP4CLR         /* 0xBF885344 */
  .extern U1EP4SET         /* 0xBF885348 */
  .extern U1EP4INV         /* 0xBF88534C */
  .extern U1EP5            /* 0xBF885350 */
  .extern U1EP5CLR         /* 0xBF885354 */
  .extern U1EP5SET         /* 0xBF885358 */
  .extern U1EP5INV         /* 0xBF88535C */
  .extern U1EP6            /* 0xBF885360 */
  .extern U1EP6CLR         /* 0xBF885364 */
  .extern U1EP6SET         /* 0xBF885368 */
  .extern U1EP6INV         /* 0xBF88536C */
  .extern U1EP7            /* 0xBF885370 */
  .extern U1EP7CLR         /* 0xBF885374 */
  .extern U1EP7SET         /* 0xBF885378 */
  .extern U1EP7INV         /* 0xBF88537C */
  .extern U1EP8            /* 0xBF885380 */
  .extern U1EP8CLR         /* 0xBF885384 */
  .extern U1EP8SET         /* 0xBF885388 */
  .extern U1EP8INV         /* 0xBF88538C */
  .extern U1EP9            /* 0xBF885390 */
  .extern U1EP9CLR         /* 0xBF885394 */
  .extern U1EP9SET         /* 0xBF885398 */
  .extern U1EP9INV         /* 0xBF88539C */
  .extern U1EP10           /* 0xBF8853A0 */
  .extern U1EP10CLR        /* 0xBF8853A4 */
  .extern U1EP10SET        /* 0xBF8853A8 */
  .extern U1EP10INV        /* 0xBF8853AC */
  .extern U1EP11           /* 0xBF8853B0 */
  .extern U1EP11CLR        /* 0xBF8853B4 */
  .extern U1EP11SET        /* 0xBF8853B8 */
  .extern U1EP11INV        /* 0xBF8853BC */
  .extern U1EP12           /* 0xBF8853C0 */
  .extern U1EP12CLR        /* 0xBF8853C4 */
  .extern U1EP12SET        /* 0xBF8853C8 */
  .extern U1EP12INV        /* 0xBF8853CC */
  .extern U1EP13           /* 0xBF8853D0 */
  .extern U1EP13CLR        /* 0xBF8853D4 */
  .extern U1EP13SET        /* 0xBF8853D8 */
  .extern U1EP13INV        /* 0xBF8853DC */
  .extern U1EP14           /* 0xBF8853E0 */
  .extern U1EP14CLR        /* 0xBF8853E4 */
  .extern U1EP14SET        /* 0xBF8853E8 */
  .extern U1EP14INV        /* 0xBF8853EC */
  .extern U1EP15           /* 0xBF8853F0 */
  .extern U1EP15CLR        /* 0xBF8853F4 */
  .extern U1EP15SET        /* 0xBF8853F8 */
  .extern U1EP15INV        /* 0xBF8853FC */
  .extern TRISB            /* 0xBF886040 */
  .extern TRISBCLR         /* 0xBF886044 */
  .extern TRISBSET         /* 0xBF886048 */
  .extern TRISBINV         /* 0xBF88604C */
  .extern PORTB            /* 0xBF886050 */
  .extern PORTBCLR         /* 0xBF886054 */
  .extern PORTBSET         /* 0xBF886058 */
  .extern PORTBINV         /* 0xBF88605C */
  .extern LATB             /* 0xBF886060 */
  .extern LATBCLR          /* 0xBF886064 */
  .extern LATBSET          /* 0xBF886068 */
  .extern LATBINV          /* 0xBF88606C */
  .extern ODCB             /* 0xBF886070 */
  .extern ODCBCLR          /* 0xBF886074 */
  .extern ODCBSET          /* 0xBF886078 */
  .extern ODCBINV          /* 0xBF88607C */
  .extern TRISC            /* 0xBF886080 */
  .extern TRISCCLR         /* 0xBF886084 */
  .extern TRISCSET         /* 0xBF886088 */
  .extern TRISCINV         /* 0xBF88608C */
  .extern PORTC            /* 0xBF886090 */
  .extern PORTCCLR         /* 0xBF886094 */
  .extern PORTCSET         /* 0xBF886098 */
  .extern PORTCINV         /* 0xBF88609C */
  .extern LATC             /* 0xBF8860A0 */
  .extern LATCCLR          /* 0xBF8860A4 */
  .extern LATCSET          /* 0xBF8860A8 */
  .extern LATCINV          /* 0xBF8860AC */
  .extern ODCC             /* 0xBF8860B0 */
  .extern ODCCCLR          /* 0xBF8860B4 */
  .extern ODCCSET          /* 0xBF8860B8 */
  .extern ODCCINV          /* 0xBF8860BC */
  .extern TRISD            /* 0xBF8860C0 */
  .extern TRISDCLR         /* 0xBF8860C4 */
  .extern TRISDSET         /* 0xBF8860C8 */
  .extern TRISDINV         /* 0xBF8860CC */
  .extern PORTD            /* 0xBF8860D0 */
  .extern PORTDCLR         /* 0xBF8860D4 */
  .extern PORTDSET         /* 0xBF8860D8 */
  .extern PORTDINV         /* 0xBF8860DC */
  .extern LATD             /* 0xBF8860E0 */
  .extern LATDCLR          /* 0xBF8860E4 */
  .extern LATDSET          /* 0xBF8860E8 */
  .extern LATDINV          /* 0xBF8860EC */
  .extern ODCD             /* 0xBF8860F0 */
  .extern ODCDCLR          /* 0xBF8860F4 */
  .extern ODCDSET          /* 0xBF8860F8 */
  .extern ODCDINV          /* 0xBF8860FC */
  .extern TRISE            /* 0xBF886100 */
  .extern TRISECLR         /* 0xBF886104 */
  .extern TRISESET         /* 0xBF886108 */
  .extern TRISEINV         /* 0xBF88610C */
  .extern PORTE            /* 0xBF886110 */
  .extern PORTECLR         /* 0xBF886114 */
  .extern PORTESET         /* 0xBF886118 */
  .extern PORTEINV         /* 0xBF88611C */
  .extern LATE             /* 0xBF886120 */
  .extern LATECLR          /* 0xBF886124 */
  .extern LATESET          /* 0xBF886128 */
  .extern LATEINV          /* 0xBF88612C */
  .extern ODCE             /* 0xBF886130 */
  .extern ODCECLR          /* 0xBF886134 */
  .extern ODCESET          /* 0xBF886138 */
  .extern ODCEINV          /* 0xBF88613C */
  .extern TRISF            /* 0xBF886140 */
  .extern TRISFCLR         /* 0xBF886144 */
  .extern TRISFSET         /* 0xBF886148 */
  .extern TRISFINV         /* 0xBF88614C */
  .extern PORTF            /* 0xBF886150 */
  .extern PORTFCLR         /* 0xBF886154 */
  .extern PORTFSET         /* 0xBF886158 */
  .extern PORTFINV         /* 0xBF88615C */
  .extern LATF             /* 0xBF886160 */
  .extern LATFCLR          /* 0xBF886164 */
  .extern LATFSET          /* 0xBF886168 */
  .extern LATFINV          /* 0xBF88616C */
  .extern ODCF             /* 0xBF886170 */
  .extern ODCFCLR          /* 0xBF886174 */
  .extern ODCFSET          /* 0xBF886178 */
  .extern ODCFINV          /* 0xBF88617C */
  .extern TRISG            /* 0xBF886180 */
  .extern TRISGCLR         /* 0xBF886184 */
  .extern TRISGSET         /* 0xBF886188 */
  .extern TRISGINV         /* 0xBF88618C */
  .extern PORTG            /* 0xBF886190 */
  .extern PORTGCLR         /* 0xBF886194 */
  .extern PORTGSET         /* 0xBF886198 */
  .extern PORTGINV         /* 0xBF88619C */
  .extern LATG             /* 0xBF8861A0 */
  .extern LATGCLR          /* 0xBF8861A4 */
  .extern LATGSET          /* 0xBF8861A8 */
  .extern LATGINV          /* 0xBF8861AC */
  .extern ODCG             /* 0xBF8861B0 */
  .extern ODCGCLR          /* 0xBF8861B4 */
  .extern ODCGSET          /* 0xBF8861B8 */
  .extern ODCGINV          /* 0xBF8861BC */
  .extern CNCON            /* 0xBF8861C0 */
  .extern CNCONCLR         /* 0xBF8861C4 */
  .extern CNCONSET         /* 0xBF8861C8 */
  .extern CNCONINV         /* 0xBF8861CC */
  .extern CNEN             /* 0xBF8861D0 */
  .extern CNENCLR          /* 0xBF8861D4 */
  .extern CNENSET          /* 0xBF8861D8 */
  .extern CNENINV          /* 0xBF8861DC */
  .extern CNPUE            /* 0xBF8861E0 */
  .extern CNPUECLR         /* 0xBF8861E4 */
  .extern CNPUESET         /* 0xBF8861E8 */
  .extern CNPUEINV         /* 0xBF8861EC */
  .extern ETHCON1          /* 0xBF889000 */
  .extern ETHCON1CLR       /* 0xBF889004 */
  .extern ETHCON1SET       /* 0xBF889008 */
  .extern ETHCON1INV       /* 0xBF88900C */
  .extern ETHCON2          /* 0xBF889010 */
  .extern ETHCON2CLR       /* 0xBF889014 */
  .extern ETHCON2SET       /* 0xBF889018 */
  .extern ETHCON2INV       /* 0xBF88901C */
  .extern ETHTXST          /* 0xBF889020 */
  .extern ETHTXSTCLR       /* 0xBF889024 */
  .extern ETHTXSTSET       /* 0xBF889028 */
  .extern ETHTXSTINV       /* 0xBF88902C */
  .extern ETHRXST          /* 0xBF889030 */
  .extern ETHRXSTCLR       /* 0xBF889034 */
  .extern ETHRXSTSET       /* 0xBF889038 */
  .extern ETHRXSTINV       /* 0xBF88903C */
  .extern ETHHT0           /* 0xBF889040 */
  .extern ETHHT0CLR        /* 0xBF889044 */
  .extern ETHHT0SET        /* 0xBF889048 */
  .extern ETHHT0INV        /* 0xBF88904C */
  .extern ETHHT1           /* 0xBF889050 */
  .extern ETHHT1CLR        /* 0xBF889054 */
  .extern ETHHT1SET        /* 0xBF889058 */
  .extern ETHHT1INV        /* 0xBF88905C */
  .extern ETHPMM0          /* 0xBF889060 */
  .extern ETHPMM0CLR       /* 0xBF889064 */
  .extern ETHPMM0SET       /* 0xBF889068 */
  .extern ETHPMM0INV       /* 0xBF88906C */
  .extern ETHPMM1          /* 0xBF889070 */
  .extern ETHPMM1CLR       /* 0xBF889074 */
  .extern ETHPMM1SET       /* 0xBF889078 */
  .extern ETHPMM1INV       /* 0xBF88907C */
  .extern ETHPMCS          /* 0xBF889080 */
  .extern ETHPMCSCLR       /* 0xBF889084 */
  .extern ETHPMCSSET       /* 0xBF889088 */
  .extern ETHPMCSINV       /* 0xBF88908C */
  .extern ETHPMO           /* 0xBF889090 */
  .extern ETHPMOCLR        /* 0xBF889094 */
  .extern ETHPMOSET        /* 0xBF889098 */
  .extern ETHPMOINV        /* 0xBF88909C */
  .extern ETHRXFC          /* 0xBF8890A0 */
  .extern ETHRXFCCLR       /* 0xBF8890A4 */
  .extern ETHRXFCSET       /* 0xBF8890A8 */
  .extern ETHRXFCINV       /* 0xBF8890AC */
  .extern ETHRXWM          /* 0xBF8890B0 */
  .extern ETHRXWMCLR       /* 0xBF8890B4 */
  .extern ETHRXWMSET       /* 0xBF8890B8 */
  .extern ETHRXWMINV       /* 0xBF8890BC */
  .extern ETHIEN           /* 0xBF8890C0 */
  .extern ETHIENCLR        /* 0xBF8890C4 */
  .extern ETHIENSET        /* 0xBF8890C8 */
  .extern ETHIENINV        /* 0xBF8890CC */
  .extern ETHIRQ           /* 0xBF8890D0 */
  .extern ETHIRQCLR        /* 0xBF8890D4 */
  .extern ETHIRQSET        /* 0xBF8890D8 */
  .extern ETHIRQINV        /* 0xBF8890DC */
  .extern ETHSTAT          /* 0xBF8890E0 */
  .extern ETHSTATCLR       /* 0xBF8890E4 */
  .extern ETHSTATSET       /* 0xBF8890E8 */
  .extern ETHSTATINV       /* 0xBF8890EC */
  .extern ETHRXOVFLOW      /* 0xBF889100 */
  .extern ETHRXOVFLOWCLR   /* 0xBF889104 */
  .extern ETHRXOVFLOWSET   /* 0xBF889108 */
  .extern ETHRXOVFLOWINV   /* 0xBF88910C */
  .extern ETHFRMTXOK       /* 0xBF889110 */
  .extern ETHFRMTXOKCLR    /* 0xBF889114 */
  .extern ETHFRMTXOKSET    /* 0xBF889118 */
  .extern ETHFRMTXOKINV    /* 0xBF88911C */
  .extern ETHSCOLFRM       /* 0xBF889120 */
  .extern ETHSCOLFRMCLR    /* 0xBF889124 */
  .extern ETHSCOLFRMSET    /* 0xBF889128 */
  .extern ETHSCOLFRMINV    /* 0xBF88912C */
  .extern ETHMCOLFRM       /* 0xBF889130 */
  .extern ETHMCOLFRMCLR    /* 0xBF889134 */
  .extern ETHMCOLFRMSET    /* 0xBF889138 */
  .extern ETHMCOLFRMINV    /* 0xBF88913C */
  .extern ETHFRMRXOK       /* 0xBF889140 */
  .extern ETHFRMRXOKCLR    /* 0xBF889144 */
  .extern ETHFRMRXOKSET    /* 0xBF889148 */
  .extern ETHFRMRXOKINV    /* 0xBF88914C */
  .extern ETHFCSERR        /* 0xBF889150 */
  .extern ETHFCSERRCLR     /* 0xBF889154 */
  .extern ETHFCSERRSET     /* 0xBF889158 */
  .extern ETHFCSERRINV     /* 0xBF88915C */
  .extern ETHALGNERR       /* 0xBF889160 */
  .extern ETHALGNERRCLR    /* 0xBF889164 */
  .extern ETHALGNERRSET    /* 0xBF889168 */
  .extern ETHALGNERRINV    /* 0xBF88916C */
  .extern EMAC1CFG1        /* 0xBF889200 */
  .extern EMACxCFG1        /* 0xBF889200 */
  .extern EMAC1CFG1CLR     /* 0xBF889204 */
  .extern EMACxCFG1CLR     /* 0xBF889204 */
  .extern EMAC1CFG1SET     /* 0xBF889208 */
  .extern EMACxCFG1SET     /* 0xBF889208 */
  .extern EMAC1CFG1INV     /* 0xBF88920C */
  .extern EMACxCFG1INV     /* 0xBF88920C */
  .extern EMAC1CFG2        /* 0xBF889210 */
  .extern EMACxCFG2        /* 0xBF889210 */
  .extern EMAC1CFG2CLR     /* 0xBF889214 */
  .extern EMACxCFG2CLR     /* 0xBF889214 */
  .extern EMAC1CFG2SET     /* 0xBF889218 */
  .extern EMACxCFG2SET     /* 0xBF889218 */
  .extern EMAC1CFG2INV     /* 0xBF88921C */
  .extern EMACxCFG2INV     /* 0xBF88921C */
  .extern EMAC1IPGT        /* 0xBF889220 */
  .extern EMACxIPGT        /* 0xBF889220 */
  .extern EMAC1IPGTCLR     /* 0xBF889224 */
  .extern EMACxIPGTCLR     /* 0xBF889224 */
  .extern EMAC1IPGTSET     /* 0xBF889228 */
  .extern EMACxIPGTSET     /* 0xBF889228 */
  .extern EMAC1IPGTINV     /* 0xBF88922C */
  .extern EMACxIPGTINV     /* 0xBF88922C */
  .extern EMAC1IPGR        /* 0xBF889230 */
  .extern EMACxIPGR        /* 0xBF889230 */
  .extern EMAC1IPGRCLR     /* 0xBF889234 */
  .extern EMACxIPGRCLR     /* 0xBF889234 */
  .extern EMAC1IPGRSET     /* 0xBF889238 */
  .extern EMACxIPGRSET     /* 0xBF889238 */
  .extern EMAC1IPGRINV     /* 0xBF88923C */
  .extern EMACxIPGRINV     /* 0xBF88923C */
  .extern EMAC1CLRT        /* 0xBF889240 */
  .extern EMACxCLRT        /* 0xBF889240 */
  .extern EMAC1CLRTCLR     /* 0xBF889244 */
  .extern EMACxCLRTCLR     /* 0xBF889244 */
  .extern EMAC1CLRTSET     /* 0xBF889248 */
  .extern EMACxCLRTSET     /* 0xBF889248 */
  .extern EMAC1CLRTINV     /* 0xBF88924C */
  .extern EMACxCLRTINV     /* 0xBF88924C */
  .extern EMAC1MAXF        /* 0xBF889250 */
  .extern EMACxMAXF        /* 0xBF889250 */
  .extern EMAC1MAXFCLR     /* 0xBF889254 */
  .extern EMACxMAXFCLR     /* 0xBF889254 */
  .extern EMAC1MAXFSET     /* 0xBF889258 */
  .extern EMACxMAXFSET     /* 0xBF889258 */
  .extern EMAC1MAXFINV     /* 0xBF88925C */
  .extern EMACxMAXFINV     /* 0xBF88925C */
  .extern EMAC1SUPP        /* 0xBF889260 */
  .extern EMACxSUPP        /* 0xBF889260 */
  .extern EMAC1SUPPCLR     /* 0xBF889264 */
  .extern EMACxSUPPCLR     /* 0xBF889264 */
  .extern EMAC1SUPPSET     /* 0xBF889268 */
  .extern EMACxSUPPSET     /* 0xBF889268 */
  .extern EMAC1SUPPINV     /* 0xBF88926C */
  .extern EMACxSUPPINV     /* 0xBF88926C */
  .extern EMAC1TEST        /* 0xBF889270 */
  .extern EMACxTEST        /* 0xBF889270 */
  .extern EMAC1TESTCLR     /* 0xBF889274 */
  .extern EMACxTESTCLR     /* 0xBF889274 */
  .extern EMAC1TESTSET     /* 0xBF889278 */
  .extern EMACxTESTSET     /* 0xBF889278 */
  .extern EMAC1TESTINV     /* 0xBF88927C */
  .extern EMACxTESTINV     /* 0xBF88927C */
  .extern EMAC1MCFG        /* 0xBF889280 */
  .extern EMACxMCFG        /* 0xBF889280 */
  .extern EMAC1MCFGCLR     /* 0xBF889284 */
  .extern EMACxMCFGCLR     /* 0xBF889284 */
  .extern EMAC1MCFGSET     /* 0xBF889288 */
  .extern EMACxMCFGSET     /* 0xBF889288 */
  .extern EMAC1MCFGINV     /* 0xBF88928C */
  .extern EMACxMCFGINV     /* 0xBF88928C */
  .extern EMAC1MCMD        /* 0xBF889290 */
  .extern EMACxMCMD        /* 0xBF889290 */
  .extern EMAC1MCMDCLR     /* 0xBF889294 */
  .extern EMACxMCMDCLR     /* 0xBF889294 */
  .extern EMAC1MCMDSET     /* 0xBF889298 */
  .extern EMACxMCMDSET     /* 0xBF889298 */
  .extern EMAC1MCMDINV     /* 0xBF88929C */
  .extern EMACxMCMDINV     /* 0xBF88929C */
  .extern EMAC1MADR        /* 0xBF8892A0 */
  .extern EMACxMADR        /* 0xBF8892A0 */
  .extern EMAC1MADRCLR     /* 0xBF8892A4 */
  .extern EMACxMADRCLR     /* 0xBF8892A4 */
  .extern EMAC1MADRSET     /* 0xBF8892A8 */
  .extern EMACxMADRSET     /* 0xBF8892A8 */
  .extern EMAC1MADRINV     /* 0xBF8892AC */
  .extern EMACxMADRINV     /* 0xBF8892AC */
  .extern EMAC1MWTD        /* 0xBF8892B0 */
  .extern EMACxMWTD        /* 0xBF8892B0 */
  .extern EMAC1MWTDCLR     /* 0xBF8892B4 */
  .extern EMACxMWTDCLR     /* 0xBF8892B4 */
  .extern EMAC1MWTDSET     /* 0xBF8892B8 */
  .extern EMACxMWTDSET     /* 0xBF8892B8 */
  .extern EMAC1MWTDINV     /* 0xBF8892BC */
  .extern EMACxMWTDINV     /* 0xBF8892BC */
  .extern EMAC1MRDD        /* 0xBF8892C0 */
  .extern EMACxMRDD        /* 0xBF8892C0 */
  .extern EMAC1MRDDCLR     /* 0xBF8892C4 */
  .extern EMACxMRDDCLR     /* 0xBF8892C4 */
  .extern EMAC1MRDDSET     /* 0xBF8892C8 */
  .extern EMACxMRDDSET     /* 0xBF8892C8 */
  .extern EMAC1MRDDINV     /* 0xBF8892CC */
  .extern EMACxMRDDINV     /* 0xBF8892CC */
  .extern EMAC1MIND        /* 0xBF8892D0 */
  .extern EMACxMIND        /* 0xBF8892D0 */
  .extern EMAC1MINDCLR     /* 0xBF8892D4 */
  .extern EMACxMINDCLR     /* 0xBF8892D4 */
  .extern EMAC1MINDSET     /* 0xBF8892D8 */
  .extern EMACxMINDSET     /* 0xBF8892D8 */
  .extern EMAC1MINDINV     /* 0xBF8892DC */
  .extern EMACxMINDINV     /* 0xBF8892DC */
  .extern EMAC1SA0         /* 0xBF889300 */
  .extern EMACxSA0         /* 0xBF889300 */
  .extern EMAC1SA0CLR      /* 0xBF889304 */
  .extern EMACxSA0CLR      /* 0xBF889304 */
  .extern EMAC1SA0SET      /* 0xBF889308 */
  .extern EMACxSA0SET      /* 0xBF889308 */
  .extern EMAC1SA0INV      /* 0xBF88930C */
  .extern EMACxSA0INV      /* 0xBF88930C */
  .extern EMAC1SA1         /* 0xBF889310 */
  .extern EMACxSA1         /* 0xBF889310 */
  .extern EMAC1SA1CLR      /* 0xBF889314 */
  .extern EMACxSA1CLR      /* 0xBF889314 */
  .extern EMAC1SA1SET      /* 0xBF889318 */
  .extern EMACxSA1SET      /* 0xBF889318 */
  .extern EMAC1SA1INV      /* 0xBF88931C */
  .extern EMACxSA1INV      /* 0xBF88931C */
  .extern EMAC1SA2         /* 0xBF889320 */
  .extern EMACxSA2         /* 0xBF889320 */
  .extern EMAC1SA2CLR      /* 0xBF889324 */
  .extern EMACxSA2CLR      /* 0xBF889324 */
  .extern EMAC1SA2SET      /* 0xBF889328 */
  .extern EMACxSA2SET      /* 0xBF889328 */
  .extern EMAC1SA2INV      /* 0xBF88932C */
  .extern EMACxSA2INV      /* 0xBF88932C */
  .extern DEVCFG3          /* 0xBFC02FF0 */
  .extern DEVCFG2          /* 0xBFC02FF4 */
  .extern DEVCFG1          /* 0xBFC02FF8 */
  .extern DEVCFG0          /* 0xBFC02FFC */
#else
#error Unknown language!
#endif

#define _WDTCON_WDTCLR_POSITION                  0x00000000
#define _WDTCON_WDTCLR_MASK                      0x00000001
#define _WDTCON_WDTCLR_LENGTH                    0x00000001

#define _WDTCON_SWDTPS_POSITION                  0x00000002
#define _WDTCON_SWDTPS_MASK                      0x0000007C
#define _WDTCON_SWDTPS_LENGTH                    0x00000005

#define _WDTCON_ON_POSITION                      0x0000000F
#define _WDTCON_ON_MASK                          0x00008000
#define _WDTCON_ON_LENGTH                        0x00000001

#define _WDTCON_SWDTPS0_POSITION                 0x00000002
#define _WDTCON_SWDTPS0_MASK                     0x00000004
#define _WDTCON_SWDTPS0_LENGTH                   0x00000001

#define _WDTCON_SWDTPS1_POSITION                 0x00000003
#define _WDTCON_SWDTPS1_MASK                     0x00000008
#define _WDTCON_SWDTPS1_LENGTH                   0x00000001

#define _WDTCON_SWDTPS2_POSITION                 0x00000004
#define _WDTCON_SWDTPS2_MASK                     0x00000010
#define _WDTCON_SWDTPS2_LENGTH                   0x00000001

#define _WDTCON_SWDTPS3_POSITION                 0x00000005
#define _WDTCON_SWDTPS3_MASK                     0x00000020
#define _WDTCON_SWDTPS3_LENGTH                   0x00000001

#define _WDTCON_SWDTPS4_POSITION                 0x00000006
#define _WDTCON_SWDTPS4_MASK                     0x00000040
#define _WDTCON_SWDTPS4_LENGTH                   0x00000001

#define _WDTCON_WDTPSTA_POSITION                 0x00000002
#define _WDTCON_WDTPSTA_MASK                     0x0000007C
#define _WDTCON_WDTPSTA_LENGTH                   0x00000005

#define _WDTCON_WDTPS_POSITION                   0x00000002
#define _WDTCON_WDTPS_MASK                       0x0000007C
#define _WDTCON_WDTPS_LENGTH                     0x00000005

#define _WDTCON_w_POSITION                       0x00000000
#define _WDTCON_w_MASK                           0xFFFFFFFF
#define _WDTCON_w_LENGTH                         0x00000020

#define _RTCCON_RTCOE_POSITION                   0x00000000
#define _RTCCON_RTCOE_MASK                       0x00000001
#define _RTCCON_RTCOE_LENGTH                     0x00000001

#define _RTCCON_HALFSEC_POSITION                 0x00000001
#define _RTCCON_HALFSEC_MASK                     0x00000002
#define _RTCCON_HALFSEC_LENGTH                   0x00000001

#define _RTCCON_RTCSYNC_POSITION                 0x00000002
#define _RTCCON_RTCSYNC_MASK                     0x00000004
#define _RTCCON_RTCSYNC_LENGTH                   0x00000001

#define _RTCCON_RTCWREN_POSITION                 0x00000003
#define _RTCCON_RTCWREN_MASK                     0x00000008
#define _RTCCON_RTCWREN_LENGTH                   0x00000001

#define _RTCCON_RTCCLKON_POSITION                0x00000006
#define _RTCCON_RTCCLKON_MASK                    0x00000040
#define _RTCCON_RTCCLKON_LENGTH                  0x00000001

#define _RTCCON_RTSECSEL_POSITION                0x00000007
#define _RTCCON_RTSECSEL_MASK                    0x00000080
#define _RTCCON_RTSECSEL_LENGTH                  0x00000001

#define _RTCCON_SIDL_POSITION                    0x0000000D
#define _RTCCON_SIDL_MASK                        0x00002000
#define _RTCCON_SIDL_LENGTH                      0x00000001

#define _RTCCON_ON_POSITION                      0x0000000F
#define _RTCCON_ON_MASK                          0x00008000
#define _RTCCON_ON_LENGTH                        0x00000001

#define _RTCCON_CAL_POSITION                     0x00000010
#define _RTCCON_CAL_MASK                         0x03FF0000
#define _RTCCON_CAL_LENGTH                       0x0000000A

#define _RTCCON_w_POSITION                       0x00000000
#define _RTCCON_w_MASK                           0xFFFFFFFF
#define _RTCCON_w_LENGTH                         0x00000020

#define _RTCALRM_ARPT_POSITION                   0x00000000
#define _RTCALRM_ARPT_MASK                       0x000000FF
#define _RTCALRM_ARPT_LENGTH                     0x00000008

#define _RTCALRM_AMASK_POSITION                  0x00000008
#define _RTCALRM_AMASK_MASK                      0x00000F00
#define _RTCALRM_AMASK_LENGTH                    0x00000004

#define _RTCALRM_ALRMSYNC_POSITION               0x0000000C
#define _RTCALRM_ALRMSYNC_MASK                   0x00001000
#define _RTCALRM_ALRMSYNC_LENGTH                 0x00000001

#define _RTCALRM_PIV_POSITION                    0x0000000D
#define _RTCALRM_PIV_MASK                        0x00002000
#define _RTCALRM_PIV_LENGTH                      0x00000001

#define _RTCALRM_CHIME_POSITION                  0x0000000E
#define _RTCALRM_CHIME_MASK                      0x00004000
#define _RTCALRM_CHIME_LENGTH                    0x00000001

#define _RTCALRM_ALRMEN_POSITION                 0x0000000F
#define _RTCALRM_ALRMEN_MASK                     0x00008000
#define _RTCALRM_ALRMEN_LENGTH                   0x00000001

#define _RTCALRM_w_POSITION                      0x00000000
#define _RTCALRM_w_MASK                          0xFFFFFFFF
#define _RTCALRM_w_LENGTH                        0x00000020

#define _RTCTIME_SEC01_POSITION                  0x00000008
#define _RTCTIME_SEC01_MASK                      0x00000F00
#define _RTCTIME_SEC01_LENGTH                    0x00000004

#define _RTCTIME_SEC10_POSITION                  0x0000000C
#define _RTCTIME_SEC10_MASK                      0x0000F000
#define _RTCTIME_SEC10_LENGTH                    0x00000004

#define _RTCTIME_MIN01_POSITION                  0x00000010
#define _RTCTIME_MIN01_MASK                      0x000F0000
#define _RTCTIME_MIN01_LENGTH                    0x00000004

#define _RTCTIME_MIN10_POSITION                  0x00000014
#define _RTCTIME_MIN10_MASK                      0x00F00000
#define _RTCTIME_MIN10_LENGTH                    0x00000004

#define _RTCTIME_HR01_POSITION                   0x00000018
#define _RTCTIME_HR01_MASK                       0x0F000000
#define _RTCTIME_HR01_LENGTH                     0x00000004

#define _RTCTIME_HR10_POSITION                   0x0000001C
#define _RTCTIME_HR10_MASK                       0xF0000000
#define _RTCTIME_HR10_LENGTH                     0x00000004

#define _RTCTIME_w_POSITION                      0x00000000
#define _RTCTIME_w_MASK                          0xFFFFFFFF
#define _RTCTIME_w_LENGTH                        0x00000020

#define _RTCDATE_WDAY01_POSITION                 0x00000000
#define _RTCDATE_WDAY01_MASK                     0x0000000F
#define _RTCDATE_WDAY01_LENGTH                   0x00000004

#define _RTCDATE_DAY01_POSITION                  0x00000008
#define _RTCDATE_DAY01_MASK                      0x00000F00
#define _RTCDATE_DAY01_LENGTH                    0x00000004

#define _RTCDATE_DAY10_POSITION                  0x0000000C
#define _RTCDATE_DAY10_MASK                      0x0000F000
#define _RTCDATE_DAY10_LENGTH                    0x00000004

#define _RTCDATE_MONTH01_POSITION                0x00000010
#define _RTCDATE_MONTH01_MASK                    0x000F0000
#define _RTCDATE_MONTH01_LENGTH                  0x00000004

#define _RTCDATE_MONTH10_POSITION                0x00000014
#define _RTCDATE_MONTH10_MASK                    0x00F00000
#define _RTCDATE_MONTH10_LENGTH                  0x00000004

#define _RTCDATE_YEAR01_POSITION                 0x00000018
#define _RTCDATE_YEAR01_MASK                     0x0F000000
#define _RTCDATE_YEAR01_LENGTH                   0x00000004

#define _RTCDATE_YEAR10_POSITION                 0x0000001C
#define _RTCDATE_YEAR10_MASK                     0xF0000000
#define _RTCDATE_YEAR10_LENGTH                   0x00000004

#define _RTCDATE_w_POSITION                      0x00000000
#define _RTCDATE_w_MASK                          0xFFFFFFFF
#define _RTCDATE_w_LENGTH                        0x00000020

#define _ALRMTIME_SEC01_POSITION                 0x00000008
#define _ALRMTIME_SEC01_MASK                     0x00000F00
#define _ALRMTIME_SEC01_LENGTH                   0x00000004

#define _ALRMTIME_SEC10_POSITION                 0x0000000C
#define _ALRMTIME_SEC10_MASK                     0x0000F000
#define _ALRMTIME_SEC10_LENGTH                   0x00000004

#define _ALRMTIME_MIN01_POSITION                 0x00000010
#define _ALRMTIME_MIN01_MASK                     0x000F0000
#define _ALRMTIME_MIN01_LENGTH                   0x00000004

#define _ALRMTIME_MIN10_POSITION                 0x00000014
#define _ALRMTIME_MIN10_MASK                     0x00F00000
#define _ALRMTIME_MIN10_LENGTH                   0x00000004

#define _ALRMTIME_HR01_POSITION                  0x00000018
#define _ALRMTIME_HR01_MASK                      0x0F000000
#define _ALRMTIME_HR01_LENGTH                    0x00000004

#define _ALRMTIME_HR10_POSITION                  0x0000001C
#define _ALRMTIME_HR10_MASK                      0xF0000000
#define _ALRMTIME_HR10_LENGTH                    0x00000004

#define _ALRMTIME_w_POSITION                     0x00000000
#define _ALRMTIME_w_MASK                         0xFFFFFFFF
#define _ALRMTIME_w_LENGTH                       0x00000020

#define _ALRMDATE_WDAY01_POSITION                0x00000000
#define _ALRMDATE_WDAY01_MASK                    0x0000000F
#define _ALRMDATE_WDAY01_LENGTH                  0x00000004

#define _ALRMDATE_DAY01_POSITION                 0x00000008
#define _ALRMDATE_DAY01_MASK                     0x00000F00
#define _ALRMDATE_DAY01_LENGTH                   0x00000004

#define _ALRMDATE_DAY10_POSITION                 0x0000000C
#define _ALRMDATE_DAY10_MASK                     0x0000F000
#define _ALRMDATE_DAY10_LENGTH                   0x00000004

#define _ALRMDATE_MONTH01_POSITION               0x00000010
#define _ALRMDATE_MONTH01_MASK                   0x000F0000
#define _ALRMDATE_MONTH01_LENGTH                 0x00000004

#define _ALRMDATE_MONTH10_POSITION               0x00000014
#define _ALRMDATE_MONTH10_MASK                   0x00F00000
#define _ALRMDATE_MONTH10_LENGTH                 0x00000004

#define _ALRMDATE_w_POSITION                     0x00000000
#define _ALRMDATE_w_MASK                         0xFFFFFFFF
#define _ALRMDATE_w_LENGTH                       0x00000020

#define _T1CON_TCS_POSITION                      0x00000001
#define _T1CON_TCS_MASK                          0x00000002
#define _T1CON_TCS_LENGTH                        0x00000001

#define _T1CON_TSYNC_POSITION                    0x00000002
#define _T1CON_TSYNC_MASK                        0x00000004
#define _T1CON_TSYNC_LENGTH                      0x00000001

#define _T1CON_TCKPS_POSITION                    0x00000004
#define _T1CON_TCKPS_MASK                        0x00000030
#define _T1CON_TCKPS_LENGTH                      0x00000002

#define _T1CON_TGATE_POSITION                    0x00000007
#define _T1CON_TGATE_MASK                        0x00000080
#define _T1CON_TGATE_LENGTH                      0x00000001

#define _T1CON_TWIP_POSITION                     0x0000000B
#define _T1CON_TWIP_MASK                         0x00000800
#define _T1CON_TWIP_LENGTH                       0x00000001

#define _T1CON_TWDIS_POSITION                    0x0000000C
#define _T1CON_TWDIS_MASK                        0x00001000
#define _T1CON_TWDIS_LENGTH                      0x00000001

#define _T1CON_SIDL_POSITION                     0x0000000D
#define _T1CON_SIDL_MASK                         0x00002000
#define _T1CON_SIDL_LENGTH                       0x00000001

#define _T1CON_ON_POSITION                       0x0000000F
#define _T1CON_ON_MASK                           0x00008000
#define _T1CON_ON_LENGTH                         0x00000001

#define _T1CON_TCKPS0_POSITION                   0x00000004
#define _T1CON_TCKPS0_MASK                       0x00000010
#define _T1CON_TCKPS0_LENGTH                     0x00000001

#define _T1CON_TCKPS1_POSITION                   0x00000005
#define _T1CON_TCKPS1_MASK                       0x00000020
#define _T1CON_TCKPS1_LENGTH                     0x00000001

#define _T1CON_TSIDL_POSITION                    0x0000000D
#define _T1CON_TSIDL_MASK                        0x00002000
#define _T1CON_TSIDL_LENGTH                      0x00000001

#define _T1CON_TON_POSITION                      0x0000000F
#define _T1CON_TON_MASK                          0x00008000
#define _T1CON_TON_LENGTH                        0x00000001

#define _T1CON_w_POSITION                        0x00000000
#define _T1CON_w_MASK                            0xFFFFFFFF
#define _T1CON_w_LENGTH                          0x00000020

#define _T2CON_T32_POSITION                      0x00000003
#define _T2CON_T32_MASK                          0x00000008
#define _T2CON_T32_LENGTH                        0x00000001

#define _T2CON_TCKPS_POSITION                    0x00000004
#define _T2CON_TCKPS_MASK                        0x00000070
#define _T2CON_TCKPS_LENGTH                      0x00000003

#define _T2CON_TGATE_POSITION                    0x00000007
#define _T2CON_TGATE_MASK                        0x00000080
#define _T2CON_TGATE_LENGTH                      0x00000001

#define _T2CON_SIDL_POSITION                     0x0000000D
#define _T2CON_SIDL_MASK                         0x00002000
#define _T2CON_SIDL_LENGTH                       0x00000001

#define _T2CON_ON_POSITION                       0x0000000F
#define _T2CON_ON_MASK                           0x00008000
#define _T2CON_ON_LENGTH                         0x00000001

#define _T2CON_TCKPS0_POSITION                   0x00000004
#define _T2CON_TCKPS0_MASK                       0x00000010
#define _T2CON_TCKPS0_LENGTH                     0x00000001

#define _T2CON_TCKPS1_POSITION                   0x00000005
#define _T2CON_TCKPS1_MASK                       0x00000020
#define _T2CON_TCKPS1_LENGTH                     0x00000001

#define _T2CON_TCKPS2_POSITION                   0x00000006
#define _T2CON_TCKPS2_MASK                       0x00000040
#define _T2CON_TCKPS2_LENGTH                     0x00000001

#define _T2CON_TSIDL_POSITION                    0x0000000D
#define _T2CON_TSIDL_MASK                        0x00002000
#define _T2CON_TSIDL_LENGTH                      0x00000001

#define _T2CON_TON_POSITION                      0x0000000F
#define _T2CON_TON_MASK                          0x00008000
#define _T2CON_TON_LENGTH                        0x00000001

#define _T2CON_w_POSITION                        0x00000000
#define _T2CON_w_MASK                            0xFFFFFFFF
#define _T2CON_w_LENGTH                          0x00000020

#define _T3CON_TCKPS_POSITION                    0x00000004
#define _T3CON_TCKPS_MASK                        0x00000070
#define _T3CON_TCKPS_LENGTH                      0x00000003

#define _T3CON_TGATE_POSITION                    0x00000007
#define _T3CON_TGATE_MASK                        0x00000080
#define _T3CON_TGATE_LENGTH                      0x00000001

#define _T3CON_SIDL_POSITION                     0x0000000D
#define _T3CON_SIDL_MASK                         0x00002000
#define _T3CON_SIDL_LENGTH                       0x00000001

#define _T3CON_ON_POSITION                       0x0000000F
#define _T3CON_ON_MASK                           0x00008000
#define _T3CON_ON_LENGTH                         0x00000001

#define _T3CON_TCKPS0_POSITION                   0x00000004
#define _T3CON_TCKPS0_MASK                       0x00000010
#define _T3CON_TCKPS0_LENGTH                     0x00000001

#define _T3CON_TCKPS1_POSITION                   0x00000005
#define _T3CON_TCKPS1_MASK                       0x00000020
#define _T3CON_TCKPS1_LENGTH                     0x00000001

#define _T3CON_TCKPS2_POSITION                   0x00000006
#define _T3CON_TCKPS2_MASK                       0x00000040
#define _T3CON_TCKPS2_LENGTH                     0x00000001

#define _T3CON_TSIDL_POSITION                    0x0000000D
#define _T3CON_TSIDL_MASK                        0x00002000
#define _T3CON_TSIDL_LENGTH                      0x00000001

#define _T3CON_TON_POSITION                      0x0000000F
#define _T3CON_TON_MASK                          0x00008000
#define _T3CON_TON_LENGTH                        0x00000001

#define _T3CON_w_POSITION                        0x00000000
#define _T3CON_w_MASK                            0xFFFFFFFF
#define _T3CON_w_LENGTH                          0x00000020

#define _T4CON_T32_POSITION                      0x00000003
#define _T4CON_T32_MASK                          0x00000008
#define _T4CON_T32_LENGTH                        0x00000001

#define _T4CON_TCKPS_POSITION                    0x00000004
#define _T4CON_TCKPS_MASK                        0x00000070
#define _T4CON_TCKPS_LENGTH                      0x00000003

#define _T4CON_TGATE_POSITION                    0x00000007
#define _T4CON_TGATE_MASK                        0x00000080
#define _T4CON_TGATE_LENGTH                      0x00000001

#define _T4CON_SIDL_POSITION                     0x0000000D
#define _T4CON_SIDL_MASK                         0x00002000
#define _T4CON_SIDL_LENGTH                       0x00000001

#define _T4CON_ON_POSITION                       0x0000000F
#define _T4CON_ON_MASK                           0x00008000
#define _T4CON_ON_LENGTH                         0x00000001

#define _T4CON_TCKPS0_POSITION                   0x00000004
#define _T4CON_TCKPS0_MASK                       0x00000010
#define _T4CON_TCKPS0_LENGTH                     0x00000001

#define _T4CON_TCKPS1_POSITION                   0x00000005
#define _T4CON_TCKPS1_MASK                       0x00000020
#define _T4CON_TCKPS1_LENGTH                     0x00000001

#define _T4CON_TCKPS2_POSITION                   0x00000006
#define _T4CON_TCKPS2_MASK                       0x00000040
#define _T4CON_TCKPS2_LENGTH                     0x00000001

#define _T4CON_TSIDL_POSITION                    0x0000000D
#define _T4CON_TSIDL_MASK                        0x00002000
#define _T4CON_TSIDL_LENGTH                      0x00000001

#define _T4CON_TON_POSITION                      0x0000000F
#define _T4CON_TON_MASK                          0x00008000
#define _T4CON_TON_LENGTH                        0x00000001

#define _T4CON_w_POSITION                        0x00000000
#define _T4CON_w_MASK                            0xFFFFFFFF
#define _T4CON_w_LENGTH                          0x00000020

#define _T5CON_TCKPS_POSITION                    0x00000004
#define _T5CON_TCKPS_MASK                        0x00000070
#define _T5CON_TCKPS_LENGTH                      0x00000003

#define _T5CON_TGATE_POSITION                    0x00000007
#define _T5CON_TGATE_MASK                        0x00000080
#define _T5CON_TGATE_LENGTH                      0x00000001

#define _T5CON_SIDL_POSITION                     0x0000000D
#define _T5CON_SIDL_MASK                         0x00002000
#define _T5CON_SIDL_LENGTH                       0x00000001

#define _T5CON_ON_POSITION                       0x0000000F
#define _T5CON_ON_MASK                           0x00008000
#define _T5CON_ON_LENGTH                         0x00000001

#define _T5CON_TCKPS0_POSITION                   0x00000004
#define _T5CON_TCKPS0_MASK                       0x00000010
#define _T5CON_TCKPS0_LENGTH                     0x00000001

#define _T5CON_TCKPS1_POSITION                   0x00000005
#define _T5CON_TCKPS1_MASK                       0x00000020
#define _T5CON_TCKPS1_LENGTH                     0x00000001

#define _T5CON_TCKPS2_POSITION                   0x00000006
#define _T5CON_TCKPS2_MASK                       0x00000040
#define _T5CON_TCKPS2_LENGTH                     0x00000001

#define _T5CON_TSIDL_POSITION                    0x0000000D
#define _T5CON_TSIDL_MASK                        0x00002000
#define _T5CON_TSIDL_LENGTH                      0x00000001

#define _T5CON_TON_POSITION                      0x0000000F
#define _T5CON_TON_MASK                          0x00008000
#define _T5CON_TON_LENGTH                        0x00000001

#define _T5CON_w_POSITION                        0x00000000
#define _T5CON_w_MASK                            0xFFFFFFFF
#define _T5CON_w_LENGTH                          0x00000020

#define _IC1CON_ICM_POSITION                     0x00000000
#define _IC1CON_ICM_MASK                         0x00000007
#define _IC1CON_ICM_LENGTH                       0x00000003

#define _IC1CON_ICBNE_POSITION                   0x00000003
#define _IC1CON_ICBNE_MASK                       0x00000008
#define _IC1CON_ICBNE_LENGTH                     0x00000001

#define _IC1CON_ICOV_POSITION                    0x00000004
#define _IC1CON_ICOV_MASK                        0x00000010
#define _IC1CON_ICOV_LENGTH                      0x00000001

#define _IC1CON_ICI_POSITION                     0x00000005
#define _IC1CON_ICI_MASK                         0x00000060
#define _IC1CON_ICI_LENGTH                       0x00000002

#define _IC1CON_ICTMR_POSITION                   0x00000007
#define _IC1CON_ICTMR_MASK                       0x00000080
#define _IC1CON_ICTMR_LENGTH                     0x00000001

#define _IC1CON_C32_POSITION                     0x00000008
#define _IC1CON_C32_MASK                         0x00000100
#define _IC1CON_C32_LENGTH                       0x00000001

#define _IC1CON_FEDGE_POSITION                   0x00000009
#define _IC1CON_FEDGE_MASK                       0x00000200
#define _IC1CON_FEDGE_LENGTH                     0x00000001

#define _IC1CON_SIDL_POSITION                    0x0000000D
#define _IC1CON_SIDL_MASK                        0x00002000
#define _IC1CON_SIDL_LENGTH                      0x00000001

#define _IC1CON_ON_POSITION                      0x0000000F
#define _IC1CON_ON_MASK                          0x00008000
#define _IC1CON_ON_LENGTH                        0x00000001

#define _IC1CON_ICM0_POSITION                    0x00000000
#define _IC1CON_ICM0_MASK                        0x00000001
#define _IC1CON_ICM0_LENGTH                      0x00000001

#define _IC1CON_ICM1_POSITION                    0x00000001
#define _IC1CON_ICM1_MASK                        0x00000002
#define _IC1CON_ICM1_LENGTH                      0x00000001

#define _IC1CON_ICM2_POSITION                    0x00000002
#define _IC1CON_ICM2_MASK                        0x00000004
#define _IC1CON_ICM2_LENGTH                      0x00000001

#define _IC1CON_ICI0_POSITION                    0x00000005
#define _IC1CON_ICI0_MASK                        0x00000020
#define _IC1CON_ICI0_LENGTH                      0x00000001

#define _IC1CON_ICI1_POSITION                    0x00000006
#define _IC1CON_ICI1_MASK                        0x00000040
#define _IC1CON_ICI1_LENGTH                      0x00000001

#define _IC1CON_ICSIDL_POSITION                  0x0000000D
#define _IC1CON_ICSIDL_MASK                      0x00002000
#define _IC1CON_ICSIDL_LENGTH                    0x00000001

#define _IC1CON_w_POSITION                       0x00000000
#define _IC1CON_w_MASK                           0xFFFFFFFF
#define _IC1CON_w_LENGTH                         0x00000020

#define _IC2CON_ICM_POSITION                     0x00000000
#define _IC2CON_ICM_MASK                         0x00000007
#define _IC2CON_ICM_LENGTH                       0x00000003

#define _IC2CON_ICBNE_POSITION                   0x00000003
#define _IC2CON_ICBNE_MASK                       0x00000008
#define _IC2CON_ICBNE_LENGTH                     0x00000001

#define _IC2CON_ICOV_POSITION                    0x00000004
#define _IC2CON_ICOV_MASK                        0x00000010
#define _IC2CON_ICOV_LENGTH                      0x00000001

#define _IC2CON_ICI_POSITION                     0x00000005
#define _IC2CON_ICI_MASK                         0x00000060
#define _IC2CON_ICI_LENGTH                       0x00000002

#define _IC2CON_ICTMR_POSITION                   0x00000007
#define _IC2CON_ICTMR_MASK                       0x00000080
#define _IC2CON_ICTMR_LENGTH                     0x00000001

#define _IC2CON_C32_POSITION                     0x00000008
#define _IC2CON_C32_MASK                         0x00000100
#define _IC2CON_C32_LENGTH                       0x00000001

#define _IC2CON_FEDGE_POSITION                   0x00000009
#define _IC2CON_FEDGE_MASK                       0x00000200
#define _IC2CON_FEDGE_LENGTH                     0x00000001

#define _IC2CON_SIDL_POSITION                    0x0000000D
#define _IC2CON_SIDL_MASK                        0x00002000
#define _IC2CON_SIDL_LENGTH                      0x00000001

#define _IC2CON_ON_POSITION                      0x0000000F
#define _IC2CON_ON_MASK                          0x00008000
#define _IC2CON_ON_LENGTH                        0x00000001

#define _IC2CON_ICM0_POSITION                    0x00000000
#define _IC2CON_ICM0_MASK                        0x00000001
#define _IC2CON_ICM0_LENGTH                      0x00000001

#define _IC2CON_ICM1_POSITION                    0x00000001
#define _IC2CON_ICM1_MASK                        0x00000002
#define _IC2CON_ICM1_LENGTH                      0x00000001

#define _IC2CON_ICM2_POSITION                    0x00000002
#define _IC2CON_ICM2_MASK                        0x00000004
#define _IC2CON_ICM2_LENGTH                      0x00000001

#define _IC2CON_ICI0_POSITION                    0x00000005
#define _IC2CON_ICI0_MASK                        0x00000020
#define _IC2CON_ICI0_LENGTH                      0x00000001

#define _IC2CON_ICI1_POSITION                    0x00000006
#define _IC2CON_ICI1_MASK                        0x00000040
#define _IC2CON_ICI1_LENGTH                      0x00000001

#define _IC2CON_ICSIDL_POSITION                  0x0000000D
#define _IC2CON_ICSIDL_MASK                      0x00002000
#define _IC2CON_ICSIDL_LENGTH                    0x00000001

#define _IC2CON_w_POSITION                       0x00000000
#define _IC2CON_w_MASK                           0xFFFFFFFF
#define _IC2CON_w_LENGTH                         0x00000020

#define _IC3CON_ICM_POSITION                     0x00000000
#define _IC3CON_ICM_MASK                         0x00000007
#define _IC3CON_ICM_LENGTH                       0x00000003

#define _IC3CON_ICBNE_POSITION                   0x00000003
#define _IC3CON_ICBNE_MASK                       0x00000008
#define _IC3CON_ICBNE_LENGTH                     0x00000001

#define _IC3CON_ICOV_POSITION                    0x00000004
#define _IC3CON_ICOV_MASK                        0x00000010
#define _IC3CON_ICOV_LENGTH                      0x00000001

#define _IC3CON_ICI_POSITION                     0x00000005
#define _IC3CON_ICI_MASK                         0x00000060
#define _IC3CON_ICI_LENGTH                       0x00000002

#define _IC3CON_ICTMR_POSITION                   0x00000007
#define _IC3CON_ICTMR_MASK                       0x00000080
#define _IC3CON_ICTMR_LENGTH                     0x00000001

#define _IC3CON_C32_POSITION                     0x00000008
#define _IC3CON_C32_MASK                         0x00000100
#define _IC3CON_C32_LENGTH                       0x00000001

#define _IC3CON_FEDGE_POSITION                   0x00000009
#define _IC3CON_FEDGE_MASK                       0x00000200
#define _IC3CON_FEDGE_LENGTH                     0x00000001

#define _IC3CON_SIDL_POSITION                    0x0000000D
#define _IC3CON_SIDL_MASK                        0x00002000
#define _IC3CON_SIDL_LENGTH                      0x00000001

#define _IC3CON_ON_POSITION                      0x0000000F
#define _IC3CON_ON_MASK                          0x00008000
#define _IC3CON_ON_LENGTH                        0x00000001

#define _IC3CON_ICM0_POSITION                    0x00000000
#define _IC3CON_ICM0_MASK                        0x00000001
#define _IC3CON_ICM0_LENGTH                      0x00000001

#define _IC3CON_ICM1_POSITION                    0x00000001
#define _IC3CON_ICM1_MASK                        0x00000002
#define _IC3CON_ICM1_LENGTH                      0x00000001

#define _IC3CON_ICM2_POSITION                    0x00000002
#define _IC3CON_ICM2_MASK                        0x00000004
#define _IC3CON_ICM2_LENGTH                      0x00000001

#define _IC3CON_ICI0_POSITION                    0x00000005
#define _IC3CON_ICI0_MASK                        0x00000020
#define _IC3CON_ICI0_LENGTH                      0x00000001

#define _IC3CON_ICI1_POSITION                    0x00000006
#define _IC3CON_ICI1_MASK                        0x00000040
#define _IC3CON_ICI1_LENGTH                      0x00000001

#define _IC3CON_ICSIDL_POSITION                  0x0000000D
#define _IC3CON_ICSIDL_MASK                      0x00002000
#define _IC3CON_ICSIDL_LENGTH                    0x00000001

#define _IC3CON_w_POSITION                       0x00000000
#define _IC3CON_w_MASK                           0xFFFFFFFF
#define _IC3CON_w_LENGTH                         0x00000020

#define _IC4CON_ICM_POSITION                     0x00000000
#define _IC4CON_ICM_MASK                         0x00000007
#define _IC4CON_ICM_LENGTH                       0x00000003

#define _IC4CON_ICBNE_POSITION                   0x00000003
#define _IC4CON_ICBNE_MASK                       0x00000008
#define _IC4CON_ICBNE_LENGTH                     0x00000001

#define _IC4CON_ICOV_POSITION                    0x00000004
#define _IC4CON_ICOV_MASK                        0x00000010
#define _IC4CON_ICOV_LENGTH                      0x00000001

#define _IC4CON_ICI_POSITION                     0x00000005
#define _IC4CON_ICI_MASK                         0x00000060
#define _IC4CON_ICI_LENGTH                       0x00000002

#define _IC4CON_ICTMR_POSITION                   0x00000007
#define _IC4CON_ICTMR_MASK                       0x00000080
#define _IC4CON_ICTMR_LENGTH                     0x00000001

#define _IC4CON_C32_POSITION                     0x00000008
#define _IC4CON_C32_MASK                         0x00000100
#define _IC4CON_C32_LENGTH                       0x00000001

#define _IC4CON_FEDGE_POSITION                   0x00000009
#define _IC4CON_FEDGE_MASK                       0x00000200
#define _IC4CON_FEDGE_LENGTH                     0x00000001

#define _IC4CON_SIDL_POSITION                    0x0000000D
#define _IC4CON_SIDL_MASK                        0x00002000
#define _IC4CON_SIDL_LENGTH                      0x00000001

#define _IC4CON_ON_POSITION                      0x0000000F
#define _IC4CON_ON_MASK                          0x00008000
#define _IC4CON_ON_LENGTH                        0x00000001

#define _IC4CON_ICM0_POSITION                    0x00000000
#define _IC4CON_ICM0_MASK                        0x00000001
#define _IC4CON_ICM0_LENGTH                      0x00000001

#define _IC4CON_ICM1_POSITION                    0x00000001
#define _IC4CON_ICM1_MASK                        0x00000002
#define _IC4CON_ICM1_LENGTH                      0x00000001

#define _IC4CON_ICM2_POSITION                    0x00000002
#define _IC4CON_ICM2_MASK                        0x00000004
#define _IC4CON_ICM2_LENGTH                      0x00000001

#define _IC4CON_ICI0_POSITION                    0x00000005
#define _IC4CON_ICI0_MASK                        0x00000020
#define _IC4CON_ICI0_LENGTH                      0x00000001

#define _IC4CON_ICI1_POSITION                    0x00000006
#define _IC4CON_ICI1_MASK                        0x00000040
#define _IC4CON_ICI1_LENGTH                      0x00000001

#define _IC4CON_ICSIDL_POSITION                  0x0000000D
#define _IC4CON_ICSIDL_MASK                      0x00002000
#define _IC4CON_ICSIDL_LENGTH                    0x00000001

#define _IC4CON_w_POSITION                       0x00000000
#define _IC4CON_w_MASK                           0xFFFFFFFF
#define _IC4CON_w_LENGTH                         0x00000020

#define _IC5CON_ICM_POSITION                     0x00000000
#define _IC5CON_ICM_MASK                         0x00000007
#define _IC5CON_ICM_LENGTH                       0x00000003

#define _IC5CON_ICBNE_POSITION                   0x00000003
#define _IC5CON_ICBNE_MASK                       0x00000008
#define _IC5CON_ICBNE_LENGTH                     0x00000001

#define _IC5CON_ICOV_POSITION                    0x00000004
#define _IC5CON_ICOV_MASK                        0x00000010
#define _IC5CON_ICOV_LENGTH                      0x00000001

#define _IC5CON_ICI_POSITION                     0x00000005
#define _IC5CON_ICI_MASK                         0x00000060
#define _IC5CON_ICI_LENGTH                       0x00000002

#define _IC5CON_ICTMR_POSITION                   0x00000007
#define _IC5CON_ICTMR_MASK                       0x00000080
#define _IC5CON_ICTMR_LENGTH                     0x00000001

#define _IC5CON_C32_POSITION                     0x00000008
#define _IC5CON_C32_MASK                         0x00000100
#define _IC5CON_C32_LENGTH                       0x00000001

#define _IC5CON_FEDGE_POSITION                   0x00000009
#define _IC5CON_FEDGE_MASK                       0x00000200
#define _IC5CON_FEDGE_LENGTH                     0x00000001

#define _IC5CON_SIDL_POSITION                    0x0000000D
#define _IC5CON_SIDL_MASK                        0x00002000
#define _IC5CON_SIDL_LENGTH                      0x00000001

#define _IC5CON_ON_POSITION                      0x0000000F
#define _IC5CON_ON_MASK                          0x00008000
#define _IC5CON_ON_LENGTH                        0x00000001

#define _IC5CON_ICM0_POSITION                    0x00000000
#define _IC5CON_ICM0_MASK                        0x00000001
#define _IC5CON_ICM0_LENGTH                      0x00000001

#define _IC5CON_ICM1_POSITION                    0x00000001
#define _IC5CON_ICM1_MASK                        0x00000002
#define _IC5CON_ICM1_LENGTH                      0x00000001

#define _IC5CON_ICM2_POSITION                    0x00000002
#define _IC5CON_ICM2_MASK                        0x00000004
#define _IC5CON_ICM2_LENGTH                      0x00000001

#define _IC5CON_ICI0_POSITION                    0x00000005
#define _IC5CON_ICI0_MASK                        0x00000020
#define _IC5CON_ICI0_LENGTH                      0x00000001

#define _IC5CON_ICI1_POSITION                    0x00000006
#define _IC5CON_ICI1_MASK                        0x00000040
#define _IC5CON_ICI1_LENGTH                      0x00000001

#define _IC5CON_ICSIDL_POSITION                  0x0000000D
#define _IC5CON_ICSIDL_MASK                      0x00002000
#define _IC5CON_ICSIDL_LENGTH                    0x00000001

#define _IC5CON_w_POSITION                       0x00000000
#define _IC5CON_w_MASK                           0xFFFFFFFF
#define _IC5CON_w_LENGTH                         0x00000020

#define _OC1CON_OCM_POSITION                     0x00000000
#define _OC1CON_OCM_MASK                         0x00000007
#define _OC1CON_OCM_LENGTH                       0x00000003

#define _OC1CON_OCTSEL_POSITION                  0x00000003
#define _OC1CON_OCTSEL_MASK                      0x00000008
#define _OC1CON_OCTSEL_LENGTH                    0x00000001

#define _OC1CON_OCFLT_POSITION                   0x00000004
#define _OC1CON_OCFLT_MASK                       0x00000010
#define _OC1CON_OCFLT_LENGTH                     0x00000001

#define _OC1CON_OC32_POSITION                    0x00000005
#define _OC1CON_OC32_MASK                        0x00000020
#define _OC1CON_OC32_LENGTH                      0x00000001

#define _OC1CON_SIDL_POSITION                    0x0000000D
#define _OC1CON_SIDL_MASK                        0x00002000
#define _OC1CON_SIDL_LENGTH                      0x00000001

#define _OC1CON_ON_POSITION                      0x0000000F
#define _OC1CON_ON_MASK                          0x00008000
#define _OC1CON_ON_LENGTH                        0x00000001

#define _OC1CON_OCM0_POSITION                    0x00000000
#define _OC1CON_OCM0_MASK                        0x00000001
#define _OC1CON_OCM0_LENGTH                      0x00000001

#define _OC1CON_OCM1_POSITION                    0x00000001
#define _OC1CON_OCM1_MASK                        0x00000002
#define _OC1CON_OCM1_LENGTH                      0x00000001

#define _OC1CON_OCM2_POSITION                    0x00000002
#define _OC1CON_OCM2_MASK                        0x00000004
#define _OC1CON_OCM2_LENGTH                      0x00000001

#define _OC1CON_OCSIDL_POSITION                  0x0000000D
#define _OC1CON_OCSIDL_MASK                      0x00002000
#define _OC1CON_OCSIDL_LENGTH                    0x00000001

#define _OC1CON_w_POSITION                       0x00000000
#define _OC1CON_w_MASK                           0xFFFFFFFF
#define _OC1CON_w_LENGTH                         0x00000020

#define _OC2CON_OCM_POSITION                     0x00000000
#define _OC2CON_OCM_MASK                         0x00000007
#define _OC2CON_OCM_LENGTH                       0x00000003

#define _OC2CON_OCTSEL_POSITION                  0x00000003
#define _OC2CON_OCTSEL_MASK                      0x00000008
#define _OC2CON_OCTSEL_LENGTH                    0x00000001

#define _OC2CON_OCFLT_POSITION                   0x00000004
#define _OC2CON_OCFLT_MASK                       0x00000010
#define _OC2CON_OCFLT_LENGTH                     0x00000001

#define _OC2CON_OC32_POSITION                    0x00000005
#define _OC2CON_OC32_MASK                        0x00000020
#define _OC2CON_OC32_LENGTH                      0x00000001

#define _OC2CON_SIDL_POSITION                    0x0000000D
#define _OC2CON_SIDL_MASK                        0x00002000
#define _OC2CON_SIDL_LENGTH                      0x00000001

#define _OC2CON_ON_POSITION                      0x0000000F
#define _OC2CON_ON_MASK                          0x00008000
#define _OC2CON_ON_LENGTH                        0x00000001

#define _OC2CON_OCM0_POSITION                    0x00000000
#define _OC2CON_OCM0_MASK                        0x00000001
#define _OC2CON_OCM0_LENGTH                      0x00000001

#define _OC2CON_OCM1_POSITION                    0x00000001
#define _OC2CON_OCM1_MASK                        0x00000002
#define _OC2CON_OCM1_LENGTH                      0x00000001

#define _OC2CON_OCM2_POSITION                    0x00000002
#define _OC2CON_OCM2_MASK                        0x00000004
#define _OC2CON_OCM2_LENGTH                      0x00000001

#define _OC2CON_OCSIDL_POSITION                  0x0000000D
#define _OC2CON_OCSIDL_MASK                      0x00002000
#define _OC2CON_OCSIDL_LENGTH                    0x00000001

#define _OC2CON_w_POSITION                       0x00000000
#define _OC2CON_w_MASK                           0xFFFFFFFF
#define _OC2CON_w_LENGTH                         0x00000020

#define _OC3CON_OCM_POSITION                     0x00000000
#define _OC3CON_OCM_MASK                         0x00000007
#define _OC3CON_OCM_LENGTH                       0x00000003

#define _OC3CON_OCTSEL_POSITION                  0x00000003
#define _OC3CON_OCTSEL_MASK                      0x00000008
#define _OC3CON_OCTSEL_LENGTH                    0x00000001

#define _OC3CON_OCFLT_POSITION                   0x00000004
#define _OC3CON_OCFLT_MASK                       0x00000010
#define _OC3CON_OCFLT_LENGTH                     0x00000001

#define _OC3CON_OC32_POSITION                    0x00000005
#define _OC3CON_OC32_MASK                        0x00000020
#define _OC3CON_OC32_LENGTH                      0x00000001

#define _OC3CON_SIDL_POSITION                    0x0000000D
#define _OC3CON_SIDL_MASK                        0x00002000
#define _OC3CON_SIDL_LENGTH                      0x00000001

#define _OC3CON_ON_POSITION                      0x0000000F
#define _OC3CON_ON_MASK                          0x00008000
#define _OC3CON_ON_LENGTH                        0x00000001

#define _OC3CON_OCM0_POSITION                    0x00000000
#define _OC3CON_OCM0_MASK                        0x00000001
#define _OC3CON_OCM0_LENGTH                      0x00000001

#define _OC3CON_OCM1_POSITION                    0x00000001
#define _OC3CON_OCM1_MASK                        0x00000002
#define _OC3CON_OCM1_LENGTH                      0x00000001

#define _OC3CON_OCM2_POSITION                    0x00000002
#define _OC3CON_OCM2_MASK                        0x00000004
#define _OC3CON_OCM2_LENGTH                      0x00000001

#define _OC3CON_OCSIDL_POSITION                  0x0000000D
#define _OC3CON_OCSIDL_MASK                      0x00002000
#define _OC3CON_OCSIDL_LENGTH                    0x00000001

#define _OC3CON_w_POSITION                       0x00000000
#define _OC3CON_w_MASK                           0xFFFFFFFF
#define _OC3CON_w_LENGTH                         0x00000020

#define _OC4CON_OCM_POSITION                     0x00000000
#define _OC4CON_OCM_MASK                         0x00000007
#define _OC4CON_OCM_LENGTH                       0x00000003

#define _OC4CON_OCTSEL_POSITION                  0x00000003
#define _OC4CON_OCTSEL_MASK                      0x00000008
#define _OC4CON_OCTSEL_LENGTH                    0x00000001

#define _OC4CON_OCFLT_POSITION                   0x00000004
#define _OC4CON_OCFLT_MASK                       0x00000010
#define _OC4CON_OCFLT_LENGTH                     0x00000001

#define _OC4CON_OC32_POSITION                    0x00000005
#define _OC4CON_OC32_MASK                        0x00000020
#define _OC4CON_OC32_LENGTH                      0x00000001

#define _OC4CON_SIDL_POSITION                    0x0000000D
#define _OC4CON_SIDL_MASK                        0x00002000
#define _OC4CON_SIDL_LENGTH                      0x00000001

#define _OC4CON_ON_POSITION                      0x0000000F
#define _OC4CON_ON_MASK                          0x00008000
#define _OC4CON_ON_LENGTH                        0x00000001

#define _OC4CON_OCM0_POSITION                    0x00000000
#define _OC4CON_OCM0_MASK                        0x00000001
#define _OC4CON_OCM0_LENGTH                      0x00000001

#define _OC4CON_OCM1_POSITION                    0x00000001
#define _OC4CON_OCM1_MASK                        0x00000002
#define _OC4CON_OCM1_LENGTH                      0x00000001

#define _OC4CON_OCM2_POSITION                    0x00000002
#define _OC4CON_OCM2_MASK                        0x00000004
#define _OC4CON_OCM2_LENGTH                      0x00000001

#define _OC4CON_OCSIDL_POSITION                  0x0000000D
#define _OC4CON_OCSIDL_MASK                      0x00002000
#define _OC4CON_OCSIDL_LENGTH                    0x00000001

#define _OC4CON_w_POSITION                       0x00000000
#define _OC4CON_w_MASK                           0xFFFFFFFF
#define _OC4CON_w_LENGTH                         0x00000020

#define _OC5CON_OCM_POSITION                     0x00000000
#define _OC5CON_OCM_MASK                         0x00000007
#define _OC5CON_OCM_LENGTH                       0x00000003

#define _OC5CON_OCTSEL_POSITION                  0x00000003
#define _OC5CON_OCTSEL_MASK                      0x00000008
#define _OC5CON_OCTSEL_LENGTH                    0x00000001

#define _OC5CON_OCFLT_POSITION                   0x00000004
#define _OC5CON_OCFLT_MASK                       0x00000010
#define _OC5CON_OCFLT_LENGTH                     0x00000001

#define _OC5CON_OC32_POSITION                    0x00000005
#define _OC5CON_OC32_MASK                        0x00000020
#define _OC5CON_OC32_LENGTH                      0x00000001

#define _OC5CON_SIDL_POSITION                    0x0000000D
#define _OC5CON_SIDL_MASK                        0x00002000
#define _OC5CON_SIDL_LENGTH                      0x00000001

#define _OC5CON_ON_POSITION                      0x0000000F
#define _OC5CON_ON_MASK                          0x00008000
#define _OC5CON_ON_LENGTH                        0x00000001

#define _OC5CON_OCM0_POSITION                    0x00000000
#define _OC5CON_OCM0_MASK                        0x00000001
#define _OC5CON_OCM0_LENGTH                      0x00000001

#define _OC5CON_OCM1_POSITION                    0x00000001
#define _OC5CON_OCM1_MASK                        0x00000002
#define _OC5CON_OCM1_LENGTH                      0x00000001

#define _OC5CON_OCM2_POSITION                    0x00000002
#define _OC5CON_OCM2_MASK                        0x00000004
#define _OC5CON_OCM2_LENGTH                      0x00000001

#define _OC5CON_OCSIDL_POSITION                  0x0000000D
#define _OC5CON_OCSIDL_MASK                      0x00002000
#define _OC5CON_OCSIDL_LENGTH                    0x00000001

#define _OC5CON_w_POSITION                       0x00000000
#define _OC5CON_w_MASK                           0xFFFFFFFF
#define _OC5CON_w_LENGTH                         0x00000020

#define _I2C1ACON_SEN_POSITION                   0x00000000
#define _I2C1ACON_SEN_MASK                       0x00000001
#define _I2C1ACON_SEN_LENGTH                     0x00000001

#define _I2C1ACON_RSEN_POSITION                  0x00000001
#define _I2C1ACON_RSEN_MASK                      0x00000002
#define _I2C1ACON_RSEN_LENGTH                    0x00000001

#define _I2C1ACON_PEN_POSITION                   0x00000002
#define _I2C1ACON_PEN_MASK                       0x00000004
#define _I2C1ACON_PEN_LENGTH                     0x00000001

#define _I2C1ACON_RCEN_POSITION                  0x00000003
#define _I2C1ACON_RCEN_MASK                      0x00000008
#define _I2C1ACON_RCEN_LENGTH                    0x00000001

#define _I2C1ACON_ACKEN_POSITION                 0x00000004
#define _I2C1ACON_ACKEN_MASK                     0x00000010
#define _I2C1ACON_ACKEN_LENGTH                   0x00000001

#define _I2C1ACON_ACKDT_POSITION                 0x00000005
#define _I2C1ACON_ACKDT_MASK                     0x00000020
#define _I2C1ACON_ACKDT_LENGTH                   0x00000001

#define _I2C1ACON_STREN_POSITION                 0x00000006
#define _I2C1ACON_STREN_MASK                     0x00000040
#define _I2C1ACON_STREN_LENGTH                   0x00000001

#define _I2C1ACON_GCEN_POSITION                  0x00000007
#define _I2C1ACON_GCEN_MASK                      0x00000080
#define _I2C1ACON_GCEN_LENGTH                    0x00000001

#define _I2C1ACON_SMEN_POSITION                  0x00000008
#define _I2C1ACON_SMEN_MASK                      0x00000100
#define _I2C1ACON_SMEN_LENGTH                    0x00000001

#define _I2C1ACON_DISSLW_POSITION                0x00000009
#define _I2C1ACON_DISSLW_MASK                    0x00000200
#define _I2C1ACON_DISSLW_LENGTH                  0x00000001

#define _I2C1ACON_A10M_POSITION                  0x0000000A
#define _I2C1ACON_A10M_MASK                      0x00000400
#define _I2C1ACON_A10M_LENGTH                    0x00000001

#define _I2C1ACON_STRICT_POSITION                0x0000000B
#define _I2C1ACON_STRICT_MASK                    0x00000800
#define _I2C1ACON_STRICT_LENGTH                  0x00000001

#define _I2C1ACON_SCLREL_POSITION                0x0000000C
#define _I2C1ACON_SCLREL_MASK                    0x00001000
#define _I2C1ACON_SCLREL_LENGTH                  0x00000001

#define _I2C1ACON_SIDL_POSITION                  0x0000000D
#define _I2C1ACON_SIDL_MASK                      0x00002000
#define _I2C1ACON_SIDL_LENGTH                    0x00000001

#define _I2C1ACON_ON_POSITION                    0x0000000F
#define _I2C1ACON_ON_MASK                        0x00008000
#define _I2C1ACON_ON_LENGTH                      0x00000001

#define _I2C1ACON_IPMIEN_POSITION                0x0000000B
#define _I2C1ACON_IPMIEN_MASK                    0x00000800
#define _I2C1ACON_IPMIEN_LENGTH                  0x00000001

#define _I2C1ACON_I2CSIDL_POSITION               0x0000000D
#define _I2C1ACON_I2CSIDL_MASK                   0x00002000
#define _I2C1ACON_I2CSIDL_LENGTH                 0x00000001

#define _I2C1ACON_I2CEN_POSITION                 0x0000000F
#define _I2C1ACON_I2CEN_MASK                     0x00008000
#define _I2C1ACON_I2CEN_LENGTH                   0x00000001

#define _I2C1ACON_w_POSITION                     0x00000000
#define _I2C1ACON_w_MASK                         0xFFFFFFFF
#define _I2C1ACON_w_LENGTH                       0x00000020

#define _I2C3CON_SEN_POSITION                    0x00000000
#define _I2C3CON_SEN_MASK                        0x00000001
#define _I2C3CON_SEN_LENGTH                      0x00000001

#define _I2C3CON_RSEN_POSITION                   0x00000001
#define _I2C3CON_RSEN_MASK                       0x00000002
#define _I2C3CON_RSEN_LENGTH                     0x00000001

#define _I2C3CON_PEN_POSITION                    0x00000002
#define _I2C3CON_PEN_MASK                        0x00000004
#define _I2C3CON_PEN_LENGTH                      0x00000001

#define _I2C3CON_RCEN_POSITION                   0x00000003
#define _I2C3CON_RCEN_MASK                       0x00000008
#define _I2C3CON_RCEN_LENGTH                     0x00000001

#define _I2C3CON_ACKEN_POSITION                  0x00000004
#define _I2C3CON_ACKEN_MASK                      0x00000010
#define _I2C3CON_ACKEN_LENGTH                    0x00000001

#define _I2C3CON_ACKDT_POSITION                  0x00000005
#define _I2C3CON_ACKDT_MASK                      0x00000020
#define _I2C3CON_ACKDT_LENGTH                    0x00000001

#define _I2C3CON_STREN_POSITION                  0x00000006
#define _I2C3CON_STREN_MASK                      0x00000040
#define _I2C3CON_STREN_LENGTH                    0x00000001

#define _I2C3CON_GCEN_POSITION                   0x00000007
#define _I2C3CON_GCEN_MASK                       0x00000080
#define _I2C3CON_GCEN_LENGTH                     0x00000001

#define _I2C3CON_SMEN_POSITION                   0x00000008
#define _I2C3CON_SMEN_MASK                       0x00000100
#define _I2C3CON_SMEN_LENGTH                     0x00000001

#define _I2C3CON_DISSLW_POSITION                 0x00000009
#define _I2C3CON_DISSLW_MASK                     0x00000200
#define _I2C3CON_DISSLW_LENGTH                   0x00000001

#define _I2C3CON_A10M_POSITION                   0x0000000A
#define _I2C3CON_A10M_MASK                       0x00000400
#define _I2C3CON_A10M_LENGTH                     0x00000001

#define _I2C3CON_STRICT_POSITION                 0x0000000B
#define _I2C3CON_STRICT_MASK                     0x00000800
#define _I2C3CON_STRICT_LENGTH                   0x00000001

#define _I2C3CON_SCLREL_POSITION                 0x0000000C
#define _I2C3CON_SCLREL_MASK                     0x00001000
#define _I2C3CON_SCLREL_LENGTH                   0x00000001

#define _I2C3CON_SIDL_POSITION                   0x0000000D
#define _I2C3CON_SIDL_MASK                       0x00002000
#define _I2C3CON_SIDL_LENGTH                     0x00000001

#define _I2C3CON_ON_POSITION                     0x0000000F
#define _I2C3CON_ON_MASK                         0x00008000
#define _I2C3CON_ON_LENGTH                       0x00000001

#define _I2C3CON_IPMIEN_POSITION                 0x0000000B
#define _I2C3CON_IPMIEN_MASK                     0x00000800
#define _I2C3CON_IPMIEN_LENGTH                   0x00000001

#define _I2C3CON_I2CSIDL_POSITION                0x0000000D
#define _I2C3CON_I2CSIDL_MASK                    0x00002000
#define _I2C3CON_I2CSIDL_LENGTH                  0x00000001

#define _I2C3CON_I2CEN_POSITION                  0x0000000F
#define _I2C3CON_I2CEN_MASK                      0x00008000
#define _I2C3CON_I2CEN_LENGTH                    0x00000001

#define _I2C3CON_w_POSITION                      0x00000000
#define _I2C3CON_w_MASK                          0xFFFFFFFF
#define _I2C3CON_w_LENGTH                        0x00000020

#define _I2C1ASTAT_TBF_POSITION                  0x00000000
#define _I2C1ASTAT_TBF_MASK                      0x00000001
#define _I2C1ASTAT_TBF_LENGTH                    0x00000001

#define _I2C1ASTAT_RBF_POSITION                  0x00000001
#define _I2C1ASTAT_RBF_MASK                      0x00000002
#define _I2C1ASTAT_RBF_LENGTH                    0x00000001

#define _I2C1ASTAT_R_W_POSITION                  0x00000002
#define _I2C1ASTAT_R_W_MASK                      0x00000004
#define _I2C1ASTAT_R_W_LENGTH                    0x00000001

#define _I2C1ASTAT_S_POSITION                    0x00000003
#define _I2C1ASTAT_S_MASK                        0x00000008
#define _I2C1ASTAT_S_LENGTH                      0x00000001

#define _I2C1ASTAT_P_POSITION                    0x00000004
#define _I2C1ASTAT_P_MASK                        0x00000010
#define _I2C1ASTAT_P_LENGTH                      0x00000001

#define _I2C1ASTAT_D_A_POSITION                  0x00000005
#define _I2C1ASTAT_D_A_MASK                      0x00000020
#define _I2C1ASTAT_D_A_LENGTH                    0x00000001

#define _I2C1ASTAT_I2COV_POSITION                0x00000006
#define _I2C1ASTAT_I2COV_MASK                    0x00000040
#define _I2C1ASTAT_I2COV_LENGTH                  0x00000001

#define _I2C1ASTAT_IWCOL_POSITION                0x00000007
#define _I2C1ASTAT_IWCOL_MASK                    0x00000080
#define _I2C1ASTAT_IWCOL_LENGTH                  0x00000001

#define _I2C1ASTAT_ADD10_POSITION                0x00000008
#define _I2C1ASTAT_ADD10_MASK                    0x00000100
#define _I2C1ASTAT_ADD10_LENGTH                  0x00000001

#define _I2C1ASTAT_GCSTAT_POSITION               0x00000009
#define _I2C1ASTAT_GCSTAT_MASK                   0x00000200
#define _I2C1ASTAT_GCSTAT_LENGTH                 0x00000001

#define _I2C1ASTAT_BCL_POSITION                  0x0000000A
#define _I2C1ASTAT_BCL_MASK                      0x00000400
#define _I2C1ASTAT_BCL_LENGTH                    0x00000001

#define _I2C1ASTAT_TRSTAT_POSITION               0x0000000E
#define _I2C1ASTAT_TRSTAT_MASK                   0x00004000
#define _I2C1ASTAT_TRSTAT_LENGTH                 0x00000001

#define _I2C1ASTAT_ACKSTAT_POSITION              0x0000000F
#define _I2C1ASTAT_ACKSTAT_MASK                  0x00008000
#define _I2C1ASTAT_ACKSTAT_LENGTH                0x00000001

#define _I2C1ASTAT_I2CPOV_POSITION               0x00000006
#define _I2C1ASTAT_I2CPOV_MASK                   0x00000040
#define _I2C1ASTAT_I2CPOV_LENGTH                 0x00000001

#define _I2C1ASTAT_w_POSITION                    0x00000000
#define _I2C1ASTAT_w_MASK                        0xFFFFFFFF
#define _I2C1ASTAT_w_LENGTH                      0x00000020

#define _I2C3STAT_TBF_POSITION                   0x00000000
#define _I2C3STAT_TBF_MASK                       0x00000001
#define _I2C3STAT_TBF_LENGTH                     0x00000001

#define _I2C3STAT_RBF_POSITION                   0x00000001
#define _I2C3STAT_RBF_MASK                       0x00000002
#define _I2C3STAT_RBF_LENGTH                     0x00000001

#define _I2C3STAT_R_W_POSITION                   0x00000002
#define _I2C3STAT_R_W_MASK                       0x00000004
#define _I2C3STAT_R_W_LENGTH                     0x00000001

#define _I2C3STAT_S_POSITION                     0x00000003
#define _I2C3STAT_S_MASK                         0x00000008
#define _I2C3STAT_S_LENGTH                       0x00000001

#define _I2C3STAT_P_POSITION                     0x00000004
#define _I2C3STAT_P_MASK                         0x00000010
#define _I2C3STAT_P_LENGTH                       0x00000001

#define _I2C3STAT_D_A_POSITION                   0x00000005
#define _I2C3STAT_D_A_MASK                       0x00000020
#define _I2C3STAT_D_A_LENGTH                     0x00000001

#define _I2C3STAT_I2COV_POSITION                 0x00000006
#define _I2C3STAT_I2COV_MASK                     0x00000040
#define _I2C3STAT_I2COV_LENGTH                   0x00000001

#define _I2C3STAT_IWCOL_POSITION                 0x00000007
#define _I2C3STAT_IWCOL_MASK                     0x00000080
#define _I2C3STAT_IWCOL_LENGTH                   0x00000001

#define _I2C3STAT_ADD10_POSITION                 0x00000008
#define _I2C3STAT_ADD10_MASK                     0x00000100
#define _I2C3STAT_ADD10_LENGTH                   0x00000001

#define _I2C3STAT_GCSTAT_POSITION                0x00000009
#define _I2C3STAT_GCSTAT_MASK                    0x00000200
#define _I2C3STAT_GCSTAT_LENGTH                  0x00000001

#define _I2C3STAT_BCL_POSITION                   0x0000000A
#define _I2C3STAT_BCL_MASK                       0x00000400
#define _I2C3STAT_BCL_LENGTH                     0x00000001

#define _I2C3STAT_TRSTAT_POSITION                0x0000000E
#define _I2C3STAT_TRSTAT_MASK                    0x00004000
#define _I2C3STAT_TRSTAT_LENGTH                  0x00000001

#define _I2C3STAT_ACKSTAT_POSITION               0x0000000F
#define _I2C3STAT_ACKSTAT_MASK                   0x00008000
#define _I2C3STAT_ACKSTAT_LENGTH                 0x00000001

#define _I2C3STAT_I2CPOV_POSITION                0x00000006
#define _I2C3STAT_I2CPOV_MASK                    0x00000040
#define _I2C3STAT_I2CPOV_LENGTH                  0x00000001

#define _I2C3STAT_w_POSITION                     0x00000000
#define _I2C3STAT_w_MASK                         0xFFFFFFFF
#define _I2C3STAT_w_LENGTH                       0x00000020

#define _I2C2ACON_SEN_POSITION                   0x00000000
#define _I2C2ACON_SEN_MASK                       0x00000001
#define _I2C2ACON_SEN_LENGTH                     0x00000001

#define _I2C2ACON_RSEN_POSITION                  0x00000001
#define _I2C2ACON_RSEN_MASK                      0x00000002
#define _I2C2ACON_RSEN_LENGTH                    0x00000001

#define _I2C2ACON_PEN_POSITION                   0x00000002
#define _I2C2ACON_PEN_MASK                       0x00000004
#define _I2C2ACON_PEN_LENGTH                     0x00000001

#define _I2C2ACON_RCEN_POSITION                  0x00000003
#define _I2C2ACON_RCEN_MASK                      0x00000008
#define _I2C2ACON_RCEN_LENGTH                    0x00000001

#define _I2C2ACON_ACKEN_POSITION                 0x00000004
#define _I2C2ACON_ACKEN_MASK                     0x00000010
#define _I2C2ACON_ACKEN_LENGTH                   0x00000001

#define _I2C2ACON_ACKDT_POSITION                 0x00000005
#define _I2C2ACON_ACKDT_MASK                     0x00000020
#define _I2C2ACON_ACKDT_LENGTH                   0x00000001

#define _I2C2ACON_STREN_POSITION                 0x00000006
#define _I2C2ACON_STREN_MASK                     0x00000040
#define _I2C2ACON_STREN_LENGTH                   0x00000001

#define _I2C2ACON_GCEN_POSITION                  0x00000007
#define _I2C2ACON_GCEN_MASK                      0x00000080
#define _I2C2ACON_GCEN_LENGTH                    0x00000001

#define _I2C2ACON_SMEN_POSITION                  0x00000008
#define _I2C2ACON_SMEN_MASK                      0x00000100
#define _I2C2ACON_SMEN_LENGTH                    0x00000001

#define _I2C2ACON_DISSLW_POSITION                0x00000009
#define _I2C2ACON_DISSLW_MASK                    0x00000200
#define _I2C2ACON_DISSLW_LENGTH                  0x00000001

#define _I2C2ACON_A10M_POSITION                  0x0000000A
#define _I2C2ACON_A10M_MASK                      0x00000400
#define _I2C2ACON_A10M_LENGTH                    0x00000001

#define _I2C2ACON_STRICT_POSITION                0x0000000B
#define _I2C2ACON_STRICT_MASK                    0x00000800
#define _I2C2ACON_STRICT_LENGTH                  0x00000001

#define _I2C2ACON_SCLREL_POSITION                0x0000000C
#define _I2C2ACON_SCLREL_MASK                    0x00001000
#define _I2C2ACON_SCLREL_LENGTH                  0x00000001

#define _I2C2ACON_SIDL_POSITION                  0x0000000D
#define _I2C2ACON_SIDL_MASK                      0x00002000
#define _I2C2ACON_SIDL_LENGTH                    0x00000001

#define _I2C2ACON_ON_POSITION                    0x0000000F
#define _I2C2ACON_ON_MASK                        0x00008000
#define _I2C2ACON_ON_LENGTH                      0x00000001

#define _I2C2ACON_IPMIEN_POSITION                0x0000000B
#define _I2C2ACON_IPMIEN_MASK                    0x00000800
#define _I2C2ACON_IPMIEN_LENGTH                  0x00000001

#define _I2C2ACON_I2CSIDL_POSITION               0x0000000D
#define _I2C2ACON_I2CSIDL_MASK                   0x00002000
#define _I2C2ACON_I2CSIDL_LENGTH                 0x00000001

#define _I2C2ACON_I2CEN_POSITION                 0x0000000F
#define _I2C2ACON_I2CEN_MASK                     0x00008000
#define _I2C2ACON_I2CEN_LENGTH                   0x00000001

#define _I2C2ACON_w_POSITION                     0x00000000
#define _I2C2ACON_w_MASK                         0xFFFFFFFF
#define _I2C2ACON_w_LENGTH                       0x00000020

#define _I2C4CON_SEN_POSITION                    0x00000000
#define _I2C4CON_SEN_MASK                        0x00000001
#define _I2C4CON_SEN_LENGTH                      0x00000001

#define _I2C4CON_RSEN_POSITION                   0x00000001
#define _I2C4CON_RSEN_MASK                       0x00000002
#define _I2C4CON_RSEN_LENGTH                     0x00000001

#define _I2C4CON_PEN_POSITION                    0x00000002
#define _I2C4CON_PEN_MASK                        0x00000004
#define _I2C4CON_PEN_LENGTH                      0x00000001

#define _I2C4CON_RCEN_POSITION                   0x00000003
#define _I2C4CON_RCEN_MASK                       0x00000008
#define _I2C4CON_RCEN_LENGTH                     0x00000001

#define _I2C4CON_ACKEN_POSITION                  0x00000004
#define _I2C4CON_ACKEN_MASK                      0x00000010
#define _I2C4CON_ACKEN_LENGTH                    0x00000001

#define _I2C4CON_ACKDT_POSITION                  0x00000005
#define _I2C4CON_ACKDT_MASK                      0x00000020
#define _I2C4CON_ACKDT_LENGTH                    0x00000001

#define _I2C4CON_STREN_POSITION                  0x00000006
#define _I2C4CON_STREN_MASK                      0x00000040
#define _I2C4CON_STREN_LENGTH                    0x00000001

#define _I2C4CON_GCEN_POSITION                   0x00000007
#define _I2C4CON_GCEN_MASK                       0x00000080
#define _I2C4CON_GCEN_LENGTH                     0x00000001

#define _I2C4CON_SMEN_POSITION                   0x00000008
#define _I2C4CON_SMEN_MASK                       0x00000100
#define _I2C4CON_SMEN_LENGTH                     0x00000001

#define _I2C4CON_DISSLW_POSITION                 0x00000009
#define _I2C4CON_DISSLW_MASK                     0x00000200
#define _I2C4CON_DISSLW_LENGTH                   0x00000001

#define _I2C4CON_A10M_POSITION                   0x0000000A
#define _I2C4CON_A10M_MASK                       0x00000400
#define _I2C4CON_A10M_LENGTH                     0x00000001

#define _I2C4CON_STRICT_POSITION                 0x0000000B
#define _I2C4CON_STRICT_MASK                     0x00000800
#define _I2C4CON_STRICT_LENGTH                   0x00000001

#define _I2C4CON_SCLREL_POSITION                 0x0000000C
#define _I2C4CON_SCLREL_MASK                     0x00001000
#define _I2C4CON_SCLREL_LENGTH                   0x00000001

#define _I2C4CON_SIDL_POSITION                   0x0000000D
#define _I2C4CON_SIDL_MASK                       0x00002000
#define _I2C4CON_SIDL_LENGTH                     0x00000001

#define _I2C4CON_ON_POSITION                     0x0000000F
#define _I2C4CON_ON_MASK                         0x00008000
#define _I2C4CON_ON_LENGTH                       0x00000001

#define _I2C4CON_IPMIEN_POSITION                 0x0000000B
#define _I2C4CON_IPMIEN_MASK                     0x00000800
#define _I2C4CON_IPMIEN_LENGTH                   0x00000001

#define _I2C4CON_I2CSIDL_POSITION                0x0000000D
#define _I2C4CON_I2CSIDL_MASK                    0x00002000
#define _I2C4CON_I2CSIDL_LENGTH                  0x00000001

#define _I2C4CON_I2CEN_POSITION                  0x0000000F
#define _I2C4CON_I2CEN_MASK                      0x00008000
#define _I2C4CON_I2CEN_LENGTH                    0x00000001

#define _I2C4CON_w_POSITION                      0x00000000
#define _I2C4CON_w_MASK                          0xFFFFFFFF
#define _I2C4CON_w_LENGTH                        0x00000020

#define _I2C2ASTAT_TBF_POSITION                  0x00000000
#define _I2C2ASTAT_TBF_MASK                      0x00000001
#define _I2C2ASTAT_TBF_LENGTH                    0x00000001

#define _I2C2ASTAT_RBF_POSITION                  0x00000001
#define _I2C2ASTAT_RBF_MASK                      0x00000002
#define _I2C2ASTAT_RBF_LENGTH                    0x00000001

#define _I2C2ASTAT_R_W_POSITION                  0x00000002
#define _I2C2ASTAT_R_W_MASK                      0x00000004
#define _I2C2ASTAT_R_W_LENGTH                    0x00000001

#define _I2C2ASTAT_S_POSITION                    0x00000003
#define _I2C2ASTAT_S_MASK                        0x00000008
#define _I2C2ASTAT_S_LENGTH                      0x00000001

#define _I2C2ASTAT_P_POSITION                    0x00000004
#define _I2C2ASTAT_P_MASK                        0x00000010
#define _I2C2ASTAT_P_LENGTH                      0x00000001

#define _I2C2ASTAT_D_A_POSITION                  0x00000005
#define _I2C2ASTAT_D_A_MASK                      0x00000020
#define _I2C2ASTAT_D_A_LENGTH                    0x00000001

#define _I2C2ASTAT_I2COV_POSITION                0x00000006
#define _I2C2ASTAT_I2COV_MASK                    0x00000040
#define _I2C2ASTAT_I2COV_LENGTH                  0x00000001

#define _I2C2ASTAT_IWCOL_POSITION                0x00000007
#define _I2C2ASTAT_IWCOL_MASK                    0x00000080
#define _I2C2ASTAT_IWCOL_LENGTH                  0x00000001

#define _I2C2ASTAT_ADD10_POSITION                0x00000008
#define _I2C2ASTAT_ADD10_MASK                    0x00000100
#define _I2C2ASTAT_ADD10_LENGTH                  0x00000001

#define _I2C2ASTAT_GCSTAT_POSITION               0x00000009
#define _I2C2ASTAT_GCSTAT_MASK                   0x00000200
#define _I2C2ASTAT_GCSTAT_LENGTH                 0x00000001

#define _I2C2ASTAT_BCL_POSITION                  0x0000000A
#define _I2C2ASTAT_BCL_MASK                      0x00000400
#define _I2C2ASTAT_BCL_LENGTH                    0x00000001

#define _I2C2ASTAT_TRSTAT_POSITION               0x0000000E
#define _I2C2ASTAT_TRSTAT_MASK                   0x00004000
#define _I2C2ASTAT_TRSTAT_LENGTH                 0x00000001

#define _I2C2ASTAT_ACKSTAT_POSITION              0x0000000F
#define _I2C2ASTAT_ACKSTAT_MASK                  0x00008000
#define _I2C2ASTAT_ACKSTAT_LENGTH                0x00000001

#define _I2C2ASTAT_I2CPOV_POSITION               0x00000006
#define _I2C2ASTAT_I2CPOV_MASK                   0x00000040
#define _I2C2ASTAT_I2CPOV_LENGTH                 0x00000001

#define _I2C2ASTAT_w_POSITION                    0x00000000
#define _I2C2ASTAT_w_MASK                        0xFFFFFFFF
#define _I2C2ASTAT_w_LENGTH                      0x00000020

#define _I2C4STAT_TBF_POSITION                   0x00000000
#define _I2C4STAT_TBF_MASK                       0x00000001
#define _I2C4STAT_TBF_LENGTH                     0x00000001

#define _I2C4STAT_RBF_POSITION                   0x00000001
#define _I2C4STAT_RBF_MASK                       0x00000002
#define _I2C4STAT_RBF_LENGTH                     0x00000001

#define _I2C4STAT_R_W_POSITION                   0x00000002
#define _I2C4STAT_R_W_MASK                       0x00000004
#define _I2C4STAT_R_W_LENGTH                     0x00000001

#define _I2C4STAT_S_POSITION                     0x00000003
#define _I2C4STAT_S_MASK                         0x00000008
#define _I2C4STAT_S_LENGTH                       0x00000001

#define _I2C4STAT_P_POSITION                     0x00000004
#define _I2C4STAT_P_MASK                         0x00000010
#define _I2C4STAT_P_LENGTH                       0x00000001

#define _I2C4STAT_D_A_POSITION                   0x00000005
#define _I2C4STAT_D_A_MASK                       0x00000020
#define _I2C4STAT_D_A_LENGTH                     0x00000001

#define _I2C4STAT_I2COV_POSITION                 0x00000006
#define _I2C4STAT_I2COV_MASK                     0x00000040
#define _I2C4STAT_I2COV_LENGTH                   0x00000001

#define _I2C4STAT_IWCOL_POSITION                 0x00000007
#define _I2C4STAT_IWCOL_MASK                     0x00000080
#define _I2C4STAT_IWCOL_LENGTH                   0x00000001

#define _I2C4STAT_ADD10_POSITION                 0x00000008
#define _I2C4STAT_ADD10_MASK                     0x00000100
#define _I2C4STAT_ADD10_LENGTH                   0x00000001

#define _I2C4STAT_GCSTAT_POSITION                0x00000009
#define _I2C4STAT_GCSTAT_MASK                    0x00000200
#define _I2C4STAT_GCSTAT_LENGTH                  0x00000001

#define _I2C4STAT_BCL_POSITION                   0x0000000A
#define _I2C4STAT_BCL_MASK                       0x00000400
#define _I2C4STAT_BCL_LENGTH                     0x00000001

#define _I2C4STAT_TRSTAT_POSITION                0x0000000E
#define _I2C4STAT_TRSTAT_MASK                    0x00004000
#define _I2C4STAT_TRSTAT_LENGTH                  0x00000001

#define _I2C4STAT_ACKSTAT_POSITION               0x0000000F
#define _I2C4STAT_ACKSTAT_MASK                   0x00008000
#define _I2C4STAT_ACKSTAT_LENGTH                 0x00000001

#define _I2C4STAT_I2CPOV_POSITION                0x00000006
#define _I2C4STAT_I2CPOV_MASK                    0x00000040
#define _I2C4STAT_I2CPOV_LENGTH                  0x00000001

#define _I2C4STAT_w_POSITION                     0x00000000
#define _I2C4STAT_w_MASK                         0xFFFFFFFF
#define _I2C4STAT_w_LENGTH                       0x00000020

#define _I2C3ACON_SEN_POSITION                   0x00000000
#define _I2C3ACON_SEN_MASK                       0x00000001
#define _I2C3ACON_SEN_LENGTH                     0x00000001

#define _I2C3ACON_RSEN_POSITION                  0x00000001
#define _I2C3ACON_RSEN_MASK                      0x00000002
#define _I2C3ACON_RSEN_LENGTH                    0x00000001

#define _I2C3ACON_PEN_POSITION                   0x00000002
#define _I2C3ACON_PEN_MASK                       0x00000004
#define _I2C3ACON_PEN_LENGTH                     0x00000001

#define _I2C3ACON_RCEN_POSITION                  0x00000003
#define _I2C3ACON_RCEN_MASK                      0x00000008
#define _I2C3ACON_RCEN_LENGTH                    0x00000001

#define _I2C3ACON_ACKEN_POSITION                 0x00000004
#define _I2C3ACON_ACKEN_MASK                     0x00000010
#define _I2C3ACON_ACKEN_LENGTH                   0x00000001

#define _I2C3ACON_ACKDT_POSITION                 0x00000005
#define _I2C3ACON_ACKDT_MASK                     0x00000020
#define _I2C3ACON_ACKDT_LENGTH                   0x00000001

#define _I2C3ACON_STREN_POSITION                 0x00000006
#define _I2C3ACON_STREN_MASK                     0x00000040
#define _I2C3ACON_STREN_LENGTH                   0x00000001

#define _I2C3ACON_GCEN_POSITION                  0x00000007
#define _I2C3ACON_GCEN_MASK                      0x00000080
#define _I2C3ACON_GCEN_LENGTH                    0x00000001

#define _I2C3ACON_SMEN_POSITION                  0x00000008
#define _I2C3ACON_SMEN_MASK                      0x00000100
#define _I2C3ACON_SMEN_LENGTH                    0x00000001

#define _I2C3ACON_DISSLW_POSITION                0x00000009
#define _I2C3ACON_DISSLW_MASK                    0x00000200
#define _I2C3ACON_DISSLW_LENGTH                  0x00000001

#define _I2C3ACON_A10M_POSITION                  0x0000000A
#define _I2C3ACON_A10M_MASK                      0x00000400
#define _I2C3ACON_A10M_LENGTH                    0x00000001

#define _I2C3ACON_STRICT_POSITION                0x0000000B
#define _I2C3ACON_STRICT_MASK                    0x00000800
#define _I2C3ACON_STRICT_LENGTH                  0x00000001

#define _I2C3ACON_SCLREL_POSITION                0x0000000C
#define _I2C3ACON_SCLREL_MASK                    0x00001000
#define _I2C3ACON_SCLREL_LENGTH                  0x00000001

#define _I2C3ACON_SIDL_POSITION                  0x0000000D
#define _I2C3ACON_SIDL_MASK                      0x00002000
#define _I2C3ACON_SIDL_LENGTH                    0x00000001

#define _I2C3ACON_ON_POSITION                    0x0000000F
#define _I2C3ACON_ON_MASK                        0x00008000
#define _I2C3ACON_ON_LENGTH                      0x00000001

#define _I2C3ACON_IPMIEN_POSITION                0x0000000B
#define _I2C3ACON_IPMIEN_MASK                    0x00000800
#define _I2C3ACON_IPMIEN_LENGTH                  0x00000001

#define _I2C3ACON_I2CSIDL_POSITION               0x0000000D
#define _I2C3ACON_I2CSIDL_MASK                   0x00002000
#define _I2C3ACON_I2CSIDL_LENGTH                 0x00000001

#define _I2C3ACON_I2CEN_POSITION                 0x0000000F
#define _I2C3ACON_I2CEN_MASK                     0x00008000
#define _I2C3ACON_I2CEN_LENGTH                   0x00000001

#define _I2C3ACON_w_POSITION                     0x00000000
#define _I2C3ACON_w_MASK                         0xFFFFFFFF
#define _I2C3ACON_w_LENGTH                       0x00000020

#define _I2C5CON_SEN_POSITION                    0x00000000
#define _I2C5CON_SEN_MASK                        0x00000001
#define _I2C5CON_SEN_LENGTH                      0x00000001

#define _I2C5CON_RSEN_POSITION                   0x00000001
#define _I2C5CON_RSEN_MASK                       0x00000002
#define _I2C5CON_RSEN_LENGTH                     0x00000001

#define _I2C5CON_PEN_POSITION                    0x00000002
#define _I2C5CON_PEN_MASK                        0x00000004
#define _I2C5CON_PEN_LENGTH                      0x00000001

#define _I2C5CON_RCEN_POSITION                   0x00000003
#define _I2C5CON_RCEN_MASK                       0x00000008
#define _I2C5CON_RCEN_LENGTH                     0x00000001

#define _I2C5CON_ACKEN_POSITION                  0x00000004
#define _I2C5CON_ACKEN_MASK                      0x00000010
#define _I2C5CON_ACKEN_LENGTH                    0x00000001

#define _I2C5CON_ACKDT_POSITION                  0x00000005
#define _I2C5CON_ACKDT_MASK                      0x00000020
#define _I2C5CON_ACKDT_LENGTH                    0x00000001

#define _I2C5CON_STREN_POSITION                  0x00000006
#define _I2C5CON_STREN_MASK                      0x00000040
#define _I2C5CON_STREN_LENGTH                    0x00000001

#define _I2C5CON_GCEN_POSITION                   0x00000007
#define _I2C5CON_GCEN_MASK                       0x00000080
#define _I2C5CON_GCEN_LENGTH                     0x00000001

#define _I2C5CON_SMEN_POSITION                   0x00000008
#define _I2C5CON_SMEN_MASK                       0x00000100
#define _I2C5CON_SMEN_LENGTH                     0x00000001

#define _I2C5CON_DISSLW_POSITION                 0x00000009
#define _I2C5CON_DISSLW_MASK                     0x00000200
#define _I2C5CON_DISSLW_LENGTH                   0x00000001

#define _I2C5CON_A10M_POSITION                   0x0000000A
#define _I2C5CON_A10M_MASK                       0x00000400
#define _I2C5CON_A10M_LENGTH                     0x00000001

#define _I2C5CON_STRICT_POSITION                 0x0000000B
#define _I2C5CON_STRICT_MASK                     0x00000800
#define _I2C5CON_STRICT_LENGTH                   0x00000001

#define _I2C5CON_SCLREL_POSITION                 0x0000000C
#define _I2C5CON_SCLREL_MASK                     0x00001000
#define _I2C5CON_SCLREL_LENGTH                   0x00000001

#define _I2C5CON_SIDL_POSITION                   0x0000000D
#define _I2C5CON_SIDL_MASK                       0x00002000
#define _I2C5CON_SIDL_LENGTH                     0x00000001

#define _I2C5CON_ON_POSITION                     0x0000000F
#define _I2C5CON_ON_MASK                         0x00008000
#define _I2C5CON_ON_LENGTH                       0x00000001

#define _I2C5CON_IPMIEN_POSITION                 0x0000000B
#define _I2C5CON_IPMIEN_MASK                     0x00000800
#define _I2C5CON_IPMIEN_LENGTH                   0x00000001

#define _I2C5CON_I2CSIDL_POSITION                0x0000000D
#define _I2C5CON_I2CSIDL_MASK                    0x00002000
#define _I2C5CON_I2CSIDL_LENGTH                  0x00000001

#define _I2C5CON_I2CEN_POSITION                  0x0000000F
#define _I2C5CON_I2CEN_MASK                      0x00008000
#define _I2C5CON_I2CEN_LENGTH                    0x00000001

#define _I2C5CON_w_POSITION                      0x00000000
#define _I2C5CON_w_MASK                          0xFFFFFFFF
#define _I2C5CON_w_LENGTH                        0x00000020

#define _I2C3ASTAT_TBF_POSITION                  0x00000000
#define _I2C3ASTAT_TBF_MASK                      0x00000001
#define _I2C3ASTAT_TBF_LENGTH                    0x00000001

#define _I2C3ASTAT_RBF_POSITION                  0x00000001
#define _I2C3ASTAT_RBF_MASK                      0x00000002
#define _I2C3ASTAT_RBF_LENGTH                    0x00000001

#define _I2C3ASTAT_R_W_POSITION                  0x00000002
#define _I2C3ASTAT_R_W_MASK                      0x00000004
#define _I2C3ASTAT_R_W_LENGTH                    0x00000001

#define _I2C3ASTAT_S_POSITION                    0x00000003
#define _I2C3ASTAT_S_MASK                        0x00000008
#define _I2C3ASTAT_S_LENGTH                      0x00000001

#define _I2C3ASTAT_P_POSITION                    0x00000004
#define _I2C3ASTAT_P_MASK                        0x00000010
#define _I2C3ASTAT_P_LENGTH                      0x00000001

#define _I2C3ASTAT_D_A_POSITION                  0x00000005
#define _I2C3ASTAT_D_A_MASK                      0x00000020
#define _I2C3ASTAT_D_A_LENGTH                    0x00000001

#define _I2C3ASTAT_I2COV_POSITION                0x00000006
#define _I2C3ASTAT_I2COV_MASK                    0x00000040
#define _I2C3ASTAT_I2COV_LENGTH                  0x00000001

#define _I2C3ASTAT_IWCOL_POSITION                0x00000007
#define _I2C3ASTAT_IWCOL_MASK                    0x00000080
#define _I2C3ASTAT_IWCOL_LENGTH                  0x00000001

#define _I2C3ASTAT_ADD10_POSITION                0x00000008
#define _I2C3ASTAT_ADD10_MASK                    0x00000100
#define _I2C3ASTAT_ADD10_LENGTH                  0x00000001

#define _I2C3ASTAT_GCSTAT_POSITION               0x00000009
#define _I2C3ASTAT_GCSTAT_MASK                   0x00000200
#define _I2C3ASTAT_GCSTAT_LENGTH                 0x00000001

#define _I2C3ASTAT_BCL_POSITION                  0x0000000A
#define _I2C3ASTAT_BCL_MASK                      0x00000400
#define _I2C3ASTAT_BCL_LENGTH                    0x00000001

#define _I2C3ASTAT_TRSTAT_POSITION               0x0000000E
#define _I2C3ASTAT_TRSTAT_MASK                   0x00004000
#define _I2C3ASTAT_TRSTAT_LENGTH                 0x00000001

#define _I2C3ASTAT_ACKSTAT_POSITION              0x0000000F
#define _I2C3ASTAT_ACKSTAT_MASK                  0x00008000
#define _I2C3ASTAT_ACKSTAT_LENGTH                0x00000001

#define _I2C3ASTAT_I2CPOV_POSITION               0x00000006
#define _I2C3ASTAT_I2CPOV_MASK                   0x00000040
#define _I2C3ASTAT_I2CPOV_LENGTH                 0x00000001

#define _I2C3ASTAT_w_POSITION                    0x00000000
#define _I2C3ASTAT_w_MASK                        0xFFFFFFFF
#define _I2C3ASTAT_w_LENGTH                      0x00000020

#define _I2C5STAT_TBF_POSITION                   0x00000000
#define _I2C5STAT_TBF_MASK                       0x00000001
#define _I2C5STAT_TBF_LENGTH                     0x00000001

#define _I2C5STAT_RBF_POSITION                   0x00000001
#define _I2C5STAT_RBF_MASK                       0x00000002
#define _I2C5STAT_RBF_LENGTH                     0x00000001

#define _I2C5STAT_R_W_POSITION                   0x00000002
#define _I2C5STAT_R_W_MASK                       0x00000004
#define _I2C5STAT_R_W_LENGTH                     0x00000001

#define _I2C5STAT_S_POSITION                     0x00000003
#define _I2C5STAT_S_MASK                         0x00000008
#define _I2C5STAT_S_LENGTH                       0x00000001

#define _I2C5STAT_P_POSITION                     0x00000004
#define _I2C5STAT_P_MASK                         0x00000010
#define _I2C5STAT_P_LENGTH                       0x00000001

#define _I2C5STAT_D_A_POSITION                   0x00000005
#define _I2C5STAT_D_A_MASK                       0x00000020
#define _I2C5STAT_D_A_LENGTH                     0x00000001

#define _I2C5STAT_I2COV_POSITION                 0x00000006
#define _I2C5STAT_I2COV_MASK                     0x00000040
#define _I2C5STAT_I2COV_LENGTH                   0x00000001

#define _I2C5STAT_IWCOL_POSITION                 0x00000007
#define _I2C5STAT_IWCOL_MASK                     0x00000080
#define _I2C5STAT_IWCOL_LENGTH                   0x00000001

#define _I2C5STAT_ADD10_POSITION                 0x00000008
#define _I2C5STAT_ADD10_MASK                     0x00000100
#define _I2C5STAT_ADD10_LENGTH                   0x00000001

#define _I2C5STAT_GCSTAT_POSITION                0x00000009
#define _I2C5STAT_GCSTAT_MASK                    0x00000200
#define _I2C5STAT_GCSTAT_LENGTH                  0x00000001

#define _I2C5STAT_BCL_POSITION                   0x0000000A
#define _I2C5STAT_BCL_MASK                       0x00000400
#define _I2C5STAT_BCL_LENGTH                     0x00000001

#define _I2C5STAT_TRSTAT_POSITION                0x0000000E
#define _I2C5STAT_TRSTAT_MASK                    0x00004000
#define _I2C5STAT_TRSTAT_LENGTH                  0x00000001

#define _I2C5STAT_ACKSTAT_POSITION               0x0000000F
#define _I2C5STAT_ACKSTAT_MASK                   0x00008000
#define _I2C5STAT_ACKSTAT_LENGTH                 0x00000001

#define _I2C5STAT_I2CPOV_POSITION                0x00000006
#define _I2C5STAT_I2CPOV_MASK                    0x00000040
#define _I2C5STAT_I2CPOV_LENGTH                  0x00000001

#define _I2C5STAT_w_POSITION                     0x00000000
#define _I2C5STAT_w_MASK                         0xFFFFFFFF
#define _I2C5STAT_w_LENGTH                       0x00000020

#define _I2C1CON_SEN_POSITION                    0x00000000
#define _I2C1CON_SEN_MASK                        0x00000001
#define _I2C1CON_SEN_LENGTH                      0x00000001

#define _I2C1CON_RSEN_POSITION                   0x00000001
#define _I2C1CON_RSEN_MASK                       0x00000002
#define _I2C1CON_RSEN_LENGTH                     0x00000001

#define _I2C1CON_PEN_POSITION                    0x00000002
#define _I2C1CON_PEN_MASK                        0x00000004
#define _I2C1CON_PEN_LENGTH                      0x00000001

#define _I2C1CON_RCEN_POSITION                   0x00000003
#define _I2C1CON_RCEN_MASK                       0x00000008
#define _I2C1CON_RCEN_LENGTH                     0x00000001

#define _I2C1CON_ACKEN_POSITION                  0x00000004
#define _I2C1CON_ACKEN_MASK                      0x00000010
#define _I2C1CON_ACKEN_LENGTH                    0x00000001

#define _I2C1CON_ACKDT_POSITION                  0x00000005
#define _I2C1CON_ACKDT_MASK                      0x00000020
#define _I2C1CON_ACKDT_LENGTH                    0x00000001

#define _I2C1CON_STREN_POSITION                  0x00000006
#define _I2C1CON_STREN_MASK                      0x00000040
#define _I2C1CON_STREN_LENGTH                    0x00000001

#define _I2C1CON_GCEN_POSITION                   0x00000007
#define _I2C1CON_GCEN_MASK                       0x00000080
#define _I2C1CON_GCEN_LENGTH                     0x00000001

#define _I2C1CON_SMEN_POSITION                   0x00000008
#define _I2C1CON_SMEN_MASK                       0x00000100
#define _I2C1CON_SMEN_LENGTH                     0x00000001

#define _I2C1CON_DISSLW_POSITION                 0x00000009
#define _I2C1CON_DISSLW_MASK                     0x00000200
#define _I2C1CON_DISSLW_LENGTH                   0x00000001

#define _I2C1CON_A10M_POSITION                   0x0000000A
#define _I2C1CON_A10M_MASK                       0x00000400
#define _I2C1CON_A10M_LENGTH                     0x00000001

#define _I2C1CON_STRICT_POSITION                 0x0000000B
#define _I2C1CON_STRICT_MASK                     0x00000800
#define _I2C1CON_STRICT_LENGTH                   0x00000001

#define _I2C1CON_SCLREL_POSITION                 0x0000000C
#define _I2C1CON_SCLREL_MASK                     0x00001000
#define _I2C1CON_SCLREL_LENGTH                   0x00000001

#define _I2C1CON_SIDL_POSITION                   0x0000000D
#define _I2C1CON_SIDL_MASK                       0x00002000
#define _I2C1CON_SIDL_LENGTH                     0x00000001

#define _I2C1CON_ON_POSITION                     0x0000000F
#define _I2C1CON_ON_MASK                         0x00008000
#define _I2C1CON_ON_LENGTH                       0x00000001

#define _I2C1CON_IPMIEN_POSITION                 0x0000000B
#define _I2C1CON_IPMIEN_MASK                     0x00000800
#define _I2C1CON_IPMIEN_LENGTH                   0x00000001

#define _I2C1CON_I2CSIDL_POSITION                0x0000000D
#define _I2C1CON_I2CSIDL_MASK                    0x00002000
#define _I2C1CON_I2CSIDL_LENGTH                  0x00000001

#define _I2C1CON_I2CEN_POSITION                  0x0000000F
#define _I2C1CON_I2CEN_MASK                      0x00008000
#define _I2C1CON_I2CEN_LENGTH                    0x00000001

#define _I2C1CON_w_POSITION                      0x00000000
#define _I2C1CON_w_MASK                          0xFFFFFFFF
#define _I2C1CON_w_LENGTH                        0x00000020

#define _I2C1STAT_TBF_POSITION                   0x00000000
#define _I2C1STAT_TBF_MASK                       0x00000001
#define _I2C1STAT_TBF_LENGTH                     0x00000001

#define _I2C1STAT_RBF_POSITION                   0x00000001
#define _I2C1STAT_RBF_MASK                       0x00000002
#define _I2C1STAT_RBF_LENGTH                     0x00000001

#define _I2C1STAT_R_W_POSITION                   0x00000002
#define _I2C1STAT_R_W_MASK                       0x00000004
#define _I2C1STAT_R_W_LENGTH                     0x00000001

#define _I2C1STAT_S_POSITION                     0x00000003
#define _I2C1STAT_S_MASK                         0x00000008
#define _I2C1STAT_S_LENGTH                       0x00000001

#define _I2C1STAT_P_POSITION                     0x00000004
#define _I2C1STAT_P_MASK                         0x00000010
#define _I2C1STAT_P_LENGTH                       0x00000001

#define _I2C1STAT_D_A_POSITION                   0x00000005
#define _I2C1STAT_D_A_MASK                       0x00000020
#define _I2C1STAT_D_A_LENGTH                     0x00000001

#define _I2C1STAT_I2COV_POSITION                 0x00000006
#define _I2C1STAT_I2COV_MASK                     0x00000040
#define _I2C1STAT_I2COV_LENGTH                   0x00000001

#define _I2C1STAT_IWCOL_POSITION                 0x00000007
#define _I2C1STAT_IWCOL_MASK                     0x00000080
#define _I2C1STAT_IWCOL_LENGTH                   0x00000001

#define _I2C1STAT_ADD10_POSITION                 0x00000008
#define _I2C1STAT_ADD10_MASK                     0x00000100
#define _I2C1STAT_ADD10_LENGTH                   0x00000001

#define _I2C1STAT_GCSTAT_POSITION                0x00000009
#define _I2C1STAT_GCSTAT_MASK                    0x00000200
#define _I2C1STAT_GCSTAT_LENGTH                  0x00000001

#define _I2C1STAT_BCL_POSITION                   0x0000000A
#define _I2C1STAT_BCL_MASK                       0x00000400
#define _I2C1STAT_BCL_LENGTH                     0x00000001

#define _I2C1STAT_TRSTAT_POSITION                0x0000000E
#define _I2C1STAT_TRSTAT_MASK                    0x00004000
#define _I2C1STAT_TRSTAT_LENGTH                  0x00000001

#define _I2C1STAT_ACKSTAT_POSITION               0x0000000F
#define _I2C1STAT_ACKSTAT_MASK                   0x00008000
#define _I2C1STAT_ACKSTAT_LENGTH                 0x00000001

#define _I2C1STAT_I2CPOV_POSITION                0x00000006
#define _I2C1STAT_I2CPOV_MASK                    0x00000040
#define _I2C1STAT_I2CPOV_LENGTH                  0x00000001

#define _I2C1STAT_w_POSITION                     0x00000000
#define _I2C1STAT_w_MASK                         0xFFFFFFFF
#define _I2C1STAT_w_LENGTH                       0x00000020

#define _SPI1ACON_SRXISEL_POSITION               0x00000000
#define _SPI1ACON_SRXISEL_MASK                   0x00000003
#define _SPI1ACON_SRXISEL_LENGTH                 0x00000002

#define _SPI1ACON_STXISEL_POSITION               0x00000002
#define _SPI1ACON_STXISEL_MASK                   0x0000000C
#define _SPI1ACON_STXISEL_LENGTH                 0x00000002

#define _SPI1ACON_MSTEN_POSITION                 0x00000005
#define _SPI1ACON_MSTEN_MASK                     0x00000020
#define _SPI1ACON_MSTEN_LENGTH                   0x00000001

#define _SPI1ACON_CKP_POSITION                   0x00000006
#define _SPI1ACON_CKP_MASK                       0x00000040
#define _SPI1ACON_CKP_LENGTH                     0x00000001

#define _SPI1ACON_SSEN_POSITION                  0x00000007
#define _SPI1ACON_SSEN_MASK                      0x00000080
#define _SPI1ACON_SSEN_LENGTH                    0x00000001

#define _SPI1ACON_CKE_POSITION                   0x00000008
#define _SPI1ACON_CKE_MASK                       0x00000100
#define _SPI1ACON_CKE_LENGTH                     0x00000001

#define _SPI1ACON_SMP_POSITION                   0x00000009
#define _SPI1ACON_SMP_MASK                       0x00000200
#define _SPI1ACON_SMP_LENGTH                     0x00000001

#define _SPI1ACON_MODE16_POSITION                0x0000000A
#define _SPI1ACON_MODE16_MASK                    0x00000400
#define _SPI1ACON_MODE16_LENGTH                  0x00000001

#define _SPI1ACON_MODE32_POSITION                0x0000000B
#define _SPI1ACON_MODE32_MASK                    0x00000800
#define _SPI1ACON_MODE32_LENGTH                  0x00000001

#define _SPI1ACON_DISSDO_POSITION                0x0000000C
#define _SPI1ACON_DISSDO_MASK                    0x00001000
#define _SPI1ACON_DISSDO_LENGTH                  0x00000001

#define _SPI1ACON_SIDL_POSITION                  0x0000000D
#define _SPI1ACON_SIDL_MASK                      0x00002000
#define _SPI1ACON_SIDL_LENGTH                    0x00000001

#define _SPI1ACON_ON_POSITION                    0x0000000F
#define _SPI1ACON_ON_MASK                        0x00008000
#define _SPI1ACON_ON_LENGTH                      0x00000001

#define _SPI1ACON_ENHBUF_POSITION                0x00000010
#define _SPI1ACON_ENHBUF_MASK                    0x00010000
#define _SPI1ACON_ENHBUF_LENGTH                  0x00000001

#define _SPI1ACON_SPIFE_POSITION                 0x00000011
#define _SPI1ACON_SPIFE_MASK                     0x00020000
#define _SPI1ACON_SPIFE_LENGTH                   0x00000001

#define _SPI1ACON_FRMCNT_POSITION                0x00000018
#define _SPI1ACON_FRMCNT_MASK                    0x07000000
#define _SPI1ACON_FRMCNT_LENGTH                  0x00000003

#define _SPI1ACON_FRMSYPW_POSITION               0x0000001B
#define _SPI1ACON_FRMSYPW_MASK                   0x08000000
#define _SPI1ACON_FRMSYPW_LENGTH                 0x00000001

#define _SPI1ACON_MSSEN_POSITION                 0x0000001C
#define _SPI1ACON_MSSEN_MASK                     0x10000000
#define _SPI1ACON_MSSEN_LENGTH                   0x00000001

#define _SPI1ACON_FRMPOL_POSITION                0x0000001D
#define _SPI1ACON_FRMPOL_MASK                    0x20000000
#define _SPI1ACON_FRMPOL_LENGTH                  0x00000001

#define _SPI1ACON_FRMSYNC_POSITION               0x0000001E
#define _SPI1ACON_FRMSYNC_MASK                   0x40000000
#define _SPI1ACON_FRMSYNC_LENGTH                 0x00000001

#define _SPI1ACON_FRMEN_POSITION                 0x0000001F
#define _SPI1ACON_FRMEN_MASK                     0x80000000
#define _SPI1ACON_FRMEN_LENGTH                   0x00000001

#define _SPI1ACON_w_POSITION                     0x00000000
#define _SPI1ACON_w_MASK                         0xFFFFFFFF
#define _SPI1ACON_w_LENGTH                       0x00000020

#define _SPI3CON_SRXISEL_POSITION                0x00000000
#define _SPI3CON_SRXISEL_MASK                    0x00000003
#define _SPI3CON_SRXISEL_LENGTH                  0x00000002

#define _SPI3CON_STXISEL_POSITION                0x00000002
#define _SPI3CON_STXISEL_MASK                    0x0000000C
#define _SPI3CON_STXISEL_LENGTH                  0x00000002

#define _SPI3CON_MSTEN_POSITION                  0x00000005
#define _SPI3CON_MSTEN_MASK                      0x00000020
#define _SPI3CON_MSTEN_LENGTH                    0x00000001

#define _SPI3CON_CKP_POSITION                    0x00000006
#define _SPI3CON_CKP_MASK                        0x00000040
#define _SPI3CON_CKP_LENGTH                      0x00000001

#define _SPI3CON_SSEN_POSITION                   0x00000007
#define _SPI3CON_SSEN_MASK                       0x00000080
#define _SPI3CON_SSEN_LENGTH                     0x00000001

#define _SPI3CON_CKE_POSITION                    0x00000008
#define _SPI3CON_CKE_MASK                        0x00000100
#define _SPI3CON_CKE_LENGTH                      0x00000001

#define _SPI3CON_SMP_POSITION                    0x00000009
#define _SPI3CON_SMP_MASK                        0x00000200
#define _SPI3CON_SMP_LENGTH                      0x00000001

#define _SPI3CON_MODE16_POSITION                 0x0000000A
#define _SPI3CON_MODE16_MASK                     0x00000400
#define _SPI3CON_MODE16_LENGTH                   0x00000001

#define _SPI3CON_MODE32_POSITION                 0x0000000B
#define _SPI3CON_MODE32_MASK                     0x00000800
#define _SPI3CON_MODE32_LENGTH                   0x00000001

#define _SPI3CON_DISSDO_POSITION                 0x0000000C
#define _SPI3CON_DISSDO_MASK                     0x00001000
#define _SPI3CON_DISSDO_LENGTH                   0x00000001

#define _SPI3CON_SIDL_POSITION                   0x0000000D
#define _SPI3CON_SIDL_MASK                       0x00002000
#define _SPI3CON_SIDL_LENGTH                     0x00000001

#define _SPI3CON_ON_POSITION                     0x0000000F
#define _SPI3CON_ON_MASK                         0x00008000
#define _SPI3CON_ON_LENGTH                       0x00000001

#define _SPI3CON_ENHBUF_POSITION                 0x00000010
#define _SPI3CON_ENHBUF_MASK                     0x00010000
#define _SPI3CON_ENHBUF_LENGTH                   0x00000001

#define _SPI3CON_SPIFE_POSITION                  0x00000011
#define _SPI3CON_SPIFE_MASK                      0x00020000
#define _SPI3CON_SPIFE_LENGTH                    0x00000001

#define _SPI3CON_FRMCNT_POSITION                 0x00000018
#define _SPI3CON_FRMCNT_MASK                     0x07000000
#define _SPI3CON_FRMCNT_LENGTH                   0x00000003

#define _SPI3CON_FRMSYPW_POSITION                0x0000001B
#define _SPI3CON_FRMSYPW_MASK                    0x08000000
#define _SPI3CON_FRMSYPW_LENGTH                  0x00000001

#define _SPI3CON_MSSEN_POSITION                  0x0000001C
#define _SPI3CON_MSSEN_MASK                      0x10000000
#define _SPI3CON_MSSEN_LENGTH                    0x00000001

#define _SPI3CON_FRMPOL_POSITION                 0x0000001D
#define _SPI3CON_FRMPOL_MASK                     0x20000000
#define _SPI3CON_FRMPOL_LENGTH                   0x00000001

#define _SPI3CON_FRMSYNC_POSITION                0x0000001E
#define _SPI3CON_FRMSYNC_MASK                    0x40000000
#define _SPI3CON_FRMSYNC_LENGTH                  0x00000001

#define _SPI3CON_FRMEN_POSITION                  0x0000001F
#define _SPI3CON_FRMEN_MASK                      0x80000000
#define _SPI3CON_FRMEN_LENGTH                    0x00000001

#define _SPI3CON_w_POSITION                      0x00000000
#define _SPI3CON_w_MASK                          0xFFFFFFFF
#define _SPI3CON_w_LENGTH                        0x00000020

#define _SPI1ASTAT_SPIRBF_POSITION               0x00000000
#define _SPI1ASTAT_SPIRBF_MASK                   0x00000001
#define _SPI1ASTAT_SPIRBF_LENGTH                 0x00000001

#define _SPI1ASTAT_SPITBF_POSITION               0x00000001
#define _SPI1ASTAT_SPITBF_MASK                   0x00000002
#define _SPI1ASTAT_SPITBF_LENGTH                 0x00000001

#define _SPI1ASTAT_SPITBE_POSITION               0x00000003
#define _SPI1ASTAT_SPITBE_MASK                   0x00000008
#define _SPI1ASTAT_SPITBE_LENGTH                 0x00000001

#define _SPI1ASTAT_SPIRBE_POSITION               0x00000005
#define _SPI1ASTAT_SPIRBE_MASK                   0x00000020
#define _SPI1ASTAT_SPIRBE_LENGTH                 0x00000001

#define _SPI1ASTAT_SPIROV_POSITION               0x00000006
#define _SPI1ASTAT_SPIROV_MASK                   0x00000040
#define _SPI1ASTAT_SPIROV_LENGTH                 0x00000001

#define _SPI1ASTAT_SRMT_POSITION                 0x00000007
#define _SPI1ASTAT_SRMT_MASK                     0x00000080
#define _SPI1ASTAT_SRMT_LENGTH                   0x00000001

#define _SPI1ASTAT_SPITUR_POSITION               0x00000008
#define _SPI1ASTAT_SPITUR_MASK                   0x00000100
#define _SPI1ASTAT_SPITUR_LENGTH                 0x00000001

#define _SPI1ASTAT_SPIBUSY_POSITION              0x0000000B
#define _SPI1ASTAT_SPIBUSY_MASK                  0x00000800
#define _SPI1ASTAT_SPIBUSY_LENGTH                0x00000001

#define _SPI1ASTAT_TXBUFELM_POSITION             0x00000010
#define _SPI1ASTAT_TXBUFELM_MASK                 0x001F0000
#define _SPI1ASTAT_TXBUFELM_LENGTH               0x00000005

#define _SPI1ASTAT_RXBUFELM_POSITION             0x00000018
#define _SPI1ASTAT_RXBUFELM_MASK                 0x1F000000
#define _SPI1ASTAT_RXBUFELM_LENGTH               0x00000005

#define _SPI1ASTAT_w_POSITION                    0x00000000
#define _SPI1ASTAT_w_MASK                        0xFFFFFFFF
#define _SPI1ASTAT_w_LENGTH                      0x00000020

#define _SPI3STAT_SPIRBF_POSITION                0x00000000
#define _SPI3STAT_SPIRBF_MASK                    0x00000001
#define _SPI3STAT_SPIRBF_LENGTH                  0x00000001

#define _SPI3STAT_SPITBF_POSITION                0x00000001
#define _SPI3STAT_SPITBF_MASK                    0x00000002
#define _SPI3STAT_SPITBF_LENGTH                  0x00000001

#define _SPI3STAT_SPITBE_POSITION                0x00000003
#define _SPI3STAT_SPITBE_MASK                    0x00000008
#define _SPI3STAT_SPITBE_LENGTH                  0x00000001

#define _SPI3STAT_SPIRBE_POSITION                0x00000005
#define _SPI3STAT_SPIRBE_MASK                    0x00000020
#define _SPI3STAT_SPIRBE_LENGTH                  0x00000001

#define _SPI3STAT_SPIROV_POSITION                0x00000006
#define _SPI3STAT_SPIROV_MASK                    0x00000040
#define _SPI3STAT_SPIROV_LENGTH                  0x00000001

#define _SPI3STAT_SRMT_POSITION                  0x00000007
#define _SPI3STAT_SRMT_MASK                      0x00000080
#define _SPI3STAT_SRMT_LENGTH                    0x00000001

#define _SPI3STAT_SPITUR_POSITION                0x00000008
#define _SPI3STAT_SPITUR_MASK                    0x00000100
#define _SPI3STAT_SPITUR_LENGTH                  0x00000001

#define _SPI3STAT_SPIBUSY_POSITION               0x0000000B
#define _SPI3STAT_SPIBUSY_MASK                   0x00000800
#define _SPI3STAT_SPIBUSY_LENGTH                 0x00000001

#define _SPI3STAT_TXBUFELM_POSITION              0x00000010
#define _SPI3STAT_TXBUFELM_MASK                  0x001F0000
#define _SPI3STAT_TXBUFELM_LENGTH                0x00000005

#define _SPI3STAT_RXBUFELM_POSITION              0x00000018
#define _SPI3STAT_RXBUFELM_MASK                  0x1F000000
#define _SPI3STAT_RXBUFELM_LENGTH                0x00000005

#define _SPI3STAT_w_POSITION                     0x00000000
#define _SPI3STAT_w_MASK                         0xFFFFFFFF
#define _SPI3STAT_w_LENGTH                       0x00000020

#define _SPI2ACON_SRXISEL_POSITION               0x00000000
#define _SPI2ACON_SRXISEL_MASK                   0x00000003
#define _SPI2ACON_SRXISEL_LENGTH                 0x00000002

#define _SPI2ACON_STXISEL_POSITION               0x00000002
#define _SPI2ACON_STXISEL_MASK                   0x0000000C
#define _SPI2ACON_STXISEL_LENGTH                 0x00000002

#define _SPI2ACON_MSTEN_POSITION                 0x00000005
#define _SPI2ACON_MSTEN_MASK                     0x00000020
#define _SPI2ACON_MSTEN_LENGTH                   0x00000001

#define _SPI2ACON_CKP_POSITION                   0x00000006
#define _SPI2ACON_CKP_MASK                       0x00000040
#define _SPI2ACON_CKP_LENGTH                     0x00000001

#define _SPI2ACON_SSEN_POSITION                  0x00000007
#define _SPI2ACON_SSEN_MASK                      0x00000080
#define _SPI2ACON_SSEN_LENGTH                    0x00000001

#define _SPI2ACON_CKE_POSITION                   0x00000008
#define _SPI2ACON_CKE_MASK                       0x00000100
#define _SPI2ACON_CKE_LENGTH                     0x00000001

#define _SPI2ACON_SMP_POSITION                   0x00000009
#define _SPI2ACON_SMP_MASK                       0x00000200
#define _SPI2ACON_SMP_LENGTH                     0x00000001

#define _SPI2ACON_MODE16_POSITION                0x0000000A
#define _SPI2ACON_MODE16_MASK                    0x00000400
#define _SPI2ACON_MODE16_LENGTH                  0x00000001

#define _SPI2ACON_MODE32_POSITION                0x0000000B
#define _SPI2ACON_MODE32_MASK                    0x00000800
#define _SPI2ACON_MODE32_LENGTH                  0x00000001

#define _SPI2ACON_DISSDO_POSITION                0x0000000C
#define _SPI2ACON_DISSDO_MASK                    0x00001000
#define _SPI2ACON_DISSDO_LENGTH                  0x00000001

#define _SPI2ACON_SIDL_POSITION                  0x0000000D
#define _SPI2ACON_SIDL_MASK                      0x00002000
#define _SPI2ACON_SIDL_LENGTH                    0x00000001

#define _SPI2ACON_ON_POSITION                    0x0000000F
#define _SPI2ACON_ON_MASK                        0x00008000
#define _SPI2ACON_ON_LENGTH                      0x00000001

#define _SPI2ACON_ENHBUF_POSITION                0x00000010
#define _SPI2ACON_ENHBUF_MASK                    0x00010000
#define _SPI2ACON_ENHBUF_LENGTH                  0x00000001

#define _SPI2ACON_SPIFE_POSITION                 0x00000011
#define _SPI2ACON_SPIFE_MASK                     0x00020000
#define _SPI2ACON_SPIFE_LENGTH                   0x00000001

#define _SPI2ACON_FRMCNT_POSITION                0x00000018
#define _SPI2ACON_FRMCNT_MASK                    0x07000000
#define _SPI2ACON_FRMCNT_LENGTH                  0x00000003

#define _SPI2ACON_FRMSYPW_POSITION               0x0000001B
#define _SPI2ACON_FRMSYPW_MASK                   0x08000000
#define _SPI2ACON_FRMSYPW_LENGTH                 0x00000001

#define _SPI2ACON_MSSEN_POSITION                 0x0000001C
#define _SPI2ACON_MSSEN_MASK                     0x10000000
#define _SPI2ACON_MSSEN_LENGTH                   0x00000001

#define _SPI2ACON_FRMPOL_POSITION                0x0000001D
#define _SPI2ACON_FRMPOL_MASK                    0x20000000
#define _SPI2ACON_FRMPOL_LENGTH                  0x00000001

#define _SPI2ACON_FRMSYNC_POSITION               0x0000001E
#define _SPI2ACON_FRMSYNC_MASK                   0x40000000
#define _SPI2ACON_FRMSYNC_LENGTH                 0x00000001

#define _SPI2ACON_FRMEN_POSITION                 0x0000001F
#define _SPI2ACON_FRMEN_MASK                     0x80000000
#define _SPI2ACON_FRMEN_LENGTH                   0x00000001

#define _SPI2ACON_w_POSITION                     0x00000000
#define _SPI2ACON_w_MASK                         0xFFFFFFFF
#define _SPI2ACON_w_LENGTH                       0x00000020

#define _SPI2CON_SRXISEL_POSITION                0x00000000
#define _SPI2CON_SRXISEL_MASK                    0x00000003
#define _SPI2CON_SRXISEL_LENGTH                  0x00000002

#define _SPI2CON_STXISEL_POSITION                0x00000002
#define _SPI2CON_STXISEL_MASK                    0x0000000C
#define _SPI2CON_STXISEL_LENGTH                  0x00000002

#define _SPI2CON_MSTEN_POSITION                  0x00000005
#define _SPI2CON_MSTEN_MASK                      0x00000020
#define _SPI2CON_MSTEN_LENGTH                    0x00000001

#define _SPI2CON_CKP_POSITION                    0x00000006
#define _SPI2CON_CKP_MASK                        0x00000040
#define _SPI2CON_CKP_LENGTH                      0x00000001

#define _SPI2CON_SSEN_POSITION                   0x00000007
#define _SPI2CON_SSEN_MASK                       0x00000080
#define _SPI2CON_SSEN_LENGTH                     0x00000001

#define _SPI2CON_CKE_POSITION                    0x00000008
#define _SPI2CON_CKE_MASK                        0x00000100
#define _SPI2CON_CKE_LENGTH                      0x00000001

#define _SPI2CON_SMP_POSITION                    0x00000009
#define _SPI2CON_SMP_MASK                        0x00000200
#define _SPI2CON_SMP_LENGTH                      0x00000001

#define _SPI2CON_MODE16_POSITION                 0x0000000A
#define _SPI2CON_MODE16_MASK                     0x00000400
#define _SPI2CON_MODE16_LENGTH                   0x00000001

#define _SPI2CON_MODE32_POSITION                 0x0000000B
#define _SPI2CON_MODE32_MASK                     0x00000800
#define _SPI2CON_MODE32_LENGTH                   0x00000001

#define _SPI2CON_DISSDO_POSITION                 0x0000000C
#define _SPI2CON_DISSDO_MASK                     0x00001000
#define _SPI2CON_DISSDO_LENGTH                   0x00000001

#define _SPI2CON_SIDL_POSITION                   0x0000000D
#define _SPI2CON_SIDL_MASK                       0x00002000
#define _SPI2CON_SIDL_LENGTH                     0x00000001

#define _SPI2CON_ON_POSITION                     0x0000000F
#define _SPI2CON_ON_MASK                         0x00008000
#define _SPI2CON_ON_LENGTH                       0x00000001

#define _SPI2CON_ENHBUF_POSITION                 0x00000010
#define _SPI2CON_ENHBUF_MASK                     0x00010000
#define _SPI2CON_ENHBUF_LENGTH                   0x00000001

#define _SPI2CON_SPIFE_POSITION                  0x00000011
#define _SPI2CON_SPIFE_MASK                      0x00020000
#define _SPI2CON_SPIFE_LENGTH                    0x00000001

#define _SPI2CON_FRMCNT_POSITION                 0x00000018
#define _SPI2CON_FRMCNT_MASK                     0x07000000
#define _SPI2CON_FRMCNT_LENGTH                   0x00000003

#define _SPI2CON_FRMSYPW_POSITION                0x0000001B
#define _SPI2CON_FRMSYPW_MASK                    0x08000000
#define _SPI2CON_FRMSYPW_LENGTH                  0x00000001

#define _SPI2CON_MSSEN_POSITION                  0x0000001C
#define _SPI2CON_MSSEN_MASK                      0x10000000
#define _SPI2CON_MSSEN_LENGTH                    0x00000001

#define _SPI2CON_FRMPOL_POSITION                 0x0000001D
#define _SPI2CON_FRMPOL_MASK                     0x20000000
#define _SPI2CON_FRMPOL_LENGTH                   0x00000001

#define _SPI2CON_FRMSYNC_POSITION                0x0000001E
#define _SPI2CON_FRMSYNC_MASK                    0x40000000
#define _SPI2CON_FRMSYNC_LENGTH                  0x00000001

#define _SPI2CON_FRMEN_POSITION                  0x0000001F
#define _SPI2CON_FRMEN_MASK                      0x80000000
#define _SPI2CON_FRMEN_LENGTH                    0x00000001

#define _SPI2CON_w_POSITION                      0x00000000
#define _SPI2CON_w_MASK                          0xFFFFFFFF
#define _SPI2CON_w_LENGTH                        0x00000020

#define _SPI2ASTAT_SPIRBF_POSITION               0x00000000
#define _SPI2ASTAT_SPIRBF_MASK                   0x00000001
#define _SPI2ASTAT_SPIRBF_LENGTH                 0x00000001

#define _SPI2ASTAT_SPITBF_POSITION               0x00000001
#define _SPI2ASTAT_SPITBF_MASK                   0x00000002
#define _SPI2ASTAT_SPITBF_LENGTH                 0x00000001

#define _SPI2ASTAT_SPITBE_POSITION               0x00000003
#define _SPI2ASTAT_SPITBE_MASK                   0x00000008
#define _SPI2ASTAT_SPITBE_LENGTH                 0x00000001

#define _SPI2ASTAT_SPIRBE_POSITION               0x00000005
#define _SPI2ASTAT_SPIRBE_MASK                   0x00000020
#define _SPI2ASTAT_SPIRBE_LENGTH                 0x00000001

#define _SPI2ASTAT_SPIROV_POSITION               0x00000006
#define _SPI2ASTAT_SPIROV_MASK                   0x00000040
#define _SPI2ASTAT_SPIROV_LENGTH                 0x00000001

#define _SPI2ASTAT_SRMT_POSITION                 0x00000007
#define _SPI2ASTAT_SRMT_MASK                     0x00000080
#define _SPI2ASTAT_SRMT_LENGTH                   0x00000001

#define _SPI2ASTAT_SPITUR_POSITION               0x00000008
#define _SPI2ASTAT_SPITUR_MASK                   0x00000100
#define _SPI2ASTAT_SPITUR_LENGTH                 0x00000001

#define _SPI2ASTAT_SPIBUSY_POSITION              0x0000000B
#define _SPI2ASTAT_SPIBUSY_MASK                  0x00000800
#define _SPI2ASTAT_SPIBUSY_LENGTH                0x00000001

#define _SPI2ASTAT_TXBUFELM_POSITION             0x00000010
#define _SPI2ASTAT_TXBUFELM_MASK                 0x001F0000
#define _SPI2ASTAT_TXBUFELM_LENGTH               0x00000005

#define _SPI2ASTAT_RXBUFELM_POSITION             0x00000018
#define _SPI2ASTAT_RXBUFELM_MASK                 0x1F000000
#define _SPI2ASTAT_RXBUFELM_LENGTH               0x00000005

#define _SPI2ASTAT_w_POSITION                    0x00000000
#define _SPI2ASTAT_w_MASK                        0xFFFFFFFF
#define _SPI2ASTAT_w_LENGTH                      0x00000020

#define _SPI2STAT_SPIRBF_POSITION                0x00000000
#define _SPI2STAT_SPIRBF_MASK                    0x00000001
#define _SPI2STAT_SPIRBF_LENGTH                  0x00000001

#define _SPI2STAT_SPITBF_POSITION                0x00000001
#define _SPI2STAT_SPITBF_MASK                    0x00000002
#define _SPI2STAT_SPITBF_LENGTH                  0x00000001

#define _SPI2STAT_SPITBE_POSITION                0x00000003
#define _SPI2STAT_SPITBE_MASK                    0x00000008
#define _SPI2STAT_SPITBE_LENGTH                  0x00000001

#define _SPI2STAT_SPIRBE_POSITION                0x00000005
#define _SPI2STAT_SPIRBE_MASK                    0x00000020
#define _SPI2STAT_SPIRBE_LENGTH                  0x00000001

#define _SPI2STAT_SPIROV_POSITION                0x00000006
#define _SPI2STAT_SPIROV_MASK                    0x00000040
#define _SPI2STAT_SPIROV_LENGTH                  0x00000001

#define _SPI2STAT_SRMT_POSITION                  0x00000007
#define _SPI2STAT_SRMT_MASK                      0x00000080
#define _SPI2STAT_SRMT_LENGTH                    0x00000001

#define _SPI2STAT_SPITUR_POSITION                0x00000008
#define _SPI2STAT_SPITUR_MASK                    0x00000100
#define _SPI2STAT_SPITUR_LENGTH                  0x00000001

#define _SPI2STAT_SPIBUSY_POSITION               0x0000000B
#define _SPI2STAT_SPIBUSY_MASK                   0x00000800
#define _SPI2STAT_SPIBUSY_LENGTH                 0x00000001

#define _SPI2STAT_TXBUFELM_POSITION              0x00000010
#define _SPI2STAT_TXBUFELM_MASK                  0x001F0000
#define _SPI2STAT_TXBUFELM_LENGTH                0x00000005

#define _SPI2STAT_RXBUFELM_POSITION              0x00000018
#define _SPI2STAT_RXBUFELM_MASK                  0x1F000000
#define _SPI2STAT_RXBUFELM_LENGTH                0x00000005

#define _SPI2STAT_w_POSITION                     0x00000000
#define _SPI2STAT_w_MASK                         0xFFFFFFFF
#define _SPI2STAT_w_LENGTH                       0x00000020

#define _SPI3ACON_SRXISEL_POSITION               0x00000000
#define _SPI3ACON_SRXISEL_MASK                   0x00000003
#define _SPI3ACON_SRXISEL_LENGTH                 0x00000002

#define _SPI3ACON_STXISEL_POSITION               0x00000002
#define _SPI3ACON_STXISEL_MASK                   0x0000000C
#define _SPI3ACON_STXISEL_LENGTH                 0x00000002

#define _SPI3ACON_MSTEN_POSITION                 0x00000005
#define _SPI3ACON_MSTEN_MASK                     0x00000020
#define _SPI3ACON_MSTEN_LENGTH                   0x00000001

#define _SPI3ACON_CKP_POSITION                   0x00000006
#define _SPI3ACON_CKP_MASK                       0x00000040
#define _SPI3ACON_CKP_LENGTH                     0x00000001

#define _SPI3ACON_SSEN_POSITION                  0x00000007
#define _SPI3ACON_SSEN_MASK                      0x00000080
#define _SPI3ACON_SSEN_LENGTH                    0x00000001

#define _SPI3ACON_CKE_POSITION                   0x00000008
#define _SPI3ACON_CKE_MASK                       0x00000100
#define _SPI3ACON_CKE_LENGTH                     0x00000001

#define _SPI3ACON_SMP_POSITION                   0x00000009
#define _SPI3ACON_SMP_MASK                       0x00000200
#define _SPI3ACON_SMP_LENGTH                     0x00000001

#define _SPI3ACON_MODE16_POSITION                0x0000000A
#define _SPI3ACON_MODE16_MASK                    0x00000400
#define _SPI3ACON_MODE16_LENGTH                  0x00000001

#define _SPI3ACON_MODE32_POSITION                0x0000000B
#define _SPI3ACON_MODE32_MASK                    0x00000800
#define _SPI3ACON_MODE32_LENGTH                  0x00000001

#define _SPI3ACON_DISSDO_POSITION                0x0000000C
#define _SPI3ACON_DISSDO_MASK                    0x00001000
#define _SPI3ACON_DISSDO_LENGTH                  0x00000001

#define _SPI3ACON_SIDL_POSITION                  0x0000000D
#define _SPI3ACON_SIDL_MASK                      0x00002000
#define _SPI3ACON_SIDL_LENGTH                    0x00000001

#define _SPI3ACON_ON_POSITION                    0x0000000F
#define _SPI3ACON_ON_MASK                        0x00008000
#define _SPI3ACON_ON_LENGTH                      0x00000001

#define _SPI3ACON_ENHBUF_POSITION                0x00000010
#define _SPI3ACON_ENHBUF_MASK                    0x00010000
#define _SPI3ACON_ENHBUF_LENGTH                  0x00000001

#define _SPI3ACON_SPIFE_POSITION                 0x00000011
#define _SPI3ACON_SPIFE_MASK                     0x00020000
#define _SPI3ACON_SPIFE_LENGTH                   0x00000001

#define _SPI3ACON_FRMCNT_POSITION                0x00000018
#define _SPI3ACON_FRMCNT_MASK                    0x07000000
#define _SPI3ACON_FRMCNT_LENGTH                  0x00000003

#define _SPI3ACON_FRMSYPW_POSITION               0x0000001B
#define _SPI3ACON_FRMSYPW_MASK                   0x08000000
#define _SPI3ACON_FRMSYPW_LENGTH                 0x00000001

#define _SPI3ACON_MSSEN_POSITION                 0x0000001C
#define _SPI3ACON_MSSEN_MASK                     0x10000000
#define _SPI3ACON_MSSEN_LENGTH                   0x00000001

#define _SPI3ACON_FRMPOL_POSITION                0x0000001D
#define _SPI3ACON_FRMPOL_MASK                    0x20000000
#define _SPI3ACON_FRMPOL_LENGTH                  0x00000001

#define _SPI3ACON_FRMSYNC_POSITION               0x0000001E
#define _SPI3ACON_FRMSYNC_MASK                   0x40000000
#define _SPI3ACON_FRMSYNC_LENGTH                 0x00000001

#define _SPI3ACON_FRMEN_POSITION                 0x0000001F
#define _SPI3ACON_FRMEN_MASK                     0x80000000
#define _SPI3ACON_FRMEN_LENGTH                   0x00000001

#define _SPI3ACON_w_POSITION                     0x00000000
#define _SPI3ACON_w_MASK                         0xFFFFFFFF
#define _SPI3ACON_w_LENGTH                       0x00000020

#define _SPI4CON_SRXISEL_POSITION                0x00000000
#define _SPI4CON_SRXISEL_MASK                    0x00000003
#define _SPI4CON_SRXISEL_LENGTH                  0x00000002

#define _SPI4CON_STXISEL_POSITION                0x00000002
#define _SPI4CON_STXISEL_MASK                    0x0000000C
#define _SPI4CON_STXISEL_LENGTH                  0x00000002

#define _SPI4CON_MSTEN_POSITION                  0x00000005
#define _SPI4CON_MSTEN_MASK                      0x00000020
#define _SPI4CON_MSTEN_LENGTH                    0x00000001

#define _SPI4CON_CKP_POSITION                    0x00000006
#define _SPI4CON_CKP_MASK                        0x00000040
#define _SPI4CON_CKP_LENGTH                      0x00000001

#define _SPI4CON_SSEN_POSITION                   0x00000007
#define _SPI4CON_SSEN_MASK                       0x00000080
#define _SPI4CON_SSEN_LENGTH                     0x00000001

#define _SPI4CON_CKE_POSITION                    0x00000008
#define _SPI4CON_CKE_MASK                        0x00000100
#define _SPI4CON_CKE_LENGTH                      0x00000001

#define _SPI4CON_SMP_POSITION                    0x00000009
#define _SPI4CON_SMP_MASK                        0x00000200
#define _SPI4CON_SMP_LENGTH                      0x00000001

#define _SPI4CON_MODE16_POSITION                 0x0000000A
#define _SPI4CON_MODE16_MASK                     0x00000400
#define _SPI4CON_MODE16_LENGTH                   0x00000001

#define _SPI4CON_MODE32_POSITION                 0x0000000B
#define _SPI4CON_MODE32_MASK                     0x00000800
#define _SPI4CON_MODE32_LENGTH                   0x00000001

#define _SPI4CON_DISSDO_POSITION                 0x0000000C
#define _SPI4CON_DISSDO_MASK                     0x00001000
#define _SPI4CON_DISSDO_LENGTH                   0x00000001

#define _SPI4CON_SIDL_POSITION                   0x0000000D
#define _SPI4CON_SIDL_MASK                       0x00002000
#define _SPI4CON_SIDL_LENGTH                     0x00000001

#define _SPI4CON_ON_POSITION                     0x0000000F
#define _SPI4CON_ON_MASK                         0x00008000
#define _SPI4CON_ON_LENGTH                       0x00000001

#define _SPI4CON_ENHBUF_POSITION                 0x00000010
#define _SPI4CON_ENHBUF_MASK                     0x00010000
#define _SPI4CON_ENHBUF_LENGTH                   0x00000001

#define _SPI4CON_SPIFE_POSITION                  0x00000011
#define _SPI4CON_SPIFE_MASK                      0x00020000
#define _SPI4CON_SPIFE_LENGTH                    0x00000001

#define _SPI4CON_FRMCNT_POSITION                 0x00000018
#define _SPI4CON_FRMCNT_MASK                     0x07000000
#define _SPI4CON_FRMCNT_LENGTH                   0x00000003

#define _SPI4CON_FRMSYPW_POSITION                0x0000001B
#define _SPI4CON_FRMSYPW_MASK                    0x08000000
#define _SPI4CON_FRMSYPW_LENGTH                  0x00000001

#define _SPI4CON_MSSEN_POSITION                  0x0000001C
#define _SPI4CON_MSSEN_MASK                      0x10000000
#define _SPI4CON_MSSEN_LENGTH                    0x00000001

#define _SPI4CON_FRMPOL_POSITION                 0x0000001D
#define _SPI4CON_FRMPOL_MASK                     0x20000000
#define _SPI4CON_FRMPOL_LENGTH                   0x00000001

#define _SPI4CON_FRMSYNC_POSITION                0x0000001E
#define _SPI4CON_FRMSYNC_MASK                    0x40000000
#define _SPI4CON_FRMSYNC_LENGTH                  0x00000001

#define _SPI4CON_FRMEN_POSITION                  0x0000001F
#define _SPI4CON_FRMEN_MASK                      0x80000000
#define _SPI4CON_FRMEN_LENGTH                    0x00000001

#define _SPI4CON_w_POSITION                      0x00000000
#define _SPI4CON_w_MASK                          0xFFFFFFFF
#define _SPI4CON_w_LENGTH                        0x00000020

#define _SPI3ASTAT_SPIRBF_POSITION               0x00000000
#define _SPI3ASTAT_SPIRBF_MASK                   0x00000001
#define _SPI3ASTAT_SPIRBF_LENGTH                 0x00000001

#define _SPI3ASTAT_SPITBF_POSITION               0x00000001
#define _SPI3ASTAT_SPITBF_MASK                   0x00000002
#define _SPI3ASTAT_SPITBF_LENGTH                 0x00000001

#define _SPI3ASTAT_SPITBE_POSITION               0x00000003
#define _SPI3ASTAT_SPITBE_MASK                   0x00000008
#define _SPI3ASTAT_SPITBE_LENGTH                 0x00000001

#define _SPI3ASTAT_SPIRBE_POSITION               0x00000005
#define _SPI3ASTAT_SPIRBE_MASK                   0x00000020
#define _SPI3ASTAT_SPIRBE_LENGTH                 0x00000001

#define _SPI3ASTAT_SPIROV_POSITION               0x00000006
#define _SPI3ASTAT_SPIROV_MASK                   0x00000040
#define _SPI3ASTAT_SPIROV_LENGTH                 0x00000001

#define _SPI3ASTAT_SRMT_POSITION                 0x00000007
#define _SPI3ASTAT_SRMT_MASK                     0x00000080
#define _SPI3ASTAT_SRMT_LENGTH                   0x00000001

#define _SPI3ASTAT_SPITUR_POSITION               0x00000008
#define _SPI3ASTAT_SPITUR_MASK                   0x00000100
#define _SPI3ASTAT_SPITUR_LENGTH                 0x00000001

#define _SPI3ASTAT_SPIBUSY_POSITION              0x0000000B
#define _SPI3ASTAT_SPIBUSY_MASK                  0x00000800
#define _SPI3ASTAT_SPIBUSY_LENGTH                0x00000001

#define _SPI3ASTAT_TXBUFELM_POSITION             0x00000010
#define _SPI3ASTAT_TXBUFELM_MASK                 0x001F0000
#define _SPI3ASTAT_TXBUFELM_LENGTH               0x00000005

#define _SPI3ASTAT_RXBUFELM_POSITION             0x00000018
#define _SPI3ASTAT_RXBUFELM_MASK                 0x1F000000
#define _SPI3ASTAT_RXBUFELM_LENGTH               0x00000005

#define _SPI3ASTAT_w_POSITION                    0x00000000
#define _SPI3ASTAT_w_MASK                        0xFFFFFFFF
#define _SPI3ASTAT_w_LENGTH                      0x00000020

#define _SPI4STAT_SPIRBF_POSITION                0x00000000
#define _SPI4STAT_SPIRBF_MASK                    0x00000001
#define _SPI4STAT_SPIRBF_LENGTH                  0x00000001

#define _SPI4STAT_SPITBF_POSITION                0x00000001
#define _SPI4STAT_SPITBF_MASK                    0x00000002
#define _SPI4STAT_SPITBF_LENGTH                  0x00000001

#define _SPI4STAT_SPITBE_POSITION                0x00000003
#define _SPI4STAT_SPITBE_MASK                    0x00000008
#define _SPI4STAT_SPITBE_LENGTH                  0x00000001

#define _SPI4STAT_SPIRBE_POSITION                0x00000005
#define _SPI4STAT_SPIRBE_MASK                    0x00000020
#define _SPI4STAT_SPIRBE_LENGTH                  0x00000001

#define _SPI4STAT_SPIROV_POSITION                0x00000006
#define _SPI4STAT_SPIROV_MASK                    0x00000040
#define _SPI4STAT_SPIROV_LENGTH                  0x00000001

#define _SPI4STAT_SRMT_POSITION                  0x00000007
#define _SPI4STAT_SRMT_MASK                      0x00000080
#define _SPI4STAT_SRMT_LENGTH                    0x00000001

#define _SPI4STAT_SPITUR_POSITION                0x00000008
#define _SPI4STAT_SPITUR_MASK                    0x00000100
#define _SPI4STAT_SPITUR_LENGTH                  0x00000001

#define _SPI4STAT_SPIBUSY_POSITION               0x0000000B
#define _SPI4STAT_SPIBUSY_MASK                   0x00000800
#define _SPI4STAT_SPIBUSY_LENGTH                 0x00000001

#define _SPI4STAT_TXBUFELM_POSITION              0x00000010
#define _SPI4STAT_TXBUFELM_MASK                  0x001F0000
#define _SPI4STAT_TXBUFELM_LENGTH                0x00000005

#define _SPI4STAT_RXBUFELM_POSITION              0x00000018
#define _SPI4STAT_RXBUFELM_MASK                  0x1F000000
#define _SPI4STAT_RXBUFELM_LENGTH                0x00000005

#define _SPI4STAT_w_POSITION                     0x00000000
#define _SPI4STAT_w_MASK                         0xFFFFFFFF
#define _SPI4STAT_w_LENGTH                       0x00000020

#define _U1AMODE_STSEL_POSITION                  0x00000000
#define _U1AMODE_STSEL_MASK                      0x00000001
#define _U1AMODE_STSEL_LENGTH                    0x00000001

#define _U1AMODE_PDSEL_POSITION                  0x00000001
#define _U1AMODE_PDSEL_MASK                      0x00000006
#define _U1AMODE_PDSEL_LENGTH                    0x00000002

#define _U1AMODE_BRGH_POSITION                   0x00000003
#define _U1AMODE_BRGH_MASK                       0x00000008
#define _U1AMODE_BRGH_LENGTH                     0x00000001

#define _U1AMODE_RXINV_POSITION                  0x00000004
#define _U1AMODE_RXINV_MASK                      0x00000010
#define _U1AMODE_RXINV_LENGTH                    0x00000001

#define _U1AMODE_ABAUD_POSITION                  0x00000005
#define _U1AMODE_ABAUD_MASK                      0x00000020
#define _U1AMODE_ABAUD_LENGTH                    0x00000001

#define _U1AMODE_LPBACK_POSITION                 0x00000006
#define _U1AMODE_LPBACK_MASK                     0x00000040
#define _U1AMODE_LPBACK_LENGTH                   0x00000001

#define _U1AMODE_WAKE_POSITION                   0x00000007
#define _U1AMODE_WAKE_MASK                       0x00000080
#define _U1AMODE_WAKE_LENGTH                     0x00000001

#define _U1AMODE_UEN_POSITION                    0x00000008
#define _U1AMODE_UEN_MASK                        0x00000300
#define _U1AMODE_UEN_LENGTH                      0x00000002

#define _U1AMODE_RTSMD_POSITION                  0x0000000B
#define _U1AMODE_RTSMD_MASK                      0x00000800
#define _U1AMODE_RTSMD_LENGTH                    0x00000001

#define _U1AMODE_IREN_POSITION                   0x0000000C
#define _U1AMODE_IREN_MASK                       0x00001000
#define _U1AMODE_IREN_LENGTH                     0x00000001

#define _U1AMODE_SIDL_POSITION                   0x0000000D
#define _U1AMODE_SIDL_MASK                       0x00002000
#define _U1AMODE_SIDL_LENGTH                     0x00000001

#define _U1AMODE_ON_POSITION                     0x0000000F
#define _U1AMODE_ON_MASK                         0x00008000
#define _U1AMODE_ON_LENGTH                       0x00000001

#define _U1AMODE_PDSEL0_POSITION                 0x00000001
#define _U1AMODE_PDSEL0_MASK                     0x00000002
#define _U1AMODE_PDSEL0_LENGTH                   0x00000001

#define _U1AMODE_PDSEL1_POSITION                 0x00000002
#define _U1AMODE_PDSEL1_MASK                     0x00000004
#define _U1AMODE_PDSEL1_LENGTH                   0x00000001

#define _U1AMODE_UEN0_POSITION                   0x00000008
#define _U1AMODE_UEN0_MASK                       0x00000100
#define _U1AMODE_UEN0_LENGTH                     0x00000001

#define _U1AMODE_UEN1_POSITION                   0x00000009
#define _U1AMODE_UEN1_MASK                       0x00000200
#define _U1AMODE_UEN1_LENGTH                     0x00000001

#define _U1AMODE_USIDL_POSITION                  0x0000000D
#define _U1AMODE_USIDL_MASK                      0x00002000
#define _U1AMODE_USIDL_LENGTH                    0x00000001

#define _U1AMODE_UARTEN_POSITION                 0x0000000F
#define _U1AMODE_UARTEN_MASK                     0x00008000
#define _U1AMODE_UARTEN_LENGTH                   0x00000001

#define _U1AMODE_w_POSITION                      0x00000000
#define _U1AMODE_w_MASK                          0xFFFFFFFF
#define _U1AMODE_w_LENGTH                        0x00000020

#define _U1MODE_STSEL_POSITION                   0x00000000
#define _U1MODE_STSEL_MASK                       0x00000001
#define _U1MODE_STSEL_LENGTH                     0x00000001

#define _U1MODE_PDSEL_POSITION                   0x00000001
#define _U1MODE_PDSEL_MASK                       0x00000006
#define _U1MODE_PDSEL_LENGTH                     0x00000002

#define _U1MODE_BRGH_POSITION                    0x00000003
#define _U1MODE_BRGH_MASK                        0x00000008
#define _U1MODE_BRGH_LENGTH                      0x00000001

#define _U1MODE_RXINV_POSITION                   0x00000004
#define _U1MODE_RXINV_MASK                       0x00000010
#define _U1MODE_RXINV_LENGTH                     0x00000001

#define _U1MODE_ABAUD_POSITION                   0x00000005
#define _U1MODE_ABAUD_MASK                       0x00000020
#define _U1MODE_ABAUD_LENGTH                     0x00000001

#define _U1MODE_LPBACK_POSITION                  0x00000006
#define _U1MODE_LPBACK_MASK                      0x00000040
#define _U1MODE_LPBACK_LENGTH                    0x00000001

#define _U1MODE_WAKE_POSITION                    0x00000007
#define _U1MODE_WAKE_MASK                        0x00000080
#define _U1MODE_WAKE_LENGTH                      0x00000001

#define _U1MODE_UEN_POSITION                     0x00000008
#define _U1MODE_UEN_MASK                         0x00000300
#define _U1MODE_UEN_LENGTH                       0x00000002

#define _U1MODE_RTSMD_POSITION                   0x0000000B
#define _U1MODE_RTSMD_MASK                       0x00000800
#define _U1MODE_RTSMD_LENGTH                     0x00000001

#define _U1MODE_IREN_POSITION                    0x0000000C
#define _U1MODE_IREN_MASK                        0x00001000
#define _U1MODE_IREN_LENGTH                      0x00000001

#define _U1MODE_SIDL_POSITION                    0x0000000D
#define _U1MODE_SIDL_MASK                        0x00002000
#define _U1MODE_SIDL_LENGTH                      0x00000001

#define _U1MODE_ON_POSITION                      0x0000000F
#define _U1MODE_ON_MASK                          0x00008000
#define _U1MODE_ON_LENGTH                        0x00000001

#define _U1MODE_PDSEL0_POSITION                  0x00000001
#define _U1MODE_PDSEL0_MASK                      0x00000002
#define _U1MODE_PDSEL0_LENGTH                    0x00000001

#define _U1MODE_PDSEL1_POSITION                  0x00000002
#define _U1MODE_PDSEL1_MASK                      0x00000004
#define _U1MODE_PDSEL1_LENGTH                    0x00000001

#define _U1MODE_UEN0_POSITION                    0x00000008
#define _U1MODE_UEN0_MASK                        0x00000100
#define _U1MODE_UEN0_LENGTH                      0x00000001

#define _U1MODE_UEN1_POSITION                    0x00000009
#define _U1MODE_UEN1_MASK                        0x00000200
#define _U1MODE_UEN1_LENGTH                      0x00000001

#define _U1MODE_USIDL_POSITION                   0x0000000D
#define _U1MODE_USIDL_MASK                       0x00002000
#define _U1MODE_USIDL_LENGTH                     0x00000001

#define _U1MODE_UARTEN_POSITION                  0x0000000F
#define _U1MODE_UARTEN_MASK                      0x00008000
#define _U1MODE_UARTEN_LENGTH                    0x00000001

#define _U1MODE_w_POSITION                       0x00000000
#define _U1MODE_w_MASK                           0xFFFFFFFF
#define _U1MODE_w_LENGTH                         0x00000020

#define _U1ASTA_URXDA_POSITION                   0x00000000
#define _U1ASTA_URXDA_MASK                       0x00000001
#define _U1ASTA_URXDA_LENGTH                     0x00000001

#define _U1ASTA_OERR_POSITION                    0x00000001
#define _U1ASTA_OERR_MASK                        0x00000002
#define _U1ASTA_OERR_LENGTH                      0x00000001

#define _U1ASTA_FERR_POSITION                    0x00000002
#define _U1ASTA_FERR_MASK                        0x00000004
#define _U1ASTA_FERR_LENGTH                      0x00000001

#define _U1ASTA_PERR_POSITION                    0x00000003
#define _U1ASTA_PERR_MASK                        0x00000008
#define _U1ASTA_PERR_LENGTH                      0x00000001

#define _U1ASTA_RIDLE_POSITION                   0x00000004
#define _U1ASTA_RIDLE_MASK                       0x00000010
#define _U1ASTA_RIDLE_LENGTH                     0x00000001

#define _U1ASTA_ADDEN_POSITION                   0x00000005
#define _U1ASTA_ADDEN_MASK                       0x00000020
#define _U1ASTA_ADDEN_LENGTH                     0x00000001

#define _U1ASTA_URXISEL_POSITION                 0x00000006
#define _U1ASTA_URXISEL_MASK                     0x000000C0
#define _U1ASTA_URXISEL_LENGTH                   0x00000002

#define _U1ASTA_TRMT_POSITION                    0x00000008
#define _U1ASTA_TRMT_MASK                        0x00000100
#define _U1ASTA_TRMT_LENGTH                      0x00000001

#define _U1ASTA_UTXBF_POSITION                   0x00000009
#define _U1ASTA_UTXBF_MASK                       0x00000200
#define _U1ASTA_UTXBF_LENGTH                     0x00000001

#define _U1ASTA_UTXEN_POSITION                   0x0000000A
#define _U1ASTA_UTXEN_MASK                       0x00000400
#define _U1ASTA_UTXEN_LENGTH                     0x00000001

#define _U1ASTA_UTXBRK_POSITION                  0x0000000B
#define _U1ASTA_UTXBRK_MASK                      0x00000800
#define _U1ASTA_UTXBRK_LENGTH                    0x00000001

#define _U1ASTA_URXEN_POSITION                   0x0000000C
#define _U1ASTA_URXEN_MASK                       0x00001000
#define _U1ASTA_URXEN_LENGTH                     0x00000001

#define _U1ASTA_UTXINV_POSITION                  0x0000000D
#define _U1ASTA_UTXINV_MASK                      0x00002000
#define _U1ASTA_UTXINV_LENGTH                    0x00000001

#define _U1ASTA_UTXISEL_POSITION                 0x0000000E
#define _U1ASTA_UTXISEL_MASK                     0x0000C000
#define _U1ASTA_UTXISEL_LENGTH                   0x00000002

#define _U1ASTA_ADDR_POSITION                    0x00000010
#define _U1ASTA_ADDR_MASK                        0x00FF0000
#define _U1ASTA_ADDR_LENGTH                      0x00000008

#define _U1ASTA_ADM_EN_POSITION                  0x00000018
#define _U1ASTA_ADM_EN_MASK                      0x01000000
#define _U1ASTA_ADM_EN_LENGTH                    0x00000001

#define _U1ASTA_URXISEL0_POSITION                0x00000006
#define _U1ASTA_URXISEL0_MASK                    0x00000040
#define _U1ASTA_URXISEL0_LENGTH                  0x00000001

#define _U1ASTA_URXISEL1_POSITION                0x00000007
#define _U1ASTA_URXISEL1_MASK                    0x00000080
#define _U1ASTA_URXISEL1_LENGTH                  0x00000001

#define _U1ASTA_UTXISEL0_POSITION                0x0000000E
#define _U1ASTA_UTXISEL0_MASK                    0x00004000
#define _U1ASTA_UTXISEL0_LENGTH                  0x00000001

#define _U1ASTA_UTXISEL1_POSITION                0x0000000F
#define _U1ASTA_UTXISEL1_MASK                    0x00008000
#define _U1ASTA_UTXISEL1_LENGTH                  0x00000001

#define _U1ASTA_UTXSEL_POSITION                  0x0000000E
#define _U1ASTA_UTXSEL_MASK                      0x0000C000
#define _U1ASTA_UTXSEL_LENGTH                    0x00000002

#define _U1ASTA_w_POSITION                       0x00000000
#define _U1ASTA_w_MASK                           0xFFFFFFFF
#define _U1ASTA_w_LENGTH                         0x00000020

#define _U1STA_URXDA_POSITION                    0x00000000
#define _U1STA_URXDA_MASK                        0x00000001
#define _U1STA_URXDA_LENGTH                      0x00000001

#define _U1STA_OERR_POSITION                     0x00000001
#define _U1STA_OERR_MASK                         0x00000002
#define _U1STA_OERR_LENGTH                       0x00000001

#define _U1STA_FERR_POSITION                     0x00000002
#define _U1STA_FERR_MASK                         0x00000004
#define _U1STA_FERR_LENGTH                       0x00000001

#define _U1STA_PERR_POSITION                     0x00000003
#define _U1STA_PERR_MASK                         0x00000008
#define _U1STA_PERR_LENGTH                       0x00000001

#define _U1STA_RIDLE_POSITION                    0x00000004
#define _U1STA_RIDLE_MASK                        0x00000010
#define _U1STA_RIDLE_LENGTH                      0x00000001

#define _U1STA_ADDEN_POSITION                    0x00000005
#define _U1STA_ADDEN_MASK                        0x00000020
#define _U1STA_ADDEN_LENGTH                      0x00000001

#define _U1STA_URXISEL_POSITION                  0x00000006
#define _U1STA_URXISEL_MASK                      0x000000C0
#define _U1STA_URXISEL_LENGTH                    0x00000002

#define _U1STA_TRMT_POSITION                     0x00000008
#define _U1STA_TRMT_MASK                         0x00000100
#define _U1STA_TRMT_LENGTH                       0x00000001

#define _U1STA_UTXBF_POSITION                    0x00000009
#define _U1STA_UTXBF_MASK                        0x00000200
#define _U1STA_UTXBF_LENGTH                      0x00000001

#define _U1STA_UTXEN_POSITION                    0x0000000A
#define _U1STA_UTXEN_MASK                        0x00000400
#define _U1STA_UTXEN_LENGTH                      0x00000001

#define _U1STA_UTXBRK_POSITION                   0x0000000B
#define _U1STA_UTXBRK_MASK                       0x00000800
#define _U1STA_UTXBRK_LENGTH                     0x00000001

#define _U1STA_URXEN_POSITION                    0x0000000C
#define _U1STA_URXEN_MASK                        0x00001000
#define _U1STA_URXEN_LENGTH                      0x00000001

#define _U1STA_UTXINV_POSITION                   0x0000000D
#define _U1STA_UTXINV_MASK                       0x00002000
#define _U1STA_UTXINV_LENGTH                     0x00000001

#define _U1STA_UTXISEL_POSITION                  0x0000000E
#define _U1STA_UTXISEL_MASK                      0x0000C000
#define _U1STA_UTXISEL_LENGTH                    0x00000002

#define _U1STA_ADDR_POSITION                     0x00000010
#define _U1STA_ADDR_MASK                         0x00FF0000
#define _U1STA_ADDR_LENGTH                       0x00000008

#define _U1STA_ADM_EN_POSITION                   0x00000018
#define _U1STA_ADM_EN_MASK                       0x01000000
#define _U1STA_ADM_EN_LENGTH                     0x00000001

#define _U1STA_URXISEL0_POSITION                 0x00000006
#define _U1STA_URXISEL0_MASK                     0x00000040
#define _U1STA_URXISEL0_LENGTH                   0x00000001

#define _U1STA_URXISEL1_POSITION                 0x00000007
#define _U1STA_URXISEL1_MASK                     0x00000080
#define _U1STA_URXISEL1_LENGTH                   0x00000001

#define _U1STA_UTXISEL0_POSITION                 0x0000000E
#define _U1STA_UTXISEL0_MASK                     0x00004000
#define _U1STA_UTXISEL0_LENGTH                   0x00000001

#define _U1STA_UTXISEL1_POSITION                 0x0000000F
#define _U1STA_UTXISEL1_MASK                     0x00008000
#define _U1STA_UTXISEL1_LENGTH                   0x00000001

#define _U1STA_UTXSEL_POSITION                   0x0000000E
#define _U1STA_UTXSEL_MASK                       0x0000C000
#define _U1STA_UTXSEL_LENGTH                     0x00000002

#define _U1STA_w_POSITION                        0x00000000
#define _U1STA_w_MASK                            0xFFFFFFFF
#define _U1STA_w_LENGTH                          0x00000020

#define _U1BMODE_STSEL_POSITION                  0x00000000
#define _U1BMODE_STSEL_MASK                      0x00000001
#define _U1BMODE_STSEL_LENGTH                    0x00000001

#define _U1BMODE_PDSEL_POSITION                  0x00000001
#define _U1BMODE_PDSEL_MASK                      0x00000006
#define _U1BMODE_PDSEL_LENGTH                    0x00000002

#define _U1BMODE_BRGH_POSITION                   0x00000003
#define _U1BMODE_BRGH_MASK                       0x00000008
#define _U1BMODE_BRGH_LENGTH                     0x00000001

#define _U1BMODE_RXINV_POSITION                  0x00000004
#define _U1BMODE_RXINV_MASK                      0x00000010
#define _U1BMODE_RXINV_LENGTH                    0x00000001

#define _U1BMODE_ABAUD_POSITION                  0x00000005
#define _U1BMODE_ABAUD_MASK                      0x00000020
#define _U1BMODE_ABAUD_LENGTH                    0x00000001

#define _U1BMODE_LPBACK_POSITION                 0x00000006
#define _U1BMODE_LPBACK_MASK                     0x00000040
#define _U1BMODE_LPBACK_LENGTH                   0x00000001

#define _U1BMODE_WAKE_POSITION                   0x00000007
#define _U1BMODE_WAKE_MASK                       0x00000080
#define _U1BMODE_WAKE_LENGTH                     0x00000001

#define _U1BMODE_IREN_POSITION                   0x0000000C
#define _U1BMODE_IREN_MASK                       0x00001000
#define _U1BMODE_IREN_LENGTH                     0x00000001

#define _U1BMODE_SIDL_POSITION                   0x0000000D
#define _U1BMODE_SIDL_MASK                       0x00002000
#define _U1BMODE_SIDL_LENGTH                     0x00000001

#define _U1BMODE_ON_POSITION                     0x0000000F
#define _U1BMODE_ON_MASK                         0x00008000
#define _U1BMODE_ON_LENGTH                       0x00000001

#define _U1BMODE_PDSEL0_POSITION                 0x00000001
#define _U1BMODE_PDSEL0_MASK                     0x00000002
#define _U1BMODE_PDSEL0_LENGTH                   0x00000001

#define _U1BMODE_PDSEL1_POSITION                 0x00000002
#define _U1BMODE_PDSEL1_MASK                     0x00000004
#define _U1BMODE_PDSEL1_LENGTH                   0x00000001

#define _U1BMODE_USIDL_POSITION                  0x0000000D
#define _U1BMODE_USIDL_MASK                      0x00002000
#define _U1BMODE_USIDL_LENGTH                    0x00000001

#define _U1BMODE_UARTEN_POSITION                 0x0000000F
#define _U1BMODE_UARTEN_MASK                     0x00008000
#define _U1BMODE_UARTEN_LENGTH                   0x00000001

#define _U1BMODE_w_POSITION                      0x00000000
#define _U1BMODE_w_MASK                          0xFFFFFFFF
#define _U1BMODE_w_LENGTH                        0x00000020

#define _U4MODE_STSEL_POSITION                   0x00000000
#define _U4MODE_STSEL_MASK                       0x00000001
#define _U4MODE_STSEL_LENGTH                     0x00000001

#define _U4MODE_PDSEL_POSITION                   0x00000001
#define _U4MODE_PDSEL_MASK                       0x00000006
#define _U4MODE_PDSEL_LENGTH                     0x00000002

#define _U4MODE_BRGH_POSITION                    0x00000003
#define _U4MODE_BRGH_MASK                        0x00000008
#define _U4MODE_BRGH_LENGTH                      0x00000001

#define _U4MODE_RXINV_POSITION                   0x00000004
#define _U4MODE_RXINV_MASK                       0x00000010
#define _U4MODE_RXINV_LENGTH                     0x00000001

#define _U4MODE_ABAUD_POSITION                   0x00000005
#define _U4MODE_ABAUD_MASK                       0x00000020
#define _U4MODE_ABAUD_LENGTH                     0x00000001

#define _U4MODE_LPBACK_POSITION                  0x00000006
#define _U4MODE_LPBACK_MASK                      0x00000040
#define _U4MODE_LPBACK_LENGTH                    0x00000001

#define _U4MODE_WAKE_POSITION                    0x00000007
#define _U4MODE_WAKE_MASK                        0x00000080
#define _U4MODE_WAKE_LENGTH                      0x00000001

#define _U4MODE_IREN_POSITION                    0x0000000C
#define _U4MODE_IREN_MASK                        0x00001000
#define _U4MODE_IREN_LENGTH                      0x00000001

#define _U4MODE_SIDL_POSITION                    0x0000000D
#define _U4MODE_SIDL_MASK                        0x00002000
#define _U4MODE_SIDL_LENGTH                      0x00000001

#define _U4MODE_ON_POSITION                      0x0000000F
#define _U4MODE_ON_MASK                          0x00008000
#define _U4MODE_ON_LENGTH                        0x00000001

#define _U4MODE_PDSEL0_POSITION                  0x00000001
#define _U4MODE_PDSEL0_MASK                      0x00000002
#define _U4MODE_PDSEL0_LENGTH                    0x00000001

#define _U4MODE_PDSEL1_POSITION                  0x00000002
#define _U4MODE_PDSEL1_MASK                      0x00000004
#define _U4MODE_PDSEL1_LENGTH                    0x00000001

#define _U4MODE_USIDL_POSITION                   0x0000000D
#define _U4MODE_USIDL_MASK                       0x00002000
#define _U4MODE_USIDL_LENGTH                     0x00000001

#define _U4MODE_UARTEN_POSITION                  0x0000000F
#define _U4MODE_UARTEN_MASK                      0x00008000
#define _U4MODE_UARTEN_LENGTH                    0x00000001

#define _U4MODE_w_POSITION                       0x00000000
#define _U4MODE_w_MASK                           0xFFFFFFFF
#define _U4MODE_w_LENGTH                         0x00000020

#define _U1BSTA_URXDA_POSITION                   0x00000000
#define _U1BSTA_URXDA_MASK                       0x00000001
#define _U1BSTA_URXDA_LENGTH                     0x00000001

#define _U1BSTA_OERR_POSITION                    0x00000001
#define _U1BSTA_OERR_MASK                        0x00000002
#define _U1BSTA_OERR_LENGTH                      0x00000001

#define _U1BSTA_FERR_POSITION                    0x00000002
#define _U1BSTA_FERR_MASK                        0x00000004
#define _U1BSTA_FERR_LENGTH                      0x00000001

#define _U1BSTA_PERR_POSITION                    0x00000003
#define _U1BSTA_PERR_MASK                        0x00000008
#define _U1BSTA_PERR_LENGTH                      0x00000001

#define _U1BSTA_RIDLE_POSITION                   0x00000004
#define _U1BSTA_RIDLE_MASK                       0x00000010
#define _U1BSTA_RIDLE_LENGTH                     0x00000001

#define _U1BSTA_ADDEN_POSITION                   0x00000005
#define _U1BSTA_ADDEN_MASK                       0x00000020
#define _U1BSTA_ADDEN_LENGTH                     0x00000001

#define _U1BSTA_URXISEL_POSITION                 0x00000006
#define _U1BSTA_URXISEL_MASK                     0x000000C0
#define _U1BSTA_URXISEL_LENGTH                   0x00000002

#define _U1BSTA_TRMT_POSITION                    0x00000008
#define _U1BSTA_TRMT_MASK                        0x00000100
#define _U1BSTA_TRMT_LENGTH                      0x00000001

#define _U1BSTA_UTXBF_POSITION                   0x00000009
#define _U1BSTA_UTXBF_MASK                       0x00000200
#define _U1BSTA_UTXBF_LENGTH                     0x00000001

#define _U1BSTA_UTXEN_POSITION                   0x0000000A
#define _U1BSTA_UTXEN_MASK                       0x00000400
#define _U1BSTA_UTXEN_LENGTH                     0x00000001

#define _U1BSTA_UTXBRK_POSITION                  0x0000000B
#define _U1BSTA_UTXBRK_MASK                      0x00000800
#define _U1BSTA_UTXBRK_LENGTH                    0x00000001

#define _U1BSTA_URXEN_POSITION                   0x0000000C
#define _U1BSTA_URXEN_MASK                       0x00001000
#define _U1BSTA_URXEN_LENGTH                     0x00000001

#define _U1BSTA_UTXINV_POSITION                  0x0000000D
#define _U1BSTA_UTXINV_MASK                      0x00002000
#define _U1BSTA_UTXINV_LENGTH                    0x00000001

#define _U1BSTA_UTXISEL_POSITION                 0x0000000E
#define _U1BSTA_UTXISEL_MASK                     0x0000C000
#define _U1BSTA_UTXISEL_LENGTH                   0x00000002

#define _U1BSTA_ADDR_POSITION                    0x00000010
#define _U1BSTA_ADDR_MASK                        0x00FF0000
#define _U1BSTA_ADDR_LENGTH                      0x00000008

#define _U1BSTA_ADM_EN_POSITION                  0x00000018
#define _U1BSTA_ADM_EN_MASK                      0x01000000
#define _U1BSTA_ADM_EN_LENGTH                    0x00000001

#define _U1BSTA_URXISEL0_POSITION                0x00000006
#define _U1BSTA_URXISEL0_MASK                    0x00000040
#define _U1BSTA_URXISEL0_LENGTH                  0x00000001

#define _U1BSTA_URXISEL1_POSITION                0x00000007
#define _U1BSTA_URXISEL1_MASK                    0x00000080
#define _U1BSTA_URXISEL1_LENGTH                  0x00000001

#define _U1BSTA_UTXISEL0_POSITION                0x0000000E
#define _U1BSTA_UTXISEL0_MASK                    0x00004000
#define _U1BSTA_UTXISEL0_LENGTH                  0x00000001

#define _U1BSTA_UTXISEL1_POSITION                0x0000000F
#define _U1BSTA_UTXISEL1_MASK                    0x00008000
#define _U1BSTA_UTXISEL1_LENGTH                  0x00000001

#define _U1BSTA_UTXSEL_POSITION                  0x0000000E
#define _U1BSTA_UTXSEL_MASK                      0x0000C000
#define _U1BSTA_UTXSEL_LENGTH                    0x00000002

#define _U1BSTA_w_POSITION                       0x00000000
#define _U1BSTA_w_MASK                           0xFFFFFFFF
#define _U1BSTA_w_LENGTH                         0x00000020

#define _U4STA_URXDA_POSITION                    0x00000000
#define _U4STA_URXDA_MASK                        0x00000001
#define _U4STA_URXDA_LENGTH                      0x00000001

#define _U4STA_OERR_POSITION                     0x00000001
#define _U4STA_OERR_MASK                         0x00000002
#define _U4STA_OERR_LENGTH                       0x00000001

#define _U4STA_FERR_POSITION                     0x00000002
#define _U4STA_FERR_MASK                         0x00000004
#define _U4STA_FERR_LENGTH                       0x00000001

#define _U4STA_PERR_POSITION                     0x00000003
#define _U4STA_PERR_MASK                         0x00000008
#define _U4STA_PERR_LENGTH                       0x00000001

#define _U4STA_RIDLE_POSITION                    0x00000004
#define _U4STA_RIDLE_MASK                        0x00000010
#define _U4STA_RIDLE_LENGTH                      0x00000001

#define _U4STA_ADDEN_POSITION                    0x00000005
#define _U4STA_ADDEN_MASK                        0x00000020
#define _U4STA_ADDEN_LENGTH                      0x00000001

#define _U4STA_URXISEL_POSITION                  0x00000006
#define _U4STA_URXISEL_MASK                      0x000000C0
#define _U4STA_URXISEL_LENGTH                    0x00000002

#define _U4STA_TRMT_POSITION                     0x00000008
#define _U4STA_TRMT_MASK                         0x00000100
#define _U4STA_TRMT_LENGTH                       0x00000001

#define _U4STA_UTXBF_POSITION                    0x00000009
#define _U4STA_UTXBF_MASK                        0x00000200
#define _U4STA_UTXBF_LENGTH                      0x00000001

#define _U4STA_UTXEN_POSITION                    0x0000000A
#define _U4STA_UTXEN_MASK                        0x00000400
#define _U4STA_UTXEN_LENGTH                      0x00000001

#define _U4STA_UTXBRK_POSITION                   0x0000000B
#define _U4STA_UTXBRK_MASK                       0x00000800
#define _U4STA_UTXBRK_LENGTH                     0x00000001

#define _U4STA_URXEN_POSITION                    0x0000000C
#define _U4STA_URXEN_MASK                        0x00001000
#define _U4STA_URXEN_LENGTH                      0x00000001

#define _U4STA_UTXINV_POSITION                   0x0000000D
#define _U4STA_UTXINV_MASK                       0x00002000
#define _U4STA_UTXINV_LENGTH                     0x00000001

#define _U4STA_UTXISEL_POSITION                  0x0000000E
#define _U4STA_UTXISEL_MASK                      0x0000C000
#define _U4STA_UTXISEL_LENGTH                    0x00000002

#define _U4STA_ADDR_POSITION                     0x00000010
#define _U4STA_ADDR_MASK                         0x00FF0000
#define _U4STA_ADDR_LENGTH                       0x00000008

#define _U4STA_ADM_EN_POSITION                   0x00000018
#define _U4STA_ADM_EN_MASK                       0x01000000
#define _U4STA_ADM_EN_LENGTH                     0x00000001

#define _U4STA_URXISEL0_POSITION                 0x00000006
#define _U4STA_URXISEL0_MASK                     0x00000040
#define _U4STA_URXISEL0_LENGTH                   0x00000001

#define _U4STA_URXISEL1_POSITION                 0x00000007
#define _U4STA_URXISEL1_MASK                     0x00000080
#define _U4STA_URXISEL1_LENGTH                   0x00000001

#define _U4STA_UTXISEL0_POSITION                 0x0000000E
#define _U4STA_UTXISEL0_MASK                     0x00004000
#define _U4STA_UTXISEL0_LENGTH                   0x00000001

#define _U4STA_UTXISEL1_POSITION                 0x0000000F
#define _U4STA_UTXISEL1_MASK                     0x00008000
#define _U4STA_UTXISEL1_LENGTH                   0x00000001

#define _U4STA_UTXSEL_POSITION                   0x0000000E
#define _U4STA_UTXSEL_MASK                       0x0000C000
#define _U4STA_UTXSEL_LENGTH                     0x00000002

#define _U4STA_w_POSITION                        0x00000000
#define _U4STA_w_MASK                            0xFFFFFFFF
#define _U4STA_w_LENGTH                          0x00000020

#define _U2AMODE_STSEL_POSITION                  0x00000000
#define _U2AMODE_STSEL_MASK                      0x00000001
#define _U2AMODE_STSEL_LENGTH                    0x00000001

#define _U2AMODE_PDSEL_POSITION                  0x00000001
#define _U2AMODE_PDSEL_MASK                      0x00000006
#define _U2AMODE_PDSEL_LENGTH                    0x00000002

#define _U2AMODE_BRGH_POSITION                   0x00000003
#define _U2AMODE_BRGH_MASK                       0x00000008
#define _U2AMODE_BRGH_LENGTH                     0x00000001

#define _U2AMODE_RXINV_POSITION                  0x00000004
#define _U2AMODE_RXINV_MASK                      0x00000010
#define _U2AMODE_RXINV_LENGTH                    0x00000001

#define _U2AMODE_ABAUD_POSITION                  0x00000005
#define _U2AMODE_ABAUD_MASK                      0x00000020
#define _U2AMODE_ABAUD_LENGTH                    0x00000001

#define _U2AMODE_LPBACK_POSITION                 0x00000006
#define _U2AMODE_LPBACK_MASK                     0x00000040
#define _U2AMODE_LPBACK_LENGTH                   0x00000001

#define _U2AMODE_WAKE_POSITION                   0x00000007
#define _U2AMODE_WAKE_MASK                       0x00000080
#define _U2AMODE_WAKE_LENGTH                     0x00000001

#define _U2AMODE_UEN_POSITION                    0x00000008
#define _U2AMODE_UEN_MASK                        0x00000300
#define _U2AMODE_UEN_LENGTH                      0x00000002

#define _U2AMODE_RTSMD_POSITION                  0x0000000B
#define _U2AMODE_RTSMD_MASK                      0x00000800
#define _U2AMODE_RTSMD_LENGTH                    0x00000001

#define _U2AMODE_IREN_POSITION                   0x0000000C
#define _U2AMODE_IREN_MASK                       0x00001000
#define _U2AMODE_IREN_LENGTH                     0x00000001

#define _U2AMODE_SIDL_POSITION                   0x0000000D
#define _U2AMODE_SIDL_MASK                       0x00002000
#define _U2AMODE_SIDL_LENGTH                     0x00000001

#define _U2AMODE_ON_POSITION                     0x0000000F
#define _U2AMODE_ON_MASK                         0x00008000
#define _U2AMODE_ON_LENGTH                       0x00000001

#define _U2AMODE_PDSEL0_POSITION                 0x00000001
#define _U2AMODE_PDSEL0_MASK                     0x00000002
#define _U2AMODE_PDSEL0_LENGTH                   0x00000001

#define _U2AMODE_PDSEL1_POSITION                 0x00000002
#define _U2AMODE_PDSEL1_MASK                     0x00000004
#define _U2AMODE_PDSEL1_LENGTH                   0x00000001

#define _U2AMODE_UEN0_POSITION                   0x00000008
#define _U2AMODE_UEN0_MASK                       0x00000100
#define _U2AMODE_UEN0_LENGTH                     0x00000001

#define _U2AMODE_UEN1_POSITION                   0x00000009
#define _U2AMODE_UEN1_MASK                       0x00000200
#define _U2AMODE_UEN1_LENGTH                     0x00000001

#define _U2AMODE_USIDL_POSITION                  0x0000000D
#define _U2AMODE_USIDL_MASK                      0x00002000
#define _U2AMODE_USIDL_LENGTH                    0x00000001

#define _U2AMODE_UARTEN_POSITION                 0x0000000F
#define _U2AMODE_UARTEN_MASK                     0x00008000
#define _U2AMODE_UARTEN_LENGTH                   0x00000001

#define _U2AMODE_w_POSITION                      0x00000000
#define _U2AMODE_w_MASK                          0xFFFFFFFF
#define _U2AMODE_w_LENGTH                        0x00000020

#define _U3MODE_STSEL_POSITION                   0x00000000
#define _U3MODE_STSEL_MASK                       0x00000001
#define _U3MODE_STSEL_LENGTH                     0x00000001

#define _U3MODE_PDSEL_POSITION                   0x00000001
#define _U3MODE_PDSEL_MASK                       0x00000006
#define _U3MODE_PDSEL_LENGTH                     0x00000002

#define _U3MODE_BRGH_POSITION                    0x00000003
#define _U3MODE_BRGH_MASK                        0x00000008
#define _U3MODE_BRGH_LENGTH                      0x00000001

#define _U3MODE_RXINV_POSITION                   0x00000004
#define _U3MODE_RXINV_MASK                       0x00000010
#define _U3MODE_RXINV_LENGTH                     0x00000001

#define _U3MODE_ABAUD_POSITION                   0x00000005
#define _U3MODE_ABAUD_MASK                       0x00000020
#define _U3MODE_ABAUD_LENGTH                     0x00000001

#define _U3MODE_LPBACK_POSITION                  0x00000006
#define _U3MODE_LPBACK_MASK                      0x00000040
#define _U3MODE_LPBACK_LENGTH                    0x00000001

#define _U3MODE_WAKE_POSITION                    0x00000007
#define _U3MODE_WAKE_MASK                        0x00000080
#define _U3MODE_WAKE_LENGTH                      0x00000001

#define _U3MODE_UEN_POSITION                     0x00000008
#define _U3MODE_UEN_MASK                         0x00000300
#define _U3MODE_UEN_LENGTH                       0x00000002

#define _U3MODE_RTSMD_POSITION                   0x0000000B
#define _U3MODE_RTSMD_MASK                       0x00000800
#define _U3MODE_RTSMD_LENGTH                     0x00000001

#define _U3MODE_IREN_POSITION                    0x0000000C
#define _U3MODE_IREN_MASK                        0x00001000
#define _U3MODE_IREN_LENGTH                      0x00000001

#define _U3MODE_SIDL_POSITION                    0x0000000D
#define _U3MODE_SIDL_MASK                        0x00002000
#define _U3MODE_SIDL_LENGTH                      0x00000001

#define _U3MODE_ON_POSITION                      0x0000000F
#define _U3MODE_ON_MASK                          0x00008000
#define _U3MODE_ON_LENGTH                        0x00000001

#define _U3MODE_PDSEL0_POSITION                  0x00000001
#define _U3MODE_PDSEL0_MASK                      0x00000002
#define _U3MODE_PDSEL0_LENGTH                    0x00000001

#define _U3MODE_PDSEL1_POSITION                  0x00000002
#define _U3MODE_PDSEL1_MASK                      0x00000004
#define _U3MODE_PDSEL1_LENGTH                    0x00000001

#define _U3MODE_UEN0_POSITION                    0x00000008
#define _U3MODE_UEN0_MASK                        0x00000100
#define _U3MODE_UEN0_LENGTH                      0x00000001

#define _U3MODE_UEN1_POSITION                    0x00000009
#define _U3MODE_UEN1_MASK                        0x00000200
#define _U3MODE_UEN1_LENGTH                      0x00000001

#define _U3MODE_USIDL_POSITION                   0x0000000D
#define _U3MODE_USIDL_MASK                       0x00002000
#define _U3MODE_USIDL_LENGTH                     0x00000001

#define _U3MODE_UARTEN_POSITION                  0x0000000F
#define _U3MODE_UARTEN_MASK                      0x00008000
#define _U3MODE_UARTEN_LENGTH                    0x00000001

#define _U3MODE_w_POSITION                       0x00000000
#define _U3MODE_w_MASK                           0xFFFFFFFF
#define _U3MODE_w_LENGTH                         0x00000020

#define _U2ASTA_URXDA_POSITION                   0x00000000
#define _U2ASTA_URXDA_MASK                       0x00000001
#define _U2ASTA_URXDA_LENGTH                     0x00000001

#define _U2ASTA_OERR_POSITION                    0x00000001
#define _U2ASTA_OERR_MASK                        0x00000002
#define _U2ASTA_OERR_LENGTH                      0x00000001

#define _U2ASTA_FERR_POSITION                    0x00000002
#define _U2ASTA_FERR_MASK                        0x00000004
#define _U2ASTA_FERR_LENGTH                      0x00000001

#define _U2ASTA_PERR_POSITION                    0x00000003
#define _U2ASTA_PERR_MASK                        0x00000008
#define _U2ASTA_PERR_LENGTH                      0x00000001

#define _U2ASTA_RIDLE_POSITION                   0x00000004
#define _U2ASTA_RIDLE_MASK                       0x00000010
#define _U2ASTA_RIDLE_LENGTH                     0x00000001

#define _U2ASTA_ADDEN_POSITION                   0x00000005
#define _U2ASTA_ADDEN_MASK                       0x00000020
#define _U2ASTA_ADDEN_LENGTH                     0x00000001

#define _U2ASTA_URXISEL_POSITION                 0x00000006
#define _U2ASTA_URXISEL_MASK                     0x000000C0
#define _U2ASTA_URXISEL_LENGTH                   0x00000002

#define _U2ASTA_TRMT_POSITION                    0x00000008
#define _U2ASTA_TRMT_MASK                        0x00000100
#define _U2ASTA_TRMT_LENGTH                      0x00000001

#define _U2ASTA_UTXBF_POSITION                   0x00000009
#define _U2ASTA_UTXBF_MASK                       0x00000200
#define _U2ASTA_UTXBF_LENGTH                     0x00000001

#define _U2ASTA_UTXEN_POSITION                   0x0000000A
#define _U2ASTA_UTXEN_MASK                       0x00000400
#define _U2ASTA_UTXEN_LENGTH                     0x00000001

#define _U2ASTA_UTXBRK_POSITION                  0x0000000B
#define _U2ASTA_UTXBRK_MASK                      0x00000800
#define _U2ASTA_UTXBRK_LENGTH                    0x00000001

#define _U2ASTA_URXEN_POSITION                   0x0000000C
#define _U2ASTA_URXEN_MASK                       0x00001000
#define _U2ASTA_URXEN_LENGTH                     0x00000001

#define _U2ASTA_UTXINV_POSITION                  0x0000000D
#define _U2ASTA_UTXINV_MASK                      0x00002000
#define _U2ASTA_UTXINV_LENGTH                    0x00000001

#define _U2ASTA_UTXISEL_POSITION                 0x0000000E
#define _U2ASTA_UTXISEL_MASK                     0x0000C000
#define _U2ASTA_UTXISEL_LENGTH                   0x00000002

#define _U2ASTA_ADDR_POSITION                    0x00000010
#define _U2ASTA_ADDR_MASK                        0x00FF0000
#define _U2ASTA_ADDR_LENGTH                      0x00000008

#define _U2ASTA_ADM_EN_POSITION                  0x00000018
#define _U2ASTA_ADM_EN_MASK                      0x01000000
#define _U2ASTA_ADM_EN_LENGTH                    0x00000001

#define _U2ASTA_URXISEL0_POSITION                0x00000006
#define _U2ASTA_URXISEL0_MASK                    0x00000040
#define _U2ASTA_URXISEL0_LENGTH                  0x00000001

#define _U2ASTA_URXISEL1_POSITION                0x00000007
#define _U2ASTA_URXISEL1_MASK                    0x00000080
#define _U2ASTA_URXISEL1_LENGTH                  0x00000001

#define _U2ASTA_UTXISEL0_POSITION                0x0000000E
#define _U2ASTA_UTXISEL0_MASK                    0x00004000
#define _U2ASTA_UTXISEL0_LENGTH                  0x00000001

#define _U2ASTA_UTXISEL1_POSITION                0x0000000F
#define _U2ASTA_UTXISEL1_MASK                    0x00008000
#define _U2ASTA_UTXISEL1_LENGTH                  0x00000001

#define _U2ASTA_UTXSEL_POSITION                  0x0000000E
#define _U2ASTA_UTXSEL_MASK                      0x0000C000
#define _U2ASTA_UTXSEL_LENGTH                    0x00000002

#define _U2ASTA_w_POSITION                       0x00000000
#define _U2ASTA_w_MASK                           0xFFFFFFFF
#define _U2ASTA_w_LENGTH                         0x00000020

#define _U3STA_URXDA_POSITION                    0x00000000
#define _U3STA_URXDA_MASK                        0x00000001
#define _U3STA_URXDA_LENGTH                      0x00000001

#define _U3STA_OERR_POSITION                     0x00000001
#define _U3STA_OERR_MASK                         0x00000002
#define _U3STA_OERR_LENGTH                       0x00000001

#define _U3STA_FERR_POSITION                     0x00000002
#define _U3STA_FERR_MASK                         0x00000004
#define _U3STA_FERR_LENGTH                       0x00000001

#define _U3STA_PERR_POSITION                     0x00000003
#define _U3STA_PERR_MASK                         0x00000008
#define _U3STA_PERR_LENGTH                       0x00000001

#define _U3STA_RIDLE_POSITION                    0x00000004
#define _U3STA_RIDLE_MASK                        0x00000010
#define _U3STA_RIDLE_LENGTH                      0x00000001

#define _U3STA_ADDEN_POSITION                    0x00000005
#define _U3STA_ADDEN_MASK                        0x00000020
#define _U3STA_ADDEN_LENGTH                      0x00000001

#define _U3STA_URXISEL_POSITION                  0x00000006
#define _U3STA_URXISEL_MASK                      0x000000C0
#define _U3STA_URXISEL_LENGTH                    0x00000002

#define _U3STA_TRMT_POSITION                     0x00000008
#define _U3STA_TRMT_MASK                         0x00000100
#define _U3STA_TRMT_LENGTH                       0x00000001

#define _U3STA_UTXBF_POSITION                    0x00000009
#define _U3STA_UTXBF_MASK                        0x00000200
#define _U3STA_UTXBF_LENGTH                      0x00000001

#define _U3STA_UTXEN_POSITION                    0x0000000A
#define _U3STA_UTXEN_MASK                        0x00000400
#define _U3STA_UTXEN_LENGTH                      0x00000001

#define _U3STA_UTXBRK_POSITION                   0x0000000B
#define _U3STA_UTXBRK_MASK                       0x00000800
#define _U3STA_UTXBRK_LENGTH                     0x00000001

#define _U3STA_URXEN_POSITION                    0x0000000C
#define _U3STA_URXEN_MASK                        0x00001000
#define _U3STA_URXEN_LENGTH                      0x00000001

#define _U3STA_UTXINV_POSITION                   0x0000000D
#define _U3STA_UTXINV_MASK                       0x00002000
#define _U3STA_UTXINV_LENGTH                     0x00000001

#define _U3STA_UTXISEL_POSITION                  0x0000000E
#define _U3STA_UTXISEL_MASK                      0x0000C000
#define _U3STA_UTXISEL_LENGTH                    0x00000002

#define _U3STA_ADDR_POSITION                     0x00000010
#define _U3STA_ADDR_MASK                         0x00FF0000
#define _U3STA_ADDR_LENGTH                       0x00000008

#define _U3STA_ADM_EN_POSITION                   0x00000018
#define _U3STA_ADM_EN_MASK                       0x01000000
#define _U3STA_ADM_EN_LENGTH                     0x00000001

#define _U3STA_URXISEL0_POSITION                 0x00000006
#define _U3STA_URXISEL0_MASK                     0x00000040
#define _U3STA_URXISEL0_LENGTH                   0x00000001

#define _U3STA_URXISEL1_POSITION                 0x00000007
#define _U3STA_URXISEL1_MASK                     0x00000080
#define _U3STA_URXISEL1_LENGTH                   0x00000001

#define _U3STA_UTXISEL0_POSITION                 0x0000000E
#define _U3STA_UTXISEL0_MASK                     0x00004000
#define _U3STA_UTXISEL0_LENGTH                   0x00000001

#define _U3STA_UTXISEL1_POSITION                 0x0000000F
#define _U3STA_UTXISEL1_MASK                     0x00008000
#define _U3STA_UTXISEL1_LENGTH                   0x00000001

#define _U3STA_UTXSEL_POSITION                   0x0000000E
#define _U3STA_UTXSEL_MASK                       0x0000C000
#define _U3STA_UTXSEL_LENGTH                     0x00000002

#define _U3STA_w_POSITION                        0x00000000
#define _U3STA_w_MASK                            0xFFFFFFFF
#define _U3STA_w_LENGTH                          0x00000020

#define _U2BMODE_STSEL_POSITION                  0x00000000
#define _U2BMODE_STSEL_MASK                      0x00000001
#define _U2BMODE_STSEL_LENGTH                    0x00000001

#define _U2BMODE_PDSEL_POSITION                  0x00000001
#define _U2BMODE_PDSEL_MASK                      0x00000006
#define _U2BMODE_PDSEL_LENGTH                    0x00000002

#define _U2BMODE_BRGH_POSITION                   0x00000003
#define _U2BMODE_BRGH_MASK                       0x00000008
#define _U2BMODE_BRGH_LENGTH                     0x00000001

#define _U2BMODE_RXINV_POSITION                  0x00000004
#define _U2BMODE_RXINV_MASK                      0x00000010
#define _U2BMODE_RXINV_LENGTH                    0x00000001

#define _U2BMODE_ABAUD_POSITION                  0x00000005
#define _U2BMODE_ABAUD_MASK                      0x00000020
#define _U2BMODE_ABAUD_LENGTH                    0x00000001

#define _U2BMODE_LPBACK_POSITION                 0x00000006
#define _U2BMODE_LPBACK_MASK                     0x00000040
#define _U2BMODE_LPBACK_LENGTH                   0x00000001

#define _U2BMODE_WAKE_POSITION                   0x00000007
#define _U2BMODE_WAKE_MASK                       0x00000080
#define _U2BMODE_WAKE_LENGTH                     0x00000001

#define _U2BMODE_IREN_POSITION                   0x0000000C
#define _U2BMODE_IREN_MASK                       0x00001000
#define _U2BMODE_IREN_LENGTH                     0x00000001

#define _U2BMODE_SIDL_POSITION                   0x0000000D
#define _U2BMODE_SIDL_MASK                       0x00002000
#define _U2BMODE_SIDL_LENGTH                     0x00000001

#define _U2BMODE_ON_POSITION                     0x0000000F
#define _U2BMODE_ON_MASK                         0x00008000
#define _U2BMODE_ON_LENGTH                       0x00000001

#define _U2BMODE_PDSEL0_POSITION                 0x00000001
#define _U2BMODE_PDSEL0_MASK                     0x00000002
#define _U2BMODE_PDSEL0_LENGTH                   0x00000001

#define _U2BMODE_PDSEL1_POSITION                 0x00000002
#define _U2BMODE_PDSEL1_MASK                     0x00000004
#define _U2BMODE_PDSEL1_LENGTH                   0x00000001

#define _U2BMODE_USIDL_POSITION                  0x0000000D
#define _U2BMODE_USIDL_MASK                      0x00002000
#define _U2BMODE_USIDL_LENGTH                    0x00000001

#define _U2BMODE_UARTEN_POSITION                 0x0000000F
#define _U2BMODE_UARTEN_MASK                     0x00008000
#define _U2BMODE_UARTEN_LENGTH                   0x00000001

#define _U2BMODE_w_POSITION                      0x00000000
#define _U2BMODE_w_MASK                          0xFFFFFFFF
#define _U2BMODE_w_LENGTH                        0x00000020

#define _U6MODE_STSEL_POSITION                   0x00000000
#define _U6MODE_STSEL_MASK                       0x00000001
#define _U6MODE_STSEL_LENGTH                     0x00000001

#define _U6MODE_PDSEL_POSITION                   0x00000001
#define _U6MODE_PDSEL_MASK                       0x00000006
#define _U6MODE_PDSEL_LENGTH                     0x00000002

#define _U6MODE_BRGH_POSITION                    0x00000003
#define _U6MODE_BRGH_MASK                        0x00000008
#define _U6MODE_BRGH_LENGTH                      0x00000001

#define _U6MODE_RXINV_POSITION                   0x00000004
#define _U6MODE_RXINV_MASK                       0x00000010
#define _U6MODE_RXINV_LENGTH                     0x00000001

#define _U6MODE_ABAUD_POSITION                   0x00000005
#define _U6MODE_ABAUD_MASK                       0x00000020
#define _U6MODE_ABAUD_LENGTH                     0x00000001

#define _U6MODE_LPBACK_POSITION                  0x00000006
#define _U6MODE_LPBACK_MASK                      0x00000040
#define _U6MODE_LPBACK_LENGTH                    0x00000001

#define _U6MODE_WAKE_POSITION                    0x00000007
#define _U6MODE_WAKE_MASK                        0x00000080
#define _U6MODE_WAKE_LENGTH                      0x00000001

#define _U6MODE_IREN_POSITION                    0x0000000C
#define _U6MODE_IREN_MASK                        0x00001000
#define _U6MODE_IREN_LENGTH                      0x00000001

#define _U6MODE_SIDL_POSITION                    0x0000000D
#define _U6MODE_SIDL_MASK                        0x00002000
#define _U6MODE_SIDL_LENGTH                      0x00000001

#define _U6MODE_ON_POSITION                      0x0000000F
#define _U6MODE_ON_MASK                          0x00008000
#define _U6MODE_ON_LENGTH                        0x00000001

#define _U6MODE_PDSEL0_POSITION                  0x00000001
#define _U6MODE_PDSEL0_MASK                      0x00000002
#define _U6MODE_PDSEL0_LENGTH                    0x00000001

#define _U6MODE_PDSEL1_POSITION                  0x00000002
#define _U6MODE_PDSEL1_MASK                      0x00000004
#define _U6MODE_PDSEL1_LENGTH                    0x00000001

#define _U6MODE_USIDL_POSITION                   0x0000000D
#define _U6MODE_USIDL_MASK                       0x00002000
#define _U6MODE_USIDL_LENGTH                     0x00000001

#define _U6MODE_UARTEN_POSITION                  0x0000000F
#define _U6MODE_UARTEN_MASK                      0x00008000
#define _U6MODE_UARTEN_LENGTH                    0x00000001

#define _U6MODE_w_POSITION                       0x00000000
#define _U6MODE_w_MASK                           0xFFFFFFFF
#define _U6MODE_w_LENGTH                         0x00000020

#define _U2BSTA_URXDA_POSITION                   0x00000000
#define _U2BSTA_URXDA_MASK                       0x00000001
#define _U2BSTA_URXDA_LENGTH                     0x00000001

#define _U2BSTA_OERR_POSITION                    0x00000001
#define _U2BSTA_OERR_MASK                        0x00000002
#define _U2BSTA_OERR_LENGTH                      0x00000001

#define _U2BSTA_FERR_POSITION                    0x00000002
#define _U2BSTA_FERR_MASK                        0x00000004
#define _U2BSTA_FERR_LENGTH                      0x00000001

#define _U2BSTA_PERR_POSITION                    0x00000003
#define _U2BSTA_PERR_MASK                        0x00000008
#define _U2BSTA_PERR_LENGTH                      0x00000001

#define _U2BSTA_RIDLE_POSITION                   0x00000004
#define _U2BSTA_RIDLE_MASK                       0x00000010
#define _U2BSTA_RIDLE_LENGTH                     0x00000001

#define _U2BSTA_ADDEN_POSITION                   0x00000005
#define _U2BSTA_ADDEN_MASK                       0x00000020
#define _U2BSTA_ADDEN_LENGTH                     0x00000001

#define _U2BSTA_URXISEL_POSITION                 0x00000006
#define _U2BSTA_URXISEL_MASK                     0x000000C0
#define _U2BSTA_URXISEL_LENGTH                   0x00000002

#define _U2BSTA_TRMT_POSITION                    0x00000008
#define _U2BSTA_TRMT_MASK                        0x00000100
#define _U2BSTA_TRMT_LENGTH                      0x00000001

#define _U2BSTA_UTXBF_POSITION                   0x00000009
#define _U2BSTA_UTXBF_MASK                       0x00000200
#define _U2BSTA_UTXBF_LENGTH                     0x00000001

#define _U2BSTA_UTXEN_POSITION                   0x0000000A
#define _U2BSTA_UTXEN_MASK                       0x00000400
#define _U2BSTA_UTXEN_LENGTH                     0x00000001

#define _U2BSTA_UTXBRK_POSITION                  0x0000000B
#define _U2BSTA_UTXBRK_MASK                      0x00000800
#define _U2BSTA_UTXBRK_LENGTH                    0x00000001

#define _U2BSTA_URXEN_POSITION                   0x0000000C
#define _U2BSTA_URXEN_MASK                       0x00001000
#define _U2BSTA_URXEN_LENGTH                     0x00000001

#define _U2BSTA_UTXINV_POSITION                  0x0000000D
#define _U2BSTA_UTXINV_MASK                      0x00002000
#define _U2BSTA_UTXINV_LENGTH                    0x00000001

#define _U2BSTA_UTXISEL_POSITION                 0x0000000E
#define _U2BSTA_UTXISEL_MASK                     0x0000C000
#define _U2BSTA_UTXISEL_LENGTH                   0x00000002

#define _U2BSTA_ADDR_POSITION                    0x00000010
#define _U2BSTA_ADDR_MASK                        0x00FF0000
#define _U2BSTA_ADDR_LENGTH                      0x00000008

#define _U2BSTA_ADM_EN_POSITION                  0x00000018
#define _U2BSTA_ADM_EN_MASK                      0x01000000
#define _U2BSTA_ADM_EN_LENGTH                    0x00000001

#define _U2BSTA_URXISEL0_POSITION                0x00000006
#define _U2BSTA_URXISEL0_MASK                    0x00000040
#define _U2BSTA_URXISEL0_LENGTH                  0x00000001

#define _U2BSTA_URXISEL1_POSITION                0x00000007
#define _U2BSTA_URXISEL1_MASK                    0x00000080
#define _U2BSTA_URXISEL1_LENGTH                  0x00000001

#define _U2BSTA_UTXISEL0_POSITION                0x0000000E
#define _U2BSTA_UTXISEL0_MASK                    0x00004000
#define _U2BSTA_UTXISEL0_LENGTH                  0x00000001

#define _U2BSTA_UTXISEL1_POSITION                0x0000000F
#define _U2BSTA_UTXISEL1_MASK                    0x00008000
#define _U2BSTA_UTXISEL1_LENGTH                  0x00000001

#define _U2BSTA_UTXSEL_POSITION                  0x0000000E
#define _U2BSTA_UTXSEL_MASK                      0x0000C000
#define _U2BSTA_UTXSEL_LENGTH                    0x00000002

#define _U2BSTA_w_POSITION                       0x00000000
#define _U2BSTA_w_MASK                           0xFFFFFFFF
#define _U2BSTA_w_LENGTH                         0x00000020

#define _U6STA_URXDA_POSITION                    0x00000000
#define _U6STA_URXDA_MASK                        0x00000001
#define _U6STA_URXDA_LENGTH                      0x00000001

#define _U6STA_OERR_POSITION                     0x00000001
#define _U6STA_OERR_MASK                         0x00000002
#define _U6STA_OERR_LENGTH                       0x00000001

#define _U6STA_FERR_POSITION                     0x00000002
#define _U6STA_FERR_MASK                         0x00000004
#define _U6STA_FERR_LENGTH                       0x00000001

#define _U6STA_PERR_POSITION                     0x00000003
#define _U6STA_PERR_MASK                         0x00000008
#define _U6STA_PERR_LENGTH                       0x00000001

#define _U6STA_RIDLE_POSITION                    0x00000004
#define _U6STA_RIDLE_MASK                        0x00000010
#define _U6STA_RIDLE_LENGTH                      0x00000001

#define _U6STA_ADDEN_POSITION                    0x00000005
#define _U6STA_ADDEN_MASK                        0x00000020
#define _U6STA_ADDEN_LENGTH                      0x00000001

#define _U6STA_URXISEL_POSITION                  0x00000006
#define _U6STA_URXISEL_MASK                      0x000000C0
#define _U6STA_URXISEL_LENGTH                    0x00000002

#define _U6STA_TRMT_POSITION                     0x00000008
#define _U6STA_TRMT_MASK                         0x00000100
#define _U6STA_TRMT_LENGTH                       0x00000001

#define _U6STA_UTXBF_POSITION                    0x00000009
#define _U6STA_UTXBF_MASK                        0x00000200
#define _U6STA_UTXBF_LENGTH                      0x00000001

#define _U6STA_UTXEN_POSITION                    0x0000000A
#define _U6STA_UTXEN_MASK                        0x00000400
#define _U6STA_UTXEN_LENGTH                      0x00000001

#define _U6STA_UTXBRK_POSITION                   0x0000000B
#define _U6STA_UTXBRK_MASK                       0x00000800
#define _U6STA_UTXBRK_LENGTH                     0x00000001

#define _U6STA_URXEN_POSITION                    0x0000000C
#define _U6STA_URXEN_MASK                        0x00001000
#define _U6STA_URXEN_LENGTH                      0x00000001

#define _U6STA_UTXINV_POSITION                   0x0000000D
#define _U6STA_UTXINV_MASK                       0x00002000
#define _U6STA_UTXINV_LENGTH                     0x00000001

#define _U6STA_UTXISEL_POSITION                  0x0000000E
#define _U6STA_UTXISEL_MASK                      0x0000C000
#define _U6STA_UTXISEL_LENGTH                    0x00000002

#define _U6STA_ADDR_POSITION                     0x00000010
#define _U6STA_ADDR_MASK                         0x00FF0000
#define _U6STA_ADDR_LENGTH                       0x00000008

#define _U6STA_ADM_EN_POSITION                   0x00000018
#define _U6STA_ADM_EN_MASK                       0x01000000
#define _U6STA_ADM_EN_LENGTH                     0x00000001

#define _U6STA_URXISEL0_POSITION                 0x00000006
#define _U6STA_URXISEL0_MASK                     0x00000040
#define _U6STA_URXISEL0_LENGTH                   0x00000001

#define _U6STA_URXISEL1_POSITION                 0x00000007
#define _U6STA_URXISEL1_MASK                     0x00000080
#define _U6STA_URXISEL1_LENGTH                   0x00000001

#define _U6STA_UTXISEL0_POSITION                 0x0000000E
#define _U6STA_UTXISEL0_MASK                     0x00004000
#define _U6STA_UTXISEL0_LENGTH                   0x00000001

#define _U6STA_UTXISEL1_POSITION                 0x0000000F
#define _U6STA_UTXISEL1_MASK                     0x00008000
#define _U6STA_UTXISEL1_LENGTH                   0x00000001

#define _U6STA_UTXSEL_POSITION                   0x0000000E
#define _U6STA_UTXSEL_MASK                       0x0000C000
#define _U6STA_UTXSEL_LENGTH                     0x00000002

#define _U6STA_w_POSITION                        0x00000000
#define _U6STA_w_MASK                            0xFFFFFFFF
#define _U6STA_w_LENGTH                          0x00000020

#define _U2MODE_STSEL_POSITION                   0x00000000
#define _U2MODE_STSEL_MASK                       0x00000001
#define _U2MODE_STSEL_LENGTH                     0x00000001

#define _U2MODE_PDSEL_POSITION                   0x00000001
#define _U2MODE_PDSEL_MASK                       0x00000006
#define _U2MODE_PDSEL_LENGTH                     0x00000002

#define _U2MODE_BRGH_POSITION                    0x00000003
#define _U2MODE_BRGH_MASK                        0x00000008
#define _U2MODE_BRGH_LENGTH                      0x00000001

#define _U2MODE_RXINV_POSITION                   0x00000004
#define _U2MODE_RXINV_MASK                       0x00000010
#define _U2MODE_RXINV_LENGTH                     0x00000001

#define _U2MODE_ABAUD_POSITION                   0x00000005
#define _U2MODE_ABAUD_MASK                       0x00000020
#define _U2MODE_ABAUD_LENGTH                     0x00000001

#define _U2MODE_LPBACK_POSITION                  0x00000006
#define _U2MODE_LPBACK_MASK                      0x00000040
#define _U2MODE_LPBACK_LENGTH                    0x00000001

#define _U2MODE_WAKE_POSITION                    0x00000007
#define _U2MODE_WAKE_MASK                        0x00000080
#define _U2MODE_WAKE_LENGTH                      0x00000001

#define _U2MODE_UEN_POSITION                     0x00000008
#define _U2MODE_UEN_MASK                         0x00000300
#define _U2MODE_UEN_LENGTH                       0x00000002

#define _U2MODE_RTSMD_POSITION                   0x0000000B
#define _U2MODE_RTSMD_MASK                       0x00000800
#define _U2MODE_RTSMD_LENGTH                     0x00000001

#define _U2MODE_IREN_POSITION                    0x0000000C
#define _U2MODE_IREN_MASK                        0x00001000
#define _U2MODE_IREN_LENGTH                      0x00000001

#define _U2MODE_SIDL_POSITION                    0x0000000D
#define _U2MODE_SIDL_MASK                        0x00002000
#define _U2MODE_SIDL_LENGTH                      0x00000001

#define _U2MODE_ON_POSITION                      0x0000000F
#define _U2MODE_ON_MASK                          0x00008000
#define _U2MODE_ON_LENGTH                        0x00000001

#define _U2MODE_PDSEL0_POSITION                  0x00000001
#define _U2MODE_PDSEL0_MASK                      0x00000002
#define _U2MODE_PDSEL0_LENGTH                    0x00000001

#define _U2MODE_PDSEL1_POSITION                  0x00000002
#define _U2MODE_PDSEL1_MASK                      0x00000004
#define _U2MODE_PDSEL1_LENGTH                    0x00000001

#define _U2MODE_UEN0_POSITION                    0x00000008
#define _U2MODE_UEN0_MASK                        0x00000100
#define _U2MODE_UEN0_LENGTH                      0x00000001

#define _U2MODE_UEN1_POSITION                    0x00000009
#define _U2MODE_UEN1_MASK                        0x00000200
#define _U2MODE_UEN1_LENGTH                      0x00000001

#define _U2MODE_USIDL_POSITION                   0x0000000D
#define _U2MODE_USIDL_MASK                       0x00002000
#define _U2MODE_USIDL_LENGTH                     0x00000001

#define _U2MODE_UARTEN_POSITION                  0x0000000F
#define _U2MODE_UARTEN_MASK                      0x00008000
#define _U2MODE_UARTEN_LENGTH                    0x00000001

#define _U2MODE_w_POSITION                       0x00000000
#define _U2MODE_w_MASK                           0xFFFFFFFF
#define _U2MODE_w_LENGTH                         0x00000020

#define _U3AMODE_STSEL_POSITION                  0x00000000
#define _U3AMODE_STSEL_MASK                      0x00000001
#define _U3AMODE_STSEL_LENGTH                    0x00000001

#define _U3AMODE_PDSEL_POSITION                  0x00000001
#define _U3AMODE_PDSEL_MASK                      0x00000006
#define _U3AMODE_PDSEL_LENGTH                    0x00000002

#define _U3AMODE_BRGH_POSITION                   0x00000003
#define _U3AMODE_BRGH_MASK                       0x00000008
#define _U3AMODE_BRGH_LENGTH                     0x00000001

#define _U3AMODE_RXINV_POSITION                  0x00000004
#define _U3AMODE_RXINV_MASK                      0x00000010
#define _U3AMODE_RXINV_LENGTH                    0x00000001

#define _U3AMODE_ABAUD_POSITION                  0x00000005
#define _U3AMODE_ABAUD_MASK                      0x00000020
#define _U3AMODE_ABAUD_LENGTH                    0x00000001

#define _U3AMODE_LPBACK_POSITION                 0x00000006
#define _U3AMODE_LPBACK_MASK                     0x00000040
#define _U3AMODE_LPBACK_LENGTH                   0x00000001

#define _U3AMODE_WAKE_POSITION                   0x00000007
#define _U3AMODE_WAKE_MASK                       0x00000080
#define _U3AMODE_WAKE_LENGTH                     0x00000001

#define _U3AMODE_UEN_POSITION                    0x00000008
#define _U3AMODE_UEN_MASK                        0x00000300
#define _U3AMODE_UEN_LENGTH                      0x00000002

#define _U3AMODE_RTSMD_POSITION                  0x0000000B
#define _U3AMODE_RTSMD_MASK                      0x00000800
#define _U3AMODE_RTSMD_LENGTH                    0x00000001

#define _U3AMODE_IREN_POSITION                   0x0000000C
#define _U3AMODE_IREN_MASK                       0x00001000
#define _U3AMODE_IREN_LENGTH                     0x00000001

#define _U3AMODE_SIDL_POSITION                   0x0000000D
#define _U3AMODE_SIDL_MASK                       0x00002000
#define _U3AMODE_SIDL_LENGTH                     0x00000001

#define _U3AMODE_ON_POSITION                     0x0000000F
#define _U3AMODE_ON_MASK                         0x00008000
#define _U3AMODE_ON_LENGTH                       0x00000001

#define _U3AMODE_PDSEL0_POSITION                 0x00000001
#define _U3AMODE_PDSEL0_MASK                     0x00000002
#define _U3AMODE_PDSEL0_LENGTH                   0x00000001

#define _U3AMODE_PDSEL1_POSITION                 0x00000002
#define _U3AMODE_PDSEL1_MASK                     0x00000004
#define _U3AMODE_PDSEL1_LENGTH                   0x00000001

#define _U3AMODE_UEN0_POSITION                   0x00000008
#define _U3AMODE_UEN0_MASK                       0x00000100
#define _U3AMODE_UEN0_LENGTH                     0x00000001

#define _U3AMODE_UEN1_POSITION                   0x00000009
#define _U3AMODE_UEN1_MASK                       0x00000200
#define _U3AMODE_UEN1_LENGTH                     0x00000001

#define _U3AMODE_USIDL_POSITION                  0x0000000D
#define _U3AMODE_USIDL_MASK                      0x00002000
#define _U3AMODE_USIDL_LENGTH                    0x00000001

#define _U3AMODE_UARTEN_POSITION                 0x0000000F
#define _U3AMODE_UARTEN_MASK                     0x00008000
#define _U3AMODE_UARTEN_LENGTH                   0x00000001

#define _U3AMODE_w_POSITION                      0x00000000
#define _U3AMODE_w_MASK                          0xFFFFFFFF
#define _U3AMODE_w_LENGTH                        0x00000020

#define _U2STA_URXDA_POSITION                    0x00000000
#define _U2STA_URXDA_MASK                        0x00000001
#define _U2STA_URXDA_LENGTH                      0x00000001

#define _U2STA_OERR_POSITION                     0x00000001
#define _U2STA_OERR_MASK                         0x00000002
#define _U2STA_OERR_LENGTH                       0x00000001

#define _U2STA_FERR_POSITION                     0x00000002
#define _U2STA_FERR_MASK                         0x00000004
#define _U2STA_FERR_LENGTH                       0x00000001

#define _U2STA_PERR_POSITION                     0x00000003
#define _U2STA_PERR_MASK                         0x00000008
#define _U2STA_PERR_LENGTH                       0x00000001

#define _U2STA_RIDLE_POSITION                    0x00000004
#define _U2STA_RIDLE_MASK                        0x00000010
#define _U2STA_RIDLE_LENGTH                      0x00000001

#define _U2STA_ADDEN_POSITION                    0x00000005
#define _U2STA_ADDEN_MASK                        0x00000020
#define _U2STA_ADDEN_LENGTH                      0x00000001

#define _U2STA_URXISEL_POSITION                  0x00000006
#define _U2STA_URXISEL_MASK                      0x000000C0
#define _U2STA_URXISEL_LENGTH                    0x00000002

#define _U2STA_TRMT_POSITION                     0x00000008
#define _U2STA_TRMT_MASK                         0x00000100
#define _U2STA_TRMT_LENGTH                       0x00000001

#define _U2STA_UTXBF_POSITION                    0x00000009
#define _U2STA_UTXBF_MASK                        0x00000200
#define _U2STA_UTXBF_LENGTH                      0x00000001

#define _U2STA_UTXEN_POSITION                    0x0000000A
#define _U2STA_UTXEN_MASK                        0x00000400
#define _U2STA_UTXEN_LENGTH                      0x00000001

#define _U2STA_UTXBRK_POSITION                   0x0000000B
#define _U2STA_UTXBRK_MASK                       0x00000800
#define _U2STA_UTXBRK_LENGTH                     0x00000001

#define _U2STA_URXEN_POSITION                    0x0000000C
#define _U2STA_URXEN_MASK                        0x00001000
#define _U2STA_URXEN_LENGTH                      0x00000001

#define _U2STA_UTXINV_POSITION                   0x0000000D
#define _U2STA_UTXINV_MASK                       0x00002000
#define _U2STA_UTXINV_LENGTH                     0x00000001

#define _U2STA_UTXISEL_POSITION                  0x0000000E
#define _U2STA_UTXISEL_MASK                      0x0000C000
#define _U2STA_UTXISEL_LENGTH                    0x00000002

#define _U2STA_ADDR_POSITION                     0x00000010
#define _U2STA_ADDR_MASK                         0x00FF0000
#define _U2STA_ADDR_LENGTH                       0x00000008

#define _U2STA_ADM_EN_POSITION                   0x00000018
#define _U2STA_ADM_EN_MASK                       0x01000000
#define _U2STA_ADM_EN_LENGTH                     0x00000001

#define _U2STA_URXISEL0_POSITION                 0x00000006
#define _U2STA_URXISEL0_MASK                     0x00000040
#define _U2STA_URXISEL0_LENGTH                   0x00000001

#define _U2STA_URXISEL1_POSITION                 0x00000007
#define _U2STA_URXISEL1_MASK                     0x00000080
#define _U2STA_URXISEL1_LENGTH                   0x00000001

#define _U2STA_UTXISEL0_POSITION                 0x0000000E
#define _U2STA_UTXISEL0_MASK                     0x00004000
#define _U2STA_UTXISEL0_LENGTH                   0x00000001

#define _U2STA_UTXISEL1_POSITION                 0x0000000F
#define _U2STA_UTXISEL1_MASK                     0x00008000
#define _U2STA_UTXISEL1_LENGTH                   0x00000001

#define _U2STA_UTXSEL_POSITION                   0x0000000E
#define _U2STA_UTXSEL_MASK                       0x0000C000
#define _U2STA_UTXSEL_LENGTH                     0x00000002

#define _U2STA_w_POSITION                        0x00000000
#define _U2STA_w_MASK                            0xFFFFFFFF
#define _U2STA_w_LENGTH                          0x00000020

#define _U3ASTA_URXDA_POSITION                   0x00000000
#define _U3ASTA_URXDA_MASK                       0x00000001
#define _U3ASTA_URXDA_LENGTH                     0x00000001

#define _U3ASTA_OERR_POSITION                    0x00000001
#define _U3ASTA_OERR_MASK                        0x00000002
#define _U3ASTA_OERR_LENGTH                      0x00000001

#define _U3ASTA_FERR_POSITION                    0x00000002
#define _U3ASTA_FERR_MASK                        0x00000004
#define _U3ASTA_FERR_LENGTH                      0x00000001

#define _U3ASTA_PERR_POSITION                    0x00000003
#define _U3ASTA_PERR_MASK                        0x00000008
#define _U3ASTA_PERR_LENGTH                      0x00000001

#define _U3ASTA_RIDLE_POSITION                   0x00000004
#define _U3ASTA_RIDLE_MASK                       0x00000010
#define _U3ASTA_RIDLE_LENGTH                     0x00000001

#define _U3ASTA_ADDEN_POSITION                   0x00000005
#define _U3ASTA_ADDEN_MASK                       0x00000020
#define _U3ASTA_ADDEN_LENGTH                     0x00000001

#define _U3ASTA_URXISEL_POSITION                 0x00000006
#define _U3ASTA_URXISEL_MASK                     0x000000C0
#define _U3ASTA_URXISEL_LENGTH                   0x00000002

#define _U3ASTA_TRMT_POSITION                    0x00000008
#define _U3ASTA_TRMT_MASK                        0x00000100
#define _U3ASTA_TRMT_LENGTH                      0x00000001

#define _U3ASTA_UTXBF_POSITION                   0x00000009
#define _U3ASTA_UTXBF_MASK                       0x00000200
#define _U3ASTA_UTXBF_LENGTH                     0x00000001

#define _U3ASTA_UTXEN_POSITION                   0x0000000A
#define _U3ASTA_UTXEN_MASK                       0x00000400
#define _U3ASTA_UTXEN_LENGTH                     0x00000001

#define _U3ASTA_UTXBRK_POSITION                  0x0000000B
#define _U3ASTA_UTXBRK_MASK                      0x00000800
#define _U3ASTA_UTXBRK_LENGTH                    0x00000001

#define _U3ASTA_URXEN_POSITION                   0x0000000C
#define _U3ASTA_URXEN_MASK                       0x00001000
#define _U3ASTA_URXEN_LENGTH                     0x00000001

#define _U3ASTA_UTXINV_POSITION                  0x0000000D
#define _U3ASTA_UTXINV_MASK                      0x00002000
#define _U3ASTA_UTXINV_LENGTH                    0x00000001

#define _U3ASTA_UTXISEL_POSITION                 0x0000000E
#define _U3ASTA_UTXISEL_MASK                     0x0000C000
#define _U3ASTA_UTXISEL_LENGTH                   0x00000002

#define _U3ASTA_ADDR_POSITION                    0x00000010
#define _U3ASTA_ADDR_MASK                        0x00FF0000
#define _U3ASTA_ADDR_LENGTH                      0x00000008

#define _U3ASTA_ADM_EN_POSITION                  0x00000018
#define _U3ASTA_ADM_EN_MASK                      0x01000000
#define _U3ASTA_ADM_EN_LENGTH                    0x00000001

#define _U3ASTA_URXISEL0_POSITION                0x00000006
#define _U3ASTA_URXISEL0_MASK                    0x00000040
#define _U3ASTA_URXISEL0_LENGTH                  0x00000001

#define _U3ASTA_URXISEL1_POSITION                0x00000007
#define _U3ASTA_URXISEL1_MASK                    0x00000080
#define _U3ASTA_URXISEL1_LENGTH                  0x00000001

#define _U3ASTA_UTXISEL0_POSITION                0x0000000E
#define _U3ASTA_UTXISEL0_MASK                    0x00004000
#define _U3ASTA_UTXISEL0_LENGTH                  0x00000001

#define _U3ASTA_UTXISEL1_POSITION                0x0000000F
#define _U3ASTA_UTXISEL1_MASK                    0x00008000
#define _U3ASTA_UTXISEL1_LENGTH                  0x00000001

#define _U3ASTA_UTXSEL_POSITION                  0x0000000E
#define _U3ASTA_UTXSEL_MASK                      0x0000C000
#define _U3ASTA_UTXSEL_LENGTH                    0x00000002

#define _U3ASTA_w_POSITION                       0x00000000
#define _U3ASTA_w_MASK                           0xFFFFFFFF
#define _U3ASTA_w_LENGTH                         0x00000020

#define _U3BMODE_STSEL_POSITION                  0x00000000
#define _U3BMODE_STSEL_MASK                      0x00000001
#define _U3BMODE_STSEL_LENGTH                    0x00000001

#define _U3BMODE_PDSEL_POSITION                  0x00000001
#define _U3BMODE_PDSEL_MASK                      0x00000006
#define _U3BMODE_PDSEL_LENGTH                    0x00000002

#define _U3BMODE_BRGH_POSITION                   0x00000003
#define _U3BMODE_BRGH_MASK                       0x00000008
#define _U3BMODE_BRGH_LENGTH                     0x00000001

#define _U3BMODE_RXINV_POSITION                  0x00000004
#define _U3BMODE_RXINV_MASK                      0x00000010
#define _U3BMODE_RXINV_LENGTH                    0x00000001

#define _U3BMODE_ABAUD_POSITION                  0x00000005
#define _U3BMODE_ABAUD_MASK                      0x00000020
#define _U3BMODE_ABAUD_LENGTH                    0x00000001

#define _U3BMODE_LPBACK_POSITION                 0x00000006
#define _U3BMODE_LPBACK_MASK                     0x00000040
#define _U3BMODE_LPBACK_LENGTH                   0x00000001

#define _U3BMODE_WAKE_POSITION                   0x00000007
#define _U3BMODE_WAKE_MASK                       0x00000080
#define _U3BMODE_WAKE_LENGTH                     0x00000001

#define _U3BMODE_IREN_POSITION                   0x0000000C
#define _U3BMODE_IREN_MASK                       0x00001000
#define _U3BMODE_IREN_LENGTH                     0x00000001

#define _U3BMODE_SIDL_POSITION                   0x0000000D
#define _U3BMODE_SIDL_MASK                       0x00002000
#define _U3BMODE_SIDL_LENGTH                     0x00000001

#define _U3BMODE_ON_POSITION                     0x0000000F
#define _U3BMODE_ON_MASK                         0x00008000
#define _U3BMODE_ON_LENGTH                       0x00000001

#define _U3BMODE_PDSEL0_POSITION                 0x00000001
#define _U3BMODE_PDSEL0_MASK                     0x00000002
#define _U3BMODE_PDSEL0_LENGTH                   0x00000001

#define _U3BMODE_PDSEL1_POSITION                 0x00000002
#define _U3BMODE_PDSEL1_MASK                     0x00000004
#define _U3BMODE_PDSEL1_LENGTH                   0x00000001

#define _U3BMODE_USIDL_POSITION                  0x0000000D
#define _U3BMODE_USIDL_MASK                      0x00002000
#define _U3BMODE_USIDL_LENGTH                    0x00000001

#define _U3BMODE_UARTEN_POSITION                 0x0000000F
#define _U3BMODE_UARTEN_MASK                     0x00008000
#define _U3BMODE_UARTEN_LENGTH                   0x00000001

#define _U3BMODE_w_POSITION                      0x00000000
#define _U3BMODE_w_MASK                          0xFFFFFFFF
#define _U3BMODE_w_LENGTH                        0x00000020

#define _U5MODE_STSEL_POSITION                   0x00000000
#define _U5MODE_STSEL_MASK                       0x00000001
#define _U5MODE_STSEL_LENGTH                     0x00000001

#define _U5MODE_PDSEL_POSITION                   0x00000001
#define _U5MODE_PDSEL_MASK                       0x00000006
#define _U5MODE_PDSEL_LENGTH                     0x00000002

#define _U5MODE_BRGH_POSITION                    0x00000003
#define _U5MODE_BRGH_MASK                        0x00000008
#define _U5MODE_BRGH_LENGTH                      0x00000001

#define _U5MODE_RXINV_POSITION                   0x00000004
#define _U5MODE_RXINV_MASK                       0x00000010
#define _U5MODE_RXINV_LENGTH                     0x00000001

#define _U5MODE_ABAUD_POSITION                   0x00000005
#define _U5MODE_ABAUD_MASK                       0x00000020
#define _U5MODE_ABAUD_LENGTH                     0x00000001

#define _U5MODE_LPBACK_POSITION                  0x00000006
#define _U5MODE_LPBACK_MASK                      0x00000040
#define _U5MODE_LPBACK_LENGTH                    0x00000001

#define _U5MODE_WAKE_POSITION                    0x00000007
#define _U5MODE_WAKE_MASK                        0x00000080
#define _U5MODE_WAKE_LENGTH                      0x00000001

#define _U5MODE_IREN_POSITION                    0x0000000C
#define _U5MODE_IREN_MASK                        0x00001000
#define _U5MODE_IREN_LENGTH                      0x00000001

#define _U5MODE_SIDL_POSITION                    0x0000000D
#define _U5MODE_SIDL_MASK                        0x00002000
#define _U5MODE_SIDL_LENGTH                      0x00000001

#define _U5MODE_ON_POSITION                      0x0000000F
#define _U5MODE_ON_MASK                          0x00008000
#define _U5MODE_ON_LENGTH                        0x00000001

#define _U5MODE_PDSEL0_POSITION                  0x00000001
#define _U5MODE_PDSEL0_MASK                      0x00000002
#define _U5MODE_PDSEL0_LENGTH                    0x00000001

#define _U5MODE_PDSEL1_POSITION                  0x00000002
#define _U5MODE_PDSEL1_MASK                      0x00000004
#define _U5MODE_PDSEL1_LENGTH                    0x00000001

#define _U5MODE_USIDL_POSITION                   0x0000000D
#define _U5MODE_USIDL_MASK                       0x00002000
#define _U5MODE_USIDL_LENGTH                     0x00000001

#define _U5MODE_UARTEN_POSITION                  0x0000000F
#define _U5MODE_UARTEN_MASK                      0x00008000
#define _U5MODE_UARTEN_LENGTH                    0x00000001

#define _U5MODE_w_POSITION                       0x00000000
#define _U5MODE_w_MASK                           0xFFFFFFFF
#define _U5MODE_w_LENGTH                         0x00000020

#define _U3BSTA_URXDA_POSITION                   0x00000000
#define _U3BSTA_URXDA_MASK                       0x00000001
#define _U3BSTA_URXDA_LENGTH                     0x00000001

#define _U3BSTA_OERR_POSITION                    0x00000001
#define _U3BSTA_OERR_MASK                        0x00000002
#define _U3BSTA_OERR_LENGTH                      0x00000001

#define _U3BSTA_FERR_POSITION                    0x00000002
#define _U3BSTA_FERR_MASK                        0x00000004
#define _U3BSTA_FERR_LENGTH                      0x00000001

#define _U3BSTA_PERR_POSITION                    0x00000003
#define _U3BSTA_PERR_MASK                        0x00000008
#define _U3BSTA_PERR_LENGTH                      0x00000001

#define _U3BSTA_RIDLE_POSITION                   0x00000004
#define _U3BSTA_RIDLE_MASK                       0x00000010
#define _U3BSTA_RIDLE_LENGTH                     0x00000001

#define _U3BSTA_ADDEN_POSITION                   0x00000005
#define _U3BSTA_ADDEN_MASK                       0x00000020
#define _U3BSTA_ADDEN_LENGTH                     0x00000001

#define _U3BSTA_URXISEL_POSITION                 0x00000006
#define _U3BSTA_URXISEL_MASK                     0x000000C0
#define _U3BSTA_URXISEL_LENGTH                   0x00000002

#define _U3BSTA_TRMT_POSITION                    0x00000008
#define _U3BSTA_TRMT_MASK                        0x00000100
#define _U3BSTA_TRMT_LENGTH                      0x00000001

#define _U3BSTA_UTXBF_POSITION                   0x00000009
#define _U3BSTA_UTXBF_MASK                       0x00000200
#define _U3BSTA_UTXBF_LENGTH                     0x00000001

#define _U3BSTA_UTXEN_POSITION                   0x0000000A
#define _U3BSTA_UTXEN_MASK                       0x00000400
#define _U3BSTA_UTXEN_LENGTH                     0x00000001

#define _U3BSTA_UTXBRK_POSITION                  0x0000000B
#define _U3BSTA_UTXBRK_MASK                      0x00000800
#define _U3BSTA_UTXBRK_LENGTH                    0x00000001

#define _U3BSTA_URXEN_POSITION                   0x0000000C
#define _U3BSTA_URXEN_MASK                       0x00001000
#define _U3BSTA_URXEN_LENGTH                     0x00000001

#define _U3BSTA_UTXINV_POSITION                  0x0000000D
#define _U3BSTA_UTXINV_MASK                      0x00002000
#define _U3BSTA_UTXINV_LENGTH                    0x00000001

#define _U3BSTA_UTXISEL_POSITION                 0x0000000E
#define _U3BSTA_UTXISEL_MASK                     0x0000C000
#define _U3BSTA_UTXISEL_LENGTH                   0x00000002

#define _U3BSTA_ADDR_POSITION                    0x00000010
#define _U3BSTA_ADDR_MASK                        0x00FF0000
#define _U3BSTA_ADDR_LENGTH                      0x00000008

#define _U3BSTA_ADM_EN_POSITION                  0x00000018
#define _U3BSTA_ADM_EN_MASK                      0x01000000
#define _U3BSTA_ADM_EN_LENGTH                    0x00000001

#define _U3BSTA_URXISEL0_POSITION                0x00000006
#define _U3BSTA_URXISEL0_MASK                    0x00000040
#define _U3BSTA_URXISEL0_LENGTH                  0x00000001

#define _U3BSTA_URXISEL1_POSITION                0x00000007
#define _U3BSTA_URXISEL1_MASK                    0x00000080
#define _U3BSTA_URXISEL1_LENGTH                  0x00000001

#define _U3BSTA_UTXISEL0_POSITION                0x0000000E
#define _U3BSTA_UTXISEL0_MASK                    0x00004000
#define _U3BSTA_UTXISEL0_LENGTH                  0x00000001

#define _U3BSTA_UTXISEL1_POSITION                0x0000000F
#define _U3BSTA_UTXISEL1_MASK                    0x00008000
#define _U3BSTA_UTXISEL1_LENGTH                  0x00000001

#define _U3BSTA_UTXSEL_POSITION                  0x0000000E
#define _U3BSTA_UTXSEL_MASK                      0x0000C000
#define _U3BSTA_UTXSEL_LENGTH                    0x00000002

#define _U3BSTA_w_POSITION                       0x00000000
#define _U3BSTA_w_MASK                           0xFFFFFFFF
#define _U3BSTA_w_LENGTH                         0x00000020

#define _U5STA_URXDA_POSITION                    0x00000000
#define _U5STA_URXDA_MASK                        0x00000001
#define _U5STA_URXDA_LENGTH                      0x00000001

#define _U5STA_OERR_POSITION                     0x00000001
#define _U5STA_OERR_MASK                         0x00000002
#define _U5STA_OERR_LENGTH                       0x00000001

#define _U5STA_FERR_POSITION                     0x00000002
#define _U5STA_FERR_MASK                         0x00000004
#define _U5STA_FERR_LENGTH                       0x00000001

#define _U5STA_PERR_POSITION                     0x00000003
#define _U5STA_PERR_MASK                         0x00000008
#define _U5STA_PERR_LENGTH                       0x00000001

#define _U5STA_RIDLE_POSITION                    0x00000004
#define _U5STA_RIDLE_MASK                        0x00000010
#define _U5STA_RIDLE_LENGTH                      0x00000001

#define _U5STA_ADDEN_POSITION                    0x00000005
#define _U5STA_ADDEN_MASK                        0x00000020
#define _U5STA_ADDEN_LENGTH                      0x00000001

#define _U5STA_URXISEL_POSITION                  0x00000006
#define _U5STA_URXISEL_MASK                      0x000000C0
#define _U5STA_URXISEL_LENGTH                    0x00000002

#define _U5STA_TRMT_POSITION                     0x00000008
#define _U5STA_TRMT_MASK                         0x00000100
#define _U5STA_TRMT_LENGTH                       0x00000001

#define _U5STA_UTXBF_POSITION                    0x00000009
#define _U5STA_UTXBF_MASK                        0x00000200
#define _U5STA_UTXBF_LENGTH                      0x00000001

#define _U5STA_UTXEN_POSITION                    0x0000000A
#define _U5STA_UTXEN_MASK                        0x00000400
#define _U5STA_UTXEN_LENGTH                      0x00000001

#define _U5STA_UTXBRK_POSITION                   0x0000000B
#define _U5STA_UTXBRK_MASK                       0x00000800
#define _U5STA_UTXBRK_LENGTH                     0x00000001

#define _U5STA_URXEN_POSITION                    0x0000000C
#define _U5STA_URXEN_MASK                        0x00001000
#define _U5STA_URXEN_LENGTH                      0x00000001

#define _U5STA_UTXINV_POSITION                   0x0000000D
#define _U5STA_UTXINV_MASK                       0x00002000
#define _U5STA_UTXINV_LENGTH                     0x00000001

#define _U5STA_UTXISEL_POSITION                  0x0000000E
#define _U5STA_UTXISEL_MASK                      0x0000C000
#define _U5STA_UTXISEL_LENGTH                    0x00000002

#define _U5STA_ADDR_POSITION                     0x00000010
#define _U5STA_ADDR_MASK                         0x00FF0000
#define _U5STA_ADDR_LENGTH                       0x00000008

#define _U5STA_ADM_EN_POSITION                   0x00000018
#define _U5STA_ADM_EN_MASK                       0x01000000
#define _U5STA_ADM_EN_LENGTH                     0x00000001

#define _U5STA_URXISEL0_POSITION                 0x00000006
#define _U5STA_URXISEL0_MASK                     0x00000040
#define _U5STA_URXISEL0_LENGTH                   0x00000001

#define _U5STA_URXISEL1_POSITION                 0x00000007
#define _U5STA_URXISEL1_MASK                     0x00000080
#define _U5STA_URXISEL1_LENGTH                   0x00000001

#define _U5STA_UTXISEL0_POSITION                 0x0000000E
#define _U5STA_UTXISEL0_MASK                     0x00004000
#define _U5STA_UTXISEL0_LENGTH                   0x00000001

#define _U5STA_UTXISEL1_POSITION                 0x0000000F
#define _U5STA_UTXISEL1_MASK                     0x00008000
#define _U5STA_UTXISEL1_LENGTH                   0x00000001

#define _U5STA_UTXSEL_POSITION                   0x0000000E
#define _U5STA_UTXSEL_MASK                       0x0000C000
#define _U5STA_UTXSEL_LENGTH                     0x00000002

#define _U5STA_w_POSITION                        0x00000000
#define _U5STA_w_MASK                            0xFFFFFFFF
#define _U5STA_w_LENGTH                          0x00000020

#define _PMCON_RDSP_POSITION                     0x00000000
#define _PMCON_RDSP_MASK                         0x00000001
#define _PMCON_RDSP_LENGTH                       0x00000001

#define _PMCON_WRSP_POSITION                     0x00000001
#define _PMCON_WRSP_MASK                         0x00000002
#define _PMCON_WRSP_LENGTH                       0x00000001

#define _PMCON_CS1P_POSITION                     0x00000003
#define _PMCON_CS1P_MASK                         0x00000008
#define _PMCON_CS1P_LENGTH                       0x00000001

#define _PMCON_CS2P_POSITION                     0x00000004
#define _PMCON_CS2P_MASK                         0x00000010
#define _PMCON_CS2P_LENGTH                       0x00000001

#define _PMCON_ALP_POSITION                      0x00000005
#define _PMCON_ALP_MASK                          0x00000020
#define _PMCON_ALP_LENGTH                        0x00000001

#define _PMCON_CSF_POSITION                      0x00000006
#define _PMCON_CSF_MASK                          0x000000C0
#define _PMCON_CSF_LENGTH                        0x00000002

#define _PMCON_PTRDEN_POSITION                   0x00000008
#define _PMCON_PTRDEN_MASK                       0x00000100
#define _PMCON_PTRDEN_LENGTH                     0x00000001

#define _PMCON_PTWREN_POSITION                   0x00000009
#define _PMCON_PTWREN_MASK                       0x00000200
#define _PMCON_PTWREN_LENGTH                     0x00000001

#define _PMCON_PMPTTL_POSITION                   0x0000000A
#define _PMCON_PMPTTL_MASK                       0x00000400
#define _PMCON_PMPTTL_LENGTH                     0x00000001

#define _PMCON_ADRMUX_POSITION                   0x0000000B
#define _PMCON_ADRMUX_MASK                       0x00001800
#define _PMCON_ADRMUX_LENGTH                     0x00000002

#define _PMCON_SIDL_POSITION                     0x0000000D
#define _PMCON_SIDL_MASK                         0x00002000
#define _PMCON_SIDL_LENGTH                       0x00000001

#define _PMCON_ON_POSITION                       0x0000000F
#define _PMCON_ON_MASK                           0x00008000
#define _PMCON_ON_LENGTH                         0x00000001

#define _PMCON_CSF0_POSITION                     0x00000006
#define _PMCON_CSF0_MASK                         0x00000040
#define _PMCON_CSF0_LENGTH                       0x00000001

#define _PMCON_CSF1_POSITION                     0x00000007
#define _PMCON_CSF1_MASK                         0x00000080
#define _PMCON_CSF1_LENGTH                       0x00000001

#define _PMCON_ADRMUX0_POSITION                  0x0000000B
#define _PMCON_ADRMUX0_MASK                      0x00000800
#define _PMCON_ADRMUX0_LENGTH                    0x00000001

#define _PMCON_ADRMUX1_POSITION                  0x0000000C
#define _PMCON_ADRMUX1_MASK                      0x00001000
#define _PMCON_ADRMUX1_LENGTH                    0x00000001

#define _PMCON_PSIDL_POSITION                    0x0000000D
#define _PMCON_PSIDL_MASK                        0x00002000
#define _PMCON_PSIDL_LENGTH                      0x00000001

#define _PMCON_PMPEN_POSITION                    0x0000000F
#define _PMCON_PMPEN_MASK                        0x00008000
#define _PMCON_PMPEN_LENGTH                      0x00000001

#define _PMCON_w_POSITION                        0x00000000
#define _PMCON_w_MASK                            0xFFFFFFFF
#define _PMCON_w_LENGTH                          0x00000020

#define _PMMODE_WAITE_POSITION                   0x00000000
#define _PMMODE_WAITE_MASK                       0x00000003
#define _PMMODE_WAITE_LENGTH                     0x00000002

#define _PMMODE_WAITM_POSITION                   0x00000002
#define _PMMODE_WAITM_MASK                       0x0000003C
#define _PMMODE_WAITM_LENGTH                     0x00000004

#define _PMMODE_WAITB_POSITION                   0x00000006
#define _PMMODE_WAITB_MASK                       0x000000C0
#define _PMMODE_WAITB_LENGTH                     0x00000002

#define _PMMODE_MODE_POSITION                    0x00000008
#define _PMMODE_MODE_MASK                        0x00000300
#define _PMMODE_MODE_LENGTH                      0x00000002

#define _PMMODE_MODE16_POSITION                  0x0000000A
#define _PMMODE_MODE16_MASK                      0x00000400
#define _PMMODE_MODE16_LENGTH                    0x00000001

#define _PMMODE_INCM_POSITION                    0x0000000B
#define _PMMODE_INCM_MASK                        0x00001800
#define _PMMODE_INCM_LENGTH                      0x00000002

#define _PMMODE_IRQM_POSITION                    0x0000000D
#define _PMMODE_IRQM_MASK                        0x00006000
#define _PMMODE_IRQM_LENGTH                      0x00000002

#define _PMMODE_BUSY_POSITION                    0x0000000F
#define _PMMODE_BUSY_MASK                        0x00008000
#define _PMMODE_BUSY_LENGTH                      0x00000001

#define _PMMODE_WAITE0_POSITION                  0x00000000
#define _PMMODE_WAITE0_MASK                      0x00000001
#define _PMMODE_WAITE0_LENGTH                    0x00000001

#define _PMMODE_WAITE1_POSITION                  0x00000001
#define _PMMODE_WAITE1_MASK                      0x00000002
#define _PMMODE_WAITE1_LENGTH                    0x00000001

#define _PMMODE_WAITM0_POSITION                  0x00000002
#define _PMMODE_WAITM0_MASK                      0x00000004
#define _PMMODE_WAITM0_LENGTH                    0x00000001

#define _PMMODE_WAITM1_POSITION                  0x00000003
#define _PMMODE_WAITM1_MASK                      0x00000008
#define _PMMODE_WAITM1_LENGTH                    0x00000001

#define _PMMODE_WAITM2_POSITION                  0x00000004
#define _PMMODE_WAITM2_MASK                      0x00000010
#define _PMMODE_WAITM2_LENGTH                    0x00000001

#define _PMMODE_WAITM3_POSITION                  0x00000005
#define _PMMODE_WAITM3_MASK                      0x00000020
#define _PMMODE_WAITM3_LENGTH                    0x00000001

#define _PMMODE_WAITB0_POSITION                  0x00000006
#define _PMMODE_WAITB0_MASK                      0x00000040
#define _PMMODE_WAITB0_LENGTH                    0x00000001

#define _PMMODE_WAITB1_POSITION                  0x00000007
#define _PMMODE_WAITB1_MASK                      0x00000080
#define _PMMODE_WAITB1_LENGTH                    0x00000001

#define _PMMODE_MODE0_POSITION                   0x00000008
#define _PMMODE_MODE0_MASK                       0x00000100
#define _PMMODE_MODE0_LENGTH                     0x00000001

#define _PMMODE_MODE1_POSITION                   0x00000009
#define _PMMODE_MODE1_MASK                       0x00000200
#define _PMMODE_MODE1_LENGTH                     0x00000001

#define _PMMODE_INCM0_POSITION                   0x0000000B
#define _PMMODE_INCM0_MASK                       0x00000800
#define _PMMODE_INCM0_LENGTH                     0x00000001

#define _PMMODE_INCM1_POSITION                   0x0000000C
#define _PMMODE_INCM1_MASK                       0x00001000
#define _PMMODE_INCM1_LENGTH                     0x00000001

#define _PMMODE_IRQM0_POSITION                   0x0000000D
#define _PMMODE_IRQM0_MASK                       0x00002000
#define _PMMODE_IRQM0_LENGTH                     0x00000001

#define _PMMODE_IRQM1_POSITION                   0x0000000E
#define _PMMODE_IRQM1_MASK                       0x00004000
#define _PMMODE_IRQM1_LENGTH                     0x00000001

#define _PMMODE_w_POSITION                       0x00000000
#define _PMMODE_w_MASK                           0xFFFFFFFF
#define _PMMODE_w_LENGTH                         0x00000020

#define _PMADDR_ADDR_POSITION                    0x00000000
#define _PMADDR_ADDR_MASK                        0x00003FFF
#define _PMADDR_ADDR_LENGTH                      0x0000000E

#define _PMADDR_CS1_POSITION                     0x0000000E
#define _PMADDR_CS1_MASK                         0x00004000
#define _PMADDR_CS1_LENGTH                       0x00000001

#define _PMADDR_CS2_POSITION                     0x0000000F
#define _PMADDR_CS2_MASK                         0x00008000
#define _PMADDR_CS2_LENGTH                       0x00000001

#define _PMADDR_PADDR_POSITION                   0x00000000
#define _PMADDR_PADDR_MASK                       0x00003FFF
#define _PMADDR_PADDR_LENGTH                     0x0000000E

#define _PMADDR_CS_POSITION                      0x0000000E
#define _PMADDR_CS_MASK                          0x0000C000
#define _PMADDR_CS_LENGTH                        0x00000002

#define _PMADDR_w_POSITION                       0x00000000
#define _PMADDR_w_MASK                           0xFFFFFFFF
#define _PMADDR_w_LENGTH                         0x00000020

#define _PMAEN_PTEN_POSITION                     0x00000000
#define _PMAEN_PTEN_MASK                         0x0000FFFF
#define _PMAEN_PTEN_LENGTH                       0x00000010

#define _PMAEN_PTEN0_POSITION                    0x00000000
#define _PMAEN_PTEN0_MASK                        0x00000001
#define _PMAEN_PTEN0_LENGTH                      0x00000001

#define _PMAEN_PTEN1_POSITION                    0x00000001
#define _PMAEN_PTEN1_MASK                        0x00000002
#define _PMAEN_PTEN1_LENGTH                      0x00000001

#define _PMAEN_PTEN2_POSITION                    0x00000002
#define _PMAEN_PTEN2_MASK                        0x00000004
#define _PMAEN_PTEN2_LENGTH                      0x00000001

#define _PMAEN_PTEN3_POSITION                    0x00000003
#define _PMAEN_PTEN3_MASK                        0x00000008
#define _PMAEN_PTEN3_LENGTH                      0x00000001

#define _PMAEN_PTEN4_POSITION                    0x00000004
#define _PMAEN_PTEN4_MASK                        0x00000010
#define _PMAEN_PTEN4_LENGTH                      0x00000001

#define _PMAEN_PTEN5_POSITION                    0x00000005
#define _PMAEN_PTEN5_MASK                        0x00000020
#define _PMAEN_PTEN5_LENGTH                      0x00000001

#define _PMAEN_PTEN6_POSITION                    0x00000006
#define _PMAEN_PTEN6_MASK                        0x00000040
#define _PMAEN_PTEN6_LENGTH                      0x00000001

#define _PMAEN_PTEN7_POSITION                    0x00000007
#define _PMAEN_PTEN7_MASK                        0x00000080
#define _PMAEN_PTEN7_LENGTH                      0x00000001

#define _PMAEN_PTEN8_POSITION                    0x00000008
#define _PMAEN_PTEN8_MASK                        0x00000100
#define _PMAEN_PTEN8_LENGTH                      0x00000001

#define _PMAEN_PTEN9_POSITION                    0x00000009
#define _PMAEN_PTEN9_MASK                        0x00000200
#define _PMAEN_PTEN9_LENGTH                      0x00000001

#define _PMAEN_PTEN10_POSITION                   0x0000000A
#define _PMAEN_PTEN10_MASK                       0x00000400
#define _PMAEN_PTEN10_LENGTH                     0x00000001

#define _PMAEN_PTEN11_POSITION                   0x0000000B
#define _PMAEN_PTEN11_MASK                       0x00000800
#define _PMAEN_PTEN11_LENGTH                     0x00000001

#define _PMAEN_PTEN12_POSITION                   0x0000000C
#define _PMAEN_PTEN12_MASK                       0x00001000
#define _PMAEN_PTEN12_LENGTH                     0x00000001

#define _PMAEN_PTEN13_POSITION                   0x0000000D
#define _PMAEN_PTEN13_MASK                       0x00002000
#define _PMAEN_PTEN13_LENGTH                     0x00000001

#define _PMAEN_PTEN14_POSITION                   0x0000000E
#define _PMAEN_PTEN14_MASK                       0x00004000
#define _PMAEN_PTEN14_LENGTH                     0x00000001

#define _PMAEN_PTEN15_POSITION                   0x0000000F
#define _PMAEN_PTEN15_MASK                       0x00008000
#define _PMAEN_PTEN15_LENGTH                     0x00000001

#define _PMAEN_w_POSITION                        0x00000000
#define _PMAEN_w_MASK                            0xFFFFFFFF
#define _PMAEN_w_LENGTH                          0x00000020

#define _PMSTAT_OB0E_POSITION                    0x00000000
#define _PMSTAT_OB0E_MASK                        0x00000001
#define _PMSTAT_OB0E_LENGTH                      0x00000001

#define _PMSTAT_OB1E_POSITION                    0x00000001
#define _PMSTAT_OB1E_MASK                        0x00000002
#define _PMSTAT_OB1E_LENGTH                      0x00000001

#define _PMSTAT_OB2E_POSITION                    0x00000002
#define _PMSTAT_OB2E_MASK                        0x00000004
#define _PMSTAT_OB2E_LENGTH                      0x00000001

#define _PMSTAT_OB3E_POSITION                    0x00000003
#define _PMSTAT_OB3E_MASK                        0x00000008
#define _PMSTAT_OB3E_LENGTH                      0x00000001

#define _PMSTAT_OBUF_POSITION                    0x00000006
#define _PMSTAT_OBUF_MASK                        0x00000040
#define _PMSTAT_OBUF_LENGTH                      0x00000001

#define _PMSTAT_OBE_POSITION                     0x00000007
#define _PMSTAT_OBE_MASK                         0x00000080
#define _PMSTAT_OBE_LENGTH                       0x00000001

#define _PMSTAT_IB0F_POSITION                    0x00000008
#define _PMSTAT_IB0F_MASK                        0x00000100
#define _PMSTAT_IB0F_LENGTH                      0x00000001

#define _PMSTAT_IB1F_POSITION                    0x00000009
#define _PMSTAT_IB1F_MASK                        0x00000200
#define _PMSTAT_IB1F_LENGTH                      0x00000001

#define _PMSTAT_IB2F_POSITION                    0x0000000A
#define _PMSTAT_IB2F_MASK                        0x00000400
#define _PMSTAT_IB2F_LENGTH                      0x00000001

#define _PMSTAT_IB3F_POSITION                    0x0000000B
#define _PMSTAT_IB3F_MASK                        0x00000800
#define _PMSTAT_IB3F_LENGTH                      0x00000001

#define _PMSTAT_IBOV_POSITION                    0x0000000E
#define _PMSTAT_IBOV_MASK                        0x00004000
#define _PMSTAT_IBOV_LENGTH                      0x00000001

#define _PMSTAT_IBF_POSITION                     0x0000000F
#define _PMSTAT_IBF_MASK                         0x00008000
#define _PMSTAT_IBF_LENGTH                       0x00000001

#define _PMSTAT_w_POSITION                       0x00000000
#define _PMSTAT_w_MASK                           0xFFFFFFFF
#define _PMSTAT_w_LENGTH                         0x00000020

#define _AD1CON1_DONE_POSITION                   0x00000000
#define _AD1CON1_DONE_MASK                       0x00000001
#define _AD1CON1_DONE_LENGTH                     0x00000001

#define _AD1CON1_SAMP_POSITION                   0x00000001
#define _AD1CON1_SAMP_MASK                       0x00000002
#define _AD1CON1_SAMP_LENGTH                     0x00000001

#define _AD1CON1_ASAM_POSITION                   0x00000002
#define _AD1CON1_ASAM_MASK                       0x00000004
#define _AD1CON1_ASAM_LENGTH                     0x00000001

#define _AD1CON1_CLRASAM_POSITION                0x00000004
#define _AD1CON1_CLRASAM_MASK                    0x00000010
#define _AD1CON1_CLRASAM_LENGTH                  0x00000001

#define _AD1CON1_SSRC_POSITION                   0x00000005
#define _AD1CON1_SSRC_MASK                       0x000000E0
#define _AD1CON1_SSRC_LENGTH                     0x00000003

#define _AD1CON1_FORM_POSITION                   0x00000008
#define _AD1CON1_FORM_MASK                       0x00000700
#define _AD1CON1_FORM_LENGTH                     0x00000003

#define _AD1CON1_SIDL_POSITION                   0x0000000D
#define _AD1CON1_SIDL_MASK                       0x00002000
#define _AD1CON1_SIDL_LENGTH                     0x00000001

#define _AD1CON1_ON_POSITION                     0x0000000F
#define _AD1CON1_ON_MASK                         0x00008000
#define _AD1CON1_ON_LENGTH                       0x00000001

#define _AD1CON1_SSRC0_POSITION                  0x00000005
#define _AD1CON1_SSRC0_MASK                      0x00000020
#define _AD1CON1_SSRC0_LENGTH                    0x00000001

#define _AD1CON1_SSRC1_POSITION                  0x00000006
#define _AD1CON1_SSRC1_MASK                      0x00000040
#define _AD1CON1_SSRC1_LENGTH                    0x00000001

#define _AD1CON1_SSRC2_POSITION                  0x00000007
#define _AD1CON1_SSRC2_MASK                      0x00000080
#define _AD1CON1_SSRC2_LENGTH                    0x00000001

#define _AD1CON1_FORM0_POSITION                  0x00000008
#define _AD1CON1_FORM0_MASK                      0x00000100
#define _AD1CON1_FORM0_LENGTH                    0x00000001

#define _AD1CON1_FORM1_POSITION                  0x00000009
#define _AD1CON1_FORM1_MASK                      0x00000200
#define _AD1CON1_FORM1_LENGTH                    0x00000001

#define _AD1CON1_FORM2_POSITION                  0x0000000A
#define _AD1CON1_FORM2_MASK                      0x00000400
#define _AD1CON1_FORM2_LENGTH                    0x00000001

#define _AD1CON1_ADSIDL_POSITION                 0x0000000D
#define _AD1CON1_ADSIDL_MASK                     0x00002000
#define _AD1CON1_ADSIDL_LENGTH                   0x00000001

#define _AD1CON1_ADON_POSITION                   0x0000000F
#define _AD1CON1_ADON_MASK                       0x00008000
#define _AD1CON1_ADON_LENGTH                     0x00000001

#define _AD1CON1_w_POSITION                      0x00000000
#define _AD1CON1_w_MASK                          0xFFFFFFFF
#define _AD1CON1_w_LENGTH                        0x00000020

#define _AD1CON2_ALTS_POSITION                   0x00000000
#define _AD1CON2_ALTS_MASK                       0x00000001
#define _AD1CON2_ALTS_LENGTH                     0x00000001

#define _AD1CON2_BUFM_POSITION                   0x00000001
#define _AD1CON2_BUFM_MASK                       0x00000002
#define _AD1CON2_BUFM_LENGTH                     0x00000001

#define _AD1CON2_SMPI_POSITION                   0x00000002
#define _AD1CON2_SMPI_MASK                       0x0000003C
#define _AD1CON2_SMPI_LENGTH                     0x00000004

#define _AD1CON2_BUFS_POSITION                   0x00000007
#define _AD1CON2_BUFS_MASK                       0x00000080
#define _AD1CON2_BUFS_LENGTH                     0x00000001

#define _AD1CON2_CSCNA_POSITION                  0x0000000A
#define _AD1CON2_CSCNA_MASK                      0x00000400
#define _AD1CON2_CSCNA_LENGTH                    0x00000001

#define _AD1CON2_OFFCAL_POSITION                 0x0000000C
#define _AD1CON2_OFFCAL_MASK                     0x00001000
#define _AD1CON2_OFFCAL_LENGTH                   0x00000001

#define _AD1CON2_VCFG_POSITION                   0x0000000D
#define _AD1CON2_VCFG_MASK                       0x0000E000
#define _AD1CON2_VCFG_LENGTH                     0x00000003

#define _AD1CON2_SMPI0_POSITION                  0x00000002
#define _AD1CON2_SMPI0_MASK                      0x00000004
#define _AD1CON2_SMPI0_LENGTH                    0x00000001

#define _AD1CON2_SMPI1_POSITION                  0x00000003
#define _AD1CON2_SMPI1_MASK                      0x00000008
#define _AD1CON2_SMPI1_LENGTH                    0x00000001

#define _AD1CON2_SMPI2_POSITION                  0x00000004
#define _AD1CON2_SMPI2_MASK                      0x00000010
#define _AD1CON2_SMPI2_LENGTH                    0x00000001

#define _AD1CON2_SMPI3_POSITION                  0x00000005
#define _AD1CON2_SMPI3_MASK                      0x00000020
#define _AD1CON2_SMPI3_LENGTH                    0x00000001

#define _AD1CON2_VCFG0_POSITION                  0x0000000D
#define _AD1CON2_VCFG0_MASK                      0x00002000
#define _AD1CON2_VCFG0_LENGTH                    0x00000001

#define _AD1CON2_VCFG1_POSITION                  0x0000000E
#define _AD1CON2_VCFG1_MASK                      0x00004000
#define _AD1CON2_VCFG1_LENGTH                    0x00000001

#define _AD1CON2_VCFG2_POSITION                  0x0000000F
#define _AD1CON2_VCFG2_MASK                      0x00008000
#define _AD1CON2_VCFG2_LENGTH                    0x00000001

#define _AD1CON2_w_POSITION                      0x00000000
#define _AD1CON2_w_MASK                          0xFFFFFFFF
#define _AD1CON2_w_LENGTH                        0x00000020

#define _AD1CON3_ADCS_POSITION                   0x00000000
#define _AD1CON3_ADCS_MASK                       0x000000FF
#define _AD1CON3_ADCS_LENGTH                     0x00000008

#define _AD1CON3_SAMC_POSITION                   0x00000008
#define _AD1CON3_SAMC_MASK                       0x00001F00
#define _AD1CON3_SAMC_LENGTH                     0x00000005

#define _AD1CON3_ADRC_POSITION                   0x0000000F
#define _AD1CON3_ADRC_MASK                       0x00008000
#define _AD1CON3_ADRC_LENGTH                     0x00000001

#define _AD1CON3_ADCS0_POSITION                  0x00000000
#define _AD1CON3_ADCS0_MASK                      0x00000001
#define _AD1CON3_ADCS0_LENGTH                    0x00000001

#define _AD1CON3_ADCS1_POSITION                  0x00000001
#define _AD1CON3_ADCS1_MASK                      0x00000002
#define _AD1CON3_ADCS1_LENGTH                    0x00000001

#define _AD1CON3_ADCS2_POSITION                  0x00000002
#define _AD1CON3_ADCS2_MASK                      0x00000004
#define _AD1CON3_ADCS2_LENGTH                    0x00000001

#define _AD1CON3_ADCS3_POSITION                  0x00000003
#define _AD1CON3_ADCS3_MASK                      0x00000008
#define _AD1CON3_ADCS3_LENGTH                    0x00000001

#define _AD1CON3_ADCS4_POSITION                  0x00000004
#define _AD1CON3_ADCS4_MASK                      0x00000010
#define _AD1CON3_ADCS4_LENGTH                    0x00000001

#define _AD1CON3_ADCS5_POSITION                  0x00000005
#define _AD1CON3_ADCS5_MASK                      0x00000020
#define _AD1CON3_ADCS5_LENGTH                    0x00000001

#define _AD1CON3_ADCS6_POSITION                  0x00000006
#define _AD1CON3_ADCS6_MASK                      0x00000040
#define _AD1CON3_ADCS6_LENGTH                    0x00000001

#define _AD1CON3_ADCS7_POSITION                  0x00000007
#define _AD1CON3_ADCS7_MASK                      0x00000080
#define _AD1CON3_ADCS7_LENGTH                    0x00000001

#define _AD1CON3_SAMC0_POSITION                  0x00000008
#define _AD1CON3_SAMC0_MASK                      0x00000100
#define _AD1CON3_SAMC0_LENGTH                    0x00000001

#define _AD1CON3_SAMC1_POSITION                  0x00000009
#define _AD1CON3_SAMC1_MASK                      0x00000200
#define _AD1CON3_SAMC1_LENGTH                    0x00000001

#define _AD1CON3_SAMC2_POSITION                  0x0000000A
#define _AD1CON3_SAMC2_MASK                      0x00000400
#define _AD1CON3_SAMC2_LENGTH                    0x00000001

#define _AD1CON3_SAMC3_POSITION                  0x0000000B
#define _AD1CON3_SAMC3_MASK                      0x00000800
#define _AD1CON3_SAMC3_LENGTH                    0x00000001

#define _AD1CON3_SAMC4_POSITION                  0x0000000C
#define _AD1CON3_SAMC4_MASK                      0x00001000
#define _AD1CON3_SAMC4_LENGTH                    0x00000001

#define _AD1CON3_w_POSITION                      0x00000000
#define _AD1CON3_w_MASK                          0xFFFFFFFF
#define _AD1CON3_w_LENGTH                        0x00000020

#define _AD1CHS_CH0SA_POSITION                   0x00000010
#define _AD1CHS_CH0SA_MASK                       0x000F0000
#define _AD1CHS_CH0SA_LENGTH                     0x00000004

#define _AD1CHS_CH0NA_POSITION                   0x00000017
#define _AD1CHS_CH0NA_MASK                       0x00800000
#define _AD1CHS_CH0NA_LENGTH                     0x00000001

#define _AD1CHS_CH0SB_POSITION                   0x00000018
#define _AD1CHS_CH0SB_MASK                       0x0F000000
#define _AD1CHS_CH0SB_LENGTH                     0x00000004

#define _AD1CHS_CH0NB_POSITION                   0x0000001F
#define _AD1CHS_CH0NB_MASK                       0x80000000
#define _AD1CHS_CH0NB_LENGTH                     0x00000001

#define _AD1CHS_CH0SA0_POSITION                  0x00000010
#define _AD1CHS_CH0SA0_MASK                      0x00010000
#define _AD1CHS_CH0SA0_LENGTH                    0x00000001

#define _AD1CHS_CH0SA1_POSITION                  0x00000011
#define _AD1CHS_CH0SA1_MASK                      0x00020000
#define _AD1CHS_CH0SA1_LENGTH                    0x00000001

#define _AD1CHS_CH0SA2_POSITION                  0x00000012
#define _AD1CHS_CH0SA2_MASK                      0x00040000
#define _AD1CHS_CH0SA2_LENGTH                    0x00000001

#define _AD1CHS_CH0SA3_POSITION                  0x00000013
#define _AD1CHS_CH0SA3_MASK                      0x00080000
#define _AD1CHS_CH0SA3_LENGTH                    0x00000001

#define _AD1CHS_CH0SB0_POSITION                  0x00000018
#define _AD1CHS_CH0SB0_MASK                      0x01000000
#define _AD1CHS_CH0SB0_LENGTH                    0x00000001

#define _AD1CHS_CH0SB1_POSITION                  0x00000019
#define _AD1CHS_CH0SB1_MASK                      0x02000000
#define _AD1CHS_CH0SB1_LENGTH                    0x00000001

#define _AD1CHS_CH0SB2_POSITION                  0x0000001A
#define _AD1CHS_CH0SB2_MASK                      0x04000000
#define _AD1CHS_CH0SB2_LENGTH                    0x00000001

#define _AD1CHS_CH0SB3_POSITION                  0x0000001B
#define _AD1CHS_CH0SB3_MASK                      0x08000000
#define _AD1CHS_CH0SB3_LENGTH                    0x00000001

#define _AD1CHS_w_POSITION                       0x00000000
#define _AD1CHS_w_MASK                           0xFFFFFFFF
#define _AD1CHS_w_LENGTH                         0x00000020

#define _AD1CSSL_CSSL_POSITION                   0x00000000
#define _AD1CSSL_CSSL_MASK                       0x0000FFFF
#define _AD1CSSL_CSSL_LENGTH                     0x00000010

#define _AD1CSSL_CSSL0_POSITION                  0x00000000
#define _AD1CSSL_CSSL0_MASK                      0x00000001
#define _AD1CSSL_CSSL0_LENGTH                    0x00000001

#define _AD1CSSL_CSSL1_POSITION                  0x00000001
#define _AD1CSSL_CSSL1_MASK                      0x00000002
#define _AD1CSSL_CSSL1_LENGTH                    0x00000001

#define _AD1CSSL_CSSL2_POSITION                  0x00000002
#define _AD1CSSL_CSSL2_MASK                      0x00000004
#define _AD1CSSL_CSSL2_LENGTH                    0x00000001

#define _AD1CSSL_CSSL3_POSITION                  0x00000003
#define _AD1CSSL_CSSL3_MASK                      0x00000008
#define _AD1CSSL_CSSL3_LENGTH                    0x00000001

#define _AD1CSSL_CSSL4_POSITION                  0x00000004
#define _AD1CSSL_CSSL4_MASK                      0x00000010
#define _AD1CSSL_CSSL4_LENGTH                    0x00000001

#define _AD1CSSL_CSSL5_POSITION                  0x00000005
#define _AD1CSSL_CSSL5_MASK                      0x00000020
#define _AD1CSSL_CSSL5_LENGTH                    0x00000001

#define _AD1CSSL_CSSL6_POSITION                  0x00000006
#define _AD1CSSL_CSSL6_MASK                      0x00000040
#define _AD1CSSL_CSSL6_LENGTH                    0x00000001

#define _AD1CSSL_CSSL7_POSITION                  0x00000007
#define _AD1CSSL_CSSL7_MASK                      0x00000080
#define _AD1CSSL_CSSL7_LENGTH                    0x00000001

#define _AD1CSSL_CSSL8_POSITION                  0x00000008
#define _AD1CSSL_CSSL8_MASK                      0x00000100
#define _AD1CSSL_CSSL8_LENGTH                    0x00000001

#define _AD1CSSL_CSSL9_POSITION                  0x00000009
#define _AD1CSSL_CSSL9_MASK                      0x00000200
#define _AD1CSSL_CSSL9_LENGTH                    0x00000001

#define _AD1CSSL_CSSL10_POSITION                 0x0000000A
#define _AD1CSSL_CSSL10_MASK                     0x00000400
#define _AD1CSSL_CSSL10_LENGTH                   0x00000001

#define _AD1CSSL_CSSL11_POSITION                 0x0000000B
#define _AD1CSSL_CSSL11_MASK                     0x00000800
#define _AD1CSSL_CSSL11_LENGTH                   0x00000001

#define _AD1CSSL_CSSL12_POSITION                 0x0000000C
#define _AD1CSSL_CSSL12_MASK                     0x00001000
#define _AD1CSSL_CSSL12_LENGTH                   0x00000001

#define _AD1CSSL_CSSL13_POSITION                 0x0000000D
#define _AD1CSSL_CSSL13_MASK                     0x00002000
#define _AD1CSSL_CSSL13_LENGTH                   0x00000001

#define _AD1CSSL_CSSL14_POSITION                 0x0000000E
#define _AD1CSSL_CSSL14_MASK                     0x00004000
#define _AD1CSSL_CSSL14_LENGTH                   0x00000001

#define _AD1CSSL_CSSL15_POSITION                 0x0000000F
#define _AD1CSSL_CSSL15_MASK                     0x00008000
#define _AD1CSSL_CSSL15_LENGTH                   0x00000001

#define _AD1CSSL_w_POSITION                      0x00000000
#define _AD1CSSL_w_MASK                          0xFFFFFFFF
#define _AD1CSSL_w_LENGTH                        0x00000020

#define _AD1PCFG_PCFG_POSITION                   0x00000000
#define _AD1PCFG_PCFG_MASK                       0x0000FFFF
#define _AD1PCFG_PCFG_LENGTH                     0x00000010

#define _AD1PCFG_PCFG0_POSITION                  0x00000000
#define _AD1PCFG_PCFG0_MASK                      0x00000001
#define _AD1PCFG_PCFG0_LENGTH                    0x00000001

#define _AD1PCFG_PCFG1_POSITION                  0x00000001
#define _AD1PCFG_PCFG1_MASK                      0x00000002
#define _AD1PCFG_PCFG1_LENGTH                    0x00000001

#define _AD1PCFG_PCFG2_POSITION                  0x00000002
#define _AD1PCFG_PCFG2_MASK                      0x00000004
#define _AD1PCFG_PCFG2_LENGTH                    0x00000001

#define _AD1PCFG_PCFG3_POSITION                  0x00000003
#define _AD1PCFG_PCFG3_MASK                      0x00000008
#define _AD1PCFG_PCFG3_LENGTH                    0x00000001

#define _AD1PCFG_PCFG4_POSITION                  0x00000004
#define _AD1PCFG_PCFG4_MASK                      0x00000010
#define _AD1PCFG_PCFG4_LENGTH                    0x00000001

#define _AD1PCFG_PCFG5_POSITION                  0x00000005
#define _AD1PCFG_PCFG5_MASK                      0x00000020
#define _AD1PCFG_PCFG5_LENGTH                    0x00000001

#define _AD1PCFG_PCFG6_POSITION                  0x00000006
#define _AD1PCFG_PCFG6_MASK                      0x00000040
#define _AD1PCFG_PCFG6_LENGTH                    0x00000001

#define _AD1PCFG_PCFG7_POSITION                  0x00000007
#define _AD1PCFG_PCFG7_MASK                      0x00000080
#define _AD1PCFG_PCFG7_LENGTH                    0x00000001

#define _AD1PCFG_PCFG8_POSITION                  0x00000008
#define _AD1PCFG_PCFG8_MASK                      0x00000100
#define _AD1PCFG_PCFG8_LENGTH                    0x00000001

#define _AD1PCFG_PCFG9_POSITION                  0x00000009
#define _AD1PCFG_PCFG9_MASK                      0x00000200
#define _AD1PCFG_PCFG9_LENGTH                    0x00000001

#define _AD1PCFG_PCFG10_POSITION                 0x0000000A
#define _AD1PCFG_PCFG10_MASK                     0x00000400
#define _AD1PCFG_PCFG10_LENGTH                   0x00000001

#define _AD1PCFG_PCFG11_POSITION                 0x0000000B
#define _AD1PCFG_PCFG11_MASK                     0x00000800
#define _AD1PCFG_PCFG11_LENGTH                   0x00000001

#define _AD1PCFG_PCFG12_POSITION                 0x0000000C
#define _AD1PCFG_PCFG12_MASK                     0x00001000
#define _AD1PCFG_PCFG12_LENGTH                   0x00000001

#define _AD1PCFG_PCFG13_POSITION                 0x0000000D
#define _AD1PCFG_PCFG13_MASK                     0x00002000
#define _AD1PCFG_PCFG13_LENGTH                   0x00000001

#define _AD1PCFG_PCFG14_POSITION                 0x0000000E
#define _AD1PCFG_PCFG14_MASK                     0x00004000
#define _AD1PCFG_PCFG14_LENGTH                   0x00000001

#define _AD1PCFG_PCFG15_POSITION                 0x0000000F
#define _AD1PCFG_PCFG15_MASK                     0x00008000
#define _AD1PCFG_PCFG15_LENGTH                   0x00000001

#define _AD1PCFG_w_POSITION                      0x00000000
#define _AD1PCFG_w_MASK                          0xFFFFFFFF
#define _AD1PCFG_w_LENGTH                        0x00000020

#define _CVRCON_CVR_POSITION                     0x00000000
#define _CVRCON_CVR_MASK                         0x0000000F
#define _CVRCON_CVR_LENGTH                       0x00000004

#define _CVRCON_CVRSS_POSITION                   0x00000004
#define _CVRCON_CVRSS_MASK                       0x00000010
#define _CVRCON_CVRSS_LENGTH                     0x00000001

#define _CVRCON_CVRR_POSITION                    0x00000005
#define _CVRCON_CVRR_MASK                        0x00000020
#define _CVRCON_CVRR_LENGTH                      0x00000001

#define _CVRCON_CVROE_POSITION                   0x00000006
#define _CVRCON_CVROE_MASK                       0x00000040
#define _CVRCON_CVROE_LENGTH                     0x00000001

#define _CVRCON_ON_POSITION                      0x0000000F
#define _CVRCON_ON_MASK                          0x00008000
#define _CVRCON_ON_LENGTH                        0x00000001

#define _CVRCON_CVR0_POSITION                    0x00000000
#define _CVRCON_CVR0_MASK                        0x00000001
#define _CVRCON_CVR0_LENGTH                      0x00000001

#define _CVRCON_CVR1_POSITION                    0x00000001
#define _CVRCON_CVR1_MASK                        0x00000002
#define _CVRCON_CVR1_LENGTH                      0x00000001

#define _CVRCON_CVR2_POSITION                    0x00000002
#define _CVRCON_CVR2_MASK                        0x00000004
#define _CVRCON_CVR2_LENGTH                      0x00000001

#define _CVRCON_CVR3_POSITION                    0x00000003
#define _CVRCON_CVR3_MASK                        0x00000008
#define _CVRCON_CVR3_LENGTH                      0x00000001

#define _CVRCON_w_POSITION                       0x00000000
#define _CVRCON_w_MASK                           0xFFFFFFFF
#define _CVRCON_w_LENGTH                         0x00000020

#define _CM1CON_CCH_POSITION                     0x00000000
#define _CM1CON_CCH_MASK                         0x00000003
#define _CM1CON_CCH_LENGTH                       0x00000002

#define _CM1CON_CREF_POSITION                    0x00000004
#define _CM1CON_CREF_MASK                        0x00000010
#define _CM1CON_CREF_LENGTH                      0x00000001

#define _CM1CON_EVPOL_POSITION                   0x00000006
#define _CM1CON_EVPOL_MASK                       0x000000C0
#define _CM1CON_EVPOL_LENGTH                     0x00000002

#define _CM1CON_COUT_POSITION                    0x00000008
#define _CM1CON_COUT_MASK                        0x00000100
#define _CM1CON_COUT_LENGTH                      0x00000001

#define _CM1CON_CPOL_POSITION                    0x0000000D
#define _CM1CON_CPOL_MASK                        0x00002000
#define _CM1CON_CPOL_LENGTH                      0x00000001

#define _CM1CON_COE_POSITION                     0x0000000E
#define _CM1CON_COE_MASK                         0x00004000
#define _CM1CON_COE_LENGTH                       0x00000001

#define _CM1CON_ON_POSITION                      0x0000000F
#define _CM1CON_ON_MASK                          0x00008000
#define _CM1CON_ON_LENGTH                        0x00000001

#define _CM1CON_CCH0_POSITION                    0x00000000
#define _CM1CON_CCH0_MASK                        0x00000001
#define _CM1CON_CCH0_LENGTH                      0x00000001

#define _CM1CON_CCH1_POSITION                    0x00000001
#define _CM1CON_CCH1_MASK                        0x00000002
#define _CM1CON_CCH1_LENGTH                      0x00000001

#define _CM1CON_EVPOL0_POSITION                  0x00000006
#define _CM1CON_EVPOL0_MASK                      0x00000040
#define _CM1CON_EVPOL0_LENGTH                    0x00000001

#define _CM1CON_EVPOL1_POSITION                  0x00000007
#define _CM1CON_EVPOL1_MASK                      0x00000080
#define _CM1CON_EVPOL1_LENGTH                    0x00000001

#define _CM1CON_w_POSITION                       0x00000000
#define _CM1CON_w_MASK                           0xFFFFFFFF
#define _CM1CON_w_LENGTH                         0x00000020

#define _CM2CON_CCH_POSITION                     0x00000000
#define _CM2CON_CCH_MASK                         0x00000003
#define _CM2CON_CCH_LENGTH                       0x00000002

#define _CM2CON_CREF_POSITION                    0x00000004
#define _CM2CON_CREF_MASK                        0x00000010
#define _CM2CON_CREF_LENGTH                      0x00000001

#define _CM2CON_EVPOL_POSITION                   0x00000006
#define _CM2CON_EVPOL_MASK                       0x000000C0
#define _CM2CON_EVPOL_LENGTH                     0x00000002

#define _CM2CON_COUT_POSITION                    0x00000008
#define _CM2CON_COUT_MASK                        0x00000100
#define _CM2CON_COUT_LENGTH                      0x00000001

#define _CM2CON_CPOL_POSITION                    0x0000000D
#define _CM2CON_CPOL_MASK                        0x00002000
#define _CM2CON_CPOL_LENGTH                      0x00000001

#define _CM2CON_COE_POSITION                     0x0000000E
#define _CM2CON_COE_MASK                         0x00004000
#define _CM2CON_COE_LENGTH                       0x00000001

#define _CM2CON_ON_POSITION                      0x0000000F
#define _CM2CON_ON_MASK                          0x00008000
#define _CM2CON_ON_LENGTH                        0x00000001

#define _CM2CON_CCH0_POSITION                    0x00000000
#define _CM2CON_CCH0_MASK                        0x00000001
#define _CM2CON_CCH0_LENGTH                      0x00000001

#define _CM2CON_CCH1_POSITION                    0x00000001
#define _CM2CON_CCH1_MASK                        0x00000002
#define _CM2CON_CCH1_LENGTH                      0x00000001

#define _CM2CON_EVPOL0_POSITION                  0x00000006
#define _CM2CON_EVPOL0_MASK                      0x00000040
#define _CM2CON_EVPOL0_LENGTH                    0x00000001

#define _CM2CON_EVPOL1_POSITION                  0x00000007
#define _CM2CON_EVPOL1_MASK                      0x00000080
#define _CM2CON_EVPOL1_LENGTH                    0x00000001

#define _CM2CON_w_POSITION                       0x00000000
#define _CM2CON_w_MASK                           0xFFFFFFFF
#define _CM2CON_w_LENGTH                         0x00000020

#define _CMSTAT_C1OUT_POSITION                   0x00000000
#define _CMSTAT_C1OUT_MASK                       0x00000001
#define _CMSTAT_C1OUT_LENGTH                     0x00000001

#define _CMSTAT_C2OUT_POSITION                   0x00000001
#define _CMSTAT_C2OUT_MASK                       0x00000002
#define _CMSTAT_C2OUT_LENGTH                     0x00000001

#define _CMSTAT_SIDL_POSITION                    0x0000000D
#define _CMSTAT_SIDL_MASK                        0x00002000
#define _CMSTAT_SIDL_LENGTH                      0x00000001

#define _CMSTAT_w_POSITION                       0x00000000
#define _CMSTAT_w_MASK                           0xFFFFFFFF
#define _CMSTAT_w_LENGTH                         0x00000020

#define _OSCCON_OSWEN_POSITION                   0x00000000
#define _OSCCON_OSWEN_MASK                       0x00000001
#define _OSCCON_OSWEN_LENGTH                     0x00000001

#define _OSCCON_SOSCEN_POSITION                  0x00000001
#define _OSCCON_SOSCEN_MASK                      0x00000002
#define _OSCCON_SOSCEN_LENGTH                    0x00000001

#define _OSCCON_UFRCEN_POSITION                  0x00000002
#define _OSCCON_UFRCEN_MASK                      0x00000004
#define _OSCCON_UFRCEN_LENGTH                    0x00000001

#define _OSCCON_CF_POSITION                      0x00000003
#define _OSCCON_CF_MASK                          0x00000008
#define _OSCCON_CF_LENGTH                        0x00000001

#define _OSCCON_SLPEN_POSITION                   0x00000004
#define _OSCCON_SLPEN_MASK                       0x00000010
#define _OSCCON_SLPEN_LENGTH                     0x00000001

#define _OSCCON_SLOCK_POSITION                   0x00000005
#define _OSCCON_SLOCK_MASK                       0x00000020
#define _OSCCON_SLOCK_LENGTH                     0x00000001

#define _OSCCON_ULOCK_POSITION                   0x00000006
#define _OSCCON_ULOCK_MASK                       0x00000040
#define _OSCCON_ULOCK_LENGTH                     0x00000001

#define _OSCCON_CLKLOCK_POSITION                 0x00000007
#define _OSCCON_CLKLOCK_MASK                     0x00000080
#define _OSCCON_CLKLOCK_LENGTH                   0x00000001

#define _OSCCON_NOSC_POSITION                    0x00000008
#define _OSCCON_NOSC_MASK                        0x00000700
#define _OSCCON_NOSC_LENGTH                      0x00000003

#define _OSCCON_COSC_POSITION                    0x0000000C
#define _OSCCON_COSC_MASK                        0x00007000
#define _OSCCON_COSC_LENGTH                      0x00000003

#define _OSCCON_PLLMULT_POSITION                 0x00000010
#define _OSCCON_PLLMULT_MASK                     0x00070000
#define _OSCCON_PLLMULT_LENGTH                   0x00000003

#define _OSCCON_PBDIV_POSITION                   0x00000013
#define _OSCCON_PBDIV_MASK                       0x00180000
#define _OSCCON_PBDIV_LENGTH                     0x00000002

#define _OSCCON_SOSCRDY_POSITION                 0x00000016
#define _OSCCON_SOSCRDY_MASK                     0x00400000
#define _OSCCON_SOSCRDY_LENGTH                   0x00000001

#define _OSCCON_FRCDIV_POSITION                  0x00000018
#define _OSCCON_FRCDIV_MASK                      0x07000000
#define _OSCCON_FRCDIV_LENGTH                    0x00000003

#define _OSCCON_PLLODIV_POSITION                 0x0000001B
#define _OSCCON_PLLODIV_MASK                     0x38000000
#define _OSCCON_PLLODIV_LENGTH                   0x00000003

#define _OSCCON_NOSC0_POSITION                   0x00000008
#define _OSCCON_NOSC0_MASK                       0x00000100
#define _OSCCON_NOSC0_LENGTH                     0x00000001

#define _OSCCON_NOSC1_POSITION                   0x00000009
#define _OSCCON_NOSC1_MASK                       0x00000200
#define _OSCCON_NOSC1_LENGTH                     0x00000001

#define _OSCCON_NOSC2_POSITION                   0x0000000A
#define _OSCCON_NOSC2_MASK                       0x00000400
#define _OSCCON_NOSC2_LENGTH                     0x00000001

#define _OSCCON_COSC0_POSITION                   0x0000000C
#define _OSCCON_COSC0_MASK                       0x00001000
#define _OSCCON_COSC0_LENGTH                     0x00000001

#define _OSCCON_COSC1_POSITION                   0x0000000D
#define _OSCCON_COSC1_MASK                       0x00002000
#define _OSCCON_COSC1_LENGTH                     0x00000001

#define _OSCCON_COSC2_POSITION                   0x0000000E
#define _OSCCON_COSC2_MASK                       0x00004000
#define _OSCCON_COSC2_LENGTH                     0x00000001

#define _OSCCON_PLLMULT0_POSITION                0x00000010
#define _OSCCON_PLLMULT0_MASK                    0x00010000
#define _OSCCON_PLLMULT0_LENGTH                  0x00000001

#define _OSCCON_PLLMULT1_POSITION                0x00000011
#define _OSCCON_PLLMULT1_MASK                    0x00020000
#define _OSCCON_PLLMULT1_LENGTH                  0x00000001

#define _OSCCON_PLLMULT2_POSITION                0x00000012
#define _OSCCON_PLLMULT2_MASK                    0x00040000
#define _OSCCON_PLLMULT2_LENGTH                  0x00000001

#define _OSCCON_PBDIV0_POSITION                  0x00000013
#define _OSCCON_PBDIV0_MASK                      0x00080000
#define _OSCCON_PBDIV0_LENGTH                    0x00000001

#define _OSCCON_PBDIV1_POSITION                  0x00000014
#define _OSCCON_PBDIV1_MASK                      0x00100000
#define _OSCCON_PBDIV1_LENGTH                    0x00000001

#define _OSCCON_FRCDIV0_POSITION                 0x00000018
#define _OSCCON_FRCDIV0_MASK                     0x01000000
#define _OSCCON_FRCDIV0_LENGTH                   0x00000001

#define _OSCCON_FRCDIV1_POSITION                 0x00000019
#define _OSCCON_FRCDIV1_MASK                     0x02000000
#define _OSCCON_FRCDIV1_LENGTH                   0x00000001

#define _OSCCON_FRCDIV2_POSITION                 0x0000001A
#define _OSCCON_FRCDIV2_MASK                     0x04000000
#define _OSCCON_FRCDIV2_LENGTH                   0x00000001

#define _OSCCON_PLLODIV0_POSITION                0x0000001B
#define _OSCCON_PLLODIV0_MASK                    0x08000000
#define _OSCCON_PLLODIV0_LENGTH                  0x00000001

#define _OSCCON_PLLODIV1_POSITION                0x0000001C
#define _OSCCON_PLLODIV1_MASK                    0x10000000
#define _OSCCON_PLLODIV1_LENGTH                  0x00000001

#define _OSCCON_PLLODIV2_POSITION                0x0000001D
#define _OSCCON_PLLODIV2_MASK                    0x20000000
#define _OSCCON_PLLODIV2_LENGTH                  0x00000001

#define _OSCCON_w_POSITION                       0x00000000
#define _OSCCON_w_MASK                           0xFFFFFFFF
#define _OSCCON_w_LENGTH                         0x00000020

#define _OSCTUN_TUN_POSITION                     0x00000000
#define _OSCTUN_TUN_MASK                         0x0000003F
#define _OSCTUN_TUN_LENGTH                       0x00000006

#define _OSCTUN_TUN0_POSITION                    0x00000000
#define _OSCTUN_TUN0_MASK                        0x00000001
#define _OSCTUN_TUN0_LENGTH                      0x00000001

#define _OSCTUN_TUN1_POSITION                    0x00000001
#define _OSCTUN_TUN1_MASK                        0x00000002
#define _OSCTUN_TUN1_LENGTH                      0x00000001

#define _OSCTUN_TUN2_POSITION                    0x00000002
#define _OSCTUN_TUN2_MASK                        0x00000004
#define _OSCTUN_TUN2_LENGTH                      0x00000001

#define _OSCTUN_TUN3_POSITION                    0x00000003
#define _OSCTUN_TUN3_MASK                        0x00000008
#define _OSCTUN_TUN3_LENGTH                      0x00000001

#define _OSCTUN_TUN4_POSITION                    0x00000004
#define _OSCTUN_TUN4_MASK                        0x00000010
#define _OSCTUN_TUN4_LENGTH                      0x00000001

#define _OSCTUN_TUN5_POSITION                    0x00000005
#define _OSCTUN_TUN5_MASK                        0x00000020
#define _OSCTUN_TUN5_LENGTH                      0x00000001

#define _OSCTUN_w_POSITION                       0x00000000
#define _OSCTUN_w_MASK                           0xFFFFFFFF
#define _OSCTUN_w_LENGTH                         0x00000020

#define _DDPCON_TDOEN_POSITION                   0x00000000
#define _DDPCON_TDOEN_MASK                       0x00000001
#define _DDPCON_TDOEN_LENGTH                     0x00000001

#define _DDPCON_TROEN_POSITION                   0x00000002
#define _DDPCON_TROEN_MASK                       0x00000004
#define _DDPCON_TROEN_LENGTH                     0x00000001

#define _DDPCON_JTAGEN_POSITION                  0x00000003
#define _DDPCON_JTAGEN_MASK                      0x00000008
#define _DDPCON_JTAGEN_LENGTH                    0x00000001

#define _DEVID_DEVID_POSITION                    0x00000000
#define _DEVID_DEVID_MASK                        0x0FFFFFFF
#define _DEVID_DEVID_LENGTH                      0x0000001C

#define _DEVID_VER_POSITION                      0x0000001C
#define _DEVID_VER_MASK                          0xF0000000
#define _DEVID_VER_LENGTH                        0x00000004

#define _NVMCON_NVMOP_POSITION                   0x00000000
#define _NVMCON_NVMOP_MASK                       0x0000000F
#define _NVMCON_NVMOP_LENGTH                     0x00000004

#define _NVMCON_LVDSTAT_POSITION                 0x0000000B
#define _NVMCON_LVDSTAT_MASK                     0x00000800
#define _NVMCON_LVDSTAT_LENGTH                   0x00000001

#define _NVMCON_LVDERR_POSITION                  0x0000000C
#define _NVMCON_LVDERR_MASK                      0x00001000
#define _NVMCON_LVDERR_LENGTH                    0x00000001

#define _NVMCON_WRERR_POSITION                   0x0000000D
#define _NVMCON_WRERR_MASK                       0x00002000
#define _NVMCON_WRERR_LENGTH                     0x00000001

#define _NVMCON_WREN_POSITION                    0x0000000E
#define _NVMCON_WREN_MASK                        0x00004000
#define _NVMCON_WREN_LENGTH                      0x00000001

#define _NVMCON_WR_POSITION                      0x0000000F
#define _NVMCON_WR_MASK                          0x00008000
#define _NVMCON_WR_LENGTH                        0x00000001

#define _NVMCON_NVMOP0_POSITION                  0x00000000
#define _NVMCON_NVMOP0_MASK                      0x00000001
#define _NVMCON_NVMOP0_LENGTH                    0x00000001

#define _NVMCON_NVMOP1_POSITION                  0x00000001
#define _NVMCON_NVMOP1_MASK                      0x00000002
#define _NVMCON_NVMOP1_LENGTH                    0x00000001

#define _NVMCON_NVMOP2_POSITION                  0x00000002
#define _NVMCON_NVMOP2_MASK                      0x00000004
#define _NVMCON_NVMOP2_LENGTH                    0x00000001

#define _NVMCON_NVMOP3_POSITION                  0x00000003
#define _NVMCON_NVMOP3_MASK                      0x00000008
#define _NVMCON_NVMOP3_LENGTH                    0x00000001

#define _NVMCON_PROGOP_POSITION                  0x00000000
#define _NVMCON_PROGOP_MASK                      0x0000000F
#define _NVMCON_PROGOP_LENGTH                    0x00000004

#define _NVMCON_PROGOP0_POSITION                 0x00000000
#define _NVMCON_PROGOP0_MASK                     0x00000001
#define _NVMCON_PROGOP0_LENGTH                   0x00000001

#define _NVMCON_PROGOP1_POSITION                 0x00000001
#define _NVMCON_PROGOP1_MASK                     0x00000002
#define _NVMCON_PROGOP1_LENGTH                   0x00000001

#define _NVMCON_PROGOP2_POSITION                 0x00000002
#define _NVMCON_PROGOP2_MASK                     0x00000004
#define _NVMCON_PROGOP2_LENGTH                   0x00000001

#define _NVMCON_PROGOP3_POSITION                 0x00000003
#define _NVMCON_PROGOP3_MASK                     0x00000008
#define _NVMCON_PROGOP3_LENGTH                   0x00000001

#define _NVMCON_w_POSITION                       0x00000000
#define _NVMCON_w_MASK                           0xFFFFFFFF
#define _NVMCON_w_LENGTH                         0x00000020

#define _RCON_POR_POSITION                       0x00000000
#define _RCON_POR_MASK                           0x00000001
#define _RCON_POR_LENGTH                         0x00000001

#define _RCON_BOR_POSITION                       0x00000001
#define _RCON_BOR_MASK                           0x00000002
#define _RCON_BOR_LENGTH                         0x00000001

#define _RCON_IDLE_POSITION                      0x00000002
#define _RCON_IDLE_MASK                          0x00000004
#define _RCON_IDLE_LENGTH                        0x00000001

#define _RCON_SLEEP_POSITION                     0x00000003
#define _RCON_SLEEP_MASK                         0x00000008
#define _RCON_SLEEP_LENGTH                       0x00000001

#define _RCON_WDTO_POSITION                      0x00000004
#define _RCON_WDTO_MASK                          0x00000010
#define _RCON_WDTO_LENGTH                        0x00000001

#define _RCON_SWR_POSITION                       0x00000006
#define _RCON_SWR_MASK                           0x00000040
#define _RCON_SWR_LENGTH                         0x00000001

#define _RCON_EXTR_POSITION                      0x00000007
#define _RCON_EXTR_MASK                          0x00000080
#define _RCON_EXTR_LENGTH                        0x00000001

#define _RCON_VREGS_POSITION                     0x00000008
#define _RCON_VREGS_MASK                         0x00000100
#define _RCON_VREGS_LENGTH                       0x00000001

#define _RCON_CMR_POSITION                       0x00000009
#define _RCON_CMR_MASK                           0x00000200
#define _RCON_CMR_LENGTH                         0x00000001

#define _RCON_w_POSITION                         0x00000000
#define _RCON_w_MASK                             0xFFFFFFFF
#define _RCON_w_LENGTH                           0x00000020

#define _RSWRST_SWRST_POSITION                   0x00000000
#define _RSWRST_SWRST_MASK                       0x00000001
#define _RSWRST_SWRST_LENGTH                     0x00000001

#define _RSWRST_w_POSITION                       0x00000000
#define _RSWRST_w_MASK                           0xFFFFFFFF
#define _RSWRST_w_LENGTH                         0x00000020

#define __DDPSTAT_APIFUL_POSITION                0x00000001
#define __DDPSTAT_APIFUL_MASK                    0x00000002
#define __DDPSTAT_APIFUL_LENGTH                  0x00000001

#define __DDPSTAT_APOFUL_POSITION                0x00000002
#define __DDPSTAT_APOFUL_MASK                    0x00000004
#define __DDPSTAT_APOFUL_LENGTH                  0x00000001

#define __DDPSTAT_STRFUL_POSITION                0x00000003
#define __DDPSTAT_STRFUL_MASK                    0x00000008
#define __DDPSTAT_STRFUL_LENGTH                  0x00000001

#define __DDPSTAT_APIOV_POSITION                 0x00000009
#define __DDPSTAT_APIOV_MASK                     0x00000200
#define __DDPSTAT_APIOV_LENGTH                   0x00000001

#define __DDPSTAT_APOOV_POSITION                 0x0000000A
#define __DDPSTAT_APOOV_MASK                     0x00000400
#define __DDPSTAT_APOOV_LENGTH                   0x00000001

#define __DDPSTAT_STOV_POSITION                  0x00000010
#define __DDPSTAT_STOV_MASK                      0xFFFF0000
#define __DDPSTAT_STOV_LENGTH                    0x00000010

#define _INTCON_INT0EP_POSITION                  0x00000000
#define _INTCON_INT0EP_MASK                      0x00000001
#define _INTCON_INT0EP_LENGTH                    0x00000001

#define _INTCON_INT1EP_POSITION                  0x00000001
#define _INTCON_INT1EP_MASK                      0x00000002
#define _INTCON_INT1EP_LENGTH                    0x00000001

#define _INTCON_INT2EP_POSITION                  0x00000002
#define _INTCON_INT2EP_MASK                      0x00000004
#define _INTCON_INT2EP_LENGTH                    0x00000001

#define _INTCON_INT3EP_POSITION                  0x00000003
#define _INTCON_INT3EP_MASK                      0x00000008
#define _INTCON_INT3EP_LENGTH                    0x00000001

#define _INTCON_INT4EP_POSITION                  0x00000004
#define _INTCON_INT4EP_MASK                      0x00000010
#define _INTCON_INT4EP_LENGTH                    0x00000001

#define _INTCON_TPC_POSITION                     0x00000008
#define _INTCON_TPC_MASK                         0x00000700
#define _INTCON_TPC_LENGTH                       0x00000003

#define _INTCON_MVEC_POSITION                    0x0000000C
#define _INTCON_MVEC_MASK                        0x00001000
#define _INTCON_MVEC_LENGTH                      0x00000001

#define _INTCON_FRZ_POSITION                     0x0000000E
#define _INTCON_FRZ_MASK                         0x00004000
#define _INTCON_FRZ_LENGTH                       0x00000001

#define _INTCON_SS0_POSITION                     0x00000010
#define _INTCON_SS0_MASK                         0x00010000
#define _INTCON_SS0_LENGTH                       0x00000001

#define _INTCON_w_POSITION                       0x00000000
#define _INTCON_w_MASK                           0xFFFFFFFF
#define _INTCON_w_LENGTH                         0x00000020

#define _INTSTAT_VEC_POSITION                    0x00000000
#define _INTSTAT_VEC_MASK                        0x0000003F
#define _INTSTAT_VEC_LENGTH                      0x00000006

#define _INTSTAT_SRIPL_POSITION                  0x00000008
#define _INTSTAT_SRIPL_MASK                      0x00000700
#define _INTSTAT_SRIPL_LENGTH                    0x00000003

#define _IFS0_CTIF_POSITION                      0x00000000
#define _IFS0_CTIF_MASK                          0x00000001
#define _IFS0_CTIF_LENGTH                        0x00000001

#define _IFS0_CS0IF_POSITION                     0x00000001
#define _IFS0_CS0IF_MASK                         0x00000002
#define _IFS0_CS0IF_LENGTH                       0x00000001

#define _IFS0_CS1IF_POSITION                     0x00000002
#define _IFS0_CS1IF_MASK                         0x00000004
#define _IFS0_CS1IF_LENGTH                       0x00000001

#define _IFS0_INT0IF_POSITION                    0x00000003
#define _IFS0_INT0IF_MASK                        0x00000008
#define _IFS0_INT0IF_LENGTH                      0x00000001

#define _IFS0_T1IF_POSITION                      0x00000004
#define _IFS0_T1IF_MASK                          0x00000010
#define _IFS0_T1IF_LENGTH                        0x00000001

#define _IFS0_IC1IF_POSITION                     0x00000005
#define _IFS0_IC1IF_MASK                         0x00000020
#define _IFS0_IC1IF_LENGTH                       0x00000001

#define _IFS0_OC1IF_POSITION                     0x00000006
#define _IFS0_OC1IF_MASK                         0x00000040
#define _IFS0_OC1IF_LENGTH                       0x00000001

#define _IFS0_INT1IF_POSITION                    0x00000007
#define _IFS0_INT1IF_MASK                        0x00000080
#define _IFS0_INT1IF_LENGTH                      0x00000001

#define _IFS0_T2IF_POSITION                      0x00000008
#define _IFS0_T2IF_MASK                          0x00000100
#define _IFS0_T2IF_LENGTH                        0x00000001

#define _IFS0_IC2IF_POSITION                     0x00000009
#define _IFS0_IC2IF_MASK                         0x00000200
#define _IFS0_IC2IF_LENGTH                       0x00000001

#define _IFS0_OC2IF_POSITION                     0x0000000A
#define _IFS0_OC2IF_MASK                         0x00000400
#define _IFS0_OC2IF_LENGTH                       0x00000001

#define _IFS0_INT2IF_POSITION                    0x0000000B
#define _IFS0_INT2IF_MASK                        0x00000800
#define _IFS0_INT2IF_LENGTH                      0x00000001

#define _IFS0_T3IF_POSITION                      0x0000000C
#define _IFS0_T3IF_MASK                          0x00001000
#define _IFS0_T3IF_LENGTH                        0x00000001

#define _IFS0_IC3IF_POSITION                     0x0000000D
#define _IFS0_IC3IF_MASK                         0x00002000
#define _IFS0_IC3IF_LENGTH                       0x00000001

#define _IFS0_OC3IF_POSITION                     0x0000000E
#define _IFS0_OC3IF_MASK                         0x00004000
#define _IFS0_OC3IF_LENGTH                       0x00000001

#define _IFS0_INT3IF_POSITION                    0x0000000F
#define _IFS0_INT3IF_MASK                        0x00008000
#define _IFS0_INT3IF_LENGTH                      0x00000001

#define _IFS0_T4IF_POSITION                      0x00000010
#define _IFS0_T4IF_MASK                          0x00010000
#define _IFS0_T4IF_LENGTH                        0x00000001

#define _IFS0_IC4IF_POSITION                     0x00000011
#define _IFS0_IC4IF_MASK                         0x00020000
#define _IFS0_IC4IF_LENGTH                       0x00000001

#define _IFS0_OC4IF_POSITION                     0x00000012
#define _IFS0_OC4IF_MASK                         0x00040000
#define _IFS0_OC4IF_LENGTH                       0x00000001

#define _IFS0_INT4IF_POSITION                    0x00000013
#define _IFS0_INT4IF_MASK                        0x00080000
#define _IFS0_INT4IF_LENGTH                      0x00000001

#define _IFS0_T5IF_POSITION                      0x00000014
#define _IFS0_T5IF_MASK                          0x00100000
#define _IFS0_T5IF_LENGTH                        0x00000001

#define _IFS0_IC5IF_POSITION                     0x00000015
#define _IFS0_IC5IF_MASK                         0x00200000
#define _IFS0_IC5IF_LENGTH                       0x00000001

#define _IFS0_OC5IF_POSITION                     0x00000016
#define _IFS0_OC5IF_MASK                         0x00400000
#define _IFS0_OC5IF_LENGTH                       0x00000001

#define _IFS0_U1EIF_POSITION                     0x0000001A
#define _IFS0_U1EIF_MASK                         0x04000000
#define _IFS0_U1EIF_LENGTH                       0x00000001

#define _IFS0_U1RXIF_POSITION                    0x0000001B
#define _IFS0_U1RXIF_MASK                        0x08000000
#define _IFS0_U1RXIF_LENGTH                      0x00000001

#define _IFS0_U1TXIF_POSITION                    0x0000001C
#define _IFS0_U1TXIF_MASK                        0x10000000
#define _IFS0_U1TXIF_LENGTH                      0x00000001

#define _IFS0_I2C1BIF_POSITION                   0x0000001D
#define _IFS0_I2C1BIF_MASK                       0x20000000
#define _IFS0_I2C1BIF_LENGTH                     0x00000001

#define _IFS0_I2C1SIF_POSITION                   0x0000001E
#define _IFS0_I2C1SIF_MASK                       0x40000000
#define _IFS0_I2C1SIF_LENGTH                     0x00000001

#define _IFS0_I2C1MIF_POSITION                   0x0000001F
#define _IFS0_I2C1MIF_MASK                       0x80000000
#define _IFS0_I2C1MIF_LENGTH                     0x00000001

#define _IFS0_U1AEIF_POSITION                    0x0000001A
#define _IFS0_U1AEIF_MASK                        0x04000000
#define _IFS0_U1AEIF_LENGTH                      0x00000001

#define _IFS0_U1ARXIF_POSITION                   0x0000001B
#define _IFS0_U1ARXIF_MASK                       0x08000000
#define _IFS0_U1ARXIF_LENGTH                     0x00000001

#define _IFS0_U1ATXIF_POSITION                   0x0000001C
#define _IFS0_U1ATXIF_MASK                       0x10000000
#define _IFS0_U1ATXIF_LENGTH                     0x00000001

#define _IFS0_SPI3EIF_POSITION                   0x0000001A
#define _IFS0_SPI3EIF_MASK                       0x04000000
#define _IFS0_SPI3EIF_LENGTH                     0x00000001

#define _IFS0_SPI3RXIF_POSITION                  0x0000001B
#define _IFS0_SPI3RXIF_MASK                      0x08000000
#define _IFS0_SPI3RXIF_LENGTH                    0x00000001

#define _IFS0_SPI3TXIF_POSITION                  0x0000001C
#define _IFS0_SPI3TXIF_MASK                      0x10000000
#define _IFS0_SPI3TXIF_LENGTH                    0x00000001

#define _IFS0_SPI1AEIF_POSITION                  0x0000001A
#define _IFS0_SPI1AEIF_MASK                      0x04000000
#define _IFS0_SPI1AEIF_LENGTH                    0x00000001

#define _IFS0_SPI1ARXIF_POSITION                 0x0000001B
#define _IFS0_SPI1ARXIF_MASK                     0x08000000
#define _IFS0_SPI1ARXIF_LENGTH                   0x00000001

#define _IFS0_SPI1ATXIF_POSITION                 0x0000001C
#define _IFS0_SPI1ATXIF_MASK                     0x10000000
#define _IFS0_SPI1ATXIF_LENGTH                   0x00000001

#define _IFS0_I2C3BIF_POSITION                   0x0000001A
#define _IFS0_I2C3BIF_MASK                       0x04000000
#define _IFS0_I2C3BIF_LENGTH                     0x00000001

#define _IFS0_I2C3SIF_POSITION                   0x0000001B
#define _IFS0_I2C3SIF_MASK                       0x08000000
#define _IFS0_I2C3SIF_LENGTH                     0x00000001

#define _IFS0_I2C3MIF_POSITION                   0x0000001C
#define _IFS0_I2C3MIF_MASK                       0x10000000
#define _IFS0_I2C3MIF_LENGTH                     0x00000001

#define _IFS0_I2C1ABIF_POSITION                  0x0000001A
#define _IFS0_I2C1ABIF_MASK                      0x04000000
#define _IFS0_I2C1ABIF_LENGTH                    0x00000001

#define _IFS0_I2C1ASIF_POSITION                  0x0000001B
#define _IFS0_I2C1ASIF_MASK                      0x08000000
#define _IFS0_I2C1ASIF_LENGTH                    0x00000001

#define _IFS0_I2C1AMIF_POSITION                  0x0000001C
#define _IFS0_I2C1AMIF_MASK                      0x10000000
#define _IFS0_I2C1AMIF_LENGTH                    0x00000001

#define _IFS0_w_POSITION                         0x00000000
#define _IFS0_w_MASK                             0xFFFFFFFF
#define _IFS0_w_LENGTH                           0x00000020

#define _IFS1_CNIF_POSITION                      0x00000000
#define _IFS1_CNIF_MASK                          0x00000001
#define _IFS1_CNIF_LENGTH                        0x00000001

#define _IFS1_AD1IF_POSITION                     0x00000001
#define _IFS1_AD1IF_MASK                         0x00000002
#define _IFS1_AD1IF_LENGTH                       0x00000001

#define _IFS1_PMPIF_POSITION                     0x00000002
#define _IFS1_PMPIF_MASK                         0x00000004
#define _IFS1_PMPIF_LENGTH                       0x00000001

#define _IFS1_CMP1IF_POSITION                    0x00000003
#define _IFS1_CMP1IF_MASK                        0x00000008
#define _IFS1_CMP1IF_LENGTH                      0x00000001

#define _IFS1_CMP2IF_POSITION                    0x00000004
#define _IFS1_CMP2IF_MASK                        0x00000010
#define _IFS1_CMP2IF_LENGTH                      0x00000001

#define _IFS1_U3EIF_POSITION                     0x00000005
#define _IFS1_U3EIF_MASK                         0x00000020
#define _IFS1_U3EIF_LENGTH                       0x00000001

#define _IFS1_U3RXIF_POSITION                    0x00000006
#define _IFS1_U3RXIF_MASK                        0x00000040
#define _IFS1_U3RXIF_LENGTH                      0x00000001

#define _IFS1_U3TXIF_POSITION                    0x00000007
#define _IFS1_U3TXIF_MASK                        0x00000080
#define _IFS1_U3TXIF_LENGTH                      0x00000001

#define _IFS1_U2EIF_POSITION                     0x00000008
#define _IFS1_U2EIF_MASK                         0x00000100
#define _IFS1_U2EIF_LENGTH                       0x00000001

#define _IFS1_U2RXIF_POSITION                    0x00000009
#define _IFS1_U2RXIF_MASK                        0x00000200
#define _IFS1_U2RXIF_LENGTH                      0x00000001

#define _IFS1_U2TXIF_POSITION                    0x0000000A
#define _IFS1_U2TXIF_MASK                        0x00000400
#define _IFS1_U2TXIF_LENGTH                      0x00000001

#define _IFS1_I2C2BIF_POSITION                   0x0000000B
#define _IFS1_I2C2BIF_MASK                       0x00000800
#define _IFS1_I2C2BIF_LENGTH                     0x00000001

#define _IFS1_I2C2SIF_POSITION                   0x0000000C
#define _IFS1_I2C2SIF_MASK                       0x00001000
#define _IFS1_I2C2SIF_LENGTH                     0x00000001

#define _IFS1_I2C2MIF_POSITION                   0x0000000D
#define _IFS1_I2C2MIF_MASK                       0x00002000
#define _IFS1_I2C2MIF_LENGTH                     0x00000001

#define _IFS1_FSCMIF_POSITION                    0x0000000E
#define _IFS1_FSCMIF_MASK                        0x00004000
#define _IFS1_FSCMIF_LENGTH                      0x00000001

#define _IFS1_RTCCIF_POSITION                    0x0000000F
#define _IFS1_RTCCIF_MASK                        0x00008000
#define _IFS1_RTCCIF_LENGTH                      0x00000001

#define _IFS1_DMA0IF_POSITION                    0x00000010
#define _IFS1_DMA0IF_MASK                        0x00010000
#define _IFS1_DMA0IF_LENGTH                      0x00000001

#define _IFS1_DMA1IF_POSITION                    0x00000011
#define _IFS1_DMA1IF_MASK                        0x00020000
#define _IFS1_DMA1IF_LENGTH                      0x00000001

#define _IFS1_DMA2IF_POSITION                    0x00000012
#define _IFS1_DMA2IF_MASK                        0x00040000
#define _IFS1_DMA2IF_LENGTH                      0x00000001

#define _IFS1_DMA3IF_POSITION                    0x00000013
#define _IFS1_DMA3IF_MASK                        0x00080000
#define _IFS1_DMA3IF_LENGTH                      0x00000001

#define _IFS1_DMA4IF_POSITION                    0x00000014
#define _IFS1_DMA4IF_MASK                        0x00100000
#define _IFS1_DMA4IF_LENGTH                      0x00000001

#define _IFS1_DMA5IF_POSITION                    0x00000015
#define _IFS1_DMA5IF_MASK                        0x00200000
#define _IFS1_DMA5IF_LENGTH                      0x00000001

#define _IFS1_DMA6IF_POSITION                    0x00000016
#define _IFS1_DMA6IF_MASK                        0x00400000
#define _IFS1_DMA6IF_LENGTH                      0x00000001

#define _IFS1_DMA7IF_POSITION                    0x00000017
#define _IFS1_DMA7IF_MASK                        0x00800000
#define _IFS1_DMA7IF_LENGTH                      0x00000001

#define _IFS1_FCEIF_POSITION                     0x00000018
#define _IFS1_FCEIF_MASK                         0x01000000
#define _IFS1_FCEIF_LENGTH                       0x00000001

#define _IFS1_USBIF_POSITION                     0x00000019
#define _IFS1_USBIF_MASK                         0x02000000
#define _IFS1_USBIF_LENGTH                       0x00000001

#define _IFS1_CAN1IF_POSITION                    0x0000001A
#define _IFS1_CAN1IF_MASK                        0x04000000
#define _IFS1_CAN1IF_LENGTH                      0x00000001

#define _IFS1_CAN2IF_POSITION                    0x0000001B
#define _IFS1_CAN2IF_MASK                        0x08000000
#define _IFS1_CAN2IF_LENGTH                      0x00000001

#define _IFS1_ETHIF_POSITION                     0x0000001C
#define _IFS1_ETHIF_MASK                         0x10000000
#define _IFS1_ETHIF_LENGTH                       0x00000001

#define _IFS1_IC1EIF_POSITION                    0x0000001D
#define _IFS1_IC1EIF_MASK                        0x20000000
#define _IFS1_IC1EIF_LENGTH                      0x00000001

#define _IFS1_IC2EIF_POSITION                    0x0000001E
#define _IFS1_IC2EIF_MASK                        0x40000000
#define _IFS1_IC2EIF_LENGTH                      0x00000001

#define _IFS1_IC3EIF_POSITION                    0x0000001F
#define _IFS1_IC3EIF_MASK                        0x80000000
#define _IFS1_IC3EIF_LENGTH                      0x00000001

#define _IFS1_U2AEIF_POSITION                    0x00000005
#define _IFS1_U2AEIF_MASK                        0x00000020
#define _IFS1_U2AEIF_LENGTH                      0x00000001

#define _IFS1_U2ARXIF_POSITION                   0x00000006
#define _IFS1_U2ARXIF_MASK                       0x00000040
#define _IFS1_U2ARXIF_LENGTH                     0x00000001

#define _IFS1_U2ATXIF_POSITION                   0x00000007
#define _IFS1_U2ATXIF_MASK                       0x00000080
#define _IFS1_U2ATXIF_LENGTH                     0x00000001

#define _IFS1_U3AEIF_POSITION                    0x00000008
#define _IFS1_U3AEIF_MASK                        0x00000100
#define _IFS1_U3AEIF_LENGTH                      0x00000001

#define _IFS1_U3ARXIF_POSITION                   0x00000009
#define _IFS1_U3ARXIF_MASK                       0x00000200
#define _IFS1_U3ARXIF_LENGTH                     0x00000001

#define _IFS1_U3ATXIF_POSITION                   0x0000000A
#define _IFS1_U3ATXIF_MASK                       0x00000400
#define _IFS1_U3ATXIF_LENGTH                     0x00000001

#define _IFS1_SPI2EIF_POSITION                   0x00000005
#define _IFS1_SPI2EIF_MASK                       0x00000020
#define _IFS1_SPI2EIF_LENGTH                     0x00000001

#define _IFS1_SPI2RXIF_POSITION                  0x00000006
#define _IFS1_SPI2RXIF_MASK                      0x00000040
#define _IFS1_SPI2RXIF_LENGTH                    0x00000001

#define _IFS1_SPI2TXIF_POSITION                  0x00000007
#define _IFS1_SPI2TXIF_MASK                      0x00000080
#define _IFS1_SPI2TXIF_LENGTH                    0x00000001

#define _IFS1_SPI4EIF_POSITION                   0x00000008
#define _IFS1_SPI4EIF_MASK                       0x00000100
#define _IFS1_SPI4EIF_LENGTH                     0x00000001

#define _IFS1_SPI4RXIF_POSITION                  0x00000009
#define _IFS1_SPI4RXIF_MASK                      0x00000200
#define _IFS1_SPI4RXIF_LENGTH                    0x00000001

#define _IFS1_SPI4TXIF_POSITION                  0x0000000A
#define _IFS1_SPI4TXIF_MASK                      0x00000400
#define _IFS1_SPI4TXIF_LENGTH                    0x00000001

#define _IFS1_SPI2AEIF_POSITION                  0x00000005
#define _IFS1_SPI2AEIF_MASK                      0x00000020
#define _IFS1_SPI2AEIF_LENGTH                    0x00000001

#define _IFS1_SPI2ARXIF_POSITION                 0x00000006
#define _IFS1_SPI2ARXIF_MASK                     0x00000040
#define _IFS1_SPI2ARXIF_LENGTH                   0x00000001

#define _IFS1_SPI2ATXIF_POSITION                 0x00000007
#define _IFS1_SPI2ATXIF_MASK                     0x00000080
#define _IFS1_SPI2ATXIF_LENGTH                   0x00000001

#define _IFS1_SPI3AEIF_POSITION                  0x00000008
#define _IFS1_SPI3AEIF_MASK                      0x00000100
#define _IFS1_SPI3AEIF_LENGTH                    0x00000001

#define _IFS1_SPI3ARXIF_POSITION                 0x00000009
#define _IFS1_SPI3ARXIF_MASK                     0x00000200
#define _IFS1_SPI3ARXIF_LENGTH                   0x00000001

#define _IFS1_SPI3ATXIF_POSITION                 0x0000000A
#define _IFS1_SPI3ATXIF_MASK                     0x00000400
#define _IFS1_SPI3ATXIF_LENGTH                   0x00000001

#define _IFS1_I2C4BIF_POSITION                   0x00000005
#define _IFS1_I2C4BIF_MASK                       0x00000020
#define _IFS1_I2C4BIF_LENGTH                     0x00000001

#define _IFS1_I2C4SIF_POSITION                   0x00000006
#define _IFS1_I2C4SIF_MASK                       0x00000040
#define _IFS1_I2C4SIF_LENGTH                     0x00000001

#define _IFS1_I2C4MIF_POSITION                   0x00000007
#define _IFS1_I2C4MIF_MASK                       0x00000080
#define _IFS1_I2C4MIF_LENGTH                     0x00000001

#define _IFS1_I2C5BIF_POSITION                   0x00000008
#define _IFS1_I2C5BIF_MASK                       0x00000100
#define _IFS1_I2C5BIF_LENGTH                     0x00000001

#define _IFS1_I2C5SIF_POSITION                   0x00000009
#define _IFS1_I2C5SIF_MASK                       0x00000200
#define _IFS1_I2C5SIF_LENGTH                     0x00000001

#define _IFS1_I2C5MIF_POSITION                   0x0000000A
#define _IFS1_I2C5MIF_MASK                       0x00000400
#define _IFS1_I2C5MIF_LENGTH                     0x00000001

#define _IFS1_I2C2ABIF_POSITION                  0x00000005
#define _IFS1_I2C2ABIF_MASK                      0x00000020
#define _IFS1_I2C2ABIF_LENGTH                    0x00000001

#define _IFS1_I2C2ASIF_POSITION                  0x00000006
#define _IFS1_I2C2ASIF_MASK                      0x00000040
#define _IFS1_I2C2ASIF_LENGTH                    0x00000001

#define _IFS1_I2C2AMIF_POSITION                  0x00000007
#define _IFS1_I2C2AMIF_MASK                      0x00000080
#define _IFS1_I2C2AMIF_LENGTH                    0x00000001

#define _IFS1_I2C3ABIF_POSITION                  0x00000008
#define _IFS1_I2C3ABIF_MASK                      0x00000100
#define _IFS1_I2C3ABIF_LENGTH                    0x00000001

#define _IFS1_I2C3ASIF_POSITION                  0x00000009
#define _IFS1_I2C3ASIF_MASK                      0x00000200
#define _IFS1_I2C3ASIF_LENGTH                    0x00000001

#define _IFS1_I2C3AMIF_POSITION                  0x0000000A
#define _IFS1_I2C3AMIF_MASK                      0x00000400
#define _IFS1_I2C3AMIF_LENGTH                    0x00000001

#define _IFS1_w_POSITION                         0x00000000
#define _IFS1_w_MASK                             0xFFFFFFFF
#define _IFS1_w_LENGTH                           0x00000020

#define _IFS2_IC4EIF_POSITION                    0x00000000
#define _IFS2_IC4EIF_MASK                        0x00000001
#define _IFS2_IC4EIF_LENGTH                      0x00000001

#define _IFS2_IC5EIF_POSITION                    0x00000001
#define _IFS2_IC5EIF_MASK                        0x00000002
#define _IFS2_IC5EIF_LENGTH                      0x00000001

#define _IFS2_PMPEIF_POSITION                    0x00000002
#define _IFS2_PMPEIF_MASK                        0x00000004
#define _IFS2_PMPEIF_LENGTH                      0x00000001

#define _IFS2_U1BEIF_POSITION                    0x00000003
#define _IFS2_U1BEIF_MASK                        0x00000008
#define _IFS2_U1BEIF_LENGTH                      0x00000001

#define _IFS2_U1BRXIF_POSITION                   0x00000004
#define _IFS2_U1BRXIF_MASK                       0x00000010
#define _IFS2_U1BRXIF_LENGTH                     0x00000001

#define _IFS2_U1BTXIF_POSITION                   0x00000005
#define _IFS2_U1BTXIF_MASK                       0x00000020
#define _IFS2_U1BTXIF_LENGTH                     0x00000001

#define _IFS2_U2BEIF_POSITION                    0x00000006
#define _IFS2_U2BEIF_MASK                        0x00000040
#define _IFS2_U2BEIF_LENGTH                      0x00000001

#define _IFS2_U2BRXIF_POSITION                   0x00000007
#define _IFS2_U2BRXIF_MASK                       0x00000080
#define _IFS2_U2BRXIF_LENGTH                     0x00000001

#define _IFS2_U2BTXIF_POSITION                   0x00000008
#define _IFS2_U2BTXIF_MASK                       0x00000100
#define _IFS2_U2BTXIF_LENGTH                     0x00000001

#define _IFS2_U3BEIF_POSITION                    0x00000009
#define _IFS2_U3BEIF_MASK                        0x00000200
#define _IFS2_U3BEIF_LENGTH                      0x00000001

#define _IFS2_U3BRXIF_POSITION                   0x0000000A
#define _IFS2_U3BRXIF_MASK                       0x00000400
#define _IFS2_U3BRXIF_LENGTH                     0x00000001

#define _IFS2_U3BTXIF_POSITION                   0x0000000B
#define _IFS2_U3BTXIF_MASK                       0x00000800
#define _IFS2_U3BTXIF_LENGTH                     0x00000001

#define _IFS2_U4EIF_POSITION                     0x00000003
#define _IFS2_U4EIF_MASK                         0x00000008
#define _IFS2_U4EIF_LENGTH                       0x00000001

#define _IFS2_U4RXIF_POSITION                    0x00000004
#define _IFS2_U4RXIF_MASK                        0x00000010
#define _IFS2_U4RXIF_LENGTH                      0x00000001

#define _IFS2_U4TXIF_POSITION                    0x00000005
#define _IFS2_U4TXIF_MASK                        0x00000020
#define _IFS2_U4TXIF_LENGTH                      0x00000001

#define _IFS2_U6EIF_POSITION                     0x00000006
#define _IFS2_U6EIF_MASK                         0x00000040
#define _IFS2_U6EIF_LENGTH                       0x00000001

#define _IFS2_U6RXIF_POSITION                    0x00000007
#define _IFS2_U6RXIF_MASK                        0x00000080
#define _IFS2_U6RXIF_LENGTH                      0x00000001

#define _IFS2_U6TXIF_POSITION                    0x00000008
#define _IFS2_U6TXIF_MASK                        0x00000100
#define _IFS2_U6TXIF_LENGTH                      0x00000001

#define _IFS2_U5EIF_POSITION                     0x00000009
#define _IFS2_U5EIF_MASK                         0x00000200
#define _IFS2_U5EIF_LENGTH                       0x00000001

#define _IFS2_U5RXIF_POSITION                    0x0000000A
#define _IFS2_U5RXIF_MASK                        0x00000400
#define _IFS2_U5RXIF_LENGTH                      0x00000001

#define _IFS2_U5TXIF_POSITION                    0x0000000B
#define _IFS2_U5TXIF_MASK                        0x00000800
#define _IFS2_U5TXIF_LENGTH                      0x00000001

#define _IFS2_w_POSITION                         0x00000000
#define _IFS2_w_MASK                             0xFFFFFFFF
#define _IFS2_w_LENGTH                           0x00000020

#define _IEC0_CTIE_POSITION                      0x00000000
#define _IEC0_CTIE_MASK                          0x00000001
#define _IEC0_CTIE_LENGTH                        0x00000001

#define _IEC0_CS0IE_POSITION                     0x00000001
#define _IEC0_CS0IE_MASK                         0x00000002
#define _IEC0_CS0IE_LENGTH                       0x00000001

#define _IEC0_CS1IE_POSITION                     0x00000002
#define _IEC0_CS1IE_MASK                         0x00000004
#define _IEC0_CS1IE_LENGTH                       0x00000001

#define _IEC0_INT0IE_POSITION                    0x00000003
#define _IEC0_INT0IE_MASK                        0x00000008
#define _IEC0_INT0IE_LENGTH                      0x00000001

#define _IEC0_T1IE_POSITION                      0x00000004
#define _IEC0_T1IE_MASK                          0x00000010
#define _IEC0_T1IE_LENGTH                        0x00000001

#define _IEC0_IC1IE_POSITION                     0x00000005
#define _IEC0_IC1IE_MASK                         0x00000020
#define _IEC0_IC1IE_LENGTH                       0x00000001

#define _IEC0_OC1IE_POSITION                     0x00000006
#define _IEC0_OC1IE_MASK                         0x00000040
#define _IEC0_OC1IE_LENGTH                       0x00000001

#define _IEC0_INT1IE_POSITION                    0x00000007
#define _IEC0_INT1IE_MASK                        0x00000080
#define _IEC0_INT1IE_LENGTH                      0x00000001

#define _IEC0_T2IE_POSITION                      0x00000008
#define _IEC0_T2IE_MASK                          0x00000100
#define _IEC0_T2IE_LENGTH                        0x00000001

#define _IEC0_IC2IE_POSITION                     0x00000009
#define _IEC0_IC2IE_MASK                         0x00000200
#define _IEC0_IC2IE_LENGTH                       0x00000001

#define _IEC0_OC2IE_POSITION                     0x0000000A
#define _IEC0_OC2IE_MASK                         0x00000400
#define _IEC0_OC2IE_LENGTH                       0x00000001

#define _IEC0_INT2IE_POSITION                    0x0000000B
#define _IEC0_INT2IE_MASK                        0x00000800
#define _IEC0_INT2IE_LENGTH                      0x00000001

#define _IEC0_T3IE_POSITION                      0x0000000C
#define _IEC0_T3IE_MASK                          0x00001000
#define _IEC0_T3IE_LENGTH                        0x00000001

#define _IEC0_IC3IE_POSITION                     0x0000000D
#define _IEC0_IC3IE_MASK                         0x00002000
#define _IEC0_IC3IE_LENGTH                       0x00000001

#define _IEC0_OC3IE_POSITION                     0x0000000E
#define _IEC0_OC3IE_MASK                         0x00004000
#define _IEC0_OC3IE_LENGTH                       0x00000001

#define _IEC0_INT3IE_POSITION                    0x0000000F
#define _IEC0_INT3IE_MASK                        0x00008000
#define _IEC0_INT3IE_LENGTH                      0x00000001

#define _IEC0_T4IE_POSITION                      0x00000010
#define _IEC0_T4IE_MASK                          0x00010000
#define _IEC0_T4IE_LENGTH                        0x00000001

#define _IEC0_IC4IE_POSITION                     0x00000011
#define _IEC0_IC4IE_MASK                         0x00020000
#define _IEC0_IC4IE_LENGTH                       0x00000001

#define _IEC0_OC4IE_POSITION                     0x00000012
#define _IEC0_OC4IE_MASK                         0x00040000
#define _IEC0_OC4IE_LENGTH                       0x00000001

#define _IEC0_INT4IE_POSITION                    0x00000013
#define _IEC0_INT4IE_MASK                        0x00080000
#define _IEC0_INT4IE_LENGTH                      0x00000001

#define _IEC0_T5IE_POSITION                      0x00000014
#define _IEC0_T5IE_MASK                          0x00100000
#define _IEC0_T5IE_LENGTH                        0x00000001

#define _IEC0_IC5IE_POSITION                     0x00000015
#define _IEC0_IC5IE_MASK                         0x00200000
#define _IEC0_IC5IE_LENGTH                       0x00000001

#define _IEC0_OC5IE_POSITION                     0x00000016
#define _IEC0_OC5IE_MASK                         0x00400000
#define _IEC0_OC5IE_LENGTH                       0x00000001

#define _IEC0_U1EIE_POSITION                     0x0000001A
#define _IEC0_U1EIE_MASK                         0x04000000
#define _IEC0_U1EIE_LENGTH                       0x00000001

#define _IEC0_U1RXIE_POSITION                    0x0000001B
#define _IEC0_U1RXIE_MASK                        0x08000000
#define _IEC0_U1RXIE_LENGTH                      0x00000001

#define _IEC0_U1TXIE_POSITION                    0x0000001C
#define _IEC0_U1TXIE_MASK                        0x10000000
#define _IEC0_U1TXIE_LENGTH                      0x00000001

#define _IEC0_I2C1BIE_POSITION                   0x0000001D
#define _IEC0_I2C1BIE_MASK                       0x20000000
#define _IEC0_I2C1BIE_LENGTH                     0x00000001

#define _IEC0_I2C1SIE_POSITION                   0x0000001E
#define _IEC0_I2C1SIE_MASK                       0x40000000
#define _IEC0_I2C1SIE_LENGTH                     0x00000001

#define _IEC0_I2C1MIE_POSITION                   0x0000001F
#define _IEC0_I2C1MIE_MASK                       0x80000000
#define _IEC0_I2C1MIE_LENGTH                     0x00000001

#define _IEC0_U1AEIE_POSITION                    0x0000001A
#define _IEC0_U1AEIE_MASK                        0x04000000
#define _IEC0_U1AEIE_LENGTH                      0x00000001

#define _IEC0_U1ARXIE_POSITION                   0x0000001B
#define _IEC0_U1ARXIE_MASK                       0x08000000
#define _IEC0_U1ARXIE_LENGTH                     0x00000001

#define _IEC0_U1ATXIE_POSITION                   0x0000001C
#define _IEC0_U1ATXIE_MASK                       0x10000000
#define _IEC0_U1ATXIE_LENGTH                     0x00000001

#define _IEC0_SPI3EIE_POSITION                   0x0000001A
#define _IEC0_SPI3EIE_MASK                       0x04000000
#define _IEC0_SPI3EIE_LENGTH                     0x00000001

#define _IEC0_SPI3RXIE_POSITION                  0x0000001B
#define _IEC0_SPI3RXIE_MASK                      0x08000000
#define _IEC0_SPI3RXIE_LENGTH                    0x00000001

#define _IEC0_SPI3TXIE_POSITION                  0x0000001C
#define _IEC0_SPI3TXIE_MASK                      0x10000000
#define _IEC0_SPI3TXIE_LENGTH                    0x00000001

#define _IEC0_SPI1AEIE_POSITION                  0x0000001A
#define _IEC0_SPI1AEIE_MASK                      0x04000000
#define _IEC0_SPI1AEIE_LENGTH                    0x00000001

#define _IEC0_SPI1ARXIE_POSITION                 0x0000001B
#define _IEC0_SPI1ARXIE_MASK                     0x08000000
#define _IEC0_SPI1ARXIE_LENGTH                   0x00000001

#define _IEC0_SPI1ATXIE_POSITION                 0x0000001C
#define _IEC0_SPI1ATXIE_MASK                     0x10000000
#define _IEC0_SPI1ATXIE_LENGTH                   0x00000001

#define _IEC0_I2C3BIE_POSITION                   0x0000001A
#define _IEC0_I2C3BIE_MASK                       0x04000000
#define _IEC0_I2C3BIE_LENGTH                     0x00000001

#define _IEC0_I2C3SIE_POSITION                   0x0000001B
#define _IEC0_I2C3SIE_MASK                       0x08000000
#define _IEC0_I2C3SIE_LENGTH                     0x00000001

#define _IEC0_I2C3MIE_POSITION                   0x0000001C
#define _IEC0_I2C3MIE_MASK                       0x10000000
#define _IEC0_I2C3MIE_LENGTH                     0x00000001

#define _IEC0_I2C1ABIE_POSITION                  0x0000001A
#define _IEC0_I2C1ABIE_MASK                      0x04000000
#define _IEC0_I2C1ABIE_LENGTH                    0x00000001

#define _IEC0_I2C1ASIE_POSITION                  0x0000001B
#define _IEC0_I2C1ASIE_MASK                      0x08000000
#define _IEC0_I2C1ASIE_LENGTH                    0x00000001

#define _IEC0_I2C1AMIE_POSITION                  0x0000001C
#define _IEC0_I2C1AMIE_MASK                      0x10000000
#define _IEC0_I2C1AMIE_LENGTH                    0x00000001

#define _IEC0_w_POSITION                         0x00000000
#define _IEC0_w_MASK                             0xFFFFFFFF
#define _IEC0_w_LENGTH                           0x00000020

#define _IEC1_CNIE_POSITION                      0x00000000
#define _IEC1_CNIE_MASK                          0x00000001
#define _IEC1_CNIE_LENGTH                        0x00000001

#define _IEC1_AD1IE_POSITION                     0x00000001
#define _IEC1_AD1IE_MASK                         0x00000002
#define _IEC1_AD1IE_LENGTH                       0x00000001

#define _IEC1_PMPIE_POSITION                     0x00000002
#define _IEC1_PMPIE_MASK                         0x00000004
#define _IEC1_PMPIE_LENGTH                       0x00000001

#define _IEC1_CMP1IE_POSITION                    0x00000003
#define _IEC1_CMP1IE_MASK                        0x00000008
#define _IEC1_CMP1IE_LENGTH                      0x00000001

#define _IEC1_CMP2IE_POSITION                    0x00000004
#define _IEC1_CMP2IE_MASK                        0x00000010
#define _IEC1_CMP2IE_LENGTH                      0x00000001

#define _IEC1_U3EIE_POSITION                     0x00000005
#define _IEC1_U3EIE_MASK                         0x00000020
#define _IEC1_U3EIE_LENGTH                       0x00000001

#define _IEC1_U3RXIE_POSITION                    0x00000006
#define _IEC1_U3RXIE_MASK                        0x00000040
#define _IEC1_U3RXIE_LENGTH                      0x00000001

#define _IEC1_U3TXIE_POSITION                    0x00000007
#define _IEC1_U3TXIE_MASK                        0x00000080
#define _IEC1_U3TXIE_LENGTH                      0x00000001

#define _IEC1_U2EIE_POSITION                     0x00000008
#define _IEC1_U2EIE_MASK                         0x00000100
#define _IEC1_U2EIE_LENGTH                       0x00000001

#define _IEC1_U2RXIE_POSITION                    0x00000009
#define _IEC1_U2RXIE_MASK                        0x00000200
#define _IEC1_U2RXIE_LENGTH                      0x00000001

#define _IEC1_U2TXIE_POSITION                    0x0000000A
#define _IEC1_U2TXIE_MASK                        0x00000400
#define _IEC1_U2TXIE_LENGTH                      0x00000001

#define _IEC1_I2C2BIE_POSITION                   0x0000000B
#define _IEC1_I2C2BIE_MASK                       0x00000800
#define _IEC1_I2C2BIE_LENGTH                     0x00000001

#define _IEC1_I2C2SIE_POSITION                   0x0000000C
#define _IEC1_I2C2SIE_MASK                       0x00001000
#define _IEC1_I2C2SIE_LENGTH                     0x00000001

#define _IEC1_I2C2MIE_POSITION                   0x0000000D
#define _IEC1_I2C2MIE_MASK                       0x00002000
#define _IEC1_I2C2MIE_LENGTH                     0x00000001

#define _IEC1_FSCMIE_POSITION                    0x0000000E
#define _IEC1_FSCMIE_MASK                        0x00004000
#define _IEC1_FSCMIE_LENGTH                      0x00000001

#define _IEC1_RTCCIE_POSITION                    0x0000000F
#define _IEC1_RTCCIE_MASK                        0x00008000
#define _IEC1_RTCCIE_LENGTH                      0x00000001

#define _IEC1_DMA0IE_POSITION                    0x00000010
#define _IEC1_DMA0IE_MASK                        0x00010000
#define _IEC1_DMA0IE_LENGTH                      0x00000001

#define _IEC1_DMA1IE_POSITION                    0x00000011
#define _IEC1_DMA1IE_MASK                        0x00020000
#define _IEC1_DMA1IE_LENGTH                      0x00000001

#define _IEC1_DMA2IE_POSITION                    0x00000012
#define _IEC1_DMA2IE_MASK                        0x00040000
#define _IEC1_DMA2IE_LENGTH                      0x00000001

#define _IEC1_DMA3IE_POSITION                    0x00000013
#define _IEC1_DMA3IE_MASK                        0x00080000
#define _IEC1_DMA3IE_LENGTH                      0x00000001

#define _IEC1_DMA4IE_POSITION                    0x00000014
#define _IEC1_DMA4IE_MASK                        0x00100000
#define _IEC1_DMA4IE_LENGTH                      0x00000001

#define _IEC1_DMA5IE_POSITION                    0x00000015
#define _IEC1_DMA5IE_MASK                        0x00200000
#define _IEC1_DMA5IE_LENGTH                      0x00000001

#define _IEC1_DMA6IE_POSITION                    0x00000016
#define _IEC1_DMA6IE_MASK                        0x00400000
#define _IEC1_DMA6IE_LENGTH                      0x00000001

#define _IEC1_DMA7IE_POSITION                    0x00000017
#define _IEC1_DMA7IE_MASK                        0x00800000
#define _IEC1_DMA7IE_LENGTH                      0x00000001

#define _IEC1_FCEIE_POSITION                     0x00000018
#define _IEC1_FCEIE_MASK                         0x01000000
#define _IEC1_FCEIE_LENGTH                       0x00000001

#define _IEC1_USBIE_POSITION                     0x00000019
#define _IEC1_USBIE_MASK                         0x02000000
#define _IEC1_USBIE_LENGTH                       0x00000001

#define _IEC1_CAN1IE_POSITION                    0x0000001A
#define _IEC1_CAN1IE_MASK                        0x04000000
#define _IEC1_CAN1IE_LENGTH                      0x00000001

#define _IEC1_CAN2IE_POSITION                    0x0000001B
#define _IEC1_CAN2IE_MASK                        0x08000000
#define _IEC1_CAN2IE_LENGTH                      0x00000001

#define _IEC1_ETHIE_POSITION                     0x0000001C
#define _IEC1_ETHIE_MASK                         0x10000000
#define _IEC1_ETHIE_LENGTH                       0x00000001

#define _IEC1_IC1EIE_POSITION                    0x0000001D
#define _IEC1_IC1EIE_MASK                        0x20000000
#define _IEC1_IC1EIE_LENGTH                      0x00000001

#define _IEC1_IC2EIE_POSITION                    0x0000001E
#define _IEC1_IC2EIE_MASK                        0x40000000
#define _IEC1_IC2EIE_LENGTH                      0x00000001

#define _IEC1_IC3EIE_POSITION                    0x0000001F
#define _IEC1_IC3EIE_MASK                        0x80000000
#define _IEC1_IC3EIE_LENGTH                      0x00000001

#define _IEC1_U2AEIE_POSITION                    0x00000005
#define _IEC1_U2AEIE_MASK                        0x00000020
#define _IEC1_U2AEIE_LENGTH                      0x00000001

#define _IEC1_U2ARXIE_POSITION                   0x00000006
#define _IEC1_U2ARXIE_MASK                       0x00000040
#define _IEC1_U2ARXIE_LENGTH                     0x00000001

#define _IEC1_U2ATXIE_POSITION                   0x00000007
#define _IEC1_U2ATXIE_MASK                       0x00000080
#define _IEC1_U2ATXIE_LENGTH                     0x00000001

#define _IEC1_U3AEIE_POSITION                    0x00000008
#define _IEC1_U3AEIE_MASK                        0x00000100
#define _IEC1_U3AEIE_LENGTH                      0x00000001

#define _IEC1_U3ARXIE_POSITION                   0x00000009
#define _IEC1_U3ARXIE_MASK                       0x00000200
#define _IEC1_U3ARXIE_LENGTH                     0x00000001

#define _IEC1_U3ATXIE_POSITION                   0x0000000A
#define _IEC1_U3ATXIE_MASK                       0x00000400
#define _IEC1_U3ATXIE_LENGTH                     0x00000001

#define _IEC1_SPI2EIE_POSITION                   0x00000005
#define _IEC1_SPI2EIE_MASK                       0x00000020
#define _IEC1_SPI2EIE_LENGTH                     0x00000001

#define _IEC1_SPI2RXIE_POSITION                  0x00000006
#define _IEC1_SPI2RXIE_MASK                      0x00000040
#define _IEC1_SPI2RXIE_LENGTH                    0x00000001

#define _IEC1_SPI2TXIE_POSITION                  0x00000007
#define _IEC1_SPI2TXIE_MASK                      0x00000080
#define _IEC1_SPI2TXIE_LENGTH                    0x00000001

#define _IEC1_SPI4EIE_POSITION                   0x00000008
#define _IEC1_SPI4EIE_MASK                       0x00000100
#define _IEC1_SPI4EIE_LENGTH                     0x00000001

#define _IEC1_SPI4RXIE_POSITION                  0x00000009
#define _IEC1_SPI4RXIE_MASK                      0x00000200
#define _IEC1_SPI4RXIE_LENGTH                    0x00000001

#define _IEC1_SPI4TXIE_POSITION                  0x0000000A
#define _IEC1_SPI4TXIE_MASK                      0x00000400
#define _IEC1_SPI4TXIE_LENGTH                    0x00000001

#define _IEC1_SPI2AEIE_POSITION                  0x00000005
#define _IEC1_SPI2AEIE_MASK                      0x00000020
#define _IEC1_SPI2AEIE_LENGTH                    0x00000001

#define _IEC1_SPI2ARXIE_POSITION                 0x00000006
#define _IEC1_SPI2ARXIE_MASK                     0x00000040
#define _IEC1_SPI2ARXIE_LENGTH                   0x00000001

#define _IEC1_SPI2ATXIE_POSITION                 0x00000007
#define _IEC1_SPI2ATXIE_MASK                     0x00000080
#define _IEC1_SPI2ATXIE_LENGTH                   0x00000001

#define _IEC1_SPI3AEIE_POSITION                  0x00000008
#define _IEC1_SPI3AEIE_MASK                      0x00000100
#define _IEC1_SPI3AEIE_LENGTH                    0x00000001

#define _IEC1_SPI3ARXIE_POSITION                 0x00000009
#define _IEC1_SPI3ARXIE_MASK                     0x00000200
#define _IEC1_SPI3ARXIE_LENGTH                   0x00000001

#define _IEC1_SPI3ATXIE_POSITION                 0x0000000A
#define _IEC1_SPI3ATXIE_MASK                     0x00000400
#define _IEC1_SPI3ATXIE_LENGTH                   0x00000001

#define _IEC1_I2C4BIE_POSITION                   0x00000005
#define _IEC1_I2C4BIE_MASK                       0x00000020
#define _IEC1_I2C4BIE_LENGTH                     0x00000001

#define _IEC1_I2C4SIE_POSITION                   0x00000006
#define _IEC1_I2C4SIE_MASK                       0x00000040
#define _IEC1_I2C4SIE_LENGTH                     0x00000001

#define _IEC1_I2C4MIE_POSITION                   0x00000007
#define _IEC1_I2C4MIE_MASK                       0x00000080
#define _IEC1_I2C4MIE_LENGTH                     0x00000001

#define _IEC1_I2C5BIE_POSITION                   0x00000008
#define _IEC1_I2C5BIE_MASK                       0x00000100
#define _IEC1_I2C5BIE_LENGTH                     0x00000001

#define _IEC1_I2C5SIE_POSITION                   0x00000009
#define _IEC1_I2C5SIE_MASK                       0x00000200
#define _IEC1_I2C5SIE_LENGTH                     0x00000001

#define _IEC1_I2C5MIE_POSITION                   0x0000000A
#define _IEC1_I2C5MIE_MASK                       0x00000400
#define _IEC1_I2C5MIE_LENGTH                     0x00000001

#define _IEC1_I2C2ABIE_POSITION                  0x00000005
#define _IEC1_I2C2ABIE_MASK                      0x00000020
#define _IEC1_I2C2ABIE_LENGTH                    0x00000001

#define _IEC1_I2C2ASIE_POSITION                  0x00000006
#define _IEC1_I2C2ASIE_MASK                      0x00000040
#define _IEC1_I2C2ASIE_LENGTH                    0x00000001

#define _IEC1_I2C2AMIE_POSITION                  0x00000007
#define _IEC1_I2C2AMIE_MASK                      0x00000080
#define _IEC1_I2C2AMIE_LENGTH                    0x00000001

#define _IEC1_I2C3ABIE_POSITION                  0x00000008
#define _IEC1_I2C3ABIE_MASK                      0x00000100
#define _IEC1_I2C3ABIE_LENGTH                    0x00000001

#define _IEC1_I2C3ASIE_POSITION                  0x00000009
#define _IEC1_I2C3ASIE_MASK                      0x00000200
#define _IEC1_I2C3ASIE_LENGTH                    0x00000001

#define _IEC1_I2C3AMIE_POSITION                  0x0000000A
#define _IEC1_I2C3AMIE_MASK                      0x00000400
#define _IEC1_I2C3AMIE_LENGTH                    0x00000001

#define _IEC1_w_POSITION                         0x00000000
#define _IEC1_w_MASK                             0xFFFFFFFF
#define _IEC1_w_LENGTH                           0x00000020

#define _IEC2_IC4EIE_POSITION                    0x00000000
#define _IEC2_IC4EIE_MASK                        0x00000001
#define _IEC2_IC4EIE_LENGTH                      0x00000001

#define _IEC2_IC5EIE_POSITION                    0x00000001
#define _IEC2_IC5EIE_MASK                        0x00000002
#define _IEC2_IC5EIE_LENGTH                      0x00000001

#define _IEC2_PMPEIE_POSITION                    0x00000002
#define _IEC2_PMPEIE_MASK                        0x00000004
#define _IEC2_PMPEIE_LENGTH                      0x00000001

#define _IEC2_U1BEIE_POSITION                    0x00000003
#define _IEC2_U1BEIE_MASK                        0x00000008
#define _IEC2_U1BEIE_LENGTH                      0x00000001

#define _IEC2_U1BRXIE_POSITION                   0x00000004
#define _IEC2_U1BRXIE_MASK                       0x00000010
#define _IEC2_U1BRXIE_LENGTH                     0x00000001

#define _IEC2_U1BTXIE_POSITION                   0x00000005
#define _IEC2_U1BTXIE_MASK                       0x00000020
#define _IEC2_U1BTXIE_LENGTH                     0x00000001

#define _IEC2_U2BEIE_POSITION                    0x00000006
#define _IEC2_U2BEIE_MASK                        0x00000040
#define _IEC2_U2BEIE_LENGTH                      0x00000001

#define _IEC2_U2BRXIE_POSITION                   0x00000007
#define _IEC2_U2BRXIE_MASK                       0x00000080
#define _IEC2_U2BRXIE_LENGTH                     0x00000001

#define _IEC2_U2BTXIE_POSITION                   0x00000008
#define _IEC2_U2BTXIE_MASK                       0x00000100
#define _IEC2_U2BTXIE_LENGTH                     0x00000001

#define _IEC2_U3BEIE_POSITION                    0x00000009
#define _IEC2_U3BEIE_MASK                        0x00000200
#define _IEC2_U3BEIE_LENGTH                      0x00000001

#define _IEC2_U3BRXIE_POSITION                   0x0000000A
#define _IEC2_U3BRXIE_MASK                       0x00000400
#define _IEC2_U3BRXIE_LENGTH                     0x00000001

#define _IEC2_U3BTXIE_POSITION                   0x0000000B
#define _IEC2_U3BTXIE_MASK                       0x00000800
#define _IEC2_U3BTXIE_LENGTH                     0x00000001

#define _IEC2_U4EIE_POSITION                     0x00000003
#define _IEC2_U4EIE_MASK                         0x00000008
#define _IEC2_U4EIE_LENGTH                       0x00000001

#define _IEC2_U4RXIE_POSITION                    0x00000004
#define _IEC2_U4RXIE_MASK                        0x00000010
#define _IEC2_U4RXIE_LENGTH                      0x00000001

#define _IEC2_U4TXIE_POSITION                    0x00000005
#define _IEC2_U4TXIE_MASK                        0x00000020
#define _IEC2_U4TXIE_LENGTH                      0x00000001

#define _IEC2_U6EIE_POSITION                     0x00000006
#define _IEC2_U6EIE_MASK                         0x00000040
#define _IEC2_U6EIE_LENGTH                       0x00000001

#define _IEC2_U6RXIE_POSITION                    0x00000007
#define _IEC2_U6RXIE_MASK                        0x00000080
#define _IEC2_U6RXIE_LENGTH                      0x00000001

#define _IEC2_U6TXIE_POSITION                    0x00000008
#define _IEC2_U6TXIE_MASK                        0x00000100
#define _IEC2_U6TXIE_LENGTH                      0x00000001

#define _IEC2_U5EIE_POSITION                     0x00000009
#define _IEC2_U5EIE_MASK                         0x00000200
#define _IEC2_U5EIE_LENGTH                       0x00000001

#define _IEC2_U5RXIE_POSITION                    0x0000000A
#define _IEC2_U5RXIE_MASK                        0x00000400
#define _IEC2_U5RXIE_LENGTH                      0x00000001

#define _IEC2_U5TXIE_POSITION                    0x0000000B
#define _IEC2_U5TXIE_MASK                        0x00000800
#define _IEC2_U5TXIE_LENGTH                      0x00000001

#define _IEC2_w_POSITION                         0x00000000
#define _IEC2_w_MASK                             0xFFFFFFFF
#define _IEC2_w_LENGTH                           0x00000020

#define _IPC0_CTIS_POSITION                      0x00000000
#define _IPC0_CTIS_MASK                          0x00000003
#define _IPC0_CTIS_LENGTH                        0x00000002

#define _IPC0_CTIP_POSITION                      0x00000002
#define _IPC0_CTIP_MASK                          0x0000001C
#define _IPC0_CTIP_LENGTH                        0x00000003

#define _IPC0_CS0IS_POSITION                     0x00000008
#define _IPC0_CS0IS_MASK                         0x00000300
#define _IPC0_CS0IS_LENGTH                       0x00000002

#define _IPC0_CS0IP_POSITION                     0x0000000A
#define _IPC0_CS0IP_MASK                         0x00001C00
#define _IPC0_CS0IP_LENGTH                       0x00000003

#define _IPC0_CS1IS_POSITION                     0x00000010
#define _IPC0_CS1IS_MASK                         0x00030000
#define _IPC0_CS1IS_LENGTH                       0x00000002

#define _IPC0_CS1IP_POSITION                     0x00000012
#define _IPC0_CS1IP_MASK                         0x001C0000
#define _IPC0_CS1IP_LENGTH                       0x00000003

#define _IPC0_INT0IS_POSITION                    0x00000018
#define _IPC0_INT0IS_MASK                        0x03000000
#define _IPC0_INT0IS_LENGTH                      0x00000002

#define _IPC0_INT0IP_POSITION                    0x0000001A
#define _IPC0_INT0IP_MASK                        0x1C000000
#define _IPC0_INT0IP_LENGTH                      0x00000003

#define _IPC0_w_POSITION                         0x00000000
#define _IPC0_w_MASK                             0xFFFFFFFF
#define _IPC0_w_LENGTH                           0x00000020

#define _IPC1_T1IS_POSITION                      0x00000000
#define _IPC1_T1IS_MASK                          0x00000003
#define _IPC1_T1IS_LENGTH                        0x00000002

#define _IPC1_T1IP_POSITION                      0x00000002
#define _IPC1_T1IP_MASK                          0x0000001C
#define _IPC1_T1IP_LENGTH                        0x00000003

#define _IPC1_IC1IS_POSITION                     0x00000008
#define _IPC1_IC1IS_MASK                         0x00000300
#define _IPC1_IC1IS_LENGTH                       0x00000002

#define _IPC1_IC1IP_POSITION                     0x0000000A
#define _IPC1_IC1IP_MASK                         0x00001C00
#define _IPC1_IC1IP_LENGTH                       0x00000003

#define _IPC1_OC1IS_POSITION                     0x00000010
#define _IPC1_OC1IS_MASK                         0x00030000
#define _IPC1_OC1IS_LENGTH                       0x00000002

#define _IPC1_OC1IP_POSITION                     0x00000012
#define _IPC1_OC1IP_MASK                         0x001C0000
#define _IPC1_OC1IP_LENGTH                       0x00000003

#define _IPC1_INT1IS_POSITION                    0x00000018
#define _IPC1_INT1IS_MASK                        0x03000000
#define _IPC1_INT1IS_LENGTH                      0x00000002

#define _IPC1_INT1IP_POSITION                    0x0000001A
#define _IPC1_INT1IP_MASK                        0x1C000000
#define _IPC1_INT1IP_LENGTH                      0x00000003

#define _IPC1_w_POSITION                         0x00000000
#define _IPC1_w_MASK                             0xFFFFFFFF
#define _IPC1_w_LENGTH                           0x00000020

#define _IPC2_T2IS_POSITION                      0x00000000
#define _IPC2_T2IS_MASK                          0x00000003
#define _IPC2_T2IS_LENGTH                        0x00000002

#define _IPC2_T2IP_POSITION                      0x00000002
#define _IPC2_T2IP_MASK                          0x0000001C
#define _IPC2_T2IP_LENGTH                        0x00000003

#define _IPC2_IC2IS_POSITION                     0x00000008
#define _IPC2_IC2IS_MASK                         0x00000300
#define _IPC2_IC2IS_LENGTH                       0x00000002

#define _IPC2_IC2IP_POSITION                     0x0000000A
#define _IPC2_IC2IP_MASK                         0x00001C00
#define _IPC2_IC2IP_LENGTH                       0x00000003

#define _IPC2_OC2IS_POSITION                     0x00000010
#define _IPC2_OC2IS_MASK                         0x00030000
#define _IPC2_OC2IS_LENGTH                       0x00000002

#define _IPC2_OC2IP_POSITION                     0x00000012
#define _IPC2_OC2IP_MASK                         0x001C0000
#define _IPC2_OC2IP_LENGTH                       0x00000003

#define _IPC2_INT2IS_POSITION                    0x00000018
#define _IPC2_INT2IS_MASK                        0x03000000
#define _IPC2_INT2IS_LENGTH                      0x00000002

#define _IPC2_INT2IP_POSITION                    0x0000001A
#define _IPC2_INT2IP_MASK                        0x1C000000
#define _IPC2_INT2IP_LENGTH                      0x00000003

#define _IPC2_w_POSITION                         0x00000000
#define _IPC2_w_MASK                             0xFFFFFFFF
#define _IPC2_w_LENGTH                           0x00000020

#define _IPC3_T3IS_POSITION                      0x00000000
#define _IPC3_T3IS_MASK                          0x00000003
#define _IPC3_T3IS_LENGTH                        0x00000002

#define _IPC3_T3IP_POSITION                      0x00000002
#define _IPC3_T3IP_MASK                          0x0000001C
#define _IPC3_T3IP_LENGTH                        0x00000003

#define _IPC3_IC3IS_POSITION                     0x00000008
#define _IPC3_IC3IS_MASK                         0x00000300
#define _IPC3_IC3IS_LENGTH                       0x00000002

#define _IPC3_IC3IP_POSITION                     0x0000000A
#define _IPC3_IC3IP_MASK                         0x00001C00
#define _IPC3_IC3IP_LENGTH                       0x00000003

#define _IPC3_OC3IS_POSITION                     0x00000010
#define _IPC3_OC3IS_MASK                         0x00030000
#define _IPC3_OC3IS_LENGTH                       0x00000002

#define _IPC3_OC3IP_POSITION                     0x00000012
#define _IPC3_OC3IP_MASK                         0x001C0000
#define _IPC3_OC3IP_LENGTH                       0x00000003

#define _IPC3_INT3IS_POSITION                    0x00000018
#define _IPC3_INT3IS_MASK                        0x03000000
#define _IPC3_INT3IS_LENGTH                      0x00000002

#define _IPC3_INT3IP_POSITION                    0x0000001A
#define _IPC3_INT3IP_MASK                        0x1C000000
#define _IPC3_INT3IP_LENGTH                      0x00000003

#define _IPC3_w_POSITION                         0x00000000
#define _IPC3_w_MASK                             0xFFFFFFFF
#define _IPC3_w_LENGTH                           0x00000020

#define _IPC4_T4IS_POSITION                      0x00000000
#define _IPC4_T4IS_MASK                          0x00000003
#define _IPC4_T4IS_LENGTH                        0x00000002

#define _IPC4_T4IP_POSITION                      0x00000002
#define _IPC4_T4IP_MASK                          0x0000001C
#define _IPC4_T4IP_LENGTH                        0x00000003

#define _IPC4_IC4IS_POSITION                     0x00000008
#define _IPC4_IC4IS_MASK                         0x00000300
#define _IPC4_IC4IS_LENGTH                       0x00000002

#define _IPC4_IC4IP_POSITION                     0x0000000A
#define _IPC4_IC4IP_MASK                         0x00001C00
#define _IPC4_IC4IP_LENGTH                       0x00000003

#define _IPC4_OC4IS_POSITION                     0x00000010
#define _IPC4_OC4IS_MASK                         0x00030000
#define _IPC4_OC4IS_LENGTH                       0x00000002

#define _IPC4_OC4IP_POSITION                     0x00000012
#define _IPC4_OC4IP_MASK                         0x001C0000
#define _IPC4_OC4IP_LENGTH                       0x00000003

#define _IPC4_INT4IS_POSITION                    0x00000018
#define _IPC4_INT4IS_MASK                        0x03000000
#define _IPC4_INT4IS_LENGTH                      0x00000002

#define _IPC4_INT4IP_POSITION                    0x0000001A
#define _IPC4_INT4IP_MASK                        0x1C000000
#define _IPC4_INT4IP_LENGTH                      0x00000003

#define _IPC4_w_POSITION                         0x00000000
#define _IPC4_w_MASK                             0xFFFFFFFF
#define _IPC4_w_LENGTH                           0x00000020

#define _IPC5_T5IS_POSITION                      0x00000000
#define _IPC5_T5IS_MASK                          0x00000003
#define _IPC5_T5IS_LENGTH                        0x00000002

#define _IPC5_T5IP_POSITION                      0x00000002
#define _IPC5_T5IP_MASK                          0x0000001C
#define _IPC5_T5IP_LENGTH                        0x00000003

#define _IPC5_IC5IS_POSITION                     0x00000008
#define _IPC5_IC5IS_MASK                         0x00000300
#define _IPC5_IC5IS_LENGTH                       0x00000002

#define _IPC5_IC5IP_POSITION                     0x0000000A
#define _IPC5_IC5IP_MASK                         0x00001C00
#define _IPC5_IC5IP_LENGTH                       0x00000003

#define _IPC5_OC5IS_POSITION                     0x00000010
#define _IPC5_OC5IS_MASK                         0x00030000
#define _IPC5_OC5IS_LENGTH                       0x00000002

#define _IPC5_OC5IP_POSITION                     0x00000012
#define _IPC5_OC5IP_MASK                         0x001C0000
#define _IPC5_OC5IP_LENGTH                       0x00000003

#define _IPC5_w_POSITION                         0x00000000
#define _IPC5_w_MASK                             0xFFFFFFFF
#define _IPC5_w_LENGTH                           0x00000020

#define _IPC6_U1IS_POSITION                      0x00000000
#define _IPC6_U1IS_MASK                          0x00000003
#define _IPC6_U1IS_LENGTH                        0x00000002

#define _IPC6_U1IP_POSITION                      0x00000002
#define _IPC6_U1IP_MASK                          0x0000001C
#define _IPC6_U1IP_LENGTH                        0x00000003

#define _IPC6_I2C1IS_POSITION                    0x00000008
#define _IPC6_I2C1IS_MASK                        0x00000300
#define _IPC6_I2C1IS_LENGTH                      0x00000002

#define _IPC6_I2C1IP_POSITION                    0x0000000A
#define _IPC6_I2C1IP_MASK                        0x00001C00
#define _IPC6_I2C1IP_LENGTH                      0x00000003

#define _IPC6_CNIS_POSITION                      0x00000010
#define _IPC6_CNIS_MASK                          0x00030000
#define _IPC6_CNIS_LENGTH                        0x00000002

#define _IPC6_CNIP_POSITION                      0x00000012
#define _IPC6_CNIP_MASK                          0x001C0000
#define _IPC6_CNIP_LENGTH                        0x00000003

#define _IPC6_AD1IS_POSITION                     0x00000018
#define _IPC6_AD1IS_MASK                         0x03000000
#define _IPC6_AD1IS_LENGTH                       0x00000002

#define _IPC6_AD1IP_POSITION                     0x0000001A
#define _IPC6_AD1IP_MASK                         0x1C000000
#define _IPC6_AD1IP_LENGTH                       0x00000003

#define _IPC6_U1AIS_POSITION                     0x00000000
#define _IPC6_U1AIS_MASK                         0x00000003
#define _IPC6_U1AIS_LENGTH                       0x00000002

#define _IPC6_U1AIP_POSITION                     0x00000002
#define _IPC6_U1AIP_MASK                         0x0000001C
#define _IPC6_U1AIP_LENGTH                       0x00000003

#define _IPC6_SPI3IS_POSITION                    0x00000000
#define _IPC6_SPI3IS_MASK                        0x00000003
#define _IPC6_SPI3IS_LENGTH                      0x00000002

#define _IPC6_SPI3IP_POSITION                    0x00000002
#define _IPC6_SPI3IP_MASK                        0x0000001C
#define _IPC6_SPI3IP_LENGTH                      0x00000003

#define _IPC6_SPI1AIS_POSITION                   0x00000000
#define _IPC6_SPI1AIS_MASK                       0x00000003
#define _IPC6_SPI1AIS_LENGTH                     0x00000002

#define _IPC6_SPI1AIP_POSITION                   0x00000002
#define _IPC6_SPI1AIP_MASK                       0x0000001C
#define _IPC6_SPI1AIP_LENGTH                     0x00000003

#define _IPC6_I2C3IS_POSITION                    0x00000000
#define _IPC6_I2C3IS_MASK                        0x00000003
#define _IPC6_I2C3IS_LENGTH                      0x00000002

#define _IPC6_I2C3IP_POSITION                    0x00000002
#define _IPC6_I2C3IP_MASK                        0x0000001C
#define _IPC6_I2C3IP_LENGTH                      0x00000003

#define _IPC6_I2C1AIS_POSITION                   0x00000000
#define _IPC6_I2C1AIS_MASK                       0x00000003
#define _IPC6_I2C1AIS_LENGTH                     0x00000002

#define _IPC6_I2C1AIP_POSITION                   0x00000002
#define _IPC6_I2C1AIP_MASK                       0x0000001C
#define _IPC6_I2C1AIP_LENGTH                     0x00000003

#define _IPC6_w_POSITION                         0x00000000
#define _IPC6_w_MASK                             0xFFFFFFFF
#define _IPC6_w_LENGTH                           0x00000020

#define _IPC7_PMPIS_POSITION                     0x00000000
#define _IPC7_PMPIS_MASK                         0x00000003
#define _IPC7_PMPIS_LENGTH                       0x00000002

#define _IPC7_PMPIP_POSITION                     0x00000002
#define _IPC7_PMPIP_MASK                         0x0000001C
#define _IPC7_PMPIP_LENGTH                       0x00000003

#define _IPC7_CMP1IS_POSITION                    0x00000008
#define _IPC7_CMP1IS_MASK                        0x00000300
#define _IPC7_CMP1IS_LENGTH                      0x00000002

#define _IPC7_CMP1IP_POSITION                    0x0000000A
#define _IPC7_CMP1IP_MASK                        0x00001C00
#define _IPC7_CMP1IP_LENGTH                      0x00000003

#define _IPC7_CMP2IS_POSITION                    0x00000010
#define _IPC7_CMP2IS_MASK                        0x00030000
#define _IPC7_CMP2IS_LENGTH                      0x00000002

#define _IPC7_CMP2IP_POSITION                    0x00000012
#define _IPC7_CMP2IP_MASK                        0x001C0000
#define _IPC7_CMP2IP_LENGTH                      0x00000003

#define _IPC7_U3IS_POSITION                      0x00000018
#define _IPC7_U3IS_MASK                          0x03000000
#define _IPC7_U3IS_LENGTH                        0x00000002

#define _IPC7_U3IP_POSITION                      0x0000001A
#define _IPC7_U3IP_MASK                          0x1C000000
#define _IPC7_U3IP_LENGTH                        0x00000003

#define _IPC7_U2AIS_POSITION                     0x00000018
#define _IPC7_U2AIS_MASK                         0x03000000
#define _IPC7_U2AIS_LENGTH                       0x00000002

#define _IPC7_U2AIP_POSITION                     0x0000001A
#define _IPC7_U2AIP_MASK                         0x1C000000
#define _IPC7_U2AIP_LENGTH                       0x00000003

#define _IPC7_SPI2IS_POSITION                    0x00000018
#define _IPC7_SPI2IS_MASK                        0x03000000
#define _IPC7_SPI2IS_LENGTH                      0x00000002

#define _IPC7_SPI2IP_POSITION                    0x0000001A
#define _IPC7_SPI2IP_MASK                        0x1C000000
#define _IPC7_SPI2IP_LENGTH                      0x00000003

#define _IPC7_SPI2AIS_POSITION                   0x00000018
#define _IPC7_SPI2AIS_MASK                       0x03000000
#define _IPC7_SPI2AIS_LENGTH                     0x00000002

#define _IPC7_SPI2AIP_POSITION                   0x0000001A
#define _IPC7_SPI2AIP_MASK                       0x1C000000
#define _IPC7_SPI2AIP_LENGTH                     0x00000003

#define _IPC7_I2C4IS_POSITION                    0x00000018
#define _IPC7_I2C4IS_MASK                        0x03000000
#define _IPC7_I2C4IS_LENGTH                      0x00000002

#define _IPC7_I2C4IP_POSITION                    0x0000001A
#define _IPC7_I2C4IP_MASK                        0x1C000000
#define _IPC7_I2C4IP_LENGTH                      0x00000003

#define _IPC7_I2C2AIS_POSITION                   0x00000018
#define _IPC7_I2C2AIS_MASK                       0x03000000
#define _IPC7_I2C2AIS_LENGTH                     0x00000002

#define _IPC7_I2C2AIP_POSITION                   0x0000001A
#define _IPC7_I2C2AIP_MASK                       0x1C000000
#define _IPC7_I2C2AIP_LENGTH                     0x00000003

#define _IPC7_w_POSITION                         0x00000000
#define _IPC7_w_MASK                             0xFFFFFFFF
#define _IPC7_w_LENGTH                           0x00000020

#define _IPC8_U2IS_POSITION                      0x00000000
#define _IPC8_U2IS_MASK                          0x00000003
#define _IPC8_U2IS_LENGTH                        0x00000002

#define _IPC8_U2IP_POSITION                      0x00000002
#define _IPC8_U2IP_MASK                          0x0000001C
#define _IPC8_U2IP_LENGTH                        0x00000003

#define _IPC8_I2C2IS_POSITION                    0x00000008
#define _IPC8_I2C2IS_MASK                        0x00000300
#define _IPC8_I2C2IS_LENGTH                      0x00000002

#define _IPC8_I2C2IP_POSITION                    0x0000000A
#define _IPC8_I2C2IP_MASK                        0x00001C00
#define _IPC8_I2C2IP_LENGTH                      0x00000003

#define _IPC8_FSCMIS_POSITION                    0x00000010
#define _IPC8_FSCMIS_MASK                        0x00030000
#define _IPC8_FSCMIS_LENGTH                      0x00000002

#define _IPC8_FSCMIP_POSITION                    0x00000012
#define _IPC8_FSCMIP_MASK                        0x001C0000
#define _IPC8_FSCMIP_LENGTH                      0x00000003

#define _IPC8_RTCCIS_POSITION                    0x00000018
#define _IPC8_RTCCIS_MASK                        0x03000000
#define _IPC8_RTCCIS_LENGTH                      0x00000002

#define _IPC8_RTCCIP_POSITION                    0x0000001A
#define _IPC8_RTCCIP_MASK                        0x1C000000
#define _IPC8_RTCCIP_LENGTH                      0x00000003

#define _IPC8_U3AIS_POSITION                     0x00000000
#define _IPC8_U3AIS_MASK                         0x00000003
#define _IPC8_U3AIS_LENGTH                       0x00000002

#define _IPC8_U3AIP_POSITION                     0x00000002
#define _IPC8_U3AIP_MASK                         0x0000001C
#define _IPC8_U3AIP_LENGTH                       0x00000003

#define _IPC8_SPI4IS_POSITION                    0x00000000
#define _IPC8_SPI4IS_MASK                        0x00000003
#define _IPC8_SPI4IS_LENGTH                      0x00000002

#define _IPC8_SPI4IP_POSITION                    0x00000002
#define _IPC8_SPI4IP_MASK                        0x0000001C
#define _IPC8_SPI4IP_LENGTH                      0x00000003

#define _IPC8_SPI3AIS_POSITION                   0x00000000
#define _IPC8_SPI3AIS_MASK                       0x00000003
#define _IPC8_SPI3AIS_LENGTH                     0x00000002

#define _IPC8_SPI3AIP_POSITION                   0x00000002
#define _IPC8_SPI3AIP_MASK                       0x0000001C
#define _IPC8_SPI3AIP_LENGTH                     0x00000003

#define _IPC8_I2C5IS_POSITION                    0x00000000
#define _IPC8_I2C5IS_MASK                        0x00000003
#define _IPC8_I2C5IS_LENGTH                      0x00000002

#define _IPC8_I2C5IP_POSITION                    0x00000002
#define _IPC8_I2C5IP_MASK                        0x0000001C
#define _IPC8_I2C5IP_LENGTH                      0x00000003

#define _IPC8_I2C3AIS_POSITION                   0x00000000
#define _IPC8_I2C3AIS_MASK                       0x00000003
#define _IPC8_I2C3AIS_LENGTH                     0x00000002

#define _IPC8_I2C3AIP_POSITION                   0x00000002
#define _IPC8_I2C3AIP_MASK                       0x0000001C
#define _IPC8_I2C3AIP_LENGTH                     0x00000003

#define _IPC8_w_POSITION                         0x00000000
#define _IPC8_w_MASK                             0xFFFFFFFF
#define _IPC8_w_LENGTH                           0x00000020

#define _IPC9_DMA0IS_POSITION                    0x00000000
#define _IPC9_DMA0IS_MASK                        0x00000003
#define _IPC9_DMA0IS_LENGTH                      0x00000002

#define _IPC9_DMA0IP_POSITION                    0x00000002
#define _IPC9_DMA0IP_MASK                        0x0000001C
#define _IPC9_DMA0IP_LENGTH                      0x00000003

#define _IPC9_DMA1IS_POSITION                    0x00000008
#define _IPC9_DMA1IS_MASK                        0x00000300
#define _IPC9_DMA1IS_LENGTH                      0x00000002

#define _IPC9_DMA1IP_POSITION                    0x0000000A
#define _IPC9_DMA1IP_MASK                        0x00001C00
#define _IPC9_DMA1IP_LENGTH                      0x00000003

#define _IPC9_DMA2IS_POSITION                    0x00000010
#define _IPC9_DMA2IS_MASK                        0x00030000
#define _IPC9_DMA2IS_LENGTH                      0x00000002

#define _IPC9_DMA2IP_POSITION                    0x00000012
#define _IPC9_DMA2IP_MASK                        0x001C0000
#define _IPC9_DMA2IP_LENGTH                      0x00000003

#define _IPC9_DMA3IS_POSITION                    0x00000018
#define _IPC9_DMA3IS_MASK                        0x03000000
#define _IPC9_DMA3IS_LENGTH                      0x00000002

#define _IPC9_DMA3IP_POSITION                    0x0000001A
#define _IPC9_DMA3IP_MASK                        0x1C000000
#define _IPC9_DMA3IP_LENGTH                      0x00000003

#define _IPC9_w_POSITION                         0x00000000
#define _IPC9_w_MASK                             0xFFFFFFFF
#define _IPC9_w_LENGTH                           0x00000020

#define _IPC10_DMA4IS_POSITION                   0x00000000
#define _IPC10_DMA4IS_MASK                       0x00000003
#define _IPC10_DMA4IS_LENGTH                     0x00000002

#define _IPC10_DMA4IP_POSITION                   0x00000002
#define _IPC10_DMA4IP_MASK                       0x0000001C
#define _IPC10_DMA4IP_LENGTH                     0x00000003

#define _IPC10_DMA5IS_POSITION                   0x00000008
#define _IPC10_DMA5IS_MASK                       0x00000300
#define _IPC10_DMA5IS_LENGTH                     0x00000002

#define _IPC10_DMA5IP_POSITION                   0x0000000A
#define _IPC10_DMA5IP_MASK                       0x00001C00
#define _IPC10_DMA5IP_LENGTH                     0x00000003

#define _IPC10_DMA6IS_POSITION                   0x00000010
#define _IPC10_DMA6IS_MASK                       0x00030000
#define _IPC10_DMA6IS_LENGTH                     0x00000002

#define _IPC10_DMA6IP_POSITION                   0x00000012
#define _IPC10_DMA6IP_MASK                       0x001C0000
#define _IPC10_DMA6IP_LENGTH                     0x00000003

#define _IPC10_DMA7IS_POSITION                   0x00000018
#define _IPC10_DMA7IS_MASK                       0x03000000
#define _IPC10_DMA7IS_LENGTH                     0x00000002

#define _IPC10_DMA7IP_POSITION                   0x0000001A
#define _IPC10_DMA7IP_MASK                       0x1C000000
#define _IPC10_DMA7IP_LENGTH                     0x00000003

#define _IPC10_w_POSITION                        0x00000000
#define _IPC10_w_MASK                            0xFFFFFFFF
#define _IPC10_w_LENGTH                          0x00000020

#define _IPC11_FCEIS_POSITION                    0x00000000
#define _IPC11_FCEIS_MASK                        0x00000003
#define _IPC11_FCEIS_LENGTH                      0x00000002

#define _IPC11_FCEIP_POSITION                    0x00000002
#define _IPC11_FCEIP_MASK                        0x0000001C
#define _IPC11_FCEIP_LENGTH                      0x00000003

#define _IPC11_USBIS_POSITION                    0x00000008
#define _IPC11_USBIS_MASK                        0x00000300
#define _IPC11_USBIS_LENGTH                      0x00000002

#define _IPC11_USBIP_POSITION                    0x0000000A
#define _IPC11_USBIP_MASK                        0x00001C00
#define _IPC11_USBIP_LENGTH                      0x00000003

#define _IPC11_CAN1IS_POSITION                   0x00000010
#define _IPC11_CAN1IS_MASK                       0x00030000
#define _IPC11_CAN1IS_LENGTH                     0x00000002

#define _IPC11_CAN1IP_POSITION                   0x00000012
#define _IPC11_CAN1IP_MASK                       0x001C0000
#define _IPC11_CAN1IP_LENGTH                     0x00000003

#define _IPC11_CAN2IS_POSITION                   0x00000018
#define _IPC11_CAN2IS_MASK                       0x03000000
#define _IPC11_CAN2IS_LENGTH                     0x00000002

#define _IPC11_CAN2IP_POSITION                   0x0000001A
#define _IPC11_CAN2IP_MASK                       0x1C000000
#define _IPC11_CAN2IP_LENGTH                     0x00000003

#define _IPC11_w_POSITION                        0x00000000
#define _IPC11_w_MASK                            0xFFFFFFFF
#define _IPC11_w_LENGTH                          0x00000020

#define _IPC12_ETHIS_POSITION                    0x00000000
#define _IPC12_ETHIS_MASK                        0x00000003
#define _IPC12_ETHIS_LENGTH                      0x00000002

#define _IPC12_ETHIP_POSITION                    0x00000002
#define _IPC12_ETHIP_MASK                        0x0000001C
#define _IPC12_ETHIP_LENGTH                      0x00000003

#define _IPC12_U4IS_POSITION                     0x00000008
#define _IPC12_U4IS_MASK                         0x00000300
#define _IPC12_U4IS_LENGTH                       0x00000002

#define _IPC12_U4IP_POSITION                     0x0000000A
#define _IPC12_U4IP_MASK                         0x00001C00
#define _IPC12_U4IP_LENGTH                       0x00000003

#define _IPC12_U6IS_POSITION                     0x00000010
#define _IPC12_U6IS_MASK                         0x00030000
#define _IPC12_U6IS_LENGTH                       0x00000002

#define _IPC12_U6IP_POSITION                     0x00000012
#define _IPC12_U6IP_MASK                         0x001C0000
#define _IPC12_U6IP_LENGTH                       0x00000003

#define _IPC12_U5IS_POSITION                     0x00000018
#define _IPC12_U5IS_MASK                         0x03000000
#define _IPC12_U5IS_LENGTH                       0x00000002

#define _IPC12_U5IP_POSITION                     0x0000001A
#define _IPC12_U5IP_MASK                         0x1C000000
#define _IPC12_U5IP_LENGTH                       0x00000003

#define _IPC12_U1BIS_POSITION                    0x00000008
#define _IPC12_U1BIS_MASK                        0x00000300
#define _IPC12_U1BIS_LENGTH                      0x00000002

#define _IPC12_U1BIP_POSITION                    0x0000000A
#define _IPC12_U1BIP_MASK                        0x00001C00
#define _IPC12_U1BIP_LENGTH                      0x00000003

#define _IPC12_U2BIS_POSITION                    0x00000010
#define _IPC12_U2BIS_MASK                        0x00030000
#define _IPC12_U2BIS_LENGTH                      0x00000002

#define _IPC12_U2BIP_POSITION                    0x00000012
#define _IPC12_U2BIP_MASK                        0x001C0000
#define _IPC12_U2BIP_LENGTH                      0x00000003

#define _IPC12_U3BIS_POSITION                    0x00000018
#define _IPC12_U3BIS_MASK                        0x03000000
#define _IPC12_U3BIS_LENGTH                      0x00000002

#define _IPC12_U3BIP_POSITION                    0x0000001A
#define _IPC12_U3BIP_MASK                        0x1C000000
#define _IPC12_U3BIP_LENGTH                      0x00000003

#define _IPC12_w_POSITION                        0x00000000
#define _IPC12_w_MASK                            0xFFFFFFFF
#define _IPC12_w_LENGTH                          0x00000020

#define _BMXCON_BMXARB_POSITION                  0x00000000
#define _BMXCON_BMXARB_MASK                      0x00000007
#define _BMXCON_BMXARB_LENGTH                    0x00000003

#define _BMXCON_BMXWSDRM_POSITION                0x00000006
#define _BMXCON_BMXWSDRM_MASK                    0x00000040
#define _BMXCON_BMXWSDRM_LENGTH                  0x00000001

#define _BMXCON_BMXERRIS_POSITION                0x00000010
#define _BMXCON_BMXERRIS_MASK                    0x00010000
#define _BMXCON_BMXERRIS_LENGTH                  0x00000001

#define _BMXCON_BMXERRDS_POSITION                0x00000011
#define _BMXCON_BMXERRDS_MASK                    0x00020000
#define _BMXCON_BMXERRDS_LENGTH                  0x00000001

#define _BMXCON_BMXERRDMA_POSITION               0x00000012
#define _BMXCON_BMXERRDMA_MASK                   0x00040000
#define _BMXCON_BMXERRDMA_LENGTH                 0x00000001

#define _BMXCON_BMXERRICD_POSITION               0x00000013
#define _BMXCON_BMXERRICD_MASK                   0x00080000
#define _BMXCON_BMXERRICD_LENGTH                 0x00000001

#define _BMXCON_BMXERRIXI_POSITION               0x00000014
#define _BMXCON_BMXERRIXI_MASK                   0x00100000
#define _BMXCON_BMXERRIXI_LENGTH                 0x00000001

#define _BMXCON_BMXCHEDMA_POSITION               0x0000001A
#define _BMXCON_BMXCHEDMA_MASK                   0x04000000
#define _BMXCON_BMXCHEDMA_LENGTH                 0x00000001

#define _BMXCON_w_POSITION                       0x00000000
#define _BMXCON_w_MASK                           0xFFFFFFFF
#define _BMXCON_w_LENGTH                         0x00000020

#define _DMACON_DMABUSY_POSITION                 0x0000000B
#define _DMACON_DMABUSY_MASK                     0x00000800
#define _DMACON_DMABUSY_LENGTH                   0x00000001

#define _DMACON_SUSPEND_POSITION                 0x0000000C
#define _DMACON_SUSPEND_MASK                     0x00001000
#define _DMACON_SUSPEND_LENGTH                   0x00000001

#define _DMACON_ON_POSITION                      0x0000000F
#define _DMACON_ON_MASK                          0x00008000
#define _DMACON_ON_LENGTH                        0x00000001

#define _DMACON_w_POSITION                       0x00000000
#define _DMACON_w_MASK                           0xFFFFFFFF
#define _DMACON_w_LENGTH                         0x00000020

#define _DMASTAT_DMACH_POSITION                  0x00000000
#define _DMASTAT_DMACH_MASK                      0x00000007
#define _DMASTAT_DMACH_LENGTH                    0x00000003

#define _DMASTAT_RDWR_POSITION                   0x00000003
#define _DMASTAT_RDWR_MASK                       0x00000008
#define _DMASTAT_RDWR_LENGTH                     0x00000001

#define _DMASTAT_w_POSITION                      0x00000000
#define _DMASTAT_w_MASK                          0xFFFFFFFF
#define _DMASTAT_w_LENGTH                        0x00000020

#define _DCRCCON_CRCCH_POSITION                  0x00000000
#define _DCRCCON_CRCCH_MASK                      0x00000007
#define _DCRCCON_CRCCH_LENGTH                    0x00000003

#define _DCRCCON_CRCTYP_POSITION                 0x00000005
#define _DCRCCON_CRCTYP_MASK                     0x00000020
#define _DCRCCON_CRCTYP_LENGTH                   0x00000001

#define _DCRCCON_CRCAPP_POSITION                 0x00000006
#define _DCRCCON_CRCAPP_MASK                     0x00000040
#define _DCRCCON_CRCAPP_LENGTH                   0x00000001

#define _DCRCCON_CRCEN_POSITION                  0x00000007
#define _DCRCCON_CRCEN_MASK                      0x00000080
#define _DCRCCON_CRCEN_LENGTH                    0x00000001

#define _DCRCCON_PLEN_POSITION                   0x00000008
#define _DCRCCON_PLEN_MASK                       0x00001F00
#define _DCRCCON_PLEN_LENGTH                     0x00000005

#define _DCRCCON_BITO_POSITION                   0x00000018
#define _DCRCCON_BITO_MASK                       0x01000000
#define _DCRCCON_BITO_LENGTH                     0x00000001

#define _DCRCCON_WBO_POSITION                    0x0000001B
#define _DCRCCON_WBO_MASK                        0x08000000
#define _DCRCCON_WBO_LENGTH                      0x00000001

#define _DCRCCON_BYTO_POSITION                   0x0000001C
#define _DCRCCON_BYTO_MASK                       0x30000000
#define _DCRCCON_BYTO_LENGTH                     0x00000002

#define _DCRCCON_w_POSITION                      0x00000000
#define _DCRCCON_w_MASK                          0xFFFFFFFF
#define _DCRCCON_w_LENGTH                        0x00000020

#define _DCH0CON_CHPRI_POSITION                  0x00000000
#define _DCH0CON_CHPRI_MASK                      0x00000003
#define _DCH0CON_CHPRI_LENGTH                    0x00000002

#define _DCH0CON_CHEDET_POSITION                 0x00000002
#define _DCH0CON_CHEDET_MASK                     0x00000004
#define _DCH0CON_CHEDET_LENGTH                   0x00000001

#define _DCH0CON_CHAEN_POSITION                  0x00000004
#define _DCH0CON_CHAEN_MASK                      0x00000010
#define _DCH0CON_CHAEN_LENGTH                    0x00000001

#define _DCH0CON_CHCHN_POSITION                  0x00000005
#define _DCH0CON_CHCHN_MASK                      0x00000020
#define _DCH0CON_CHCHN_LENGTH                    0x00000001

#define _DCH0CON_CHAED_POSITION                  0x00000006
#define _DCH0CON_CHAED_MASK                      0x00000040
#define _DCH0CON_CHAED_LENGTH                    0x00000001

#define _DCH0CON_CHEN_POSITION                   0x00000007
#define _DCH0CON_CHEN_MASK                       0x00000080
#define _DCH0CON_CHEN_LENGTH                     0x00000001

#define _DCH0CON_CHCHNS_POSITION                 0x00000008
#define _DCH0CON_CHCHNS_MASK                     0x00000100
#define _DCH0CON_CHCHNS_LENGTH                   0x00000001

#define _DCH0CON_CHBUSY_POSITION                 0x0000000F
#define _DCH0CON_CHBUSY_MASK                     0x00008000
#define _DCH0CON_CHBUSY_LENGTH                   0x00000001

#define _DCH0CON_w_POSITION                      0x00000000
#define _DCH0CON_w_MASK                          0xFFFFFFFF
#define _DCH0CON_w_LENGTH                        0x00000020

#define _DCH0ECON_AIRQEN_POSITION                0x00000003
#define _DCH0ECON_AIRQEN_MASK                    0x00000008
#define _DCH0ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH0ECON_SIRQEN_POSITION                0x00000004
#define _DCH0ECON_SIRQEN_MASK                    0x00000010
#define _DCH0ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH0ECON_PATEN_POSITION                 0x00000005
#define _DCH0ECON_PATEN_MASK                     0x00000020
#define _DCH0ECON_PATEN_LENGTH                   0x00000001

#define _DCH0ECON_CABORT_POSITION                0x00000006
#define _DCH0ECON_CABORT_MASK                    0x00000040
#define _DCH0ECON_CABORT_LENGTH                  0x00000001

#define _DCH0ECON_CFORCE_POSITION                0x00000007
#define _DCH0ECON_CFORCE_MASK                    0x00000080
#define _DCH0ECON_CFORCE_LENGTH                  0x00000001

#define _DCH0ECON_CHSIRQ_POSITION                0x00000008
#define _DCH0ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH0ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH0ECON_CHAIRQ_POSITION                0x00000010
#define _DCH0ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH0ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH0ECON_w_POSITION                     0x00000000
#define _DCH0ECON_w_MASK                         0xFFFFFFFF
#define _DCH0ECON_w_LENGTH                       0x00000020

#define _DCH0INT_CHERIF_POSITION                 0x00000000
#define _DCH0INT_CHERIF_MASK                     0x00000001
#define _DCH0INT_CHERIF_LENGTH                   0x00000001

#define _DCH0INT_CHTAIF_POSITION                 0x00000001
#define _DCH0INT_CHTAIF_MASK                     0x00000002
#define _DCH0INT_CHTAIF_LENGTH                   0x00000001

#define _DCH0INT_CHCCIF_POSITION                 0x00000002
#define _DCH0INT_CHCCIF_MASK                     0x00000004
#define _DCH0INT_CHCCIF_LENGTH                   0x00000001

#define _DCH0INT_CHBCIF_POSITION                 0x00000003
#define _DCH0INT_CHBCIF_MASK                     0x00000008
#define _DCH0INT_CHBCIF_LENGTH                   0x00000001

#define _DCH0INT_CHDHIF_POSITION                 0x00000004
#define _DCH0INT_CHDHIF_MASK                     0x00000010
#define _DCH0INT_CHDHIF_LENGTH                   0x00000001

#define _DCH0INT_CHDDIF_POSITION                 0x00000005
#define _DCH0INT_CHDDIF_MASK                     0x00000020
#define _DCH0INT_CHDDIF_LENGTH                   0x00000001

#define _DCH0INT_CHSHIF_POSITION                 0x00000006
#define _DCH0INT_CHSHIF_MASK                     0x00000040
#define _DCH0INT_CHSHIF_LENGTH                   0x00000001

#define _DCH0INT_CHSDIF_POSITION                 0x00000007
#define _DCH0INT_CHSDIF_MASK                     0x00000080
#define _DCH0INT_CHSDIF_LENGTH                   0x00000001

#define _DCH0INT_CHERIE_POSITION                 0x00000010
#define _DCH0INT_CHERIE_MASK                     0x00010000
#define _DCH0INT_CHERIE_LENGTH                   0x00000001

#define _DCH0INT_CHTAIE_POSITION                 0x00000011
#define _DCH0INT_CHTAIE_MASK                     0x00020000
#define _DCH0INT_CHTAIE_LENGTH                   0x00000001

#define _DCH0INT_CHCCIE_POSITION                 0x00000012
#define _DCH0INT_CHCCIE_MASK                     0x00040000
#define _DCH0INT_CHCCIE_LENGTH                   0x00000001

#define _DCH0INT_CHBCIE_POSITION                 0x00000013
#define _DCH0INT_CHBCIE_MASK                     0x00080000
#define _DCH0INT_CHBCIE_LENGTH                   0x00000001

#define _DCH0INT_CHDHIE_POSITION                 0x00000014
#define _DCH0INT_CHDHIE_MASK                     0x00100000
#define _DCH0INT_CHDHIE_LENGTH                   0x00000001

#define _DCH0INT_CHDDIE_POSITION                 0x00000015
#define _DCH0INT_CHDDIE_MASK                     0x00200000
#define _DCH0INT_CHDDIE_LENGTH                   0x00000001

#define _DCH0INT_CHSHIE_POSITION                 0x00000016
#define _DCH0INT_CHSHIE_MASK                     0x00400000
#define _DCH0INT_CHSHIE_LENGTH                   0x00000001

#define _DCH0INT_CHSDIE_POSITION                 0x00000017
#define _DCH0INT_CHSDIE_MASK                     0x00800000
#define _DCH0INT_CHSDIE_LENGTH                   0x00000001

#define _DCH0INT_w_POSITION                      0x00000000
#define _DCH0INT_w_MASK                          0xFFFFFFFF
#define _DCH0INT_w_LENGTH                        0x00000020

#define _DCH1CON_CHPRI_POSITION                  0x00000000
#define _DCH1CON_CHPRI_MASK                      0x00000003
#define _DCH1CON_CHPRI_LENGTH                    0x00000002

#define _DCH1CON_CHEDET_POSITION                 0x00000002
#define _DCH1CON_CHEDET_MASK                     0x00000004
#define _DCH1CON_CHEDET_LENGTH                   0x00000001

#define _DCH1CON_CHAEN_POSITION                  0x00000004
#define _DCH1CON_CHAEN_MASK                      0x00000010
#define _DCH1CON_CHAEN_LENGTH                    0x00000001

#define _DCH1CON_CHCHN_POSITION                  0x00000005
#define _DCH1CON_CHCHN_MASK                      0x00000020
#define _DCH1CON_CHCHN_LENGTH                    0x00000001

#define _DCH1CON_CHAED_POSITION                  0x00000006
#define _DCH1CON_CHAED_MASK                      0x00000040
#define _DCH1CON_CHAED_LENGTH                    0x00000001

#define _DCH1CON_CHEN_POSITION                   0x00000007
#define _DCH1CON_CHEN_MASK                       0x00000080
#define _DCH1CON_CHEN_LENGTH                     0x00000001

#define _DCH1CON_CHCHNS_POSITION                 0x00000008
#define _DCH1CON_CHCHNS_MASK                     0x00000100
#define _DCH1CON_CHCHNS_LENGTH                   0x00000001

#define _DCH1CON_CHBUSY_POSITION                 0x0000000F
#define _DCH1CON_CHBUSY_MASK                     0x00008000
#define _DCH1CON_CHBUSY_LENGTH                   0x00000001

#define _DCH1CON_w_POSITION                      0x00000000
#define _DCH1CON_w_MASK                          0xFFFFFFFF
#define _DCH1CON_w_LENGTH                        0x00000020

#define _DCH1ECON_AIRQEN_POSITION                0x00000003
#define _DCH1ECON_AIRQEN_MASK                    0x00000008
#define _DCH1ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH1ECON_SIRQEN_POSITION                0x00000004
#define _DCH1ECON_SIRQEN_MASK                    0x00000010
#define _DCH1ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH1ECON_PATEN_POSITION                 0x00000005
#define _DCH1ECON_PATEN_MASK                     0x00000020
#define _DCH1ECON_PATEN_LENGTH                   0x00000001

#define _DCH1ECON_CABORT_POSITION                0x00000006
#define _DCH1ECON_CABORT_MASK                    0x00000040
#define _DCH1ECON_CABORT_LENGTH                  0x00000001

#define _DCH1ECON_CFORCE_POSITION                0x00000007
#define _DCH1ECON_CFORCE_MASK                    0x00000080
#define _DCH1ECON_CFORCE_LENGTH                  0x00000001

#define _DCH1ECON_CHSIRQ_POSITION                0x00000008
#define _DCH1ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH1ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH1ECON_CHAIRQ_POSITION                0x00000010
#define _DCH1ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH1ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH1ECON_w_POSITION                     0x00000000
#define _DCH1ECON_w_MASK                         0xFFFFFFFF
#define _DCH1ECON_w_LENGTH                       0x00000020

#define _DCH1INT_CHERIF_POSITION                 0x00000000
#define _DCH1INT_CHERIF_MASK                     0x00000001
#define _DCH1INT_CHERIF_LENGTH                   0x00000001

#define _DCH1INT_CHTAIF_POSITION                 0x00000001
#define _DCH1INT_CHTAIF_MASK                     0x00000002
#define _DCH1INT_CHTAIF_LENGTH                   0x00000001

#define _DCH1INT_CHCCIF_POSITION                 0x00000002
#define _DCH1INT_CHCCIF_MASK                     0x00000004
#define _DCH1INT_CHCCIF_LENGTH                   0x00000001

#define _DCH1INT_CHBCIF_POSITION                 0x00000003
#define _DCH1INT_CHBCIF_MASK                     0x00000008
#define _DCH1INT_CHBCIF_LENGTH                   0x00000001

#define _DCH1INT_CHDHIF_POSITION                 0x00000004
#define _DCH1INT_CHDHIF_MASK                     0x00000010
#define _DCH1INT_CHDHIF_LENGTH                   0x00000001

#define _DCH1INT_CHDDIF_POSITION                 0x00000005
#define _DCH1INT_CHDDIF_MASK                     0x00000020
#define _DCH1INT_CHDDIF_LENGTH                   0x00000001

#define _DCH1INT_CHSHIF_POSITION                 0x00000006
#define _DCH1INT_CHSHIF_MASK                     0x00000040
#define _DCH1INT_CHSHIF_LENGTH                   0x00000001

#define _DCH1INT_CHSDIF_POSITION                 0x00000007
#define _DCH1INT_CHSDIF_MASK                     0x00000080
#define _DCH1INT_CHSDIF_LENGTH                   0x00000001

#define _DCH1INT_CHERIE_POSITION                 0x00000010
#define _DCH1INT_CHERIE_MASK                     0x00010000
#define _DCH1INT_CHERIE_LENGTH                   0x00000001

#define _DCH1INT_CHTAIE_POSITION                 0x00000011
#define _DCH1INT_CHTAIE_MASK                     0x00020000
#define _DCH1INT_CHTAIE_LENGTH                   0x00000001

#define _DCH1INT_CHCCIE_POSITION                 0x00000012
#define _DCH1INT_CHCCIE_MASK                     0x00040000
#define _DCH1INT_CHCCIE_LENGTH                   0x00000001

#define _DCH1INT_CHBCIE_POSITION                 0x00000013
#define _DCH1INT_CHBCIE_MASK                     0x00080000
#define _DCH1INT_CHBCIE_LENGTH                   0x00000001

#define _DCH1INT_CHDHIE_POSITION                 0x00000014
#define _DCH1INT_CHDHIE_MASK                     0x00100000
#define _DCH1INT_CHDHIE_LENGTH                   0x00000001

#define _DCH1INT_CHDDIE_POSITION                 0x00000015
#define _DCH1INT_CHDDIE_MASK                     0x00200000
#define _DCH1INT_CHDDIE_LENGTH                   0x00000001

#define _DCH1INT_CHSHIE_POSITION                 0x00000016
#define _DCH1INT_CHSHIE_MASK                     0x00400000
#define _DCH1INT_CHSHIE_LENGTH                   0x00000001

#define _DCH1INT_CHSDIE_POSITION                 0x00000017
#define _DCH1INT_CHSDIE_MASK                     0x00800000
#define _DCH1INT_CHSDIE_LENGTH                   0x00000001

#define _DCH1INT_w_POSITION                      0x00000000
#define _DCH1INT_w_MASK                          0xFFFFFFFF
#define _DCH1INT_w_LENGTH                        0x00000020

#define _DCH2CON_CHPRI_POSITION                  0x00000000
#define _DCH2CON_CHPRI_MASK                      0x00000003
#define _DCH2CON_CHPRI_LENGTH                    0x00000002

#define _DCH2CON_CHEDET_POSITION                 0x00000002
#define _DCH2CON_CHEDET_MASK                     0x00000004
#define _DCH2CON_CHEDET_LENGTH                   0x00000001

#define _DCH2CON_CHAEN_POSITION                  0x00000004
#define _DCH2CON_CHAEN_MASK                      0x00000010
#define _DCH2CON_CHAEN_LENGTH                    0x00000001

#define _DCH2CON_CHCHN_POSITION                  0x00000005
#define _DCH2CON_CHCHN_MASK                      0x00000020
#define _DCH2CON_CHCHN_LENGTH                    0x00000001

#define _DCH2CON_CHAED_POSITION                  0x00000006
#define _DCH2CON_CHAED_MASK                      0x00000040
#define _DCH2CON_CHAED_LENGTH                    0x00000001

#define _DCH2CON_CHEN_POSITION                   0x00000007
#define _DCH2CON_CHEN_MASK                       0x00000080
#define _DCH2CON_CHEN_LENGTH                     0x00000001

#define _DCH2CON_CHCHNS_POSITION                 0x00000008
#define _DCH2CON_CHCHNS_MASK                     0x00000100
#define _DCH2CON_CHCHNS_LENGTH                   0x00000001

#define _DCH2CON_CHBUSY_POSITION                 0x0000000F
#define _DCH2CON_CHBUSY_MASK                     0x00008000
#define _DCH2CON_CHBUSY_LENGTH                   0x00000001

#define _DCH2CON_w_POSITION                      0x00000000
#define _DCH2CON_w_MASK                          0xFFFFFFFF
#define _DCH2CON_w_LENGTH                        0x00000020

#define _DCH2ECON_AIRQEN_POSITION                0x00000003
#define _DCH2ECON_AIRQEN_MASK                    0x00000008
#define _DCH2ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH2ECON_SIRQEN_POSITION                0x00000004
#define _DCH2ECON_SIRQEN_MASK                    0x00000010
#define _DCH2ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH2ECON_PATEN_POSITION                 0x00000005
#define _DCH2ECON_PATEN_MASK                     0x00000020
#define _DCH2ECON_PATEN_LENGTH                   0x00000001

#define _DCH2ECON_CABORT_POSITION                0x00000006
#define _DCH2ECON_CABORT_MASK                    0x00000040
#define _DCH2ECON_CABORT_LENGTH                  0x00000001

#define _DCH2ECON_CFORCE_POSITION                0x00000007
#define _DCH2ECON_CFORCE_MASK                    0x00000080
#define _DCH2ECON_CFORCE_LENGTH                  0x00000001

#define _DCH2ECON_CHSIRQ_POSITION                0x00000008
#define _DCH2ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH2ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH2ECON_CHAIRQ_POSITION                0x00000010
#define _DCH2ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH2ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH2ECON_w_POSITION                     0x00000000
#define _DCH2ECON_w_MASK                         0xFFFFFFFF
#define _DCH2ECON_w_LENGTH                       0x00000020

#define _DCH2INT_CHERIF_POSITION                 0x00000000
#define _DCH2INT_CHERIF_MASK                     0x00000001
#define _DCH2INT_CHERIF_LENGTH                   0x00000001

#define _DCH2INT_CHTAIF_POSITION                 0x00000001
#define _DCH2INT_CHTAIF_MASK                     0x00000002
#define _DCH2INT_CHTAIF_LENGTH                   0x00000001

#define _DCH2INT_CHCCIF_POSITION                 0x00000002
#define _DCH2INT_CHCCIF_MASK                     0x00000004
#define _DCH2INT_CHCCIF_LENGTH                   0x00000001

#define _DCH2INT_CHBCIF_POSITION                 0x00000003
#define _DCH2INT_CHBCIF_MASK                     0x00000008
#define _DCH2INT_CHBCIF_LENGTH                   0x00000001

#define _DCH2INT_CHDHIF_POSITION                 0x00000004
#define _DCH2INT_CHDHIF_MASK                     0x00000010
#define _DCH2INT_CHDHIF_LENGTH                   0x00000001

#define _DCH2INT_CHDDIF_POSITION                 0x00000005
#define _DCH2INT_CHDDIF_MASK                     0x00000020
#define _DCH2INT_CHDDIF_LENGTH                   0x00000001

#define _DCH2INT_CHSHIF_POSITION                 0x00000006
#define _DCH2INT_CHSHIF_MASK                     0x00000040
#define _DCH2INT_CHSHIF_LENGTH                   0x00000001

#define _DCH2INT_CHSDIF_POSITION                 0x00000007
#define _DCH2INT_CHSDIF_MASK                     0x00000080
#define _DCH2INT_CHSDIF_LENGTH                   0x00000001

#define _DCH2INT_CHERIE_POSITION                 0x00000010
#define _DCH2INT_CHERIE_MASK                     0x00010000
#define _DCH2INT_CHERIE_LENGTH                   0x00000001

#define _DCH2INT_CHTAIE_POSITION                 0x00000011
#define _DCH2INT_CHTAIE_MASK                     0x00020000
#define _DCH2INT_CHTAIE_LENGTH                   0x00000001

#define _DCH2INT_CHCCIE_POSITION                 0x00000012
#define _DCH2INT_CHCCIE_MASK                     0x00040000
#define _DCH2INT_CHCCIE_LENGTH                   0x00000001

#define _DCH2INT_CHBCIE_POSITION                 0x00000013
#define _DCH2INT_CHBCIE_MASK                     0x00080000
#define _DCH2INT_CHBCIE_LENGTH                   0x00000001

#define _DCH2INT_CHDHIE_POSITION                 0x00000014
#define _DCH2INT_CHDHIE_MASK                     0x00100000
#define _DCH2INT_CHDHIE_LENGTH                   0x00000001

#define _DCH2INT_CHDDIE_POSITION                 0x00000015
#define _DCH2INT_CHDDIE_MASK                     0x00200000
#define _DCH2INT_CHDDIE_LENGTH                   0x00000001

#define _DCH2INT_CHSHIE_POSITION                 0x00000016
#define _DCH2INT_CHSHIE_MASK                     0x00400000
#define _DCH2INT_CHSHIE_LENGTH                   0x00000001

#define _DCH2INT_CHSDIE_POSITION                 0x00000017
#define _DCH2INT_CHSDIE_MASK                     0x00800000
#define _DCH2INT_CHSDIE_LENGTH                   0x00000001

#define _DCH2INT_w_POSITION                      0x00000000
#define _DCH2INT_w_MASK                          0xFFFFFFFF
#define _DCH2INT_w_LENGTH                        0x00000020

#define _DCH3CON_CHPRI_POSITION                  0x00000000
#define _DCH3CON_CHPRI_MASK                      0x00000003
#define _DCH3CON_CHPRI_LENGTH                    0x00000002

#define _DCH3CON_CHEDET_POSITION                 0x00000002
#define _DCH3CON_CHEDET_MASK                     0x00000004
#define _DCH3CON_CHEDET_LENGTH                   0x00000001

#define _DCH3CON_CHAEN_POSITION                  0x00000004
#define _DCH3CON_CHAEN_MASK                      0x00000010
#define _DCH3CON_CHAEN_LENGTH                    0x00000001

#define _DCH3CON_CHCHN_POSITION                  0x00000005
#define _DCH3CON_CHCHN_MASK                      0x00000020
#define _DCH3CON_CHCHN_LENGTH                    0x00000001

#define _DCH3CON_CHAED_POSITION                  0x00000006
#define _DCH3CON_CHAED_MASK                      0x00000040
#define _DCH3CON_CHAED_LENGTH                    0x00000001

#define _DCH3CON_CHEN_POSITION                   0x00000007
#define _DCH3CON_CHEN_MASK                       0x00000080
#define _DCH3CON_CHEN_LENGTH                     0x00000001

#define _DCH3CON_CHCHNS_POSITION                 0x00000008
#define _DCH3CON_CHCHNS_MASK                     0x00000100
#define _DCH3CON_CHCHNS_LENGTH                   0x00000001

#define _DCH3CON_CHBUSY_POSITION                 0x0000000F
#define _DCH3CON_CHBUSY_MASK                     0x00008000
#define _DCH3CON_CHBUSY_LENGTH                   0x00000001

#define _DCH3CON_w_POSITION                      0x00000000
#define _DCH3CON_w_MASK                          0xFFFFFFFF
#define _DCH3CON_w_LENGTH                        0x00000020

#define _DCH3ECON_AIRQEN_POSITION                0x00000003
#define _DCH3ECON_AIRQEN_MASK                    0x00000008
#define _DCH3ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH3ECON_SIRQEN_POSITION                0x00000004
#define _DCH3ECON_SIRQEN_MASK                    0x00000010
#define _DCH3ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH3ECON_PATEN_POSITION                 0x00000005
#define _DCH3ECON_PATEN_MASK                     0x00000020
#define _DCH3ECON_PATEN_LENGTH                   0x00000001

#define _DCH3ECON_CABORT_POSITION                0x00000006
#define _DCH3ECON_CABORT_MASK                    0x00000040
#define _DCH3ECON_CABORT_LENGTH                  0x00000001

#define _DCH3ECON_CFORCE_POSITION                0x00000007
#define _DCH3ECON_CFORCE_MASK                    0x00000080
#define _DCH3ECON_CFORCE_LENGTH                  0x00000001

#define _DCH3ECON_CHSIRQ_POSITION                0x00000008
#define _DCH3ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH3ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH3ECON_CHAIRQ_POSITION                0x00000010
#define _DCH3ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH3ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH3ECON_w_POSITION                     0x00000000
#define _DCH3ECON_w_MASK                         0xFFFFFFFF
#define _DCH3ECON_w_LENGTH                       0x00000020

#define _DCH3INT_CHERIF_POSITION                 0x00000000
#define _DCH3INT_CHERIF_MASK                     0x00000001
#define _DCH3INT_CHERIF_LENGTH                   0x00000001

#define _DCH3INT_CHTAIF_POSITION                 0x00000001
#define _DCH3INT_CHTAIF_MASK                     0x00000002
#define _DCH3INT_CHTAIF_LENGTH                   0x00000001

#define _DCH3INT_CHCCIF_POSITION                 0x00000002
#define _DCH3INT_CHCCIF_MASK                     0x00000004
#define _DCH3INT_CHCCIF_LENGTH                   0x00000001

#define _DCH3INT_CHBCIF_POSITION                 0x00000003
#define _DCH3INT_CHBCIF_MASK                     0x00000008
#define _DCH3INT_CHBCIF_LENGTH                   0x00000001

#define _DCH3INT_CHDHIF_POSITION                 0x00000004
#define _DCH3INT_CHDHIF_MASK                     0x00000010
#define _DCH3INT_CHDHIF_LENGTH                   0x00000001

#define _DCH3INT_CHDDIF_POSITION                 0x00000005
#define _DCH3INT_CHDDIF_MASK                     0x00000020
#define _DCH3INT_CHDDIF_LENGTH                   0x00000001

#define _DCH3INT_CHSHIF_POSITION                 0x00000006
#define _DCH3INT_CHSHIF_MASK                     0x00000040
#define _DCH3INT_CHSHIF_LENGTH                   0x00000001

#define _DCH3INT_CHSDIF_POSITION                 0x00000007
#define _DCH3INT_CHSDIF_MASK                     0x00000080
#define _DCH3INT_CHSDIF_LENGTH                   0x00000001

#define _DCH3INT_CHERIE_POSITION                 0x00000010
#define _DCH3INT_CHERIE_MASK                     0x00010000
#define _DCH3INT_CHERIE_LENGTH                   0x00000001

#define _DCH3INT_CHTAIE_POSITION                 0x00000011
#define _DCH3INT_CHTAIE_MASK                     0x00020000
#define _DCH3INT_CHTAIE_LENGTH                   0x00000001

#define _DCH3INT_CHCCIE_POSITION                 0x00000012
#define _DCH3INT_CHCCIE_MASK                     0x00040000
#define _DCH3INT_CHCCIE_LENGTH                   0x00000001

#define _DCH3INT_CHBCIE_POSITION                 0x00000013
#define _DCH3INT_CHBCIE_MASK                     0x00080000
#define _DCH3INT_CHBCIE_LENGTH                   0x00000001

#define _DCH3INT_CHDHIE_POSITION                 0x00000014
#define _DCH3INT_CHDHIE_MASK                     0x00100000
#define _DCH3INT_CHDHIE_LENGTH                   0x00000001

#define _DCH3INT_CHDDIE_POSITION                 0x00000015
#define _DCH3INT_CHDDIE_MASK                     0x00200000
#define _DCH3INT_CHDDIE_LENGTH                   0x00000001

#define _DCH3INT_CHSHIE_POSITION                 0x00000016
#define _DCH3INT_CHSHIE_MASK                     0x00400000
#define _DCH3INT_CHSHIE_LENGTH                   0x00000001

#define _DCH3INT_CHSDIE_POSITION                 0x00000017
#define _DCH3INT_CHSDIE_MASK                     0x00800000
#define _DCH3INT_CHSDIE_LENGTH                   0x00000001

#define _DCH3INT_w_POSITION                      0x00000000
#define _DCH3INT_w_MASK                          0xFFFFFFFF
#define _DCH3INT_w_LENGTH                        0x00000020

#define _DCH4CON_CHPRI_POSITION                  0x00000000
#define _DCH4CON_CHPRI_MASK                      0x00000003
#define _DCH4CON_CHPRI_LENGTH                    0x00000002

#define _DCH4CON_CHEDET_POSITION                 0x00000002
#define _DCH4CON_CHEDET_MASK                     0x00000004
#define _DCH4CON_CHEDET_LENGTH                   0x00000001

#define _DCH4CON_CHAEN_POSITION                  0x00000004
#define _DCH4CON_CHAEN_MASK                      0x00000010
#define _DCH4CON_CHAEN_LENGTH                    0x00000001

#define _DCH4CON_CHCHN_POSITION                  0x00000005
#define _DCH4CON_CHCHN_MASK                      0x00000020
#define _DCH4CON_CHCHN_LENGTH                    0x00000001

#define _DCH4CON_CHAED_POSITION                  0x00000006
#define _DCH4CON_CHAED_MASK                      0x00000040
#define _DCH4CON_CHAED_LENGTH                    0x00000001

#define _DCH4CON_CHEN_POSITION                   0x00000007
#define _DCH4CON_CHEN_MASK                       0x00000080
#define _DCH4CON_CHEN_LENGTH                     0x00000001

#define _DCH4CON_CHCHNS_POSITION                 0x00000008
#define _DCH4CON_CHCHNS_MASK                     0x00000100
#define _DCH4CON_CHCHNS_LENGTH                   0x00000001

#define _DCH4CON_CHBUSY_POSITION                 0x0000000F
#define _DCH4CON_CHBUSY_MASK                     0x00008000
#define _DCH4CON_CHBUSY_LENGTH                   0x00000001

#define _DCH4CON_w_POSITION                      0x00000000
#define _DCH4CON_w_MASK                          0xFFFFFFFF
#define _DCH4CON_w_LENGTH                        0x00000020

#define _DCH4ECON_AIRQEN_POSITION                0x00000003
#define _DCH4ECON_AIRQEN_MASK                    0x00000008
#define _DCH4ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH4ECON_SIRQEN_POSITION                0x00000004
#define _DCH4ECON_SIRQEN_MASK                    0x00000010
#define _DCH4ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH4ECON_PATEN_POSITION                 0x00000005
#define _DCH4ECON_PATEN_MASK                     0x00000020
#define _DCH4ECON_PATEN_LENGTH                   0x00000001

#define _DCH4ECON_CABORT_POSITION                0x00000006
#define _DCH4ECON_CABORT_MASK                    0x00000040
#define _DCH4ECON_CABORT_LENGTH                  0x00000001

#define _DCH4ECON_CFORCE_POSITION                0x00000007
#define _DCH4ECON_CFORCE_MASK                    0x00000080
#define _DCH4ECON_CFORCE_LENGTH                  0x00000001

#define _DCH4ECON_CHSIRQ_POSITION                0x00000008
#define _DCH4ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH4ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH4ECON_CHAIRQ_POSITION                0x00000010
#define _DCH4ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH4ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH4ECON_w_POSITION                     0x00000000
#define _DCH4ECON_w_MASK                         0xFFFFFFFF
#define _DCH4ECON_w_LENGTH                       0x00000020

#define _DCH4INT_CHERIF_POSITION                 0x00000000
#define _DCH4INT_CHERIF_MASK                     0x00000001
#define _DCH4INT_CHERIF_LENGTH                   0x00000001

#define _DCH4INT_CHTAIF_POSITION                 0x00000001
#define _DCH4INT_CHTAIF_MASK                     0x00000002
#define _DCH4INT_CHTAIF_LENGTH                   0x00000001

#define _DCH4INT_CHCCIF_POSITION                 0x00000002
#define _DCH4INT_CHCCIF_MASK                     0x00000004
#define _DCH4INT_CHCCIF_LENGTH                   0x00000001

#define _DCH4INT_CHBCIF_POSITION                 0x00000003
#define _DCH4INT_CHBCIF_MASK                     0x00000008
#define _DCH4INT_CHBCIF_LENGTH                   0x00000001

#define _DCH4INT_CHDHIF_POSITION                 0x00000004
#define _DCH4INT_CHDHIF_MASK                     0x00000010
#define _DCH4INT_CHDHIF_LENGTH                   0x00000001

#define _DCH4INT_CHDDIF_POSITION                 0x00000005
#define _DCH4INT_CHDDIF_MASK                     0x00000020
#define _DCH4INT_CHDDIF_LENGTH                   0x00000001

#define _DCH4INT_CHSHIF_POSITION                 0x00000006
#define _DCH4INT_CHSHIF_MASK                     0x00000040
#define _DCH4INT_CHSHIF_LENGTH                   0x00000001

#define _DCH4INT_CHSDIF_POSITION                 0x00000007
#define _DCH4INT_CHSDIF_MASK                     0x00000080
#define _DCH4INT_CHSDIF_LENGTH                   0x00000001

#define _DCH4INT_CHERIE_POSITION                 0x00000010
#define _DCH4INT_CHERIE_MASK                     0x00010000
#define _DCH4INT_CHERIE_LENGTH                   0x00000001

#define _DCH4INT_CHTAIE_POSITION                 0x00000011
#define _DCH4INT_CHTAIE_MASK                     0x00020000
#define _DCH4INT_CHTAIE_LENGTH                   0x00000001

#define _DCH4INT_CHCCIE_POSITION                 0x00000012
#define _DCH4INT_CHCCIE_MASK                     0x00040000
#define _DCH4INT_CHCCIE_LENGTH                   0x00000001

#define _DCH4INT_CHBCIE_POSITION                 0x00000013
#define _DCH4INT_CHBCIE_MASK                     0x00080000
#define _DCH4INT_CHBCIE_LENGTH                   0x00000001

#define _DCH4INT_CHDHIE_POSITION                 0x00000014
#define _DCH4INT_CHDHIE_MASK                     0x00100000
#define _DCH4INT_CHDHIE_LENGTH                   0x00000001

#define _DCH4INT_CHDDIE_POSITION                 0x00000015
#define _DCH4INT_CHDDIE_MASK                     0x00200000
#define _DCH4INT_CHDDIE_LENGTH                   0x00000001

#define _DCH4INT_CHSHIE_POSITION                 0x00000016
#define _DCH4INT_CHSHIE_MASK                     0x00400000
#define _DCH4INT_CHSHIE_LENGTH                   0x00000001

#define _DCH4INT_CHSDIE_POSITION                 0x00000017
#define _DCH4INT_CHSDIE_MASK                     0x00800000
#define _DCH4INT_CHSDIE_LENGTH                   0x00000001

#define _DCH4INT_w_POSITION                      0x00000000
#define _DCH4INT_w_MASK                          0xFFFFFFFF
#define _DCH4INT_w_LENGTH                        0x00000020

#define _DCH5CON_CHPRI_POSITION                  0x00000000
#define _DCH5CON_CHPRI_MASK                      0x00000003
#define _DCH5CON_CHPRI_LENGTH                    0x00000002

#define _DCH5CON_CHEDET_POSITION                 0x00000002
#define _DCH5CON_CHEDET_MASK                     0x00000004
#define _DCH5CON_CHEDET_LENGTH                   0x00000001

#define _DCH5CON_CHAEN_POSITION                  0x00000004
#define _DCH5CON_CHAEN_MASK                      0x00000010
#define _DCH5CON_CHAEN_LENGTH                    0x00000001

#define _DCH5CON_CHCHN_POSITION                  0x00000005
#define _DCH5CON_CHCHN_MASK                      0x00000020
#define _DCH5CON_CHCHN_LENGTH                    0x00000001

#define _DCH5CON_CHAED_POSITION                  0x00000006
#define _DCH5CON_CHAED_MASK                      0x00000040
#define _DCH5CON_CHAED_LENGTH                    0x00000001

#define _DCH5CON_CHEN_POSITION                   0x00000007
#define _DCH5CON_CHEN_MASK                       0x00000080
#define _DCH5CON_CHEN_LENGTH                     0x00000001

#define _DCH5CON_CHCHNS_POSITION                 0x00000008
#define _DCH5CON_CHCHNS_MASK                     0x00000100
#define _DCH5CON_CHCHNS_LENGTH                   0x00000001

#define _DCH5CON_CHBUSY_POSITION                 0x0000000F
#define _DCH5CON_CHBUSY_MASK                     0x00008000
#define _DCH5CON_CHBUSY_LENGTH                   0x00000001

#define _DCH5CON_w_POSITION                      0x00000000
#define _DCH5CON_w_MASK                          0xFFFFFFFF
#define _DCH5CON_w_LENGTH                        0x00000020

#define _DCH5ECON_AIRQEN_POSITION                0x00000003
#define _DCH5ECON_AIRQEN_MASK                    0x00000008
#define _DCH5ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH5ECON_SIRQEN_POSITION                0x00000004
#define _DCH5ECON_SIRQEN_MASK                    0x00000010
#define _DCH5ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH5ECON_PATEN_POSITION                 0x00000005
#define _DCH5ECON_PATEN_MASK                     0x00000020
#define _DCH5ECON_PATEN_LENGTH                   0x00000001

#define _DCH5ECON_CABORT_POSITION                0x00000006
#define _DCH5ECON_CABORT_MASK                    0x00000040
#define _DCH5ECON_CABORT_LENGTH                  0x00000001

#define _DCH5ECON_CFORCE_POSITION                0x00000007
#define _DCH5ECON_CFORCE_MASK                    0x00000080
#define _DCH5ECON_CFORCE_LENGTH                  0x00000001

#define _DCH5ECON_CHSIRQ_POSITION                0x00000008
#define _DCH5ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH5ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH5ECON_CHAIRQ_POSITION                0x00000010
#define _DCH5ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH5ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH5ECON_w_POSITION                     0x00000000
#define _DCH5ECON_w_MASK                         0xFFFFFFFF
#define _DCH5ECON_w_LENGTH                       0x00000020

#define _DCH5INT_CHERIF_POSITION                 0x00000000
#define _DCH5INT_CHERIF_MASK                     0x00000001
#define _DCH5INT_CHERIF_LENGTH                   0x00000001

#define _DCH5INT_CHTAIF_POSITION                 0x00000001
#define _DCH5INT_CHTAIF_MASK                     0x00000002
#define _DCH5INT_CHTAIF_LENGTH                   0x00000001

#define _DCH5INT_CHCCIF_POSITION                 0x00000002
#define _DCH5INT_CHCCIF_MASK                     0x00000004
#define _DCH5INT_CHCCIF_LENGTH                   0x00000001

#define _DCH5INT_CHBCIF_POSITION                 0x00000003
#define _DCH5INT_CHBCIF_MASK                     0x00000008
#define _DCH5INT_CHBCIF_LENGTH                   0x00000001

#define _DCH5INT_CHDHIF_POSITION                 0x00000004
#define _DCH5INT_CHDHIF_MASK                     0x00000010
#define _DCH5INT_CHDHIF_LENGTH                   0x00000001

#define _DCH5INT_CHDDIF_POSITION                 0x00000005
#define _DCH5INT_CHDDIF_MASK                     0x00000020
#define _DCH5INT_CHDDIF_LENGTH                   0x00000001

#define _DCH5INT_CHSHIF_POSITION                 0x00000006
#define _DCH5INT_CHSHIF_MASK                     0x00000040
#define _DCH5INT_CHSHIF_LENGTH                   0x00000001

#define _DCH5INT_CHSDIF_POSITION                 0x00000007
#define _DCH5INT_CHSDIF_MASK                     0x00000080
#define _DCH5INT_CHSDIF_LENGTH                   0x00000001

#define _DCH5INT_CHERIE_POSITION                 0x00000010
#define _DCH5INT_CHERIE_MASK                     0x00010000
#define _DCH5INT_CHERIE_LENGTH                   0x00000001

#define _DCH5INT_CHTAIE_POSITION                 0x00000011
#define _DCH5INT_CHTAIE_MASK                     0x00020000
#define _DCH5INT_CHTAIE_LENGTH                   0x00000001

#define _DCH5INT_CHCCIE_POSITION                 0x00000012
#define _DCH5INT_CHCCIE_MASK                     0x00040000
#define _DCH5INT_CHCCIE_LENGTH                   0x00000001

#define _DCH5INT_CHBCIE_POSITION                 0x00000013
#define _DCH5INT_CHBCIE_MASK                     0x00080000
#define _DCH5INT_CHBCIE_LENGTH                   0x00000001

#define _DCH5INT_CHDHIE_POSITION                 0x00000014
#define _DCH5INT_CHDHIE_MASK                     0x00100000
#define _DCH5INT_CHDHIE_LENGTH                   0x00000001

#define _DCH5INT_CHDDIE_POSITION                 0x00000015
#define _DCH5INT_CHDDIE_MASK                     0x00200000
#define _DCH5INT_CHDDIE_LENGTH                   0x00000001

#define _DCH5INT_CHSHIE_POSITION                 0x00000016
#define _DCH5INT_CHSHIE_MASK                     0x00400000
#define _DCH5INT_CHSHIE_LENGTH                   0x00000001

#define _DCH5INT_CHSDIE_POSITION                 0x00000017
#define _DCH5INT_CHSDIE_MASK                     0x00800000
#define _DCH5INT_CHSDIE_LENGTH                   0x00000001

#define _DCH5INT_w_POSITION                      0x00000000
#define _DCH5INT_w_MASK                          0xFFFFFFFF
#define _DCH5INT_w_LENGTH                        0x00000020

#define _DCH6CON_CHPRI_POSITION                  0x00000000
#define _DCH6CON_CHPRI_MASK                      0x00000003
#define _DCH6CON_CHPRI_LENGTH                    0x00000002

#define _DCH6CON_CHEDET_POSITION                 0x00000002
#define _DCH6CON_CHEDET_MASK                     0x00000004
#define _DCH6CON_CHEDET_LENGTH                   0x00000001

#define _DCH6CON_CHAEN_POSITION                  0x00000004
#define _DCH6CON_CHAEN_MASK                      0x00000010
#define _DCH6CON_CHAEN_LENGTH                    0x00000001

#define _DCH6CON_CHCHN_POSITION                  0x00000005
#define _DCH6CON_CHCHN_MASK                      0x00000020
#define _DCH6CON_CHCHN_LENGTH                    0x00000001

#define _DCH6CON_CHAED_POSITION                  0x00000006
#define _DCH6CON_CHAED_MASK                      0x00000040
#define _DCH6CON_CHAED_LENGTH                    0x00000001

#define _DCH6CON_CHEN_POSITION                   0x00000007
#define _DCH6CON_CHEN_MASK                       0x00000080
#define _DCH6CON_CHEN_LENGTH                     0x00000001

#define _DCH6CON_CHCHNS_POSITION                 0x00000008
#define _DCH6CON_CHCHNS_MASK                     0x00000100
#define _DCH6CON_CHCHNS_LENGTH                   0x00000001

#define _DCH6CON_CHBUSY_POSITION                 0x0000000F
#define _DCH6CON_CHBUSY_MASK                     0x00008000
#define _DCH6CON_CHBUSY_LENGTH                   0x00000001

#define _DCH6CON_w_POSITION                      0x00000000
#define _DCH6CON_w_MASK                          0xFFFFFFFF
#define _DCH6CON_w_LENGTH                        0x00000020

#define _DCH6ECON_AIRQEN_POSITION                0x00000003
#define _DCH6ECON_AIRQEN_MASK                    0x00000008
#define _DCH6ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH6ECON_SIRQEN_POSITION                0x00000004
#define _DCH6ECON_SIRQEN_MASK                    0x00000010
#define _DCH6ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH6ECON_PATEN_POSITION                 0x00000005
#define _DCH6ECON_PATEN_MASK                     0x00000020
#define _DCH6ECON_PATEN_LENGTH                   0x00000001

#define _DCH6ECON_CABORT_POSITION                0x00000006
#define _DCH6ECON_CABORT_MASK                    0x00000040
#define _DCH6ECON_CABORT_LENGTH                  0x00000001

#define _DCH6ECON_CFORCE_POSITION                0x00000007
#define _DCH6ECON_CFORCE_MASK                    0x00000080
#define _DCH6ECON_CFORCE_LENGTH                  0x00000001

#define _DCH6ECON_CHSIRQ_POSITION                0x00000008
#define _DCH6ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH6ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH6ECON_CHAIRQ_POSITION                0x00000010
#define _DCH6ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH6ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH6ECON_w_POSITION                     0x00000000
#define _DCH6ECON_w_MASK                         0xFFFFFFFF
#define _DCH6ECON_w_LENGTH                       0x00000020

#define _DCH6INT_CHERIF_POSITION                 0x00000000
#define _DCH6INT_CHERIF_MASK                     0x00000001
#define _DCH6INT_CHERIF_LENGTH                   0x00000001

#define _DCH6INT_CHTAIF_POSITION                 0x00000001
#define _DCH6INT_CHTAIF_MASK                     0x00000002
#define _DCH6INT_CHTAIF_LENGTH                   0x00000001

#define _DCH6INT_CHCCIF_POSITION                 0x00000002
#define _DCH6INT_CHCCIF_MASK                     0x00000004
#define _DCH6INT_CHCCIF_LENGTH                   0x00000001

#define _DCH6INT_CHBCIF_POSITION                 0x00000003
#define _DCH6INT_CHBCIF_MASK                     0x00000008
#define _DCH6INT_CHBCIF_LENGTH                   0x00000001

#define _DCH6INT_CHDHIF_POSITION                 0x00000004
#define _DCH6INT_CHDHIF_MASK                     0x00000010
#define _DCH6INT_CHDHIF_LENGTH                   0x00000001

#define _DCH6INT_CHDDIF_POSITION                 0x00000005
#define _DCH6INT_CHDDIF_MASK                     0x00000020
#define _DCH6INT_CHDDIF_LENGTH                   0x00000001

#define _DCH6INT_CHSHIF_POSITION                 0x00000006
#define _DCH6INT_CHSHIF_MASK                     0x00000040
#define _DCH6INT_CHSHIF_LENGTH                   0x00000001

#define _DCH6INT_CHSDIF_POSITION                 0x00000007
#define _DCH6INT_CHSDIF_MASK                     0x00000080
#define _DCH6INT_CHSDIF_LENGTH                   0x00000001

#define _DCH6INT_CHERIE_POSITION                 0x00000010
#define _DCH6INT_CHERIE_MASK                     0x00010000
#define _DCH6INT_CHERIE_LENGTH                   0x00000001

#define _DCH6INT_CHTAIE_POSITION                 0x00000011
#define _DCH6INT_CHTAIE_MASK                     0x00020000
#define _DCH6INT_CHTAIE_LENGTH                   0x00000001

#define _DCH6INT_CHCCIE_POSITION                 0x00000012
#define _DCH6INT_CHCCIE_MASK                     0x00040000
#define _DCH6INT_CHCCIE_LENGTH                   0x00000001

#define _DCH6INT_CHBCIE_POSITION                 0x00000013
#define _DCH6INT_CHBCIE_MASK                     0x00080000
#define _DCH6INT_CHBCIE_LENGTH                   0x00000001

#define _DCH6INT_CHDHIE_POSITION                 0x00000014
#define _DCH6INT_CHDHIE_MASK                     0x00100000
#define _DCH6INT_CHDHIE_LENGTH                   0x00000001

#define _DCH6INT_CHDDIE_POSITION                 0x00000015
#define _DCH6INT_CHDDIE_MASK                     0x00200000
#define _DCH6INT_CHDDIE_LENGTH                   0x00000001

#define _DCH6INT_CHSHIE_POSITION                 0x00000016
#define _DCH6INT_CHSHIE_MASK                     0x00400000
#define _DCH6INT_CHSHIE_LENGTH                   0x00000001

#define _DCH6INT_CHSDIE_POSITION                 0x00000017
#define _DCH6INT_CHSDIE_MASK                     0x00800000
#define _DCH6INT_CHSDIE_LENGTH                   0x00000001

#define _DCH6INT_w_POSITION                      0x00000000
#define _DCH6INT_w_MASK                          0xFFFFFFFF
#define _DCH6INT_w_LENGTH                        0x00000020

#define _DCH7CON_CHPRI_POSITION                  0x00000000
#define _DCH7CON_CHPRI_MASK                      0x00000003
#define _DCH7CON_CHPRI_LENGTH                    0x00000002

#define _DCH7CON_CHEDET_POSITION                 0x00000002
#define _DCH7CON_CHEDET_MASK                     0x00000004
#define _DCH7CON_CHEDET_LENGTH                   0x00000001

#define _DCH7CON_CHAEN_POSITION                  0x00000004
#define _DCH7CON_CHAEN_MASK                      0x00000010
#define _DCH7CON_CHAEN_LENGTH                    0x00000001

#define _DCH7CON_CHCHN_POSITION                  0x00000005
#define _DCH7CON_CHCHN_MASK                      0x00000020
#define _DCH7CON_CHCHN_LENGTH                    0x00000001

#define _DCH7CON_CHAED_POSITION                  0x00000006
#define _DCH7CON_CHAED_MASK                      0x00000040
#define _DCH7CON_CHAED_LENGTH                    0x00000001

#define _DCH7CON_CHEN_POSITION                   0x00000007
#define _DCH7CON_CHEN_MASK                       0x00000080
#define _DCH7CON_CHEN_LENGTH                     0x00000001

#define _DCH7CON_CHCHNS_POSITION                 0x00000008
#define _DCH7CON_CHCHNS_MASK                     0x00000100
#define _DCH7CON_CHCHNS_LENGTH                   0x00000001

#define _DCH7CON_CHBUSY_POSITION                 0x0000000F
#define _DCH7CON_CHBUSY_MASK                     0x00008000
#define _DCH7CON_CHBUSY_LENGTH                   0x00000001

#define _DCH7CON_w_POSITION                      0x00000000
#define _DCH7CON_w_MASK                          0xFFFFFFFF
#define _DCH7CON_w_LENGTH                        0x00000020

#define _DCH7ECON_AIRQEN_POSITION                0x00000003
#define _DCH7ECON_AIRQEN_MASK                    0x00000008
#define _DCH7ECON_AIRQEN_LENGTH                  0x00000001

#define _DCH7ECON_SIRQEN_POSITION                0x00000004
#define _DCH7ECON_SIRQEN_MASK                    0x00000010
#define _DCH7ECON_SIRQEN_LENGTH                  0x00000001

#define _DCH7ECON_PATEN_POSITION                 0x00000005
#define _DCH7ECON_PATEN_MASK                     0x00000020
#define _DCH7ECON_PATEN_LENGTH                   0x00000001

#define _DCH7ECON_CABORT_POSITION                0x00000006
#define _DCH7ECON_CABORT_MASK                    0x00000040
#define _DCH7ECON_CABORT_LENGTH                  0x00000001

#define _DCH7ECON_CFORCE_POSITION                0x00000007
#define _DCH7ECON_CFORCE_MASK                    0x00000080
#define _DCH7ECON_CFORCE_LENGTH                  0x00000001

#define _DCH7ECON_CHSIRQ_POSITION                0x00000008
#define _DCH7ECON_CHSIRQ_MASK                    0x0000FF00
#define _DCH7ECON_CHSIRQ_LENGTH                  0x00000008

#define _DCH7ECON_CHAIRQ_POSITION                0x00000010
#define _DCH7ECON_CHAIRQ_MASK                    0x00FF0000
#define _DCH7ECON_CHAIRQ_LENGTH                  0x00000008

#define _DCH7ECON_w_POSITION                     0x00000000
#define _DCH7ECON_w_MASK                         0xFFFFFFFF
#define _DCH7ECON_w_LENGTH                       0x00000020

#define _DCH7INT_CHERIF_POSITION                 0x00000000
#define _DCH7INT_CHERIF_MASK                     0x00000001
#define _DCH7INT_CHERIF_LENGTH                   0x00000001

#define _DCH7INT_CHTAIF_POSITION                 0x00000001
#define _DCH7INT_CHTAIF_MASK                     0x00000002
#define _DCH7INT_CHTAIF_LENGTH                   0x00000001

#define _DCH7INT_CHCCIF_POSITION                 0x00000002
#define _DCH7INT_CHCCIF_MASK                     0x00000004
#define _DCH7INT_CHCCIF_LENGTH                   0x00000001

#define _DCH7INT_CHBCIF_POSITION                 0x00000003
#define _DCH7INT_CHBCIF_MASK                     0x00000008
#define _DCH7INT_CHBCIF_LENGTH                   0x00000001

#define _DCH7INT_CHDHIF_POSITION                 0x00000004
#define _DCH7INT_CHDHIF_MASK                     0x00000010
#define _DCH7INT_CHDHIF_LENGTH                   0x00000001

#define _DCH7INT_CHDDIF_POSITION                 0x00000005
#define _DCH7INT_CHDDIF_MASK                     0x00000020
#define _DCH7INT_CHDDIF_LENGTH                   0x00000001

#define _DCH7INT_CHSHIF_POSITION                 0x00000006
#define _DCH7INT_CHSHIF_MASK                     0x00000040
#define _DCH7INT_CHSHIF_LENGTH                   0x00000001

#define _DCH7INT_CHSDIF_POSITION                 0x00000007
#define _DCH7INT_CHSDIF_MASK                     0x00000080
#define _DCH7INT_CHSDIF_LENGTH                   0x00000001

#define _DCH7INT_CHERIE_POSITION                 0x00000010
#define _DCH7INT_CHERIE_MASK                     0x00010000
#define _DCH7INT_CHERIE_LENGTH                   0x00000001

#define _DCH7INT_CHTAIE_POSITION                 0x00000011
#define _DCH7INT_CHTAIE_MASK                     0x00020000
#define _DCH7INT_CHTAIE_LENGTH                   0x00000001

#define _DCH7INT_CHCCIE_POSITION                 0x00000012
#define _DCH7INT_CHCCIE_MASK                     0x00040000
#define _DCH7INT_CHCCIE_LENGTH                   0x00000001

#define _DCH7INT_CHBCIE_POSITION                 0x00000013
#define _DCH7INT_CHBCIE_MASK                     0x00080000
#define _DCH7INT_CHBCIE_LENGTH                   0x00000001

#define _DCH7INT_CHDHIE_POSITION                 0x00000014
#define _DCH7INT_CHDHIE_MASK                     0x00100000
#define _DCH7INT_CHDHIE_LENGTH                   0x00000001

#define _DCH7INT_CHDDIE_POSITION                 0x00000015
#define _DCH7INT_CHDDIE_MASK                     0x00200000
#define _DCH7INT_CHDDIE_LENGTH                   0x00000001

#define _DCH7INT_CHSHIE_POSITION                 0x00000016
#define _DCH7INT_CHSHIE_MASK                     0x00400000
#define _DCH7INT_CHSHIE_LENGTH                   0x00000001

#define _DCH7INT_CHSDIE_POSITION                 0x00000017
#define _DCH7INT_CHSDIE_MASK                     0x00800000
#define _DCH7INT_CHSDIE_LENGTH                   0x00000001

#define _DCH7INT_w_POSITION                      0x00000000
#define _DCH7INT_w_MASK                          0xFFFFFFFF
#define _DCH7INT_w_LENGTH                        0x00000020

#define _CHECON_PFMWS_POSITION                   0x00000000
#define _CHECON_PFMWS_MASK                       0x00000007
#define _CHECON_PFMWS_LENGTH                     0x00000003

#define _CHECON_PREFEN_POSITION                  0x00000004
#define _CHECON_PREFEN_MASK                      0x00000030
#define _CHECON_PREFEN_LENGTH                    0x00000002

#define _CHECON_DCSZ_POSITION                    0x00000008
#define _CHECON_DCSZ_MASK                        0x00000300
#define _CHECON_DCSZ_LENGTH                      0x00000002

#define _CHECON_CHECOH_POSITION                  0x00000010
#define _CHECON_CHECOH_MASK                      0x00010000
#define _CHECON_CHECOH_LENGTH                    0x00000001

#define _CHECON_w_POSITION                       0x00000000
#define _CHECON_w_MASK                           0xFFFFFFFF
#define _CHECON_w_LENGTH                         0x00000020

#define _CHETAG_LTYPE_POSITION                   0x00000001
#define _CHETAG_LTYPE_MASK                       0x00000002
#define _CHETAG_LTYPE_LENGTH                     0x00000001

#define _CHETAG_LLOCK_POSITION                   0x00000002
#define _CHETAG_LLOCK_MASK                       0x00000004
#define _CHETAG_LLOCK_LENGTH                     0x00000001

#define _CHETAG_LVALID_POSITION                  0x00000003
#define _CHETAG_LVALID_MASK                      0x00000008
#define _CHETAG_LVALID_LENGTH                    0x00000001

#define _CHETAG_LTAG_POSITION                    0x00000004
#define _CHETAG_LTAG_MASK                        0x00FFFFF0
#define _CHETAG_LTAG_LENGTH                      0x00000014

#define _CHETAG_LTAGBOOT_POSITION                0x0000001F
#define _CHETAG_LTAGBOOT_MASK                    0x80000000
#define _CHETAG_LTAGBOOT_LENGTH                  0x00000001

#define _CHETAG_w_POSITION                       0x00000000
#define _CHETAG_w_MASK                           0xFFFFFFFF
#define _CHETAG_w_LENGTH                         0x00000020

#define _U1OTGIR_VBUSVDIF_POSITION               0x00000000
#define _U1OTGIR_VBUSVDIF_MASK                   0x00000001
#define _U1OTGIR_VBUSVDIF_LENGTH                 0x00000001

#define _U1OTGIR_SESENDIF_POSITION               0x00000002
#define _U1OTGIR_SESENDIF_MASK                   0x00000004
#define _U1OTGIR_SESENDIF_LENGTH                 0x00000001

#define _U1OTGIR_SESVDIF_POSITION                0x00000003
#define _U1OTGIR_SESVDIF_MASK                    0x00000008
#define _U1OTGIR_SESVDIF_LENGTH                  0x00000001

#define _U1OTGIR_ACTVIF_POSITION                 0x00000004
#define _U1OTGIR_ACTVIF_MASK                     0x00000010
#define _U1OTGIR_ACTVIF_LENGTH                   0x00000001

#define _U1OTGIR_LSTATEIF_POSITION               0x00000005
#define _U1OTGIR_LSTATEIF_MASK                   0x00000020
#define _U1OTGIR_LSTATEIF_LENGTH                 0x00000001

#define _U1OTGIR_T1MSECIF_POSITION               0x00000006
#define _U1OTGIR_T1MSECIF_MASK                   0x00000040
#define _U1OTGIR_T1MSECIF_LENGTH                 0x00000001

#define _U1OTGIR_IDIF_POSITION                   0x00000007
#define _U1OTGIR_IDIF_MASK                       0x00000080
#define _U1OTGIR_IDIF_LENGTH                     0x00000001

#define _U1OTGIE_VBUSVDIE_POSITION               0x00000000
#define _U1OTGIE_VBUSVDIE_MASK                   0x00000001
#define _U1OTGIE_VBUSVDIE_LENGTH                 0x00000001

#define _U1OTGIE_SESENDIE_POSITION               0x00000002
#define _U1OTGIE_SESENDIE_MASK                   0x00000004
#define _U1OTGIE_SESENDIE_LENGTH                 0x00000001

#define _U1OTGIE_SESVDIE_POSITION                0x00000003
#define _U1OTGIE_SESVDIE_MASK                    0x00000008
#define _U1OTGIE_SESVDIE_LENGTH                  0x00000001

#define _U1OTGIE_ACTVIE_POSITION                 0x00000004
#define _U1OTGIE_ACTVIE_MASK                     0x00000010
#define _U1OTGIE_ACTVIE_LENGTH                   0x00000001

#define _U1OTGIE_LSTATEIE_POSITION               0x00000005
#define _U1OTGIE_LSTATEIE_MASK                   0x00000020
#define _U1OTGIE_LSTATEIE_LENGTH                 0x00000001

#define _U1OTGIE_T1MSECIE_POSITION               0x00000006
#define _U1OTGIE_T1MSECIE_MASK                   0x00000040
#define _U1OTGIE_T1MSECIE_LENGTH                 0x00000001

#define _U1OTGIE_IDIE_POSITION                   0x00000007
#define _U1OTGIE_IDIE_MASK                       0x00000080
#define _U1OTGIE_IDIE_LENGTH                     0x00000001

#define _U1OTGSTAT_VBUSVD_POSITION               0x00000000
#define _U1OTGSTAT_VBUSVD_MASK                   0x00000001
#define _U1OTGSTAT_VBUSVD_LENGTH                 0x00000001

#define _U1OTGSTAT_SESEND_POSITION               0x00000002
#define _U1OTGSTAT_SESEND_MASK                   0x00000004
#define _U1OTGSTAT_SESEND_LENGTH                 0x00000001

#define _U1OTGSTAT_SESVD_POSITION                0x00000003
#define _U1OTGSTAT_SESVD_MASK                    0x00000008
#define _U1OTGSTAT_SESVD_LENGTH                  0x00000001

#define _U1OTGSTAT_LSTATE_POSITION               0x00000005
#define _U1OTGSTAT_LSTATE_MASK                   0x00000020
#define _U1OTGSTAT_LSTATE_LENGTH                 0x00000001

#define _U1OTGSTAT_ID_POSITION                   0x00000007
#define _U1OTGSTAT_ID_MASK                       0x00000080
#define _U1OTGSTAT_ID_LENGTH                     0x00000001

#define _U1OTGCON_VBUSDIS_POSITION               0x00000000
#define _U1OTGCON_VBUSDIS_MASK                   0x00000001
#define _U1OTGCON_VBUSDIS_LENGTH                 0x00000001

#define _U1OTGCON_VBUSCHG_POSITION               0x00000001
#define _U1OTGCON_VBUSCHG_MASK                   0x00000002
#define _U1OTGCON_VBUSCHG_LENGTH                 0x00000001

#define _U1OTGCON_OTGEN_POSITION                 0x00000002
#define _U1OTGCON_OTGEN_MASK                     0x00000004
#define _U1OTGCON_OTGEN_LENGTH                   0x00000001

#define _U1OTGCON_VBUSON_POSITION                0x00000003
#define _U1OTGCON_VBUSON_MASK                    0x00000008
#define _U1OTGCON_VBUSON_LENGTH                  0x00000001

#define _U1OTGCON_DMPULDWN_POSITION              0x00000004
#define _U1OTGCON_DMPULDWN_MASK                  0x00000010
#define _U1OTGCON_DMPULDWN_LENGTH                0x00000001

#define _U1OTGCON_DPPULDWN_POSITION              0x00000005
#define _U1OTGCON_DPPULDWN_MASK                  0x00000020
#define _U1OTGCON_DPPULDWN_LENGTH                0x00000001

#define _U1OTGCON_DMPULUP_POSITION               0x00000006
#define _U1OTGCON_DMPULUP_MASK                   0x00000040
#define _U1OTGCON_DMPULUP_LENGTH                 0x00000001

#define _U1OTGCON_DPPULUP_POSITION               0x00000007
#define _U1OTGCON_DPPULUP_MASK                   0x00000080
#define _U1OTGCON_DPPULUP_LENGTH                 0x00000001

#define _U1PWRC_USBPWR_POSITION                  0x00000000
#define _U1PWRC_USBPWR_MASK                      0x00000001
#define _U1PWRC_USBPWR_LENGTH                    0x00000001

#define _U1PWRC_USUSPEND_POSITION                0x00000001
#define _U1PWRC_USUSPEND_MASK                    0x00000002
#define _U1PWRC_USUSPEND_LENGTH                  0x00000001

#define _U1PWRC_USBBUSY_POSITION                 0x00000003
#define _U1PWRC_USBBUSY_MASK                     0x00000008
#define _U1PWRC_USBBUSY_LENGTH                   0x00000001

#define _U1PWRC_USLPGRD_POSITION                 0x00000004
#define _U1PWRC_USLPGRD_MASK                     0x00000010
#define _U1PWRC_USLPGRD_LENGTH                   0x00000001

#define _U1PWRC_UACTPND_POSITION                 0x00000007
#define _U1PWRC_UACTPND_MASK                     0x00000080
#define _U1PWRC_UACTPND_LENGTH                   0x00000001

#define _U1IR_URSTIF_DETACHIF_POSITION           0x00000000
#define _U1IR_URSTIF_DETACHIF_MASK               0x00000001
#define _U1IR_URSTIF_DETACHIF_LENGTH             0x00000001

#define _U1IR_UERRIF_POSITION                    0x00000001
#define _U1IR_UERRIF_MASK                        0x00000002
#define _U1IR_UERRIF_LENGTH                      0x00000001

#define _U1IR_SOFIF_POSITION                     0x00000002
#define _U1IR_SOFIF_MASK                         0x00000004
#define _U1IR_SOFIF_LENGTH                       0x00000001

#define _U1IR_TRNIF_POSITION                     0x00000003
#define _U1IR_TRNIF_MASK                         0x00000008
#define _U1IR_TRNIF_LENGTH                       0x00000001

#define _U1IR_IDLEIF_POSITION                    0x00000004
#define _U1IR_IDLEIF_MASK                        0x00000010
#define _U1IR_IDLEIF_LENGTH                      0x00000001

#define _U1IR_RESUMEIF_POSITION                  0x00000005
#define _U1IR_RESUMEIF_MASK                      0x00000020
#define _U1IR_RESUMEIF_LENGTH                    0x00000001

#define _U1IR_ATTACHIF_POSITION                  0x00000006
#define _U1IR_ATTACHIF_MASK                      0x00000040
#define _U1IR_ATTACHIF_LENGTH                    0x00000001

#define _U1IR_STALLIF_POSITION                   0x00000007
#define _U1IR_STALLIF_MASK                       0x00000080
#define _U1IR_STALLIF_LENGTH                     0x00000001

#define _U1IR_DETACHIF_POSITION                  0x00000000
#define _U1IR_DETACHIF_MASK                      0x00000001
#define _U1IR_DETACHIF_LENGTH                    0x00000001

#define _U1IR_URSTIF_POSITION                    0x00000000
#define _U1IR_URSTIF_MASK                        0x00000001
#define _U1IR_URSTIF_LENGTH                      0x00000001

#define _U1IE_URSTIE_DETACHIE_POSITION           0x00000000
#define _U1IE_URSTIE_DETACHIE_MASK               0x00000001
#define _U1IE_URSTIE_DETACHIE_LENGTH             0x00000001

#define _U1IE_UERRIE_POSITION                    0x00000001
#define _U1IE_UERRIE_MASK                        0x00000002
#define _U1IE_UERRIE_LENGTH                      0x00000001

#define _U1IE_SOFIE_POSITION                     0x00000002
#define _U1IE_SOFIE_MASK                         0x00000004
#define _U1IE_SOFIE_LENGTH                       0x00000001

#define _U1IE_TRNIE_POSITION                     0x00000003
#define _U1IE_TRNIE_MASK                         0x00000008
#define _U1IE_TRNIE_LENGTH                       0x00000001

#define _U1IE_IDLEIE_POSITION                    0x00000004
#define _U1IE_IDLEIE_MASK                        0x00000010
#define _U1IE_IDLEIE_LENGTH                      0x00000001

#define _U1IE_RESUMEIE_POSITION                  0x00000005
#define _U1IE_RESUMEIE_MASK                      0x00000020
#define _U1IE_RESUMEIE_LENGTH                    0x00000001

#define _U1IE_ATTACHIE_POSITION                  0x00000006
#define _U1IE_ATTACHIE_MASK                      0x00000040
#define _U1IE_ATTACHIE_LENGTH                    0x00000001

#define _U1IE_STALLIE_POSITION                   0x00000007
#define _U1IE_STALLIE_MASK                       0x00000080
#define _U1IE_STALLIE_LENGTH                     0x00000001

#define _U1IE_DETACHIE_POSITION                  0x00000000
#define _U1IE_DETACHIE_MASK                      0x00000001
#define _U1IE_DETACHIE_LENGTH                    0x00000001

#define _U1IE_URSTIE_POSITION                    0x00000000
#define _U1IE_URSTIE_MASK                        0x00000001
#define _U1IE_URSTIE_LENGTH                      0x00000001

#define _U1EIR_PIDEF_POSITION                    0x00000000
#define _U1EIR_PIDEF_MASK                        0x00000001
#define _U1EIR_PIDEF_LENGTH                      0x00000001

#define _U1EIR_CRC5EF_EOFEF_POSITION             0x00000001
#define _U1EIR_CRC5EF_EOFEF_MASK                 0x00000002
#define _U1EIR_CRC5EF_EOFEF_LENGTH               0x00000001

#define _U1EIR_CRC16EF_POSITION                  0x00000002
#define _U1EIR_CRC16EF_MASK                      0x00000004
#define _U1EIR_CRC16EF_LENGTH                    0x00000001

#define _U1EIR_DFN8EF_POSITION                   0x00000003
#define _U1EIR_DFN8EF_MASK                       0x00000008
#define _U1EIR_DFN8EF_LENGTH                     0x00000001

#define _U1EIR_BTOEF_POSITION                    0x00000004
#define _U1EIR_BTOEF_MASK                        0x00000010
#define _U1EIR_BTOEF_LENGTH                      0x00000001

#define _U1EIR_DMAEF_POSITION                    0x00000005
#define _U1EIR_DMAEF_MASK                        0x00000020
#define _U1EIR_DMAEF_LENGTH                      0x00000001

#define _U1EIR_BMXEF_POSITION                    0x00000006
#define _U1EIR_BMXEF_MASK                        0x00000040
#define _U1EIR_BMXEF_LENGTH                      0x00000001

#define _U1EIR_BTSEF_POSITION                    0x00000007
#define _U1EIR_BTSEF_MASK                        0x00000080
#define _U1EIR_BTSEF_LENGTH                      0x00000001

#define _U1EIR_CRC5EF_POSITION                   0x00000001
#define _U1EIR_CRC5EF_MASK                       0x00000002
#define _U1EIR_CRC5EF_LENGTH                     0x00000001

#define _U1EIR_EOFEF_POSITION                    0x00000001
#define _U1EIR_EOFEF_MASK                        0x00000002
#define _U1EIR_EOFEF_LENGTH                      0x00000001

#define _U1EIE_PIDEE_POSITION                    0x00000000
#define _U1EIE_PIDEE_MASK                        0x00000001
#define _U1EIE_PIDEE_LENGTH                      0x00000001

#define _U1EIE_CRC5EE_EOFEE_POSITION             0x00000001
#define _U1EIE_CRC5EE_EOFEE_MASK                 0x00000002
#define _U1EIE_CRC5EE_EOFEE_LENGTH               0x00000001

#define _U1EIE_CRC16EE_POSITION                  0x00000002
#define _U1EIE_CRC16EE_MASK                      0x00000004
#define _U1EIE_CRC16EE_LENGTH                    0x00000001

#define _U1EIE_DFN8EE_POSITION                   0x00000003
#define _U1EIE_DFN8EE_MASK                       0x00000008
#define _U1EIE_DFN8EE_LENGTH                     0x00000001

#define _U1EIE_BTOEE_POSITION                    0x00000004
#define _U1EIE_BTOEE_MASK                        0x00000010
#define _U1EIE_BTOEE_LENGTH                      0x00000001

#define _U1EIE_DMAEE_POSITION                    0x00000005
#define _U1EIE_DMAEE_MASK                        0x00000020
#define _U1EIE_DMAEE_LENGTH                      0x00000001

#define _U1EIE_BMXEE_POSITION                    0x00000006
#define _U1EIE_BMXEE_MASK                        0x00000040
#define _U1EIE_BMXEE_LENGTH                      0x00000001

#define _U1EIE_BTSEE_POSITION                    0x00000007
#define _U1EIE_BTSEE_MASK                        0x00000080
#define _U1EIE_BTSEE_LENGTH                      0x00000001

#define _U1EIE_CRC5EE_POSITION                   0x00000001
#define _U1EIE_CRC5EE_MASK                       0x00000002
#define _U1EIE_CRC5EE_LENGTH                     0x00000001

#define _U1EIE_EOFEE_POSITION                    0x00000001
#define _U1EIE_EOFEE_MASK                        0x00000002
#define _U1EIE_EOFEE_LENGTH                      0x00000001

#define _U1STAT_PPBI_POSITION                    0x00000002
#define _U1STAT_PPBI_MASK                        0x00000004
#define _U1STAT_PPBI_LENGTH                      0x00000001

#define _U1STAT_DIR_POSITION                     0x00000003
#define _U1STAT_DIR_MASK                         0x00000008
#define _U1STAT_DIR_LENGTH                       0x00000001

#define _U1STAT_ENDPT0_POSITION                  0x00000004
#define _U1STAT_ENDPT0_MASK                      0x00000010
#define _U1STAT_ENDPT0_LENGTH                    0x00000001

#define _U1STAT_ENDPT_POSITION                   0x00000004
#define _U1STAT_ENDPT_MASK                       0x000000F0
#define _U1STAT_ENDPT_LENGTH                     0x00000004

#define _U1STAT_ENDPT1_POSITION                  0x00000005
#define _U1STAT_ENDPT1_MASK                      0x00000020
#define _U1STAT_ENDPT1_LENGTH                    0x00000001

#define _U1STAT_ENDPT2_POSITION                  0x00000006
#define _U1STAT_ENDPT2_MASK                      0x00000040
#define _U1STAT_ENDPT2_LENGTH                    0x00000001

#define _U1STAT_ENDPT3_POSITION                  0x00000007
#define _U1STAT_ENDPT3_MASK                      0x00000080
#define _U1STAT_ENDPT3_LENGTH                    0x00000001

#define _U1CON_USBEN_SOFEN_POSITION              0x00000000
#define _U1CON_USBEN_SOFEN_MASK                  0x00000001
#define _U1CON_USBEN_SOFEN_LENGTH                0x00000001

#define _U1CON_PPBRST_POSITION                   0x00000001
#define _U1CON_PPBRST_MASK                       0x00000002
#define _U1CON_PPBRST_LENGTH                     0x00000001

#define _U1CON_RESUME_POSITION                   0x00000002
#define _U1CON_RESUME_MASK                       0x00000004
#define _U1CON_RESUME_LENGTH                     0x00000001

#define _U1CON_HOSTEN_POSITION                   0x00000003
#define _U1CON_HOSTEN_MASK                       0x00000008
#define _U1CON_HOSTEN_LENGTH                     0x00000001

#define _U1CON_USBRST_POSITION                   0x00000004
#define _U1CON_USBRST_MASK                       0x00000010
#define _U1CON_USBRST_LENGTH                     0x00000001

#define _U1CON_PKTDIS_TOKBUSY_POSITION           0x00000005
#define _U1CON_PKTDIS_TOKBUSY_MASK               0x00000020
#define _U1CON_PKTDIS_TOKBUSY_LENGTH             0x00000001

#define _U1CON_SE0_POSITION                      0x00000006
#define _U1CON_SE0_MASK                          0x00000040
#define _U1CON_SE0_LENGTH                        0x00000001

#define _U1CON_JSTATE_POSITION                   0x00000007
#define _U1CON_JSTATE_MASK                       0x00000080
#define _U1CON_JSTATE_LENGTH                     0x00000001

#define _U1CON_USBEN_POSITION                    0x00000000
#define _U1CON_USBEN_MASK                        0x00000001
#define _U1CON_USBEN_LENGTH                      0x00000001

#define _U1CON_SOFEN_POSITION                    0x00000000
#define _U1CON_SOFEN_MASK                        0x00000001
#define _U1CON_SOFEN_LENGTH                      0x00000001

#define _U1CON_PKTDIS_POSITION                   0x00000005
#define _U1CON_PKTDIS_MASK                       0x00000020
#define _U1CON_PKTDIS_LENGTH                     0x00000001

#define _U1CON_TOKBUSY_POSITION                  0x00000005
#define _U1CON_TOKBUSY_MASK                      0x00000020
#define _U1CON_TOKBUSY_LENGTH                    0x00000001

#define _U1ADDR_DEVADDR_POSITION                 0x00000000
#define _U1ADDR_DEVADDR_MASK                     0x0000007F
#define _U1ADDR_DEVADDR_LENGTH                   0x00000007

#define _U1ADDR_LSPDEN_POSITION                  0x00000007
#define _U1ADDR_LSPDEN_MASK                      0x00000080
#define _U1ADDR_LSPDEN_LENGTH                    0x00000001

#define _U1ADDR_DEVADDR0_POSITION                0x00000000
#define _U1ADDR_DEVADDR0_MASK                    0x00000001
#define _U1ADDR_DEVADDR0_LENGTH                  0x00000001

#define _U1ADDR_DEVADDR1_POSITION                0x00000001
#define _U1ADDR_DEVADDR1_MASK                    0x00000002
#define _U1ADDR_DEVADDR1_LENGTH                  0x00000001

#define _U1ADDR_DEVADDR2_POSITION                0x00000002
#define _U1ADDR_DEVADDR2_MASK                    0x00000004
#define _U1ADDR_DEVADDR2_LENGTH                  0x00000001

#define _U1ADDR_DEVADDR3_POSITION                0x00000003
#define _U1ADDR_DEVADDR3_MASK                    0x00000008
#define _U1ADDR_DEVADDR3_LENGTH                  0x00000001

#define _U1ADDR_DEVADDR4_POSITION                0x00000004
#define _U1ADDR_DEVADDR4_MASK                    0x00000010
#define _U1ADDR_DEVADDR4_LENGTH                  0x00000001

#define _U1ADDR_DEVADDR5_POSITION                0x00000005
#define _U1ADDR_DEVADDR5_MASK                    0x00000020
#define _U1ADDR_DEVADDR5_LENGTH                  0x00000001

#define _U1ADDR_DEVADDR6_POSITION                0x00000006
#define _U1ADDR_DEVADDR6_MASK                    0x00000040
#define _U1ADDR_DEVADDR6_LENGTH                  0x00000001

#define _U1BDTP1_BDTPTRL_POSITION                0x00000001
#define _U1BDTP1_BDTPTRL_MASK                    0x000000FE
#define _U1BDTP1_BDTPTRL_LENGTH                  0x00000007

#define _U1FRML_FRML_POSITION                    0x00000000
#define _U1FRML_FRML_MASK                        0x000000FF
#define _U1FRML_FRML_LENGTH                      0x00000008

#define _U1FRML_FRM0_POSITION                    0x00000000
#define _U1FRML_FRM0_MASK                        0x00000001
#define _U1FRML_FRM0_LENGTH                      0x00000001

#define _U1FRML_FRM1_POSITION                    0x00000001
#define _U1FRML_FRM1_MASK                        0x00000002
#define _U1FRML_FRM1_LENGTH                      0x00000001

#define _U1FRML_FRM2_POSITION                    0x00000002
#define _U1FRML_FRM2_MASK                        0x00000004
#define _U1FRML_FRM2_LENGTH                      0x00000001

#define _U1FRML_FRM3_POSITION                    0x00000003
#define _U1FRML_FRM3_MASK                        0x00000008
#define _U1FRML_FRM3_LENGTH                      0x00000001

#define _U1FRML_FRM4_POSITION                    0x00000004
#define _U1FRML_FRM4_MASK                        0x00000010
#define _U1FRML_FRM4_LENGTH                      0x00000001

#define _U1FRML_FRM5_POSITION                    0x00000005
#define _U1FRML_FRM5_MASK                        0x00000020
#define _U1FRML_FRM5_LENGTH                      0x00000001

#define _U1FRML_FRM6_POSITION                    0x00000006
#define _U1FRML_FRM6_MASK                        0x00000040
#define _U1FRML_FRM6_LENGTH                      0x00000001

#define _U1FRML_FRM7_POSITION                    0x00000007
#define _U1FRML_FRM7_MASK                        0x00000080
#define _U1FRML_FRM7_LENGTH                      0x00000001

#define _U1FRMH_FRMH_POSITION                    0x00000000
#define _U1FRMH_FRMH_MASK                        0x00000007
#define _U1FRMH_FRMH_LENGTH                      0x00000003

#define _U1FRMH_FRM8_POSITION                    0x00000000
#define _U1FRMH_FRM8_MASK                        0x00000001
#define _U1FRMH_FRM8_LENGTH                      0x00000001

#define _U1FRMH_FRM9_POSITION                    0x00000001
#define _U1FRMH_FRM9_MASK                        0x00000002
#define _U1FRMH_FRM9_LENGTH                      0x00000001

#define _U1FRMH_FRM10_POSITION                   0x00000002
#define _U1FRMH_FRM10_MASK                       0x00000004
#define _U1FRMH_FRM10_LENGTH                     0x00000001

#define _U1TOK_EP_POSITION                       0x00000000
#define _U1TOK_EP_MASK                           0x0000000F
#define _U1TOK_EP_LENGTH                         0x00000004

#define _U1TOK_PID_POSITION                      0x00000004
#define _U1TOK_PID_MASK                          0x000000F0
#define _U1TOK_PID_LENGTH                        0x00000004

#define _U1TOK_EP0_POSITION                      0x00000000
#define _U1TOK_EP0_MASK                          0x00000001
#define _U1TOK_EP0_LENGTH                        0x00000001

#define _U1TOK_EP1_POSITION                      0x00000001
#define _U1TOK_EP1_MASK                          0x00000002
#define _U1TOK_EP1_LENGTH                        0x00000001

#define _U1TOK_EP2_POSITION                      0x00000002
#define _U1TOK_EP2_MASK                          0x00000004
#define _U1TOK_EP2_LENGTH                        0x00000001

#define _U1TOK_EP3_POSITION                      0x00000003
#define _U1TOK_EP3_MASK                          0x00000008
#define _U1TOK_EP3_LENGTH                        0x00000001

#define _U1TOK_PID0_POSITION                     0x00000004
#define _U1TOK_PID0_MASK                         0x00000010
#define _U1TOK_PID0_LENGTH                       0x00000001

#define _U1TOK_PID1_POSITION                     0x00000005
#define _U1TOK_PID1_MASK                         0x00000020
#define _U1TOK_PID1_LENGTH                       0x00000001

#define _U1TOK_PID2_POSITION                     0x00000006
#define _U1TOK_PID2_MASK                         0x00000040
#define _U1TOK_PID2_LENGTH                       0x00000001

#define _U1TOK_PID3_POSITION                     0x00000007
#define _U1TOK_PID3_MASK                         0x00000080
#define _U1TOK_PID3_LENGTH                       0x00000001

#define _U1SOF_CNT_POSITION                      0x00000000
#define _U1SOF_CNT_MASK                          0x000000FF
#define _U1SOF_CNT_LENGTH                        0x00000008

#define _U1BDTP2_BDTPTRH_POSITION                0x00000000
#define _U1BDTP2_BDTPTRH_MASK                    0x000000FF
#define _U1BDTP2_BDTPTRH_LENGTH                  0x00000008

#define _U1BDTP3_BDTPTRU_POSITION                0x00000000
#define _U1BDTP3_BDTPTRU_MASK                    0x000000FF
#define _U1BDTP3_BDTPTRU_LENGTH                  0x00000008

#define _U1CNFG1_UASUSPND_POSITION               0x00000000
#define _U1CNFG1_UASUSPND_MASK                   0x00000001
#define _U1CNFG1_UASUSPND_LENGTH                 0x00000001

#define _U1CNFG1_USBSIDL_POSITION                0x00000004
#define _U1CNFG1_USBSIDL_MASK                    0x00000010
#define _U1CNFG1_USBSIDL_LENGTH                  0x00000001

#define _U1CNFG1_UOEMON_POSITION                 0x00000006
#define _U1CNFG1_UOEMON_MASK                     0x00000040
#define _U1CNFG1_UOEMON_LENGTH                   0x00000001

#define _U1CNFG1_UTEYE_POSITION                  0x00000007
#define _U1CNFG1_UTEYE_MASK                      0x00000080
#define _U1CNFG1_UTEYE_LENGTH                    0x00000001

#define _U1EP0_EPHSHK_POSITION                   0x00000000
#define _U1EP0_EPHSHK_MASK                       0x00000001
#define _U1EP0_EPHSHK_LENGTH                     0x00000001

#define _U1EP0_EPSTALL_POSITION                  0x00000001
#define _U1EP0_EPSTALL_MASK                      0x00000002
#define _U1EP0_EPSTALL_LENGTH                    0x00000001

#define _U1EP0_EPTXEN_POSITION                   0x00000002
#define _U1EP0_EPTXEN_MASK                       0x00000004
#define _U1EP0_EPTXEN_LENGTH                     0x00000001

#define _U1EP0_EPRXEN_POSITION                   0x00000003
#define _U1EP0_EPRXEN_MASK                       0x00000008
#define _U1EP0_EPRXEN_LENGTH                     0x00000001

#define _U1EP0_EPCONDIS_POSITION                 0x00000004
#define _U1EP0_EPCONDIS_MASK                     0x00000010
#define _U1EP0_EPCONDIS_LENGTH                   0x00000001

#define _U1EP0_RETRYDIS_POSITION                 0x00000006
#define _U1EP0_RETRYDIS_MASK                     0x00000040
#define _U1EP0_RETRYDIS_LENGTH                   0x00000001

#define _U1EP0_LSPD_POSITION                     0x00000007
#define _U1EP0_LSPD_MASK                         0x00000080
#define _U1EP0_LSPD_LENGTH                       0x00000001

#define _U1EP1_EPHSHK_POSITION                   0x00000000
#define _U1EP1_EPHSHK_MASK                       0x00000001
#define _U1EP1_EPHSHK_LENGTH                     0x00000001

#define _U1EP1_EPSTALL_POSITION                  0x00000001
#define _U1EP1_EPSTALL_MASK                      0x00000002
#define _U1EP1_EPSTALL_LENGTH                    0x00000001

#define _U1EP1_EPTXEN_POSITION                   0x00000002
#define _U1EP1_EPTXEN_MASK                       0x00000004
#define _U1EP1_EPTXEN_LENGTH                     0x00000001

#define _U1EP1_EPRXEN_POSITION                   0x00000003
#define _U1EP1_EPRXEN_MASK                       0x00000008
#define _U1EP1_EPRXEN_LENGTH                     0x00000001

#define _U1EP1_EPCONDIS_POSITION                 0x00000004
#define _U1EP1_EPCONDIS_MASK                     0x00000010
#define _U1EP1_EPCONDIS_LENGTH                   0x00000001

#define _U1EP2_EPHSHK_POSITION                   0x00000000
#define _U1EP2_EPHSHK_MASK                       0x00000001
#define _U1EP2_EPHSHK_LENGTH                     0x00000001

#define _U1EP2_EPSTALL_POSITION                  0x00000001
#define _U1EP2_EPSTALL_MASK                      0x00000002
#define _U1EP2_EPSTALL_LENGTH                    0x00000001

#define _U1EP2_EPTXEN_POSITION                   0x00000002
#define _U1EP2_EPTXEN_MASK                       0x00000004
#define _U1EP2_EPTXEN_LENGTH                     0x00000001

#define _U1EP2_EPRXEN_POSITION                   0x00000003
#define _U1EP2_EPRXEN_MASK                       0x00000008
#define _U1EP2_EPRXEN_LENGTH                     0x00000001

#define _U1EP2_EPCONDIS_POSITION                 0x00000004
#define _U1EP2_EPCONDIS_MASK                     0x00000010
#define _U1EP2_EPCONDIS_LENGTH                   0x00000001

#define _U1EP3_EPHSHK_POSITION                   0x00000000
#define _U1EP3_EPHSHK_MASK                       0x00000001
#define _U1EP3_EPHSHK_LENGTH                     0x00000001

#define _U1EP3_EPSTALL_POSITION                  0x00000001
#define _U1EP3_EPSTALL_MASK                      0x00000002
#define _U1EP3_EPSTALL_LENGTH                    0x00000001

#define _U1EP3_EPTXEN_POSITION                   0x00000002
#define _U1EP3_EPTXEN_MASK                       0x00000004
#define _U1EP3_EPTXEN_LENGTH                     0x00000001

#define _U1EP3_EPRXEN_POSITION                   0x00000003
#define _U1EP3_EPRXEN_MASK                       0x00000008
#define _U1EP3_EPRXEN_LENGTH                     0x00000001

#define _U1EP3_EPCONDIS_POSITION                 0x00000004
#define _U1EP3_EPCONDIS_MASK                     0x00000010
#define _U1EP3_EPCONDIS_LENGTH                   0x00000001

#define _U1EP4_EPHSHK_POSITION                   0x00000000
#define _U1EP4_EPHSHK_MASK                       0x00000001
#define _U1EP4_EPHSHK_LENGTH                     0x00000001

#define _U1EP4_EPSTALL_POSITION                  0x00000001
#define _U1EP4_EPSTALL_MASK                      0x00000002
#define _U1EP4_EPSTALL_LENGTH                    0x00000001

#define _U1EP4_EPTXEN_POSITION                   0x00000002
#define _U1EP4_EPTXEN_MASK                       0x00000004
#define _U1EP4_EPTXEN_LENGTH                     0x00000001

#define _U1EP4_EPRXEN_POSITION                   0x00000003
#define _U1EP4_EPRXEN_MASK                       0x00000008
#define _U1EP4_EPRXEN_LENGTH                     0x00000001

#define _U1EP4_EPCONDIS_POSITION                 0x00000004
#define _U1EP4_EPCONDIS_MASK                     0x00000010
#define _U1EP4_EPCONDIS_LENGTH                   0x00000001

#define _U1EP5_EPHSHK_POSITION                   0x00000000
#define _U1EP5_EPHSHK_MASK                       0x00000001
#define _U1EP5_EPHSHK_LENGTH                     0x00000001

#define _U1EP5_EPSTALL_POSITION                  0x00000001
#define _U1EP5_EPSTALL_MASK                      0x00000002
#define _U1EP5_EPSTALL_LENGTH                    0x00000001

#define _U1EP5_EPTXEN_POSITION                   0x00000002
#define _U1EP5_EPTXEN_MASK                       0x00000004
#define _U1EP5_EPTXEN_LENGTH                     0x00000001

#define _U1EP5_EPRXEN_POSITION                   0x00000003
#define _U1EP5_EPRXEN_MASK                       0x00000008
#define _U1EP5_EPRXEN_LENGTH                     0x00000001

#define _U1EP5_EPCONDIS_POSITION                 0x00000004
#define _U1EP5_EPCONDIS_MASK                     0x00000010
#define _U1EP5_EPCONDIS_LENGTH                   0x00000001

#define _U1EP6_EPHSHK_POSITION                   0x00000000
#define _U1EP6_EPHSHK_MASK                       0x00000001
#define _U1EP6_EPHSHK_LENGTH                     0x00000001

#define _U1EP6_EPSTALL_POSITION                  0x00000001
#define _U1EP6_EPSTALL_MASK                      0x00000002
#define _U1EP6_EPSTALL_LENGTH                    0x00000001

#define _U1EP6_EPTXEN_POSITION                   0x00000002
#define _U1EP6_EPTXEN_MASK                       0x00000004
#define _U1EP6_EPTXEN_LENGTH                     0x00000001

#define _U1EP6_EPRXEN_POSITION                   0x00000003
#define _U1EP6_EPRXEN_MASK                       0x00000008
#define _U1EP6_EPRXEN_LENGTH                     0x00000001

#define _U1EP6_EPCONDIS_POSITION                 0x00000004
#define _U1EP6_EPCONDIS_MASK                     0x00000010
#define _U1EP6_EPCONDIS_LENGTH                   0x00000001

#define _U1EP7_EPHSHK_POSITION                   0x00000000
#define _U1EP7_EPHSHK_MASK                       0x00000001
#define _U1EP7_EPHSHK_LENGTH                     0x00000001

#define _U1EP7_EPSTALL_POSITION                  0x00000001
#define _U1EP7_EPSTALL_MASK                      0x00000002
#define _U1EP7_EPSTALL_LENGTH                    0x00000001

#define _U1EP7_EPTXEN_POSITION                   0x00000002
#define _U1EP7_EPTXEN_MASK                       0x00000004
#define _U1EP7_EPTXEN_LENGTH                     0x00000001

#define _U1EP7_EPRXEN_POSITION                   0x00000003
#define _U1EP7_EPRXEN_MASK                       0x00000008
#define _U1EP7_EPRXEN_LENGTH                     0x00000001

#define _U1EP7_EPCONDIS_POSITION                 0x00000004
#define _U1EP7_EPCONDIS_MASK                     0x00000010
#define _U1EP7_EPCONDIS_LENGTH                   0x00000001

#define _U1EP8_EPHSHK_POSITION                   0x00000000
#define _U1EP8_EPHSHK_MASK                       0x00000001
#define _U1EP8_EPHSHK_LENGTH                     0x00000001

#define _U1EP8_EPSTALL_POSITION                  0x00000001
#define _U1EP8_EPSTALL_MASK                      0x00000002
#define _U1EP8_EPSTALL_LENGTH                    0x00000001

#define _U1EP8_EPTXEN_POSITION                   0x00000002
#define _U1EP8_EPTXEN_MASK                       0x00000004
#define _U1EP8_EPTXEN_LENGTH                     0x00000001

#define _U1EP8_EPRXEN_POSITION                   0x00000003
#define _U1EP8_EPRXEN_MASK                       0x00000008
#define _U1EP8_EPRXEN_LENGTH                     0x00000001

#define _U1EP8_EPCONDIS_POSITION                 0x00000004
#define _U1EP8_EPCONDIS_MASK                     0x00000010
#define _U1EP8_EPCONDIS_LENGTH                   0x00000001

#define _U1EP9_EPHSHK_POSITION                   0x00000000
#define _U1EP9_EPHSHK_MASK                       0x00000001
#define _U1EP9_EPHSHK_LENGTH                     0x00000001

#define _U1EP9_EPSTALL_POSITION                  0x00000001
#define _U1EP9_EPSTALL_MASK                      0x00000002
#define _U1EP9_EPSTALL_LENGTH                    0x00000001

#define _U1EP9_EPTXEN_POSITION                   0x00000002
#define _U1EP9_EPTXEN_MASK                       0x00000004
#define _U1EP9_EPTXEN_LENGTH                     0x00000001

#define _U1EP9_EPRXEN_POSITION                   0x00000003
#define _U1EP9_EPRXEN_MASK                       0x00000008
#define _U1EP9_EPRXEN_LENGTH                     0x00000001

#define _U1EP9_EPCONDIS_POSITION                 0x00000004
#define _U1EP9_EPCONDIS_MASK                     0x00000010
#define _U1EP9_EPCONDIS_LENGTH                   0x00000001

#define _U1EP10_EPHSHK_POSITION                  0x00000000
#define _U1EP10_EPHSHK_MASK                      0x00000001
#define _U1EP10_EPHSHK_LENGTH                    0x00000001

#define _U1EP10_EPSTALL_POSITION                 0x00000001
#define _U1EP10_EPSTALL_MASK                     0x00000002
#define _U1EP10_EPSTALL_LENGTH                   0x00000001

#define _U1EP10_EPTXEN_POSITION                  0x00000002
#define _U1EP10_EPTXEN_MASK                      0x00000004
#define _U1EP10_EPTXEN_LENGTH                    0x00000001

#define _U1EP10_EPRXEN_POSITION                  0x00000003
#define _U1EP10_EPRXEN_MASK                      0x00000008
#define _U1EP10_EPRXEN_LENGTH                    0x00000001

#define _U1EP10_EPCONDIS_POSITION                0x00000004
#define _U1EP10_EPCONDIS_MASK                    0x00000010
#define _U1EP10_EPCONDIS_LENGTH                  0x00000001

#define _U1EP11_EPHSHK_POSITION                  0x00000000
#define _U1EP11_EPHSHK_MASK                      0x00000001
#define _U1EP11_EPHSHK_LENGTH                    0x00000001

#define _U1EP11_EPSTALL_POSITION                 0x00000001
#define _U1EP11_EPSTALL_MASK                     0x00000002
#define _U1EP11_EPSTALL_LENGTH                   0x00000001

#define _U1EP11_EPTXEN_POSITION                  0x00000002
#define _U1EP11_EPTXEN_MASK                      0x00000004
#define _U1EP11_EPTXEN_LENGTH                    0x00000001

#define _U1EP11_EPRXEN_POSITION                  0x00000003
#define _U1EP11_EPRXEN_MASK                      0x00000008
#define _U1EP11_EPRXEN_LENGTH                    0x00000001

#define _U1EP11_EPCONDIS_POSITION                0x00000004
#define _U1EP11_EPCONDIS_MASK                    0x00000010
#define _U1EP11_EPCONDIS_LENGTH                  0x00000001

#define _U1EP12_EPHSHK_POSITION                  0x00000000
#define _U1EP12_EPHSHK_MASK                      0x00000001
#define _U1EP12_EPHSHK_LENGTH                    0x00000001

#define _U1EP12_EPSTALL_POSITION                 0x00000001
#define _U1EP12_EPSTALL_MASK                     0x00000002
#define _U1EP12_EPSTALL_LENGTH                   0x00000001

#define _U1EP12_EPTXEN_POSITION                  0x00000002
#define _U1EP12_EPTXEN_MASK                      0x00000004
#define _U1EP12_EPTXEN_LENGTH                    0x00000001

#define _U1EP12_EPRXEN_POSITION                  0x00000003
#define _U1EP12_EPRXEN_MASK                      0x00000008
#define _U1EP12_EPRXEN_LENGTH                    0x00000001

#define _U1EP12_EPCONDIS_POSITION                0x00000004
#define _U1EP12_EPCONDIS_MASK                    0x00000010
#define _U1EP12_EPCONDIS_LENGTH                  0x00000001

#define _U1EP13_EPHSHK_POSITION                  0x00000000
#define _U1EP13_EPHSHK_MASK                      0x00000001
#define _U1EP13_EPHSHK_LENGTH                    0x00000001

#define _U1EP13_EPSTALL_POSITION                 0x00000001
#define _U1EP13_EPSTALL_MASK                     0x00000002
#define _U1EP13_EPSTALL_LENGTH                   0x00000001

#define _U1EP13_EPTXEN_POSITION                  0x00000002
#define _U1EP13_EPTXEN_MASK                      0x00000004
#define _U1EP13_EPTXEN_LENGTH                    0x00000001

#define _U1EP13_EPRXEN_POSITION                  0x00000003
#define _U1EP13_EPRXEN_MASK                      0x00000008
#define _U1EP13_EPRXEN_LENGTH                    0x00000001

#define _U1EP13_EPCONDIS_POSITION                0x00000004
#define _U1EP13_EPCONDIS_MASK                    0x00000010
#define _U1EP13_EPCONDIS_LENGTH                  0x00000001

#define _U1EP14_EPHSHK_POSITION                  0x00000000
#define _U1EP14_EPHSHK_MASK                      0x00000001
#define _U1EP14_EPHSHK_LENGTH                    0x00000001

#define _U1EP14_EPSTALL_POSITION                 0x00000001
#define _U1EP14_EPSTALL_MASK                     0x00000002
#define _U1EP14_EPSTALL_LENGTH                   0x00000001

#define _U1EP14_EPTXEN_POSITION                  0x00000002
#define _U1EP14_EPTXEN_MASK                      0x00000004
#define _U1EP14_EPTXEN_LENGTH                    0x00000001

#define _U1EP14_EPRXEN_POSITION                  0x00000003
#define _U1EP14_EPRXEN_MASK                      0x00000008
#define _U1EP14_EPRXEN_LENGTH                    0x00000001

#define _U1EP14_EPCONDIS_POSITION                0x00000004
#define _U1EP14_EPCONDIS_MASK                    0x00000010
#define _U1EP14_EPCONDIS_LENGTH                  0x00000001

#define _U1EP15_EPHSHK_POSITION                  0x00000000
#define _U1EP15_EPHSHK_MASK                      0x00000001
#define _U1EP15_EPHSHK_LENGTH                    0x00000001

#define _U1EP15_EPSTALL_POSITION                 0x00000001
#define _U1EP15_EPSTALL_MASK                     0x00000002
#define _U1EP15_EPSTALL_LENGTH                   0x00000001

#define _U1EP15_EPTXEN_POSITION                  0x00000002
#define _U1EP15_EPTXEN_MASK                      0x00000004
#define _U1EP15_EPTXEN_LENGTH                    0x00000001

#define _U1EP15_EPRXEN_POSITION                  0x00000003
#define _U1EP15_EPRXEN_MASK                      0x00000008
#define _U1EP15_EPRXEN_LENGTH                    0x00000001

#define _U1EP15_EPCONDIS_POSITION                0x00000004
#define _U1EP15_EPCONDIS_MASK                    0x00000010
#define _U1EP15_EPCONDIS_LENGTH                  0x00000001

#define _TRISB_TRISB0_POSITION                   0x00000000
#define _TRISB_TRISB0_MASK                       0x00000001
#define _TRISB_TRISB0_LENGTH                     0x00000001

#define _TRISB_TRISB1_POSITION                   0x00000001
#define _TRISB_TRISB1_MASK                       0x00000002
#define _TRISB_TRISB1_LENGTH                     0x00000001

#define _TRISB_TRISB2_POSITION                   0x00000002
#define _TRISB_TRISB2_MASK                       0x00000004
#define _TRISB_TRISB2_LENGTH                     0x00000001

#define _TRISB_TRISB3_POSITION                   0x00000003
#define _TRISB_TRISB3_MASK                       0x00000008
#define _TRISB_TRISB3_LENGTH                     0x00000001

#define _TRISB_TRISB4_POSITION                   0x00000004
#define _TRISB_TRISB4_MASK                       0x00000010
#define _TRISB_TRISB4_LENGTH                     0x00000001

#define _TRISB_TRISB5_POSITION                   0x00000005
#define _TRISB_TRISB5_MASK                       0x00000020
#define _TRISB_TRISB5_LENGTH                     0x00000001

#define _TRISB_TRISB6_POSITION                   0x00000006
#define _TRISB_TRISB6_MASK                       0x00000040
#define _TRISB_TRISB6_LENGTH                     0x00000001

#define _TRISB_TRISB7_POSITION                   0x00000007
#define _TRISB_TRISB7_MASK                       0x00000080
#define _TRISB_TRISB7_LENGTH                     0x00000001

#define _TRISB_TRISB8_POSITION                   0x00000008
#define _TRISB_TRISB8_MASK                       0x00000100
#define _TRISB_TRISB8_LENGTH                     0x00000001

#define _TRISB_TRISB9_POSITION                   0x00000009
#define _TRISB_TRISB9_MASK                       0x00000200
#define _TRISB_TRISB9_LENGTH                     0x00000001

#define _TRISB_TRISB10_POSITION                  0x0000000A
#define _TRISB_TRISB10_MASK                      0x00000400
#define _TRISB_TRISB10_LENGTH                    0x00000001

#define _TRISB_TRISB11_POSITION                  0x0000000B
#define _TRISB_TRISB11_MASK                      0x00000800
#define _TRISB_TRISB11_LENGTH                    0x00000001

#define _TRISB_TRISB12_POSITION                  0x0000000C
#define _TRISB_TRISB12_MASK                      0x00001000
#define _TRISB_TRISB12_LENGTH                    0x00000001

#define _TRISB_TRISB13_POSITION                  0x0000000D
#define _TRISB_TRISB13_MASK                      0x00002000
#define _TRISB_TRISB13_LENGTH                    0x00000001

#define _TRISB_TRISB14_POSITION                  0x0000000E
#define _TRISB_TRISB14_MASK                      0x00004000
#define _TRISB_TRISB14_LENGTH                    0x00000001

#define _TRISB_TRISB15_POSITION                  0x0000000F
#define _TRISB_TRISB15_MASK                      0x00008000
#define _TRISB_TRISB15_LENGTH                    0x00000001

#define _TRISB_w_POSITION                        0x00000000
#define _TRISB_w_MASK                            0xFFFFFFFF
#define _TRISB_w_LENGTH                          0x00000020

#define _PORTB_RB0_POSITION                      0x00000000
#define _PORTB_RB0_MASK                          0x00000001
#define _PORTB_RB0_LENGTH                        0x00000001

#define _PORTB_RB1_POSITION                      0x00000001
#define _PORTB_RB1_MASK                          0x00000002
#define _PORTB_RB1_LENGTH                        0x00000001

#define _PORTB_RB2_POSITION                      0x00000002
#define _PORTB_RB2_MASK                          0x00000004
#define _PORTB_RB2_LENGTH                        0x00000001

#define _PORTB_RB3_POSITION                      0x00000003
#define _PORTB_RB3_MASK                          0x00000008
#define _PORTB_RB3_LENGTH                        0x00000001

#define _PORTB_RB4_POSITION                      0x00000004
#define _PORTB_RB4_MASK                          0x00000010
#define _PORTB_RB4_LENGTH                        0x00000001

#define _PORTB_RB5_POSITION                      0x00000005
#define _PORTB_RB5_MASK                          0x00000020
#define _PORTB_RB5_LENGTH                        0x00000001

#define _PORTB_RB6_POSITION                      0x00000006
#define _PORTB_RB6_MASK                          0x00000040
#define _PORTB_RB6_LENGTH                        0x00000001

#define _PORTB_RB7_POSITION                      0x00000007
#define _PORTB_RB7_MASK                          0x00000080
#define _PORTB_RB7_LENGTH                        0x00000001

#define _PORTB_RB8_POSITION                      0x00000008
#define _PORTB_RB8_MASK                          0x00000100
#define _PORTB_RB8_LENGTH                        0x00000001

#define _PORTB_RB9_POSITION                      0x00000009
#define _PORTB_RB9_MASK                          0x00000200
#define _PORTB_RB9_LENGTH                        0x00000001

#define _PORTB_RB10_POSITION                     0x0000000A
#define _PORTB_RB10_MASK                         0x00000400
#define _PORTB_RB10_LENGTH                       0x00000001

#define _PORTB_RB11_POSITION                     0x0000000B
#define _PORTB_RB11_MASK                         0x00000800
#define _PORTB_RB11_LENGTH                       0x00000001

#define _PORTB_RB12_POSITION                     0x0000000C
#define _PORTB_RB12_MASK                         0x00001000
#define _PORTB_RB12_LENGTH                       0x00000001

#define _PORTB_RB13_POSITION                     0x0000000D
#define _PORTB_RB13_MASK                         0x00002000
#define _PORTB_RB13_LENGTH                       0x00000001

#define _PORTB_RB14_POSITION                     0x0000000E
#define _PORTB_RB14_MASK                         0x00004000
#define _PORTB_RB14_LENGTH                       0x00000001

#define _PORTB_RB15_POSITION                     0x0000000F
#define _PORTB_RB15_MASK                         0x00008000
#define _PORTB_RB15_LENGTH                       0x00000001

#define _PORTB_w_POSITION                        0x00000000
#define _PORTB_w_MASK                            0xFFFFFFFF
#define _PORTB_w_LENGTH                          0x00000020

#define _LATB_LATB0_POSITION                     0x00000000
#define _LATB_LATB0_MASK                         0x00000001
#define _LATB_LATB0_LENGTH                       0x00000001

#define _LATB_LATB1_POSITION                     0x00000001
#define _LATB_LATB1_MASK                         0x00000002
#define _LATB_LATB1_LENGTH                       0x00000001

#define _LATB_LATB2_POSITION                     0x00000002
#define _LATB_LATB2_MASK                         0x00000004
#define _LATB_LATB2_LENGTH                       0x00000001

#define _LATB_LATB3_POSITION                     0x00000003
#define _LATB_LATB3_MASK                         0x00000008
#define _LATB_LATB3_LENGTH                       0x00000001

#define _LATB_LATB4_POSITION                     0x00000004
#define _LATB_LATB4_MASK                         0x00000010
#define _LATB_LATB4_LENGTH                       0x00000001

#define _LATB_LATB5_POSITION                     0x00000005
#define _LATB_LATB5_MASK                         0x00000020
#define _LATB_LATB5_LENGTH                       0x00000001

#define _LATB_LATB6_POSITION                     0x00000006
#define _LATB_LATB6_MASK                         0x00000040
#define _LATB_LATB6_LENGTH                       0x00000001

#define _LATB_LATB7_POSITION                     0x00000007
#define _LATB_LATB7_MASK                         0x00000080
#define _LATB_LATB7_LENGTH                       0x00000001

#define _LATB_LATB8_POSITION                     0x00000008
#define _LATB_LATB8_MASK                         0x00000100
#define _LATB_LATB8_LENGTH                       0x00000001

#define _LATB_LATB9_POSITION                     0x00000009
#define _LATB_LATB9_MASK                         0x00000200
#define _LATB_LATB9_LENGTH                       0x00000001

#define _LATB_LATB10_POSITION                    0x0000000A
#define _LATB_LATB10_MASK                        0x00000400
#define _LATB_LATB10_LENGTH                      0x00000001

#define _LATB_LATB11_POSITION                    0x0000000B
#define _LATB_LATB11_MASK                        0x00000800
#define _LATB_LATB11_LENGTH                      0x00000001

#define _LATB_LATB12_POSITION                    0x0000000C
#define _LATB_LATB12_MASK                        0x00001000
#define _LATB_LATB12_LENGTH                      0x00000001

#define _LATB_LATB13_POSITION                    0x0000000D
#define _LATB_LATB13_MASK                        0x00002000
#define _LATB_LATB13_LENGTH                      0x00000001

#define _LATB_LATB14_POSITION                    0x0000000E
#define _LATB_LATB14_MASK                        0x00004000
#define _LATB_LATB14_LENGTH                      0x00000001

#define _LATB_LATB15_POSITION                    0x0000000F
#define _LATB_LATB15_MASK                        0x00008000
#define _LATB_LATB15_LENGTH                      0x00000001

#define _LATB_w_POSITION                         0x00000000
#define _LATB_w_MASK                             0xFFFFFFFF
#define _LATB_w_LENGTH                           0x00000020

#define _ODCB_ODCB0_POSITION                     0x00000000
#define _ODCB_ODCB0_MASK                         0x00000001
#define _ODCB_ODCB0_LENGTH                       0x00000001

#define _ODCB_ODCB1_POSITION                     0x00000001
#define _ODCB_ODCB1_MASK                         0x00000002
#define _ODCB_ODCB1_LENGTH                       0x00000001

#define _ODCB_ODCB2_POSITION                     0x00000002
#define _ODCB_ODCB2_MASK                         0x00000004
#define _ODCB_ODCB2_LENGTH                       0x00000001

#define _ODCB_ODCB3_POSITION                     0x00000003
#define _ODCB_ODCB3_MASK                         0x00000008
#define _ODCB_ODCB3_LENGTH                       0x00000001

#define _ODCB_ODCB4_POSITION                     0x00000004
#define _ODCB_ODCB4_MASK                         0x00000010
#define _ODCB_ODCB4_LENGTH                       0x00000001

#define _ODCB_ODCB5_POSITION                     0x00000005
#define _ODCB_ODCB5_MASK                         0x00000020
#define _ODCB_ODCB5_LENGTH                       0x00000001

#define _ODCB_ODCB6_POSITION                     0x00000006
#define _ODCB_ODCB6_MASK                         0x00000040
#define _ODCB_ODCB6_LENGTH                       0x00000001

#define _ODCB_ODCB7_POSITION                     0x00000007
#define _ODCB_ODCB7_MASK                         0x00000080
#define _ODCB_ODCB7_LENGTH                       0x00000001

#define _ODCB_ODCB8_POSITION                     0x00000008
#define _ODCB_ODCB8_MASK                         0x00000100
#define _ODCB_ODCB8_LENGTH                       0x00000001

#define _ODCB_ODCB9_POSITION                     0x00000009
#define _ODCB_ODCB9_MASK                         0x00000200
#define _ODCB_ODCB9_LENGTH                       0x00000001

#define _ODCB_ODCB10_POSITION                    0x0000000A
#define _ODCB_ODCB10_MASK                        0x00000400
#define _ODCB_ODCB10_LENGTH                      0x00000001

#define _ODCB_ODCB11_POSITION                    0x0000000B
#define _ODCB_ODCB11_MASK                        0x00000800
#define _ODCB_ODCB11_LENGTH                      0x00000001

#define _ODCB_ODCB12_POSITION                    0x0000000C
#define _ODCB_ODCB12_MASK                        0x00001000
#define _ODCB_ODCB12_LENGTH                      0x00000001

#define _ODCB_ODCB13_POSITION                    0x0000000D
#define _ODCB_ODCB13_MASK                        0x00002000
#define _ODCB_ODCB13_LENGTH                      0x00000001

#define _ODCB_ODCB14_POSITION                    0x0000000E
#define _ODCB_ODCB14_MASK                        0x00004000
#define _ODCB_ODCB14_LENGTH                      0x00000001

#define _ODCB_ODCB15_POSITION                    0x0000000F
#define _ODCB_ODCB15_MASK                        0x00008000
#define _ODCB_ODCB15_LENGTH                      0x00000001

#define _ODCB_w_POSITION                         0x00000000
#define _ODCB_w_MASK                             0xFFFFFFFF
#define _ODCB_w_LENGTH                           0x00000020

#define _TRISC_TRISC12_POSITION                  0x0000000C
#define _TRISC_TRISC12_MASK                      0x00001000
#define _TRISC_TRISC12_LENGTH                    0x00000001

#define _TRISC_TRISC13_POSITION                  0x0000000D
#define _TRISC_TRISC13_MASK                      0x00002000
#define _TRISC_TRISC13_LENGTH                    0x00000001

#define _TRISC_TRISC14_POSITION                  0x0000000E
#define _TRISC_TRISC14_MASK                      0x00004000
#define _TRISC_TRISC14_LENGTH                    0x00000001

#define _TRISC_TRISC15_POSITION                  0x0000000F
#define _TRISC_TRISC15_MASK                      0x00008000
#define _TRISC_TRISC15_LENGTH                    0x00000001

#define _TRISC_w_POSITION                        0x00000000
#define _TRISC_w_MASK                            0xFFFFFFFF
#define _TRISC_w_LENGTH                          0x00000020

#define _PORTC_RC12_POSITION                     0x0000000C
#define _PORTC_RC12_MASK                         0x00001000
#define _PORTC_RC12_LENGTH                       0x00000001

#define _PORTC_RC13_POSITION                     0x0000000D
#define _PORTC_RC13_MASK                         0x00002000
#define _PORTC_RC13_LENGTH                       0x00000001

#define _PORTC_RC14_POSITION                     0x0000000E
#define _PORTC_RC14_MASK                         0x00004000
#define _PORTC_RC14_LENGTH                       0x00000001

#define _PORTC_RC15_POSITION                     0x0000000F
#define _PORTC_RC15_MASK                         0x00008000
#define _PORTC_RC15_LENGTH                       0x00000001

#define _PORTC_w_POSITION                        0x00000000
#define _PORTC_w_MASK                            0xFFFFFFFF
#define _PORTC_w_LENGTH                          0x00000020

#define _LATC_LATC12_POSITION                    0x0000000C
#define _LATC_LATC12_MASK                        0x00001000
#define _LATC_LATC12_LENGTH                      0x00000001

#define _LATC_LATC13_POSITION                    0x0000000D
#define _LATC_LATC13_MASK                        0x00002000
#define _LATC_LATC13_LENGTH                      0x00000001

#define _LATC_LATC14_POSITION                    0x0000000E
#define _LATC_LATC14_MASK                        0x00004000
#define _LATC_LATC14_LENGTH                      0x00000001

#define _LATC_LATC15_POSITION                    0x0000000F
#define _LATC_LATC15_MASK                        0x00008000
#define _LATC_LATC15_LENGTH                      0x00000001

#define _LATC_w_POSITION                         0x00000000
#define _LATC_w_MASK                             0xFFFFFFFF
#define _LATC_w_LENGTH                           0x00000020

#define _ODCC_ODCC12_POSITION                    0x0000000C
#define _ODCC_ODCC12_MASK                        0x00001000
#define _ODCC_ODCC12_LENGTH                      0x00000001

#define _ODCC_ODCC13_POSITION                    0x0000000D
#define _ODCC_ODCC13_MASK                        0x00002000
#define _ODCC_ODCC13_LENGTH                      0x00000001

#define _ODCC_ODCC14_POSITION                    0x0000000E
#define _ODCC_ODCC14_MASK                        0x00004000
#define _ODCC_ODCC14_LENGTH                      0x00000001

#define _ODCC_ODCC15_POSITION                    0x0000000F
#define _ODCC_ODCC15_MASK                        0x00008000
#define _ODCC_ODCC15_LENGTH                      0x00000001

#define _ODCC_w_POSITION                         0x00000000
#define _ODCC_w_MASK                             0xFFFFFFFF
#define _ODCC_w_LENGTH                           0x00000020

#define _TRISD_TRISD0_POSITION                   0x00000000
#define _TRISD_TRISD0_MASK                       0x00000001
#define _TRISD_TRISD0_LENGTH                     0x00000001

#define _TRISD_TRISD1_POSITION                   0x00000001
#define _TRISD_TRISD1_MASK                       0x00000002
#define _TRISD_TRISD1_LENGTH                     0x00000001

#define _TRISD_TRISD2_POSITION                   0x00000002
#define _TRISD_TRISD2_MASK                       0x00000004
#define _TRISD_TRISD2_LENGTH                     0x00000001

#define _TRISD_TRISD3_POSITION                   0x00000003
#define _TRISD_TRISD3_MASK                       0x00000008
#define _TRISD_TRISD3_LENGTH                     0x00000001

#define _TRISD_TRISD4_POSITION                   0x00000004
#define _TRISD_TRISD4_MASK                       0x00000010
#define _TRISD_TRISD4_LENGTH                     0x00000001

#define _TRISD_TRISD5_POSITION                   0x00000005
#define _TRISD_TRISD5_MASK                       0x00000020
#define _TRISD_TRISD5_LENGTH                     0x00000001

#define _TRISD_TRISD6_POSITION                   0x00000006
#define _TRISD_TRISD6_MASK                       0x00000040
#define _TRISD_TRISD6_LENGTH                     0x00000001

#define _TRISD_TRISD7_POSITION                   0x00000007
#define _TRISD_TRISD7_MASK                       0x00000080
#define _TRISD_TRISD7_LENGTH                     0x00000001

#define _TRISD_TRISD8_POSITION                   0x00000008
#define _TRISD_TRISD8_MASK                       0x00000100
#define _TRISD_TRISD8_LENGTH                     0x00000001

#define _TRISD_TRISD9_POSITION                   0x00000009
#define _TRISD_TRISD9_MASK                       0x00000200
#define _TRISD_TRISD9_LENGTH                     0x00000001

#define _TRISD_TRISD10_POSITION                  0x0000000A
#define _TRISD_TRISD10_MASK                      0x00000400
#define _TRISD_TRISD10_LENGTH                    0x00000001

#define _TRISD_TRISD11_POSITION                  0x0000000B
#define _TRISD_TRISD11_MASK                      0x00000800
#define _TRISD_TRISD11_LENGTH                    0x00000001

#define _TRISD_w_POSITION                        0x00000000
#define _TRISD_w_MASK                            0xFFFFFFFF
#define _TRISD_w_LENGTH                          0x00000020

#define _PORTD_RD0_POSITION                      0x00000000
#define _PORTD_RD0_MASK                          0x00000001
#define _PORTD_RD0_LENGTH                        0x00000001

#define _PORTD_RD1_POSITION                      0x00000001
#define _PORTD_RD1_MASK                          0x00000002
#define _PORTD_RD1_LENGTH                        0x00000001

#define _PORTD_RD2_POSITION                      0x00000002
#define _PORTD_RD2_MASK                          0x00000004
#define _PORTD_RD2_LENGTH                        0x00000001

#define _PORTD_RD3_POSITION                      0x00000003
#define _PORTD_RD3_MASK                          0x00000008
#define _PORTD_RD3_LENGTH                        0x00000001

#define _PORTD_RD4_POSITION                      0x00000004
#define _PORTD_RD4_MASK                          0x00000010
#define _PORTD_RD4_LENGTH                        0x00000001

#define _PORTD_RD5_POSITION                      0x00000005
#define _PORTD_RD5_MASK                          0x00000020
#define _PORTD_RD5_LENGTH                        0x00000001

#define _PORTD_RD6_POSITION                      0x00000006
#define _PORTD_RD6_MASK                          0x00000040
#define _PORTD_RD6_LENGTH                        0x00000001

#define _PORTD_RD7_POSITION                      0x00000007
#define _PORTD_RD7_MASK                          0x00000080
#define _PORTD_RD7_LENGTH                        0x00000001

#define _PORTD_RD8_POSITION                      0x00000008
#define _PORTD_RD8_MASK                          0x00000100
#define _PORTD_RD8_LENGTH                        0x00000001

#define _PORTD_RD9_POSITION                      0x00000009
#define _PORTD_RD9_MASK                          0x00000200
#define _PORTD_RD9_LENGTH                        0x00000001

#define _PORTD_RD10_POSITION                     0x0000000A
#define _PORTD_RD10_MASK                         0x00000400
#define _PORTD_RD10_LENGTH                       0x00000001

#define _PORTD_RD11_POSITION                     0x0000000B
#define _PORTD_RD11_MASK                         0x00000800
#define _PORTD_RD11_LENGTH                       0x00000001

#define _PORTD_w_POSITION                        0x00000000
#define _PORTD_w_MASK                            0xFFFFFFFF
#define _PORTD_w_LENGTH                          0x00000020

#define _LATD_LATD0_POSITION                     0x00000000
#define _LATD_LATD0_MASK                         0x00000001
#define _LATD_LATD0_LENGTH                       0x00000001

#define _LATD_LATD1_POSITION                     0x00000001
#define _LATD_LATD1_MASK                         0x00000002
#define _LATD_LATD1_LENGTH                       0x00000001

#define _LATD_LATD2_POSITION                     0x00000002
#define _LATD_LATD2_MASK                         0x00000004
#define _LATD_LATD2_LENGTH                       0x00000001

#define _LATD_LATD3_POSITION                     0x00000003
#define _LATD_LATD3_MASK                         0x00000008
#define _LATD_LATD3_LENGTH                       0x00000001

#define _LATD_LATD4_POSITION                     0x00000004
#define _LATD_LATD4_MASK                         0x00000010
#define _LATD_LATD4_LENGTH                       0x00000001

#define _LATD_LATD5_POSITION                     0x00000005
#define _LATD_LATD5_MASK                         0x00000020
#define _LATD_LATD5_LENGTH                       0x00000001

#define _LATD_LATD6_POSITION                     0x00000006
#define _LATD_LATD6_MASK                         0x00000040
#define _LATD_LATD6_LENGTH                       0x00000001

#define _LATD_LATD7_POSITION                     0x00000007
#define _LATD_LATD7_MASK                         0x00000080
#define _LATD_LATD7_LENGTH                       0x00000001

#define _LATD_LATD8_POSITION                     0x00000008
#define _LATD_LATD8_MASK                         0x00000100
#define _LATD_LATD8_LENGTH                       0x00000001

#define _LATD_LATD9_POSITION                     0x00000009
#define _LATD_LATD9_MASK                         0x00000200
#define _LATD_LATD9_LENGTH                       0x00000001

#define _LATD_LATD10_POSITION                    0x0000000A
#define _LATD_LATD10_MASK                        0x00000400
#define _LATD_LATD10_LENGTH                      0x00000001

#define _LATD_LATD11_POSITION                    0x0000000B
#define _LATD_LATD11_MASK                        0x00000800
#define _LATD_LATD11_LENGTH                      0x00000001

#define _LATD_w_POSITION                         0x00000000
#define _LATD_w_MASK                             0xFFFFFFFF
#define _LATD_w_LENGTH                           0x00000020

#define _ODCD_ODCD0_POSITION                     0x00000000
#define _ODCD_ODCD0_MASK                         0x00000001
#define _ODCD_ODCD0_LENGTH                       0x00000001

#define _ODCD_ODCD1_POSITION                     0x00000001
#define _ODCD_ODCD1_MASK                         0x00000002
#define _ODCD_ODCD1_LENGTH                       0x00000001

#define _ODCD_ODCD2_POSITION                     0x00000002
#define _ODCD_ODCD2_MASK                         0x00000004
#define _ODCD_ODCD2_LENGTH                       0x00000001

#define _ODCD_ODCD3_POSITION                     0x00000003
#define _ODCD_ODCD3_MASK                         0x00000008
#define _ODCD_ODCD3_LENGTH                       0x00000001

#define _ODCD_ODCD4_POSITION                     0x00000004
#define _ODCD_ODCD4_MASK                         0x00000010
#define _ODCD_ODCD4_LENGTH                       0x00000001

#define _ODCD_ODCD5_POSITION                     0x00000005
#define _ODCD_ODCD5_MASK                         0x00000020
#define _ODCD_ODCD5_LENGTH                       0x00000001

#define _ODCD_ODCD6_POSITION                     0x00000006
#define _ODCD_ODCD6_MASK                         0x00000040
#define _ODCD_ODCD6_LENGTH                       0x00000001

#define _ODCD_ODCD7_POSITION                     0x00000007
#define _ODCD_ODCD7_MASK                         0x00000080
#define _ODCD_ODCD7_LENGTH                       0x00000001

#define _ODCD_ODCD8_POSITION                     0x00000008
#define _ODCD_ODCD8_MASK                         0x00000100
#define _ODCD_ODCD8_LENGTH                       0x00000001

#define _ODCD_ODCD9_POSITION                     0x00000009
#define _ODCD_ODCD9_MASK                         0x00000200
#define _ODCD_ODCD9_LENGTH                       0x00000001

#define _ODCD_ODCD10_POSITION                    0x0000000A
#define _ODCD_ODCD10_MASK                        0x00000400
#define _ODCD_ODCD10_LENGTH                      0x00000001

#define _ODCD_ODCD11_POSITION                    0x0000000B
#define _ODCD_ODCD11_MASK                        0x00000800
#define _ODCD_ODCD11_LENGTH                      0x00000001

#define _ODCD_w_POSITION                         0x00000000
#define _ODCD_w_MASK                             0xFFFFFFFF
#define _ODCD_w_LENGTH                           0x00000020

#define _TRISE_TRISE0_POSITION                   0x00000000
#define _TRISE_TRISE0_MASK                       0x00000001
#define _TRISE_TRISE0_LENGTH                     0x00000001

#define _TRISE_TRISE1_POSITION                   0x00000001
#define _TRISE_TRISE1_MASK                       0x00000002
#define _TRISE_TRISE1_LENGTH                     0x00000001

#define _TRISE_TRISE2_POSITION                   0x00000002
#define _TRISE_TRISE2_MASK                       0x00000004
#define _TRISE_TRISE2_LENGTH                     0x00000001

#define _TRISE_TRISE3_POSITION                   0x00000003
#define _TRISE_TRISE3_MASK                       0x00000008
#define _TRISE_TRISE3_LENGTH                     0x00000001

#define _TRISE_TRISE4_POSITION                   0x00000004
#define _TRISE_TRISE4_MASK                       0x00000010
#define _TRISE_TRISE4_LENGTH                     0x00000001

#define _TRISE_TRISE5_POSITION                   0x00000005
#define _TRISE_TRISE5_MASK                       0x00000020
#define _TRISE_TRISE5_LENGTH                     0x00000001

#define _TRISE_TRISE6_POSITION                   0x00000006
#define _TRISE_TRISE6_MASK                       0x00000040
#define _TRISE_TRISE6_LENGTH                     0x00000001

#define _TRISE_TRISE7_POSITION                   0x00000007
#define _TRISE_TRISE7_MASK                       0x00000080
#define _TRISE_TRISE7_LENGTH                     0x00000001

#define _TRISE_w_POSITION                        0x00000000
#define _TRISE_w_MASK                            0xFFFFFFFF
#define _TRISE_w_LENGTH                          0x00000020

#define _PORTE_RE0_POSITION                      0x00000000
#define _PORTE_RE0_MASK                          0x00000001
#define _PORTE_RE0_LENGTH                        0x00000001

#define _PORTE_RE1_POSITION                      0x00000001
#define _PORTE_RE1_MASK                          0x00000002
#define _PORTE_RE1_LENGTH                        0x00000001

#define _PORTE_RE2_POSITION                      0x00000002
#define _PORTE_RE2_MASK                          0x00000004
#define _PORTE_RE2_LENGTH                        0x00000001

#define _PORTE_RE3_POSITION                      0x00000003
#define _PORTE_RE3_MASK                          0x00000008
#define _PORTE_RE3_LENGTH                        0x00000001

#define _PORTE_RE4_POSITION                      0x00000004
#define _PORTE_RE4_MASK                          0x00000010
#define _PORTE_RE4_LENGTH                        0x00000001

#define _PORTE_RE5_POSITION                      0x00000005
#define _PORTE_RE5_MASK                          0x00000020
#define _PORTE_RE5_LENGTH                        0x00000001

#define _PORTE_RE6_POSITION                      0x00000006
#define _PORTE_RE6_MASK                          0x00000040
#define _PORTE_RE6_LENGTH                        0x00000001

#define _PORTE_RE7_POSITION                      0x00000007
#define _PORTE_RE7_MASK                          0x00000080
#define _PORTE_RE7_LENGTH                        0x00000001

#define _PORTE_w_POSITION                        0x00000000
#define _PORTE_w_MASK                            0xFFFFFFFF
#define _PORTE_w_LENGTH                          0x00000020

#define _LATE_LATE0_POSITION                     0x00000000
#define _LATE_LATE0_MASK                         0x00000001
#define _LATE_LATE0_LENGTH                       0x00000001

#define _LATE_LATE1_POSITION                     0x00000001
#define _LATE_LATE1_MASK                         0x00000002
#define _LATE_LATE1_LENGTH                       0x00000001

#define _LATE_LATE2_POSITION                     0x00000002
#define _LATE_LATE2_MASK                         0x00000004
#define _LATE_LATE2_LENGTH                       0x00000001

#define _LATE_LATE3_POSITION                     0x00000003
#define _LATE_LATE3_MASK                         0x00000008
#define _LATE_LATE3_LENGTH                       0x00000001

#define _LATE_LATE4_POSITION                     0x00000004
#define _LATE_LATE4_MASK                         0x00000010
#define _LATE_LATE4_LENGTH                       0x00000001

#define _LATE_LATE5_POSITION                     0x00000005
#define _LATE_LATE5_MASK                         0x00000020
#define _LATE_LATE5_LENGTH                       0x00000001

#define _LATE_LATE6_POSITION                     0x00000006
#define _LATE_LATE6_MASK                         0x00000040
#define _LATE_LATE6_LENGTH                       0x00000001

#define _LATE_LATE7_POSITION                     0x00000007
#define _LATE_LATE7_MASK                         0x00000080
#define _LATE_LATE7_LENGTH                       0x00000001

#define _LATE_w_POSITION                         0x00000000
#define _LATE_w_MASK                             0xFFFFFFFF
#define _LATE_w_LENGTH                           0x00000020

#define _ODCE_ODCE0_POSITION                     0x00000000
#define _ODCE_ODCE0_MASK                         0x00000001
#define _ODCE_ODCE0_LENGTH                       0x00000001

#define _ODCE_ODCE1_POSITION                     0x00000001
#define _ODCE_ODCE1_MASK                         0x00000002
#define _ODCE_ODCE1_LENGTH                       0x00000001

#define _ODCE_ODCE2_POSITION                     0x00000002
#define _ODCE_ODCE2_MASK                         0x00000004
#define _ODCE_ODCE2_LENGTH                       0x00000001

#define _ODCE_ODCE3_POSITION                     0x00000003
#define _ODCE_ODCE3_MASK                         0x00000008
#define _ODCE_ODCE3_LENGTH                       0x00000001

#define _ODCE_ODCE4_POSITION                     0x00000004
#define _ODCE_ODCE4_MASK                         0x00000010
#define _ODCE_ODCE4_LENGTH                       0x00000001

#define _ODCE_ODCE5_POSITION                     0x00000005
#define _ODCE_ODCE5_MASK                         0x00000020
#define _ODCE_ODCE5_LENGTH                       0x00000001

#define _ODCE_ODCE6_POSITION                     0x00000006
#define _ODCE_ODCE6_MASK                         0x00000040
#define _ODCE_ODCE6_LENGTH                       0x00000001

#define _ODCE_ODCE7_POSITION                     0x00000007
#define _ODCE_ODCE7_MASK                         0x00000080
#define _ODCE_ODCE7_LENGTH                       0x00000001

#define _ODCE_w_POSITION                         0x00000000
#define _ODCE_w_MASK                             0xFFFFFFFF
#define _ODCE_w_LENGTH                           0x00000020

#define _TRISF_TRISF0_POSITION                   0x00000000
#define _TRISF_TRISF0_MASK                       0x00000001
#define _TRISF_TRISF0_LENGTH                     0x00000001

#define _TRISF_TRISF1_POSITION                   0x00000001
#define _TRISF_TRISF1_MASK                       0x00000002
#define _TRISF_TRISF1_LENGTH                     0x00000001

#define _TRISF_TRISF2_POSITION                   0x00000002
#define _TRISF_TRISF2_MASK                       0x00000004
#define _TRISF_TRISF2_LENGTH                     0x00000001

#define _TRISF_TRISF3_POSITION                   0x00000003
#define _TRISF_TRISF3_MASK                       0x00000008
#define _TRISF_TRISF3_LENGTH                     0x00000001

#define _TRISF_TRISF4_POSITION                   0x00000004
#define _TRISF_TRISF4_MASK                       0x00000010
#define _TRISF_TRISF4_LENGTH                     0x00000001

#define _TRISF_TRISF5_POSITION                   0x00000005
#define _TRISF_TRISF5_MASK                       0x00000020
#define _TRISF_TRISF5_LENGTH                     0x00000001

#define _TRISF_w_POSITION                        0x00000000
#define _TRISF_w_MASK                            0xFFFFFFFF
#define _TRISF_w_LENGTH                          0x00000020

#define _PORTF_RF0_POSITION                      0x00000000
#define _PORTF_RF0_MASK                          0x00000001
#define _PORTF_RF0_LENGTH                        0x00000001

#define _PORTF_RF1_POSITION                      0x00000001
#define _PORTF_RF1_MASK                          0x00000002
#define _PORTF_RF1_LENGTH                        0x00000001

#define _PORTF_RF2_POSITION                      0x00000002
#define _PORTF_RF2_MASK                          0x00000004
#define _PORTF_RF2_LENGTH                        0x00000001

#define _PORTF_RF3_POSITION                      0x00000003
#define _PORTF_RF3_MASK                          0x00000008
#define _PORTF_RF3_LENGTH                        0x00000001

#define _PORTF_RF4_POSITION                      0x00000004
#define _PORTF_RF4_MASK                          0x00000010
#define _PORTF_RF4_LENGTH                        0x00000001

#define _PORTF_RF5_POSITION                      0x00000005
#define _PORTF_RF5_MASK                          0x00000020
#define _PORTF_RF5_LENGTH                        0x00000001

#define _PORTF_w_POSITION                        0x00000000
#define _PORTF_w_MASK                            0xFFFFFFFF
#define _PORTF_w_LENGTH                          0x00000020

#define _LATF_LATF0_POSITION                     0x00000000
#define _LATF_LATF0_MASK                         0x00000001
#define _LATF_LATF0_LENGTH                       0x00000001

#define _LATF_LATF1_POSITION                     0x00000001
#define _LATF_LATF1_MASK                         0x00000002
#define _LATF_LATF1_LENGTH                       0x00000001

#define _LATF_LATF2_POSITION                     0x00000002
#define _LATF_LATF2_MASK                         0x00000004
#define _LATF_LATF2_LENGTH                       0x00000001

#define _LATF_LATF3_POSITION                     0x00000003
#define _LATF_LATF3_MASK                         0x00000008
#define _LATF_LATF3_LENGTH                       0x00000001

#define _LATF_LATF4_POSITION                     0x00000004
#define _LATF_LATF4_MASK                         0x00000010
#define _LATF_LATF4_LENGTH                       0x00000001

#define _LATF_LATF5_POSITION                     0x00000005
#define _LATF_LATF5_MASK                         0x00000020
#define _LATF_LATF5_LENGTH                       0x00000001

#define _LATF_w_POSITION                         0x00000000
#define _LATF_w_MASK                             0xFFFFFFFF
#define _LATF_w_LENGTH                           0x00000020

#define _ODCF_ODCF0_POSITION                     0x00000000
#define _ODCF_ODCF0_MASK                         0x00000001
#define _ODCF_ODCF0_LENGTH                       0x00000001

#define _ODCF_ODCF1_POSITION                     0x00000001
#define _ODCF_ODCF1_MASK                         0x00000002
#define _ODCF_ODCF1_LENGTH                       0x00000001

#define _ODCF_ODCF2_POSITION                     0x00000002
#define _ODCF_ODCF2_MASK                         0x00000004
#define _ODCF_ODCF2_LENGTH                       0x00000001

#define _ODCF_ODCF3_POSITION                     0x00000003
#define _ODCF_ODCF3_MASK                         0x00000008
#define _ODCF_ODCF3_LENGTH                       0x00000001

#define _ODCF_ODCF4_POSITION                     0x00000004
#define _ODCF_ODCF4_MASK                         0x00000010
#define _ODCF_ODCF4_LENGTH                       0x00000001

#define _ODCF_ODCF5_POSITION                     0x00000005
#define _ODCF_ODCF5_MASK                         0x00000020
#define _ODCF_ODCF5_LENGTH                       0x00000001

#define _ODCF_w_POSITION                         0x00000000
#define _ODCF_w_MASK                             0xFFFFFFFF
#define _ODCF_w_LENGTH                           0x00000020

#define _TRISG_TRISG2_POSITION                   0x00000002
#define _TRISG_TRISG2_MASK                       0x00000004
#define _TRISG_TRISG2_LENGTH                     0x00000001

#define _TRISG_TRISG3_POSITION                   0x00000003
#define _TRISG_TRISG3_MASK                       0x00000008
#define _TRISG_TRISG3_LENGTH                     0x00000001

#define _TRISG_TRISG6_POSITION                   0x00000006
#define _TRISG_TRISG6_MASK                       0x00000040
#define _TRISG_TRISG6_LENGTH                     0x00000001

#define _TRISG_TRISG7_POSITION                   0x00000007
#define _TRISG_TRISG7_MASK                       0x00000080
#define _TRISG_TRISG7_LENGTH                     0x00000001

#define _TRISG_TRISG8_POSITION                   0x00000008
#define _TRISG_TRISG8_MASK                       0x00000100
#define _TRISG_TRISG8_LENGTH                     0x00000001

#define _TRISG_TRISG9_POSITION                   0x00000009
#define _TRISG_TRISG9_MASK                       0x00000200
#define _TRISG_TRISG9_LENGTH                     0x00000001

#define _TRISG_w_POSITION                        0x00000000
#define _TRISG_w_MASK                            0xFFFFFFFF
#define _TRISG_w_LENGTH                          0x00000020

#define _PORTG_RG2_POSITION                      0x00000002
#define _PORTG_RG2_MASK                          0x00000004
#define _PORTG_RG2_LENGTH                        0x00000001

#define _PORTG_RG3_POSITION                      0x00000003
#define _PORTG_RG3_MASK                          0x00000008
#define _PORTG_RG3_LENGTH                        0x00000001

#define _PORTG_RG6_POSITION                      0x00000006
#define _PORTG_RG6_MASK                          0x00000040
#define _PORTG_RG6_LENGTH                        0x00000001

#define _PORTG_RG7_POSITION                      0x00000007
#define _PORTG_RG7_MASK                          0x00000080
#define _PORTG_RG7_LENGTH                        0x00000001

#define _PORTG_RG8_POSITION                      0x00000008
#define _PORTG_RG8_MASK                          0x00000100
#define _PORTG_RG8_LENGTH                        0x00000001

#define _PORTG_RG9_POSITION                      0x00000009
#define _PORTG_RG9_MASK                          0x00000200
#define _PORTG_RG9_LENGTH                        0x00000001

#define _PORTG_w_POSITION                        0x00000000
#define _PORTG_w_MASK                            0xFFFFFFFF
#define _PORTG_w_LENGTH                          0x00000020

#define _LATG_LATG2_POSITION                     0x00000002
#define _LATG_LATG2_MASK                         0x00000004
#define _LATG_LATG2_LENGTH                       0x00000001

#define _LATG_LATG3_POSITION                     0x00000003
#define _LATG_LATG3_MASK                         0x00000008
#define _LATG_LATG3_LENGTH                       0x00000001

#define _LATG_LATG6_POSITION                     0x00000006
#define _LATG_LATG6_MASK                         0x00000040
#define _LATG_LATG6_LENGTH                       0x00000001

#define _LATG_LATG7_POSITION                     0x00000007
#define _LATG_LATG7_MASK                         0x00000080
#define _LATG_LATG7_LENGTH                       0x00000001

#define _LATG_LATG8_POSITION                     0x00000008
#define _LATG_LATG8_MASK                         0x00000100
#define _LATG_LATG8_LENGTH                       0x00000001

#define _LATG_LATG9_POSITION                     0x00000009
#define _LATG_LATG9_MASK                         0x00000200
#define _LATG_LATG9_LENGTH                       0x00000001

#define _LATG_w_POSITION                         0x00000000
#define _LATG_w_MASK                             0xFFFFFFFF
#define _LATG_w_LENGTH                           0x00000020

#define _ODCG_ODCG2_POSITION                     0x00000002
#define _ODCG_ODCG2_MASK                         0x00000004
#define _ODCG_ODCG2_LENGTH                       0x00000001

#define _ODCG_ODCG3_POSITION                     0x00000003
#define _ODCG_ODCG3_MASK                         0x00000008
#define _ODCG_ODCG3_LENGTH                       0x00000001

#define _ODCG_ODCG6_POSITION                     0x00000006
#define _ODCG_ODCG6_MASK                         0x00000040
#define _ODCG_ODCG6_LENGTH                       0x00000001

#define _ODCG_ODCG7_POSITION                     0x00000007
#define _ODCG_ODCG7_MASK                         0x00000080
#define _ODCG_ODCG7_LENGTH                       0x00000001

#define _ODCG_ODCG8_POSITION                     0x00000008
#define _ODCG_ODCG8_MASK                         0x00000100
#define _ODCG_ODCG8_LENGTH                       0x00000001

#define _ODCG_ODCG9_POSITION                     0x00000009
#define _ODCG_ODCG9_MASK                         0x00000200
#define _ODCG_ODCG9_LENGTH                       0x00000001

#define _ODCG_w_POSITION                         0x00000000
#define _ODCG_w_MASK                             0xFFFFFFFF
#define _ODCG_w_LENGTH                           0x00000020

#define _CNCON_SIDL_POSITION                     0x0000000D
#define _CNCON_SIDL_MASK                         0x00002000
#define _CNCON_SIDL_LENGTH                       0x00000001

#define _CNCON_ON_POSITION                       0x0000000F
#define _CNCON_ON_MASK                           0x00008000
#define _CNCON_ON_LENGTH                         0x00000001

#define _CNCON_w_POSITION                        0x00000000
#define _CNCON_w_MASK                            0xFFFFFFFF
#define _CNCON_w_LENGTH                          0x00000020

#define _CNEN_CNEN0_POSITION                     0x00000000
#define _CNEN_CNEN0_MASK                         0x00000001
#define _CNEN_CNEN0_LENGTH                       0x00000001

#define _CNEN_CNEN1_POSITION                     0x00000001
#define _CNEN_CNEN1_MASK                         0x00000002
#define _CNEN_CNEN1_LENGTH                       0x00000001

#define _CNEN_CNEN2_POSITION                     0x00000002
#define _CNEN_CNEN2_MASK                         0x00000004
#define _CNEN_CNEN2_LENGTH                       0x00000001

#define _CNEN_CNEN3_POSITION                     0x00000003
#define _CNEN_CNEN3_MASK                         0x00000008
#define _CNEN_CNEN3_LENGTH                       0x00000001

#define _CNEN_CNEN4_POSITION                     0x00000004
#define _CNEN_CNEN4_MASK                         0x00000010
#define _CNEN_CNEN4_LENGTH                       0x00000001

#define _CNEN_CNEN5_POSITION                     0x00000005
#define _CNEN_CNEN5_MASK                         0x00000020
#define _CNEN_CNEN5_LENGTH                       0x00000001

#define _CNEN_CNEN6_POSITION                     0x00000006
#define _CNEN_CNEN6_MASK                         0x00000040
#define _CNEN_CNEN6_LENGTH                       0x00000001

#define _CNEN_CNEN7_POSITION                     0x00000007
#define _CNEN_CNEN7_MASK                         0x00000080
#define _CNEN_CNEN7_LENGTH                       0x00000001

#define _CNEN_CNEN8_POSITION                     0x00000008
#define _CNEN_CNEN8_MASK                         0x00000100
#define _CNEN_CNEN8_LENGTH                       0x00000001

#define _CNEN_CNEN9_POSITION                     0x00000009
#define _CNEN_CNEN9_MASK                         0x00000200
#define _CNEN_CNEN9_LENGTH                       0x00000001

#define _CNEN_CNEN10_POSITION                    0x0000000A
#define _CNEN_CNEN10_MASK                        0x00000400
#define _CNEN_CNEN10_LENGTH                      0x00000001

#define _CNEN_CNEN11_POSITION                    0x0000000B
#define _CNEN_CNEN11_MASK                        0x00000800
#define _CNEN_CNEN11_LENGTH                      0x00000001

#define _CNEN_CNEN12_POSITION                    0x0000000C
#define _CNEN_CNEN12_MASK                        0x00001000
#define _CNEN_CNEN12_LENGTH                      0x00000001

#define _CNEN_CNEN13_POSITION                    0x0000000D
#define _CNEN_CNEN13_MASK                        0x00002000
#define _CNEN_CNEN13_LENGTH                      0x00000001

#define _CNEN_CNEN14_POSITION                    0x0000000E
#define _CNEN_CNEN14_MASK                        0x00004000
#define _CNEN_CNEN14_LENGTH                      0x00000001

#define _CNEN_CNEN15_POSITION                    0x0000000F
#define _CNEN_CNEN15_MASK                        0x00008000
#define _CNEN_CNEN15_LENGTH                      0x00000001

#define _CNEN_CNEN16_POSITION                    0x00000010
#define _CNEN_CNEN16_MASK                        0x00010000
#define _CNEN_CNEN16_LENGTH                      0x00000001

#define _CNEN_CNEN17_POSITION                    0x00000011
#define _CNEN_CNEN17_MASK                        0x00020000
#define _CNEN_CNEN17_LENGTH                      0x00000001

#define _CNEN_CNEN18_POSITION                    0x00000012
#define _CNEN_CNEN18_MASK                        0x00040000
#define _CNEN_CNEN18_LENGTH                      0x00000001

#define _CNEN_w_POSITION                         0x00000000
#define _CNEN_w_MASK                             0xFFFFFFFF
#define _CNEN_w_LENGTH                           0x00000020

#define _CNPUE_CNPUE0_POSITION                   0x00000000
#define _CNPUE_CNPUE0_MASK                       0x00000001
#define _CNPUE_CNPUE0_LENGTH                     0x00000001

#define _CNPUE_CNPUE1_POSITION                   0x00000001
#define _CNPUE_CNPUE1_MASK                       0x00000002
#define _CNPUE_CNPUE1_LENGTH                     0x00000001

#define _CNPUE_CNPUE2_POSITION                   0x00000002
#define _CNPUE_CNPUE2_MASK                       0x00000004
#define _CNPUE_CNPUE2_LENGTH                     0x00000001

#define _CNPUE_CNPUE3_POSITION                   0x00000003
#define _CNPUE_CNPUE3_MASK                       0x00000008
#define _CNPUE_CNPUE3_LENGTH                     0x00000001

#define _CNPUE_CNPUE4_POSITION                   0x00000004
#define _CNPUE_CNPUE4_MASK                       0x00000010
#define _CNPUE_CNPUE4_LENGTH                     0x00000001

#define _CNPUE_CNPUE5_POSITION                   0x00000005
#define _CNPUE_CNPUE5_MASK                       0x00000020
#define _CNPUE_CNPUE5_LENGTH                     0x00000001

#define _CNPUE_CNPUE6_POSITION                   0x00000006
#define _CNPUE_CNPUE6_MASK                       0x00000040
#define _CNPUE_CNPUE6_LENGTH                     0x00000001

#define _CNPUE_CNPUE7_POSITION                   0x00000007
#define _CNPUE_CNPUE7_MASK                       0x00000080
#define _CNPUE_CNPUE7_LENGTH                     0x00000001

#define _CNPUE_CNPUE8_POSITION                   0x00000008
#define _CNPUE_CNPUE8_MASK                       0x00000100
#define _CNPUE_CNPUE8_LENGTH                     0x00000001

#define _CNPUE_CNPUE9_POSITION                   0x00000009
#define _CNPUE_CNPUE9_MASK                       0x00000200
#define _CNPUE_CNPUE9_LENGTH                     0x00000001

#define _CNPUE_CNPUE10_POSITION                  0x0000000A
#define _CNPUE_CNPUE10_MASK                      0x00000400
#define _CNPUE_CNPUE10_LENGTH                    0x00000001

#define _CNPUE_CNPUE11_POSITION                  0x0000000B
#define _CNPUE_CNPUE11_MASK                      0x00000800
#define _CNPUE_CNPUE11_LENGTH                    0x00000001

#define _CNPUE_CNPUE12_POSITION                  0x0000000C
#define _CNPUE_CNPUE12_MASK                      0x00001000
#define _CNPUE_CNPUE12_LENGTH                    0x00000001

#define _CNPUE_CNPUE13_POSITION                  0x0000000D
#define _CNPUE_CNPUE13_MASK                      0x00002000
#define _CNPUE_CNPUE13_LENGTH                    0x00000001

#define _CNPUE_CNPUE14_POSITION                  0x0000000E
#define _CNPUE_CNPUE14_MASK                      0x00004000
#define _CNPUE_CNPUE14_LENGTH                    0x00000001

#define _CNPUE_CNPUE15_POSITION                  0x0000000F
#define _CNPUE_CNPUE15_MASK                      0x00008000
#define _CNPUE_CNPUE15_LENGTH                    0x00000001

#define _CNPUE_CNPUE16_POSITION                  0x00000010
#define _CNPUE_CNPUE16_MASK                      0x00010000
#define _CNPUE_CNPUE16_LENGTH                    0x00000001

#define _CNPUE_CNPUE17_POSITION                  0x00000011
#define _CNPUE_CNPUE17_MASK                      0x00020000
#define _CNPUE_CNPUE17_LENGTH                    0x00000001

#define _CNPUE_CNPUE18_POSITION                  0x00000012
#define _CNPUE_CNPUE18_MASK                      0x00040000
#define _CNPUE_CNPUE18_LENGTH                    0x00000001

#define _CNPUE_w_POSITION                        0x00000000
#define _CNPUE_w_MASK                            0xFFFFFFFF
#define _CNPUE_w_LENGTH                          0x00000020

#define _ETHCON1_BUFCDEC_POSITION                0x00000000
#define _ETHCON1_BUFCDEC_MASK                    0x00000001
#define _ETHCON1_BUFCDEC_LENGTH                  0x00000001

#define _ETHCON1_MANFC_POSITION                  0x00000004
#define _ETHCON1_MANFC_MASK                      0x00000010
#define _ETHCON1_MANFC_LENGTH                    0x00000001

#define _ETHCON1_AUTOFC_POSITION                 0x00000007
#define _ETHCON1_AUTOFC_MASK                     0x00000080
#define _ETHCON1_AUTOFC_LENGTH                   0x00000001

#define _ETHCON1_RXEN_POSITION                   0x00000008
#define _ETHCON1_RXEN_MASK                       0x00000100
#define _ETHCON1_RXEN_LENGTH                     0x00000001

#define _ETHCON1_TXRTS_POSITION                  0x00000009
#define _ETHCON1_TXRTS_MASK                      0x00000200
#define _ETHCON1_TXRTS_LENGTH                    0x00000001

#define _ETHCON1_SIDL_POSITION                   0x0000000D
#define _ETHCON1_SIDL_MASK                       0x00002000
#define _ETHCON1_SIDL_LENGTH                     0x00000001

#define _ETHCON1_ON_POSITION                     0x0000000F
#define _ETHCON1_ON_MASK                         0x00008000
#define _ETHCON1_ON_LENGTH                       0x00000001

#define _ETHCON1_PTV_POSITION                    0x00000010
#define _ETHCON1_PTV_MASK                        0xFFFF0000
#define _ETHCON1_PTV_LENGTH                      0x00000010

#define _ETHCON1_w_POSITION                      0x00000000
#define _ETHCON1_w_MASK                          0xFFFFFFFF
#define _ETHCON1_w_LENGTH                        0x00000020

#define _ETHCON2_RXBUF_SZ_POSITION               0x00000004
#define _ETHCON2_RXBUF_SZ_MASK                   0x000007F0
#define _ETHCON2_RXBUF_SZ_LENGTH                 0x00000007

#define _ETHCON2_w_POSITION                      0x00000000
#define _ETHCON2_w_MASK                          0xFFFFFFFF
#define _ETHCON2_w_LENGTH                        0x00000020

#define _ETHTXST_TXSTADDR_POSITION               0x00000002
#define _ETHTXST_TXSTADDR_MASK                   0xFFFFFFFC
#define _ETHTXST_TXSTADDR_LENGTH                 0x0000001E

#define _ETHTXST_w_POSITION                      0x00000000
#define _ETHTXST_w_MASK                          0xFFFFFFFF
#define _ETHTXST_w_LENGTH                        0x00000020

#define _ETHRXST_RXSTADDR_POSITION               0x00000002
#define _ETHRXST_RXSTADDR_MASK                   0xFFFFFFFC
#define _ETHRXST_RXSTADDR_LENGTH                 0x0000001E

#define _ETHRXST_w_POSITION                      0x00000000
#define _ETHRXST_w_MASK                          0xFFFFFFFF
#define _ETHRXST_w_LENGTH                        0x00000020

#define _ETHPMCS_PMCS_POSITION                   0x00000000
#define _ETHPMCS_PMCS_MASK                       0x0000FFFF
#define _ETHPMCS_PMCS_LENGTH                     0x00000010

#define _ETHPMCS_w_POSITION                      0x00000000
#define _ETHPMCS_w_MASK                          0xFFFFFFFF
#define _ETHPMCS_w_LENGTH                        0x00000020

#define _ETHPMO_PMO_POSITION                     0x00000000
#define _ETHPMO_PMO_MASK                         0x0000FFFF
#define _ETHPMO_PMO_LENGTH                       0x00000010

#define _ETHPMO_w_POSITION                       0x00000000
#define _ETHPMO_w_MASK                           0xFFFFFFFF
#define _ETHPMO_w_LENGTH                         0x00000020

#define _ETHRXFC_BCEN_POSITION                   0x00000000
#define _ETHRXFC_BCEN_MASK                       0x00000001
#define _ETHRXFC_BCEN_LENGTH                     0x00000001

#define _ETHRXFC_MCEN_POSITION                   0x00000001
#define _ETHRXFC_MCEN_MASK                       0x00000002
#define _ETHRXFC_MCEN_LENGTH                     0x00000001

#define _ETHRXFC_NOTMEEN_POSITION                0x00000002
#define _ETHRXFC_NOTMEEN_MASK                    0x00000004
#define _ETHRXFC_NOTMEEN_LENGTH                  0x00000001

#define _ETHRXFC_UCEN_POSITION                   0x00000003
#define _ETHRXFC_UCEN_MASK                       0x00000008
#define _ETHRXFC_UCEN_LENGTH                     0x00000001

#define _ETHRXFC_RUNTEN_POSITION                 0x00000004
#define _ETHRXFC_RUNTEN_MASK                     0x00000010
#define _ETHRXFC_RUNTEN_LENGTH                   0x00000001

#define _ETHRXFC_RUNTERREN_POSITION              0x00000005
#define _ETHRXFC_RUNTERREN_MASK                  0x00000020
#define _ETHRXFC_RUNTERREN_LENGTH                0x00000001

#define _ETHRXFC_CRCOKEN_POSITION                0x00000006
#define _ETHRXFC_CRCOKEN_MASK                    0x00000040
#define _ETHRXFC_CRCOKEN_LENGTH                  0x00000001

#define _ETHRXFC_CRCERREN_POSITION               0x00000007
#define _ETHRXFC_CRCERREN_MASK                   0x00000080
#define _ETHRXFC_CRCERREN_LENGTH                 0x00000001

#define _ETHRXFC_PMMODE_POSITION                 0x00000008
#define _ETHRXFC_PMMODE_MASK                     0x00000F00
#define _ETHRXFC_PMMODE_LENGTH                   0x00000004

#define _ETHRXFC_NOTPM_POSITION                  0x0000000C
#define _ETHRXFC_NOTPM_MASK                      0x00001000
#define _ETHRXFC_NOTPM_LENGTH                    0x00000001

#define _ETHRXFC_MPEN_POSITION                   0x0000000E
#define _ETHRXFC_MPEN_MASK                       0x00004000
#define _ETHRXFC_MPEN_LENGTH                     0x00000001

#define _ETHRXFC_HTEN_POSITION                   0x0000000F
#define _ETHRXFC_HTEN_MASK                       0x00008000
#define _ETHRXFC_HTEN_LENGTH                     0x00000001

#define _ETHRXFC_w_POSITION                      0x00000000
#define _ETHRXFC_w_MASK                          0xFFFFFFFF
#define _ETHRXFC_w_LENGTH                        0x00000020

#define _ETHRXWM_RXEWM_POSITION                  0x00000000
#define _ETHRXWM_RXEWM_MASK                      0x000000FF
#define _ETHRXWM_RXEWM_LENGTH                    0x00000008

#define _ETHRXWM_RXFWM_POSITION                  0x00000010
#define _ETHRXWM_RXFWM_MASK                      0x00FF0000
#define _ETHRXWM_RXFWM_LENGTH                    0x00000008

#define _ETHRXWM_w_POSITION                      0x00000000
#define _ETHRXWM_w_MASK                          0xFFFFFFFF
#define _ETHRXWM_w_LENGTH                        0x00000020

#define _ETHIEN_RXOVFLWIE_POSITION               0x00000000
#define _ETHIEN_RXOVFLWIE_MASK                   0x00000001
#define _ETHIEN_RXOVFLWIE_LENGTH                 0x00000001

#define _ETHIEN_RXBUFNAIE_POSITION               0x00000001
#define _ETHIEN_RXBUFNAIE_MASK                   0x00000002
#define _ETHIEN_RXBUFNAIE_LENGTH                 0x00000001

#define _ETHIEN_TXABORTIE_POSITION               0x00000002
#define _ETHIEN_TXABORTIE_MASK                   0x00000004
#define _ETHIEN_TXABORTIE_LENGTH                 0x00000001

#define _ETHIEN_TXDONEIE_POSITION                0x00000003
#define _ETHIEN_TXDONEIE_MASK                    0x00000008
#define _ETHIEN_TXDONEIE_LENGTH                  0x00000001

#define _ETHIEN_RXACTIE_POSITION                 0x00000005
#define _ETHIEN_RXACTIE_MASK                     0x00000020
#define _ETHIEN_RXACTIE_LENGTH                   0x00000001

#define _ETHIEN_PKTPENDIE_POSITION               0x00000006
#define _ETHIEN_PKTPENDIE_MASK                   0x00000040
#define _ETHIEN_PKTPENDIE_LENGTH                 0x00000001

#define _ETHIEN_RXDONEIE_POSITION                0x00000007
#define _ETHIEN_RXDONEIE_MASK                    0x00000080
#define _ETHIEN_RXDONEIE_LENGTH                  0x00000001

#define _ETHIEN_FWMARKIE_POSITION                0x00000008
#define _ETHIEN_FWMARKIE_MASK                    0x00000100
#define _ETHIEN_FWMARKIE_LENGTH                  0x00000001

#define _ETHIEN_EWMARKIE_POSITION                0x00000009
#define _ETHIEN_EWMARKIE_MASK                    0x00000200
#define _ETHIEN_EWMARKIE_LENGTH                  0x00000001

#define _ETHIEN_RXBUSEIE_POSITION                0x0000000D
#define _ETHIEN_RXBUSEIE_MASK                    0x00002000
#define _ETHIEN_RXBUSEIE_LENGTH                  0x00000001

#define _ETHIEN_TXBUSEIE_POSITION                0x0000000E
#define _ETHIEN_TXBUSEIE_MASK                    0x00004000
#define _ETHIEN_TXBUSEIE_LENGTH                  0x00000001

#define _ETHIEN_w_POSITION                       0x00000000
#define _ETHIEN_w_MASK                           0xFFFFFFFF
#define _ETHIEN_w_LENGTH                         0x00000020

#define _ETHIRQ_RXOVFLW_POSITION                 0x00000000
#define _ETHIRQ_RXOVFLW_MASK                     0x00000001
#define _ETHIRQ_RXOVFLW_LENGTH                   0x00000001

#define _ETHIRQ_RXBUFNA_POSITION                 0x00000001
#define _ETHIRQ_RXBUFNA_MASK                     0x00000002
#define _ETHIRQ_RXBUFNA_LENGTH                   0x00000001

#define _ETHIRQ_TXABORT_POSITION                 0x00000002
#define _ETHIRQ_TXABORT_MASK                     0x00000004
#define _ETHIRQ_TXABORT_LENGTH                   0x00000001

#define _ETHIRQ_TXDONE_POSITION                  0x00000003
#define _ETHIRQ_TXDONE_MASK                      0x00000008
#define _ETHIRQ_TXDONE_LENGTH                    0x00000001

#define _ETHIRQ_RXACT_POSITION                   0x00000005
#define _ETHIRQ_RXACT_MASK                       0x00000020
#define _ETHIRQ_RXACT_LENGTH                     0x00000001

#define _ETHIRQ_PKTPEND_POSITION                 0x00000006
#define _ETHIRQ_PKTPEND_MASK                     0x00000040
#define _ETHIRQ_PKTPEND_LENGTH                   0x00000001

#define _ETHIRQ_RXDONE_POSITION                  0x00000007
#define _ETHIRQ_RXDONE_MASK                      0x00000080
#define _ETHIRQ_RXDONE_LENGTH                    0x00000001

#define _ETHIRQ_FWMARK_POSITION                  0x00000008
#define _ETHIRQ_FWMARK_MASK                      0x00000100
#define _ETHIRQ_FWMARK_LENGTH                    0x00000001

#define _ETHIRQ_EWMARK_POSITION                  0x00000009
#define _ETHIRQ_EWMARK_MASK                      0x00000200
#define _ETHIRQ_EWMARK_LENGTH                    0x00000001

#define _ETHIRQ_RXBUSE_POSITION                  0x0000000D
#define _ETHIRQ_RXBUSE_MASK                      0x00002000
#define _ETHIRQ_RXBUSE_LENGTH                    0x00000001

#define _ETHIRQ_TXBUSE_POSITION                  0x0000000E
#define _ETHIRQ_TXBUSE_MASK                      0x00004000
#define _ETHIRQ_TXBUSE_LENGTH                    0x00000001

#define _ETHIRQ_w_POSITION                       0x00000000
#define _ETHIRQ_w_MASK                           0xFFFFFFFF
#define _ETHIRQ_w_LENGTH                         0x00000020

#define _ETHSTAT_RXBUSY_POSITION                 0x00000005
#define _ETHSTAT_RXBUSY_MASK                     0x00000020
#define _ETHSTAT_RXBUSY_LENGTH                   0x00000001

#define _ETHSTAT_TXBUSY_POSITION                 0x00000006
#define _ETHSTAT_TXBUSY_MASK                     0x00000040
#define _ETHSTAT_TXBUSY_LENGTH                   0x00000001

#define _ETHSTAT_BUSY_POSITION                   0x00000007
#define _ETHSTAT_BUSY_MASK                       0x00000080
#define _ETHSTAT_BUSY_LENGTH                     0x00000001

#define _ETHSTAT_BUFCNT_POSITION                 0x00000010
#define _ETHSTAT_BUFCNT_MASK                     0x00FF0000
#define _ETHSTAT_BUFCNT_LENGTH                   0x00000008

#define _ETHSTAT_ETHBUSY_POSITION                0x00000007
#define _ETHSTAT_ETHBUSY_MASK                    0x00000080
#define _ETHSTAT_ETHBUSY_LENGTH                  0x00000001

#define _ETHSTAT_w_POSITION                      0x00000000
#define _ETHSTAT_w_MASK                          0xFFFFFFFF
#define _ETHSTAT_w_LENGTH                        0x00000020

#define _ETHRXOVFLOW_RXOVFLWCNT_POSITION         0x00000000
#define _ETHRXOVFLOW_RXOVFLWCNT_MASK             0x0000FFFF
#define _ETHRXOVFLOW_RXOVFLWCNT_LENGTH           0x00000010

#define _ETHRXOVFLOW_w_POSITION                  0x00000000
#define _ETHRXOVFLOW_w_MASK                      0xFFFFFFFF
#define _ETHRXOVFLOW_w_LENGTH                    0x00000020

#define _ETHFRMTXOK_FRMTXOKCNT_POSITION          0x00000000
#define _ETHFRMTXOK_FRMTXOKCNT_MASK              0x0000FFFF
#define _ETHFRMTXOK_FRMTXOKCNT_LENGTH            0x00000010

#define _ETHFRMTXOK_w_POSITION                   0x00000000
#define _ETHFRMTXOK_w_MASK                       0xFFFFFFFF
#define _ETHFRMTXOK_w_LENGTH                     0x00000020

#define _ETHSCOLFRM_SCOLFRMCNT_POSITION          0x00000000
#define _ETHSCOLFRM_SCOLFRMCNT_MASK              0x0000FFFF
#define _ETHSCOLFRM_SCOLFRMCNT_LENGTH            0x00000010

#define _ETHSCOLFRM_w_POSITION                   0x00000000
#define _ETHSCOLFRM_w_MASK                       0xFFFFFFFF
#define _ETHSCOLFRM_w_LENGTH                     0x00000020

#define _ETHMCOLFRM_MCOLFRMCNT_POSITION          0x00000000
#define _ETHMCOLFRM_MCOLFRMCNT_MASK              0x0000FFFF
#define _ETHMCOLFRM_MCOLFRMCNT_LENGTH            0x00000010

#define _ETHMCOLFRM_MCOLFRM_CNT_POSITION         0x00000000
#define _ETHMCOLFRM_MCOLFRM_CNT_MASK             0x0000FFFF
#define _ETHMCOLFRM_MCOLFRM_CNT_LENGTH           0x00000010

#define _ETHMCOLFRM_w_POSITION                   0x00000000
#define _ETHMCOLFRM_w_MASK                       0xFFFFFFFF
#define _ETHMCOLFRM_w_LENGTH                     0x00000020

#define _ETHFRMRXOK_FRMRXOKCNT_POSITION          0x00000000
#define _ETHFRMRXOK_FRMRXOKCNT_MASK              0x0000FFFF
#define _ETHFRMRXOK_FRMRXOKCNT_LENGTH            0x00000010

#define _ETHFRMRXOK_w_POSITION                   0x00000000
#define _ETHFRMRXOK_w_MASK                       0xFFFFFFFF
#define _ETHFRMRXOK_w_LENGTH                     0x00000020

#define _ETHFCSERR_FCSERRCNT_POSITION            0x00000000
#define _ETHFCSERR_FCSERRCNT_MASK                0x0000FFFF
#define _ETHFCSERR_FCSERRCNT_LENGTH              0x00000010

#define _ETHFCSERR_w_POSITION                    0x00000000
#define _ETHFCSERR_w_MASK                        0xFFFFFFFF
#define _ETHFCSERR_w_LENGTH                      0x00000020

#define _ETHALGNERR_ALGNERRCNT_POSITION          0x00000000
#define _ETHALGNERR_ALGNERRCNT_MASK              0x0000FFFF
#define _ETHALGNERR_ALGNERRCNT_LENGTH            0x00000010

#define _ETHALGNERR_w_POSITION                   0x00000000
#define _ETHALGNERR_w_MASK                       0xFFFFFFFF
#define _ETHALGNERR_w_LENGTH                     0x00000020

#define _EMAC1CFG1_RXENABLE_POSITION             0x00000000
#define _EMAC1CFG1_RXENABLE_MASK                 0x00000001
#define _EMAC1CFG1_RXENABLE_LENGTH               0x00000001

#define _EMAC1CFG1_PASSALL_POSITION              0x00000001
#define _EMAC1CFG1_PASSALL_MASK                  0x00000002
#define _EMAC1CFG1_PASSALL_LENGTH                0x00000001

#define _EMAC1CFG1_RXPAUSE_POSITION              0x00000002
#define _EMAC1CFG1_RXPAUSE_MASK                  0x00000004
#define _EMAC1CFG1_RXPAUSE_LENGTH                0x00000001

#define _EMAC1CFG1_TXPAUSE_POSITION              0x00000003
#define _EMAC1CFG1_TXPAUSE_MASK                  0x00000008
#define _EMAC1CFG1_TXPAUSE_LENGTH                0x00000001

#define _EMAC1CFG1_LOOPBACK_POSITION             0x00000004
#define _EMAC1CFG1_LOOPBACK_MASK                 0x00000010
#define _EMAC1CFG1_LOOPBACK_LENGTH               0x00000001

#define _EMAC1CFG1_RESETTFUN_POSITION            0x00000008
#define _EMAC1CFG1_RESETTFUN_MASK                0x00000100
#define _EMAC1CFG1_RESETTFUN_LENGTH              0x00000001

#define _EMAC1CFG1_RESETTMCS_POSITION            0x00000009
#define _EMAC1CFG1_RESETTMCS_MASK                0x00000200
#define _EMAC1CFG1_RESETTMCS_LENGTH              0x00000001

#define _EMAC1CFG1_RESETRFUN_POSITION            0x0000000A
#define _EMAC1CFG1_RESETRFUN_MASK                0x00000400
#define _EMAC1CFG1_RESETRFUN_LENGTH              0x00000001

#define _EMAC1CFG1_RESETRMCS_POSITION            0x0000000B
#define _EMAC1CFG1_RESETRMCS_MASK                0x00000800
#define _EMAC1CFG1_RESETRMCS_LENGTH              0x00000001

#define _EMAC1CFG1_SIMRESET_POSITION             0x0000000E
#define _EMAC1CFG1_SIMRESET_MASK                 0x00004000
#define _EMAC1CFG1_SIMRESET_LENGTH               0x00000001

#define _EMAC1CFG1_SOFTRESET_POSITION            0x0000000F
#define _EMAC1CFG1_SOFTRESET_MASK                0x00008000
#define _EMAC1CFG1_SOFTRESET_LENGTH              0x00000001

#define _EMAC1CFG1_w_POSITION                    0x00000000
#define _EMAC1CFG1_w_MASK                        0xFFFFFFFF
#define _EMAC1CFG1_w_LENGTH                      0x00000020

#define _EMACxCFG1_RXENABLE_POSITION             0x00000000
#define _EMACxCFG1_RXENABLE_MASK                 0x00000001
#define _EMACxCFG1_RXENABLE_LENGTH               0x00000001

#define _EMACxCFG1_PASSALL_POSITION              0x00000001
#define _EMACxCFG1_PASSALL_MASK                  0x00000002
#define _EMACxCFG1_PASSALL_LENGTH                0x00000001

#define _EMACxCFG1_RXPAUSE_POSITION              0x00000002
#define _EMACxCFG1_RXPAUSE_MASK                  0x00000004
#define _EMACxCFG1_RXPAUSE_LENGTH                0x00000001

#define _EMACxCFG1_TXPAUSE_POSITION              0x00000003
#define _EMACxCFG1_TXPAUSE_MASK                  0x00000008
#define _EMACxCFG1_TXPAUSE_LENGTH                0x00000001

#define _EMACxCFG1_LOOPBACK_POSITION             0x00000004
#define _EMACxCFG1_LOOPBACK_MASK                 0x00000010
#define _EMACxCFG1_LOOPBACK_LENGTH               0x00000001

#define _EMACxCFG1_RESETTFUN_POSITION            0x00000008
#define _EMACxCFG1_RESETTFUN_MASK                0x00000100
#define _EMACxCFG1_RESETTFUN_LENGTH              0x00000001

#define _EMACxCFG1_RESETTMCS_POSITION            0x00000009
#define _EMACxCFG1_RESETTMCS_MASK                0x00000200
#define _EMACxCFG1_RESETTMCS_LENGTH              0x00000001

#define _EMACxCFG1_RESETRFUN_POSITION            0x0000000A
#define _EMACxCFG1_RESETRFUN_MASK                0x00000400
#define _EMACxCFG1_RESETRFUN_LENGTH              0x00000001

#define _EMACxCFG1_RESETRMCS_POSITION            0x0000000B
#define _EMACxCFG1_RESETRMCS_MASK                0x00000800
#define _EMACxCFG1_RESETRMCS_LENGTH              0x00000001

#define _EMACxCFG1_SIMRESET_POSITION             0x0000000E
#define _EMACxCFG1_SIMRESET_MASK                 0x00004000
#define _EMACxCFG1_SIMRESET_LENGTH               0x00000001

#define _EMACxCFG1_SOFTRESET_POSITION            0x0000000F
#define _EMACxCFG1_SOFTRESET_MASK                0x00008000
#define _EMACxCFG1_SOFTRESET_LENGTH              0x00000001

#define _EMACxCFG1_w_POSITION                    0x00000000
#define _EMACxCFG1_w_MASK                        0xFFFFFFFF
#define _EMACxCFG1_w_LENGTH                      0x00000020

#define _EMAC1CFG2_FULLDPLX_POSITION             0x00000000
#define _EMAC1CFG2_FULLDPLX_MASK                 0x00000001
#define _EMAC1CFG2_FULLDPLX_LENGTH               0x00000001

#define _EMAC1CFG2_LENGTHCK_POSITION             0x00000001
#define _EMAC1CFG2_LENGTHCK_MASK                 0x00000002
#define _EMAC1CFG2_LENGTHCK_LENGTH               0x00000001

#define _EMAC1CFG2_HUGEFRM_POSITION              0x00000002
#define _EMAC1CFG2_HUGEFRM_MASK                  0x00000004
#define _EMAC1CFG2_HUGEFRM_LENGTH                0x00000001

#define _EMAC1CFG2_DELAYCRC_POSITION             0x00000003
#define _EMAC1CFG2_DELAYCRC_MASK                 0x00000008
#define _EMAC1CFG2_DELAYCRC_LENGTH               0x00000001

#define _EMAC1CFG2_CRCENABLE_POSITION            0x00000004
#define _EMAC1CFG2_CRCENABLE_MASK                0x00000010
#define _EMAC1CFG2_CRCENABLE_LENGTH              0x00000001

#define _EMAC1CFG2_PADENABLE_POSITION            0x00000005
#define _EMAC1CFG2_PADENABLE_MASK                0x00000020
#define _EMAC1CFG2_PADENABLE_LENGTH              0x00000001

#define _EMAC1CFG2_VLANPAD_POSITION              0x00000006
#define _EMAC1CFG2_VLANPAD_MASK                  0x00000040
#define _EMAC1CFG2_VLANPAD_LENGTH                0x00000001

#define _EMAC1CFG2_AUTOPAD_POSITION              0x00000007
#define _EMAC1CFG2_AUTOPAD_MASK                  0x00000080
#define _EMAC1CFG2_AUTOPAD_LENGTH                0x00000001

#define _EMAC1CFG2_PUREPRE_POSITION              0x00000008
#define _EMAC1CFG2_PUREPRE_MASK                  0x00000100
#define _EMAC1CFG2_PUREPRE_LENGTH                0x00000001

#define _EMAC1CFG2_LONGPRE_POSITION              0x00000009
#define _EMAC1CFG2_LONGPRE_MASK                  0x00000200
#define _EMAC1CFG2_LONGPRE_LENGTH                0x00000001

#define _EMAC1CFG2_NOBKOFF_POSITION              0x0000000C
#define _EMAC1CFG2_NOBKOFF_MASK                  0x00001000
#define _EMAC1CFG2_NOBKOFF_LENGTH                0x00000001

#define _EMAC1CFG2_BPNOBKOFF_POSITION            0x0000000D
#define _EMAC1CFG2_BPNOBKOFF_MASK                0x00002000
#define _EMAC1CFG2_BPNOBKOFF_LENGTH              0x00000001

#define _EMAC1CFG2_EXCESSDFR_POSITION            0x0000000E
#define _EMAC1CFG2_EXCESSDFR_MASK                0x00004000
#define _EMAC1CFG2_EXCESSDFR_LENGTH              0x00000001

#define _EMAC1CFG2_w_POSITION                    0x00000000
#define _EMAC1CFG2_w_MASK                        0xFFFFFFFF
#define _EMAC1CFG2_w_LENGTH                      0x00000020

#define _EMACxCFG2_FULLDPLX_POSITION             0x00000000
#define _EMACxCFG2_FULLDPLX_MASK                 0x00000001
#define _EMACxCFG2_FULLDPLX_LENGTH               0x00000001

#define _EMACxCFG2_LENGTHCK_POSITION             0x00000001
#define _EMACxCFG2_LENGTHCK_MASK                 0x00000002
#define _EMACxCFG2_LENGTHCK_LENGTH               0x00000001

#define _EMACxCFG2_HUGEFRM_POSITION              0x00000002
#define _EMACxCFG2_HUGEFRM_MASK                  0x00000004
#define _EMACxCFG2_HUGEFRM_LENGTH                0x00000001

#define _EMACxCFG2_DELAYCRC_POSITION             0x00000003
#define _EMACxCFG2_DELAYCRC_MASK                 0x00000008
#define _EMACxCFG2_DELAYCRC_LENGTH               0x00000001

#define _EMACxCFG2_CRCENABLE_POSITION            0x00000004
#define _EMACxCFG2_CRCENABLE_MASK                0x00000010
#define _EMACxCFG2_CRCENABLE_LENGTH              0x00000001

#define _EMACxCFG2_PADENABLE_POSITION            0x00000005
#define _EMACxCFG2_PADENABLE_MASK                0x00000020
#define _EMACxCFG2_PADENABLE_LENGTH              0x00000001

#define _EMACxCFG2_VLANPAD_POSITION              0x00000006
#define _EMACxCFG2_VLANPAD_MASK                  0x00000040
#define _EMACxCFG2_VLANPAD_LENGTH                0x00000001

#define _EMACxCFG2_AUTOPAD_POSITION              0x00000007
#define _EMACxCFG2_AUTOPAD_MASK                  0x00000080
#define _EMACxCFG2_AUTOPAD_LENGTH                0x00000001

#define _EMACxCFG2_PUREPRE_POSITION              0x00000008
#define _EMACxCFG2_PUREPRE_MASK                  0x00000100
#define _EMACxCFG2_PUREPRE_LENGTH                0x00000001

#define _EMACxCFG2_LONGPRE_POSITION              0x00000009
#define _EMACxCFG2_LONGPRE_MASK                  0x00000200
#define _EMACxCFG2_LONGPRE_LENGTH                0x00000001

#define _EMACxCFG2_NOBKOFF_POSITION              0x0000000C
#define _EMACxCFG2_NOBKOFF_MASK                  0x00001000
#define _EMACxCFG2_NOBKOFF_LENGTH                0x00000001

#define _EMACxCFG2_BPNOBKOFF_POSITION            0x0000000D
#define _EMACxCFG2_BPNOBKOFF_MASK                0x00002000
#define _EMACxCFG2_BPNOBKOFF_LENGTH              0x00000001

#define _EMACxCFG2_EXCESSDFR_POSITION            0x0000000E
#define _EMACxCFG2_EXCESSDFR_MASK                0x00004000
#define _EMACxCFG2_EXCESSDFR_LENGTH              0x00000001

#define _EMACxCFG2_w_POSITION                    0x00000000
#define _EMACxCFG2_w_MASK                        0xFFFFFFFF
#define _EMACxCFG2_w_LENGTH                      0x00000020

#define _EMAC1IPGT_B2BIPKTGP_POSITION            0x00000000
#define _EMAC1IPGT_B2BIPKTGP_MASK                0x0000007F
#define _EMAC1IPGT_B2BIPKTGP_LENGTH              0x00000007

#define _EMAC1IPGT_w_POSITION                    0x00000000
#define _EMAC1IPGT_w_MASK                        0xFFFFFFFF
#define _EMAC1IPGT_w_LENGTH                      0x00000020

#define _EMACxIPGT_B2BIPKTGP_POSITION            0x00000000
#define _EMACxIPGT_B2BIPKTGP_MASK                0x0000007F
#define _EMACxIPGT_B2BIPKTGP_LENGTH              0x00000007

#define _EMACxIPGT_w_POSITION                    0x00000000
#define _EMACxIPGT_w_MASK                        0xFFFFFFFF
#define _EMACxIPGT_w_LENGTH                      0x00000020

#define _EMAC1IPGR_NB2BIPKTGP2_POSITION          0x00000000
#define _EMAC1IPGR_NB2BIPKTGP2_MASK              0x0000007F
#define _EMAC1IPGR_NB2BIPKTGP2_LENGTH            0x00000007

#define _EMAC1IPGR_NB2BIPKTGP1_POSITION          0x00000008
#define _EMAC1IPGR_NB2BIPKTGP1_MASK              0x00007F00
#define _EMAC1IPGR_NB2BIPKTGP1_LENGTH            0x00000007

#define _EMAC1IPGR_w_POSITION                    0x00000000
#define _EMAC1IPGR_w_MASK                        0xFFFFFFFF
#define _EMAC1IPGR_w_LENGTH                      0x00000020

#define _EMACxIPGR_NB2BIPKTGP2_POSITION          0x00000000
#define _EMACxIPGR_NB2BIPKTGP2_MASK              0x0000007F
#define _EMACxIPGR_NB2BIPKTGP2_LENGTH            0x00000007

#define _EMACxIPGR_NB2BIPKTGP1_POSITION          0x00000008
#define _EMACxIPGR_NB2BIPKTGP1_MASK              0x00007F00
#define _EMACxIPGR_NB2BIPKTGP1_LENGTH            0x00000007

#define _EMACxIPGR_w_POSITION                    0x00000000
#define _EMACxIPGR_w_MASK                        0xFFFFFFFF
#define _EMACxIPGR_w_LENGTH                      0x00000020

#define _EMAC1CLRT_RETX_POSITION                 0x00000000
#define _EMAC1CLRT_RETX_MASK                     0x0000000F
#define _EMAC1CLRT_RETX_LENGTH                   0x00000004

#define _EMAC1CLRT_CWINDOW_POSITION              0x00000008
#define _EMAC1CLRT_CWINDOW_MASK                  0x00003F00
#define _EMAC1CLRT_CWINDOW_LENGTH                0x00000006

#define _EMAC1CLRT_w_POSITION                    0x00000000
#define _EMAC1CLRT_w_MASK                        0xFFFFFFFF
#define _EMAC1CLRT_w_LENGTH                      0x00000020

#define _EMACxCLRT_RETX_POSITION                 0x00000000
#define _EMACxCLRT_RETX_MASK                     0x0000000F
#define _EMACxCLRT_RETX_LENGTH                   0x00000004

#define _EMACxCLRT_CWINDOW_POSITION              0x00000008
#define _EMACxCLRT_CWINDOW_MASK                  0x00003F00
#define _EMACxCLRT_CWINDOW_LENGTH                0x00000006

#define _EMACxCLRT_w_POSITION                    0x00000000
#define _EMACxCLRT_w_MASK                        0xFFFFFFFF
#define _EMACxCLRT_w_LENGTH                      0x00000020

#define _EMAC1MAXF_MACMAXF_POSITION              0x00000000
#define _EMAC1MAXF_MACMAXF_MASK                  0x0000FFFF
#define _EMAC1MAXF_MACMAXF_LENGTH                0x00000010

#define _EMAC1MAXF_w_POSITION                    0x00000000
#define _EMAC1MAXF_w_MASK                        0xFFFFFFFF
#define _EMAC1MAXF_w_LENGTH                      0x00000020

#define _EMACxMAXF_MACMAXF_POSITION              0x00000000
#define _EMACxMAXF_MACMAXF_MASK                  0x0000FFFF
#define _EMACxMAXF_MACMAXF_LENGTH                0x00000010

#define _EMACxMAXF_w_POSITION                    0x00000000
#define _EMACxMAXF_w_MASK                        0xFFFFFFFF
#define _EMACxMAXF_w_LENGTH                      0x00000020

#define _EMAC1SUPP_SPEEDRMII_POSITION            0x00000008
#define _EMAC1SUPP_SPEEDRMII_MASK                0x00000100
#define _EMAC1SUPP_SPEEDRMII_LENGTH              0x00000001

#define _EMAC1SUPP_RESETRMII_POSITION            0x0000000B
#define _EMAC1SUPP_RESETRMII_MASK                0x00000800
#define _EMAC1SUPP_RESETRMII_LENGTH              0x00000001

#define _EMAC1SUPP_w_POSITION                    0x00000000
#define _EMAC1SUPP_w_MASK                        0xFFFFFFFF
#define _EMAC1SUPP_w_LENGTH                      0x00000020

#define _EMACxSUPP_SPEEDRMII_POSITION            0x00000008
#define _EMACxSUPP_SPEEDRMII_MASK                0x00000100
#define _EMACxSUPP_SPEEDRMII_LENGTH              0x00000001

#define _EMACxSUPP_RESETRMII_POSITION            0x0000000B
#define _EMACxSUPP_RESETRMII_MASK                0x00000800
#define _EMACxSUPP_RESETRMII_LENGTH              0x00000001

#define _EMACxSUPP_w_POSITION                    0x00000000
#define _EMACxSUPP_w_MASK                        0xFFFFFFFF
#define _EMACxSUPP_w_LENGTH                      0x00000020

#define _EMAC1TEST_SHRTQNTA_POSITION             0x00000000
#define _EMAC1TEST_SHRTQNTA_MASK                 0x00000001
#define _EMAC1TEST_SHRTQNTA_LENGTH               0x00000001

#define _EMAC1TEST_TESTPAUSE_POSITION            0x00000001
#define _EMAC1TEST_TESTPAUSE_MASK                0x00000002
#define _EMAC1TEST_TESTPAUSE_LENGTH              0x00000001

#define _EMAC1TEST_TESTBP_POSITION               0x00000002
#define _EMAC1TEST_TESTBP_MASK                   0x00000004
#define _EMAC1TEST_TESTBP_LENGTH                 0x00000001

#define _EMAC1TEST_w_POSITION                    0x00000000
#define _EMAC1TEST_w_MASK                        0xFFFFFFFF
#define _EMAC1TEST_w_LENGTH                      0x00000020

#define _EMACxTEST_SHRTQNTA_POSITION             0x00000000
#define _EMACxTEST_SHRTQNTA_MASK                 0x00000001
#define _EMACxTEST_SHRTQNTA_LENGTH               0x00000001

#define _EMACxTEST_TESTPAUSE_POSITION            0x00000001
#define _EMACxTEST_TESTPAUSE_MASK                0x00000002
#define _EMACxTEST_TESTPAUSE_LENGTH              0x00000001

#define _EMACxTEST_TESTBP_POSITION               0x00000002
#define _EMACxTEST_TESTBP_MASK                   0x00000004
#define _EMACxTEST_TESTBP_LENGTH                 0x00000001

#define _EMACxTEST_w_POSITION                    0x00000000
#define _EMACxTEST_w_MASK                        0xFFFFFFFF
#define _EMACxTEST_w_LENGTH                      0x00000020

#define _EMAC1MCFG_SCANINC_POSITION              0x00000000
#define _EMAC1MCFG_SCANINC_MASK                  0x00000001
#define _EMAC1MCFG_SCANINC_LENGTH                0x00000001

#define _EMAC1MCFG_NOPRE_POSITION                0x00000001
#define _EMAC1MCFG_NOPRE_MASK                    0x00000002
#define _EMAC1MCFG_NOPRE_LENGTH                  0x00000001

#define _EMAC1MCFG_CLKSEL_POSITION               0x00000002
#define _EMAC1MCFG_CLKSEL_MASK                   0x0000003C
#define _EMAC1MCFG_CLKSEL_LENGTH                 0x00000004

#define _EMAC1MCFG_RESETMGMT_POSITION            0x0000000F
#define _EMAC1MCFG_RESETMGMT_MASK                0x00008000
#define _EMAC1MCFG_RESETMGMT_LENGTH              0x00000001

#define _EMAC1MCFG_w_POSITION                    0x00000000
#define _EMAC1MCFG_w_MASK                        0xFFFFFFFF
#define _EMAC1MCFG_w_LENGTH                      0x00000020

#define _EMACxMCFG_SCANINC_POSITION              0x00000000
#define _EMACxMCFG_SCANINC_MASK                  0x00000001
#define _EMACxMCFG_SCANINC_LENGTH                0x00000001

#define _EMACxMCFG_NOPRE_POSITION                0x00000001
#define _EMACxMCFG_NOPRE_MASK                    0x00000002
#define _EMACxMCFG_NOPRE_LENGTH                  0x00000001

#define _EMACxMCFG_CLKSEL_POSITION               0x00000002
#define _EMACxMCFG_CLKSEL_MASK                   0x0000003C
#define _EMACxMCFG_CLKSEL_LENGTH                 0x00000004

#define _EMACxMCFG_RESETMGMT_POSITION            0x0000000F
#define _EMACxMCFG_RESETMGMT_MASK                0x00008000
#define _EMACxMCFG_RESETMGMT_LENGTH              0x00000001

#define _EMACxMCFG_w_POSITION                    0x00000000
#define _EMACxMCFG_w_MASK                        0xFFFFFFFF
#define _EMACxMCFG_w_LENGTH                      0x00000020

#define _EMAC1MCMD_READ_POSITION                 0x00000000
#define _EMAC1MCMD_READ_MASK                     0x00000001
#define _EMAC1MCMD_READ_LENGTH                   0x00000001

#define _EMAC1MCMD_SCAN_POSITION                 0x00000001
#define _EMAC1MCMD_SCAN_MASK                     0x00000002
#define _EMAC1MCMD_SCAN_LENGTH                   0x00000001

#define _EMAC1MCMD_w_POSITION                    0x00000000
#define _EMAC1MCMD_w_MASK                        0xFFFFFFFF
#define _EMAC1MCMD_w_LENGTH                      0x00000020

#define _EMACxMCMD_READ_POSITION                 0x00000000
#define _EMACxMCMD_READ_MASK                     0x00000001
#define _EMACxMCMD_READ_LENGTH                   0x00000001

#define _EMACxMCMD_SCAN_POSITION                 0x00000001
#define _EMACxMCMD_SCAN_MASK                     0x00000002
#define _EMACxMCMD_SCAN_LENGTH                   0x00000001

#define _EMACxMCMD_w_POSITION                    0x00000000
#define _EMACxMCMD_w_MASK                        0xFFFFFFFF
#define _EMACxMCMD_w_LENGTH                      0x00000020

#define _EMAC1MADR_REGADDR_POSITION              0x00000000
#define _EMAC1MADR_REGADDR_MASK                  0x0000001F
#define _EMAC1MADR_REGADDR_LENGTH                0x00000005

#define _EMAC1MADR_PHYADDR_POSITION              0x00000008
#define _EMAC1MADR_PHYADDR_MASK                  0x00001F00
#define _EMAC1MADR_PHYADDR_LENGTH                0x00000005

#define _EMAC1MADR_w_POSITION                    0x00000000
#define _EMAC1MADR_w_MASK                        0xFFFFFFFF
#define _EMAC1MADR_w_LENGTH                      0x00000020

#define _EMACxMADR_REGADDR_POSITION              0x00000000
#define _EMACxMADR_REGADDR_MASK                  0x0000001F
#define _EMACxMADR_REGADDR_LENGTH                0x00000005

#define _EMACxMADR_PHYADDR_POSITION              0x00000008
#define _EMACxMADR_PHYADDR_MASK                  0x00001F00
#define _EMACxMADR_PHYADDR_LENGTH                0x00000005

#define _EMACxMADR_w_POSITION                    0x00000000
#define _EMACxMADR_w_MASK                        0xFFFFFFFF
#define _EMACxMADR_w_LENGTH                      0x00000020

#define _EMAC1MWTD_MWTD_POSITION                 0x00000000
#define _EMAC1MWTD_MWTD_MASK                     0x0000FFFF
#define _EMAC1MWTD_MWTD_LENGTH                   0x00000010

#define _EMAC1MWTD_w_POSITION                    0x00000000
#define _EMAC1MWTD_w_MASK                        0xFFFFFFFF
#define _EMAC1MWTD_w_LENGTH                      0x00000020

#define _EMACxMWTD_MWTD_POSITION                 0x00000000
#define _EMACxMWTD_MWTD_MASK                     0x0000FFFF
#define _EMACxMWTD_MWTD_LENGTH                   0x00000010

#define _EMACxMWTD_w_POSITION                    0x00000000
#define _EMACxMWTD_w_MASK                        0xFFFFFFFF
#define _EMACxMWTD_w_LENGTH                      0x00000020

#define _EMAC1MRDD_MRDD_POSITION                 0x00000000
#define _EMAC1MRDD_MRDD_MASK                     0x0000FFFF
#define _EMAC1MRDD_MRDD_LENGTH                   0x00000010

#define _EMAC1MRDD_w_POSITION                    0x00000000
#define _EMAC1MRDD_w_MASK                        0xFFFFFFFF
#define _EMAC1MRDD_w_LENGTH                      0x00000020

#define _EMACxMRDD_MRDD_POSITION                 0x00000000
#define _EMACxMRDD_MRDD_MASK                     0x0000FFFF
#define _EMACxMRDD_MRDD_LENGTH                   0x00000010

#define _EMACxMRDD_w_POSITION                    0x00000000
#define _EMACxMRDD_w_MASK                        0xFFFFFFFF
#define _EMACxMRDD_w_LENGTH                      0x00000020

#define _EMAC1MIND_MIIMBUSY_POSITION             0x00000000
#define _EMAC1MIND_MIIMBUSY_MASK                 0x00000001
#define _EMAC1MIND_MIIMBUSY_LENGTH               0x00000001

#define _EMAC1MIND_SCAN_POSITION                 0x00000001
#define _EMAC1MIND_SCAN_MASK                     0x00000002
#define _EMAC1MIND_SCAN_LENGTH                   0x00000001

#define _EMAC1MIND_NOTVALID_POSITION             0x00000002
#define _EMAC1MIND_NOTVALID_MASK                 0x00000004
#define _EMAC1MIND_NOTVALID_LENGTH               0x00000001

#define _EMAC1MIND_LINKFAIL_POSITION             0x00000003
#define _EMAC1MIND_LINKFAIL_MASK                 0x00000008
#define _EMAC1MIND_LINKFAIL_LENGTH               0x00000001

#define _EMAC1MIND_w_POSITION                    0x00000000
#define _EMAC1MIND_w_MASK                        0xFFFFFFFF
#define _EMAC1MIND_w_LENGTH                      0x00000020

#define _EMACxMIND_MIIMBUSY_POSITION             0x00000000
#define _EMACxMIND_MIIMBUSY_MASK                 0x00000001
#define _EMACxMIND_MIIMBUSY_LENGTH               0x00000001

#define _EMACxMIND_SCAN_POSITION                 0x00000001
#define _EMACxMIND_SCAN_MASK                     0x00000002
#define _EMACxMIND_SCAN_LENGTH                   0x00000001

#define _EMACxMIND_NOTVALID_POSITION             0x00000002
#define _EMACxMIND_NOTVALID_MASK                 0x00000004
#define _EMACxMIND_NOTVALID_LENGTH               0x00000001

#define _EMACxMIND_LINKFAIL_POSITION             0x00000003
#define _EMACxMIND_LINKFAIL_MASK                 0x00000008
#define _EMACxMIND_LINKFAIL_LENGTH               0x00000001

#define _EMACxMIND_w_POSITION                    0x00000000
#define _EMACxMIND_w_MASK                        0xFFFFFFFF
#define _EMACxMIND_w_LENGTH                      0x00000020

#define _EMAC1SA0_STNADDR5_POSITION              0x00000000
#define _EMAC1SA0_STNADDR5_MASK                  0x000000FF
#define _EMAC1SA0_STNADDR5_LENGTH                0x00000008

#define _EMAC1SA0_STNADDR6_POSITION              0x00000008
#define _EMAC1SA0_STNADDR6_MASK                  0x0000FF00
#define _EMAC1SA0_STNADDR6_LENGTH                0x00000008

#define _EMAC1SA0_w_POSITION                     0x00000000
#define _EMAC1SA0_w_MASK                         0xFFFFFFFF
#define _EMAC1SA0_w_LENGTH                       0x00000020

#define _EMACxSA0_STNADDR5_POSITION              0x00000000
#define _EMACxSA0_STNADDR5_MASK                  0x000000FF
#define _EMACxSA0_STNADDR5_LENGTH                0x00000008

#define _EMACxSA0_STNADDR6_POSITION              0x00000008
#define _EMACxSA0_STNADDR6_MASK                  0x0000FF00
#define _EMACxSA0_STNADDR6_LENGTH                0x00000008

#define _EMACxSA0_w_POSITION                     0x00000000
#define _EMACxSA0_w_MASK                         0xFFFFFFFF
#define _EMACxSA0_w_LENGTH                       0x00000020

#define _EMAC1SA1_STNADDR3_POSITION              0x00000000
#define _EMAC1SA1_STNADDR3_MASK                  0x000000FF
#define _EMAC1SA1_STNADDR3_LENGTH                0x00000008

#define _EMAC1SA1_STNADDR4_POSITION              0x00000008
#define _EMAC1SA1_STNADDR4_MASK                  0x0000FF00
#define _EMAC1SA1_STNADDR4_LENGTH                0x00000008

#define _EMAC1SA1_w_POSITION                     0x00000000
#define _EMAC1SA1_w_MASK                         0xFFFFFFFF
#define _EMAC1SA1_w_LENGTH                       0x00000020

#define _EMACxSA1_STNADDR3_POSITION              0x00000000
#define _EMACxSA1_STNADDR3_MASK                  0x000000FF
#define _EMACxSA1_STNADDR3_LENGTH                0x00000008

#define _EMACxSA1_STNADDR4_POSITION              0x00000008
#define _EMACxSA1_STNADDR4_MASK                  0x0000FF00
#define _EMACxSA1_STNADDR4_LENGTH                0x00000008

#define _EMACxSA1_w_POSITION                     0x00000000
#define _EMACxSA1_w_MASK                         0xFFFFFFFF
#define _EMACxSA1_w_LENGTH                       0x00000020

#define _EMAC1SA2_STNADDR1_POSITION              0x00000000
#define _EMAC1SA2_STNADDR1_MASK                  0x000000FF
#define _EMAC1SA2_STNADDR1_LENGTH                0x00000008

#define _EMAC1SA2_STNADDR2_POSITION              0x00000008
#define _EMAC1SA2_STNADDR2_MASK                  0x0000FF00
#define _EMAC1SA2_STNADDR2_LENGTH                0x00000008

#define _EMAC1SA2_w_POSITION                     0x00000000
#define _EMAC1SA2_w_MASK                         0xFFFFFFFF
#define _EMAC1SA2_w_LENGTH                       0x00000020

#define _EMACxSA2_STNADDR1_POSITION              0x00000000
#define _EMACxSA2_STNADDR1_MASK                  0x000000FF
#define _EMACxSA2_STNADDR1_LENGTH                0x00000008

#define _EMACxSA2_STNADDR2_POSITION              0x00000008
#define _EMACxSA2_STNADDR2_MASK                  0x0000FF00
#define _EMACxSA2_STNADDR2_LENGTH                0x00000008

#define _EMACxSA2_w_POSITION                     0x00000000
#define _EMACxSA2_w_MASK                         0xFFFFFFFF
#define _EMACxSA2_w_LENGTH                       0x00000020

#define _DEVCFG3_USERID_POSITION                 0x00000000
#define _DEVCFG3_USERID_MASK                     0x0000FFFF
#define _DEVCFG3_USERID_LENGTH                   0x00000010

#define _DEVCFG3_FSRSSEL_POSITION                0x00000010
#define _DEVCFG3_FSRSSEL_MASK                    0x00070000
#define _DEVCFG3_FSRSSEL_LENGTH                  0x00000003

#define _DEVCFG3_FMIIEN_POSITION                 0x00000018
#define _DEVCFG3_FMIIEN_MASK                     0x01000000
#define _DEVCFG3_FMIIEN_LENGTH                   0x00000001

#define _DEVCFG3_FETHIO_POSITION                 0x00000019
#define _DEVCFG3_FETHIO_MASK                     0x02000000
#define _DEVCFG3_FETHIO_LENGTH                   0x00000001

#define _DEVCFG3_FUSBIDIO_POSITION               0x0000001E
#define _DEVCFG3_FUSBIDIO_MASK                   0x40000000
#define _DEVCFG3_FUSBIDIO_LENGTH                 0x00000001

#define _DEVCFG3_FVBUSONIO_POSITION              0x0000001F
#define _DEVCFG3_FVBUSONIO_MASK                  0x80000000
#define _DEVCFG3_FVBUSONIO_LENGTH                0x00000001

#define _DEVCFG3_w_POSITION                      0x00000000
#define _DEVCFG3_w_MASK                          0xFFFFFFFF
#define _DEVCFG3_w_LENGTH                        0x00000020

#define _DEVCFG2_FPLLIDIV_POSITION               0x00000000
#define _DEVCFG2_FPLLIDIV_MASK                   0x00000007
#define _DEVCFG2_FPLLIDIV_LENGTH                 0x00000003

#define _DEVCFG2_FPLLMUL_POSITION                0x00000004
#define _DEVCFG2_FPLLMUL_MASK                    0x00000070
#define _DEVCFG2_FPLLMUL_LENGTH                  0x00000003

#define _DEVCFG2_UPLLIDIV_POSITION               0x00000008
#define _DEVCFG2_UPLLIDIV_MASK                   0x00000700
#define _DEVCFG2_UPLLIDIV_LENGTH                 0x00000003

#define _DEVCFG2_UPLLEN_POSITION                 0x0000000F
#define _DEVCFG2_UPLLEN_MASK                     0x00008000
#define _DEVCFG2_UPLLEN_LENGTH                   0x00000001

#define _DEVCFG2_FPLLODIV_POSITION               0x00000010
#define _DEVCFG2_FPLLODIV_MASK                   0x00070000
#define _DEVCFG2_FPLLODIV_LENGTH                 0x00000003

#define _DEVCFG2_w_POSITION                      0x00000000
#define _DEVCFG2_w_MASK                          0xFFFFFFFF
#define _DEVCFG2_w_LENGTH                        0x00000020

#define _DEVCFG1_FNOSC_POSITION                  0x00000000
#define _DEVCFG1_FNOSC_MASK                      0x00000007
#define _DEVCFG1_FNOSC_LENGTH                    0x00000003

#define _DEVCFG1_FSOSCEN_POSITION                0x00000005
#define _DEVCFG1_FSOSCEN_MASK                    0x00000020
#define _DEVCFG1_FSOSCEN_LENGTH                  0x00000001

#define _DEVCFG1_IESO_POSITION                   0x00000007
#define _DEVCFG1_IESO_MASK                       0x00000080
#define _DEVCFG1_IESO_LENGTH                     0x00000001

#define _DEVCFG1_POSCMOD_POSITION                0x00000008
#define _DEVCFG1_POSCMOD_MASK                    0x00000300
#define _DEVCFG1_POSCMOD_LENGTH                  0x00000002

#define _DEVCFG1_OSCIOFNC_POSITION               0x0000000A
#define _DEVCFG1_OSCIOFNC_MASK                   0x00000400
#define _DEVCFG1_OSCIOFNC_LENGTH                 0x00000001

#define _DEVCFG1_FPBDIV_POSITION                 0x0000000C
#define _DEVCFG1_FPBDIV_MASK                     0x00003000
#define _DEVCFG1_FPBDIV_LENGTH                   0x00000002

#define _DEVCFG1_FCKSM_POSITION                  0x0000000E
#define _DEVCFG1_FCKSM_MASK                      0x0000C000
#define _DEVCFG1_FCKSM_LENGTH                    0x00000002

#define _DEVCFG1_WDTPS_POSITION                  0x00000010
#define _DEVCFG1_WDTPS_MASK                      0x001F0000
#define _DEVCFG1_WDTPS_LENGTH                    0x00000005

#define _DEVCFG1_FWDTEN_POSITION                 0x00000017
#define _DEVCFG1_FWDTEN_MASK                     0x00800000
#define _DEVCFG1_FWDTEN_LENGTH                   0x00000001

#define _DEVCFG1_w_POSITION                      0x00000000
#define _DEVCFG1_w_MASK                          0xFFFFFFFF
#define _DEVCFG1_w_LENGTH                        0x00000020

#define _DEVCFG0_DEBUG_POSITION                  0x00000000
#define _DEVCFG0_DEBUG_MASK                      0x00000003
#define _DEVCFG0_DEBUG_LENGTH                    0x00000002

#define _DEVCFG0_RESERVED1_POSITION              0x00000002
#define _DEVCFG0_RESERVED1_MASK                  0x00000004
#define _DEVCFG0_RESERVED1_LENGTH                0x00000001

#define _DEVCFG0_ICESEL_POSITION                 0x00000003
#define _DEVCFG0_ICESEL_MASK                     0x00000008
#define _DEVCFG0_ICESEL_LENGTH                   0x00000001

#define _DEVCFG0_PWP_POSITION                    0x0000000C
#define _DEVCFG0_PWP_MASK                        0x000FF000
#define _DEVCFG0_PWP_LENGTH                      0x00000008

#define _DEVCFG0_BWP_POSITION                    0x00000018
#define _DEVCFG0_BWP_MASK                        0x01000000
#define _DEVCFG0_BWP_LENGTH                      0x00000001

#define _DEVCFG0_CP_POSITION                     0x0000001C
#define _DEVCFG0_CP_MASK                         0x10000000
#define _DEVCFG0_CP_LENGTH                       0x00000001

#define _DEVCFG0_RESERVED_POSITION               0x0000001F
#define _DEVCFG0_RESERVED_MASK                   0x80000000
#define _DEVCFG0_RESERVED_LENGTH                 0x00000001

#define _DEVCFG0_FDEBUG_POSITION                 0x00000000
#define _DEVCFG0_FDEBUG_MASK                     0x00000003
#define _DEVCFG0_FDEBUG_LENGTH                   0x00000002

#define _DEVCFG0_w_POSITION                      0x00000000
#define _DEVCFG0_w_MASK                          0xFFFFFFFF
#define _DEVCFG0_w_LENGTH                        0x00000020

/* Vector Numbers */
#define _CORE_TIMER_VECTOR                       0
#define _CORE_SOFTWARE_0_VECTOR                  1
#define _CORE_SOFTWARE_1_VECTOR                  2
#define _EXTERNAL_0_VECTOR                       3
#define _TIMER_1_VECTOR                          4
#define _INPUT_CAPTURE_1_VECTOR                  5
#define _OUTPUT_COMPARE_1_VECTOR                 6
#define _EXTERNAL_1_VECTOR                       7
#define _TIMER_2_VECTOR                          8
#define _INPUT_CAPTURE_2_VECTOR                  9
#define _OUTPUT_COMPARE_2_VECTOR                 10
#define _EXTERNAL_2_VECTOR                       11
#define _TIMER_3_VECTOR                          12
#define _INPUT_CAPTURE_3_VECTOR                  13
#define _OUTPUT_COMPARE_3_VECTOR                 14
#define _EXTERNAL_3_VECTOR                       15
#define _TIMER_4_VECTOR                          16
#define _INPUT_CAPTURE_4_VECTOR                  17
#define _OUTPUT_COMPARE_4_VECTOR                 18
#define _EXTERNAL_4_VECTOR                       19
#define _TIMER_5_VECTOR                          20
#define _INPUT_CAPTURE_5_VECTOR                  21
#define _OUTPUT_COMPARE_5_VECTOR                 22
#define _SPI_1_VECTOR                            23
#define _I2C_3_VECTOR                            24
#define _I2C_1A_VECTOR                           24
#define _SPI_3_VECTOR                            24
#define _SPI_1A_VECTOR                           24
#define _UART_1_VECTOR                           24
#define _UART_1A_VECTOR                          24
#define _I2C_1_VECTOR                            25
#define _CHANGE_NOTICE_VECTOR                    26
#define _ADC_VECTOR                              27
#define _PMP_VECTOR                              28
#define _COMPARATOR_1_VECTOR                     29
#define _COMPARATOR_2_VECTOR                     30
#define _I2C_4_VECTOR                            31
#define _I2C_2A_VECTOR                           31
#define _SPI_2_VECTOR                            31
#define _SPI_2A_VECTOR                           31
#define _UART_3_VECTOR                           31
#define _UART_2A_VECTOR                          31
#define _I2C_5_VECTOR                            32
#define _I2C_3A_VECTOR                           32
#define _SPI_4_VECTOR                            32
#define _SPI_3A_VECTOR                           32
#define _UART_2_VECTOR                           32
#define _UART_3A_VECTOR                          32
#define _I2C_2_VECTOR                            33
#define _FAIL_SAFE_MONITOR_VECTOR                34
#define _RTCC_VECTOR                             35
#define _DMA_0_VECTOR                            36
#define _DMA_1_VECTOR                            37
#define _DMA_2_VECTOR                            38
#define _DMA_3_VECTOR                            39
#define _DMA_4_VECTOR                            40
#define _DMA_5_VECTOR                            41
#define _DMA_6_VECTOR                            42
#define _DMA_7_VECTOR                            43
#define _FCE_VECTOR                              44
#define _USB_1_VECTOR                            45
#define _CAN_1_VECTOR                            46
#define _CAN_2_VECTOR                            47
#define _ETH_VECTOR                              48
#define _UART_1B_VECTOR                          49
#define _UART_4_VECTOR                           49
#define _UART_6_VECTOR                           50
#define _UART_2B_VECTOR                          50
#define _UART_5_VECTOR                           51
#define _UART_3B_VECTOR                          51

/* IRQ Numbers */
#define _CORE_TIMER_IRQ                          0
#define _CORE_SOFTWARE_0_IRQ                     1
#define _CORE_SOFTWARE_1_IRQ                     2
#define _EXTERNAL_0_IRQ                          3
#define _TIMER_1_IRQ                             4
#define _INPUT_CAPTURE_1_IRQ                     5
#define _OUTPUT_COMPARE_1_IRQ                    6
#define _EXTERNAL_1_IRQ                          7
#define _TIMER_2_IRQ                             8
#define _INPUT_CAPTURE_2_IRQ                     9
#define _OUTPUT_COMPARE_2_IRQ                    10
#define _EXTERNAL_2_IRQ                          11
#define _TIMER_3_IRQ                             12
#define _INPUT_CAPTURE_3_IRQ                     13
#define _OUTPUT_COMPARE_3_IRQ                    14
#define _EXTERNAL_3_IRQ                          15
#define _TIMER_4_IRQ                             16
#define _INPUT_CAPTURE_4_IRQ                     17
#define _OUTPUT_COMPARE_4_IRQ                    18
#define _EXTERNAL_4_IRQ                          19
#define _TIMER_5_IRQ                             20
#define _INPUT_CAPTURE_5_IRQ                     21
#define _OUTPUT_COMPARE_5_IRQ                    22
#define _SPI1_ERR_IRQ                            23
#define _SPI1_RX_IRQ                             24
#define _SPI1_TX_IRQ                             25
#define _I2C1A_ERR_IRQ                           26
#define _I2C3_BUS_IRQ                            26
#define _SPI1A_ERR_IRQ                           26
#define _SPI3_ERR_IRQ                            26
#define _UART1A_ERR_IRQ                          26
#define _UART1_ERR_IRQ                           26
#define _I2C1A_RX_IRQ                            27
#define _I2C3_SLAVE_IRQ                          27
#define _SPI1A_RX_IRQ                            27
#define _SPI3_RX_IRQ                             27
#define _UART1A_RX_IRQ                           27
#define _UART1_RX_IRQ                            27
#define _I2C1A_TX_IRQ                            28
#define _I2C3_MASTER_IRQ                         28
#define _SPI1A_TX_IRQ                            28
#define _SPI3_TX_IRQ                             28
#define _UART1A_TX_IRQ                           28
#define _UART1_TX_IRQ                            28
#define _I2C1_BUS_IRQ                            29
#define _I2C1_SLAVE_IRQ                          30
#define _I2C1_MASTER_IRQ                         31
#define _CHANGE_NOTICE_IRQ                       32
#define _ADC_IRQ                                 33
#define _PMP_IRQ                                 34
#define _COMPARATOR_1_IRQ                        35
#define _COMPARATOR_2_IRQ                        36
#define _I2C2A_ERR_IRQ                           37
#define _I2C4_BUS_IRQ                            37
#define _SPI2_ERR_IRQ                            37
#define _SPI2A_ERR_IRQ                           37
#define _UART2A_ERR_IRQ                          37
#define _UART3_ERR_IRQ                           37
#define _I2C2A_RX_IRQ                            38
#define _I2C4_SLAVE_IRQ                          38
#define _SPI2_RX_IRQ                             38
#define _SPI2A_RX_IRQ                            38
#define _UART2A_RX_IRQ                           38
#define _UART3_RX_IRQ                            38
#define _I2C2A_TX_IRQ                            39
#define _I2C4_MASTER_IRQ                         39
#define _SPI2A_TX_IRQ                            39
#define _SPI2_TX_IRQ                             39
#define _UART2A_TX_IRQ                           39
#define _UART3_TX_IRQ                            39
#define _I2C3A_ERR_IRQ                           40
#define _I2C5_BUS_IRQ                            40
#define _SPI3A_ERR_IRQ                           40
#define _SPI4_ERR_IRQ                            40
#define _UART2_ERR_IRQ                           40
#define _UART3A_ERR_IRQ                          40
#define _I2C3A_RX_IRQ                            41
#define _I2C5_SLAVE_IRQ                          41
#define _SPI3A_RX_IRQ                            41
#define _SPI4_RX_IRQ                             41
#define _UART2_RX_IRQ                            41
#define _UART3A_RX_IRQ                           41
#define _I2C3A_TX_IRQ                            42
#define _I2C5_MASTER_IRQ                         42
#define _SPI3A_TX_IRQ                            42
#define _SPI4_TX_IRQ                             42
#define _UART2_TX_IRQ                            42
#define _UART3A_TX_IRQ                           42
#define _I2C2_BUS_IRQ                            43
#define _I2C2_SLAVE_IRQ                          44
#define _I2C2_MASTER_IRQ                         45
#define _FAIL_SAFE_MONITOR_IRQ                   46
#define _RTCC_IRQ                                47
#define _DMA0_IRQ                                48
#define _DMA1_IRQ                                49
#define _DMA2_IRQ                                50
#define _DMA3_IRQ                                51
#define _DMA4_IRQ                                52
#define _DMA5_IRQ                                53
#define _DMA6_IRQ                                54
#define _DMA7_IRQ                                55
#define _FLASH_CONTROL_IRQ                       56
#define _USB_IRQ                                 57
#define _CAN1_IRQ                                58
#define _CAN2_IRQ                                59
#define _ETHERNET_IRQ                            60
#define _INPUT_CAPTURE_ERROR_1_IRQ               61
#define _INPUT_CAPTURE_ERROR_2_IRQ               62
#define _INPUT_CAPTURE_ERROR_3_IRQ               63
#define _INPUT_CAPTURE_ERROR_4_IRQ               64
#define _INPUT_CAPTURE_ERROR_5_IRQ               65
#define _PMP_ERROR_IRQ                           66
#define _UART1B_ERR_IRQ                          67
#define _UART4_ERR_IRQ                           67
#define _UART1B_RX_IRQ                           68
#define _UART4_RX_IRQ                            68
#define _UART1B_TX_IRQ                           69
#define _UART4_TX_IRQ                            69
#define _UART2B_ERR_IRQ                          70
#define _UART6_ERR_IRQ                           70
#define _UART2B_RX_IRQ                           71
#define _UART6_RX_IRQ                            71
#define _UART2B_TX_IRQ                           72
#define _UART6_TX_IRQ                            72
#define _UART3B_ERR_IRQ                          73
#define _UART5_ERR_IRQ                           73
#define _UART3B_RX_IRQ                           74
#define _UART5_RX_IRQ                            74
#define _UART3B_TX_IRQ                           75
#define _UART5_TX_IRQ                            75

/* Device Peripherals */
#define _ADC10
#define __APPI
#define __APPO
#define _BMX
#define _CFG
#define _CMP
#define _CVR
#define __DDPSTAT
#define _DMAC
#define _DMAC0
#define _DMAC1
#define _DMAC2
#define _DMAC3
#define _DMAC4
#define _DMAC5
#define _DMAC6
#define _DMAC7
#define _ETH
#define _I2C1
#define _I2C1A
#define _I2C2A
#define _I2C3
#define _I2C3A
#define _I2C4
#define _I2C5
#define _ICAP1
#define _ICAP2
#define _ICAP3
#define _ICAP4
#define _ICAP5
#define _INT
#define _NVM
#define _OCMP1
#define _OCMP2
#define _OCMP3
#define _OCMP4
#define _OCMP5
#define _OSC
#define _PCACHE
#define _PMP
#define _PORTB
#define _PORTC
#define _PORTD
#define _PORTE
#define _PORTF
#define _PORTG
#define _RCON
#define _RTCC
#define _SPI1A
#define _SPI2
#define _SPI2A
#define _SPI3
#define _SPI3A
#define _SPI4
#define __STRO
#define _TMR1
#define _TMR2
#define _TMR23
#define _TMR3
#define _TMR4
#define _TMR45
#define _TMR5
#define _UART1
#define _UART1A
#define _UART1B
#define _UART2
#define _UART2A
#define _UART2B
#define _UART3
#define _UART3A
#define _UART3B
#define _UART4
#define _UART5
#define _UART6
#define _USB
#define _WDT

/* Base Addresses for Peripherals */
#define _ADC10_BASE_ADDRESS                      0xBF809000
#define __APPI_BASE_ADDRESS                      0xBF880190
#define __APPO_BASE_ADDRESS                      0xBF880180
#define _BMX_BASE_ADDRESS                        0xBF882000
#define _CFG_BASE_ADDRESS                        0xBF80F200
#define _CMP_BASE_ADDRESS                        0xBF80A000
#define _CVR_BASE_ADDRESS                        0xBF809800
#define __DDPSTAT_BASE_ADDRESS                   0xBF880140
#define _DMAC_BASE_ADDRESS                       0xBF883000
#define _DMAC0_BASE_ADDRESS                      0xBF883060
#define _DMAC1_BASE_ADDRESS                      0xBF883120
#define _DMAC2_BASE_ADDRESS                      0xBF8831E0
#define _DMAC3_BASE_ADDRESS                      0xBF8832A0
#define _DMAC4_BASE_ADDRESS                      0xBF883360
#define _DMAC5_BASE_ADDRESS                      0xBF883420
#define _DMAC6_BASE_ADDRESS                      0xBF8834E0
#define _DMAC7_BASE_ADDRESS                      0xBF8835A0
#define _ETH_BASE_ADDRESS                        0xBF889000
#define _I2C1_BASE_ADDRESS                       0xBF805300
#define _I2C1A_BASE_ADDRESS                      0xBF805000
#define _I2C2A_BASE_ADDRESS                      0xBF805100
#define _I2C3_BASE_ADDRESS                       0xBF805000
#define _I2C3A_BASE_ADDRESS                      0xBF805200
#define _I2C4_BASE_ADDRESS                       0xBF805100
#define _I2C5_BASE_ADDRESS                       0xBF805200
#define _ICAP1_BASE_ADDRESS                      0xBF802000
#define _ICAP2_BASE_ADDRESS                      0xBF802200
#define _ICAP3_BASE_ADDRESS                      0xBF802400
#define _ICAP4_BASE_ADDRESS                      0xBF802600
#define _ICAP5_BASE_ADDRESS                      0xBF802800
#define _INT_BASE_ADDRESS                        0xBF881000
#define _NVM_BASE_ADDRESS                        0xBF80F400
#define _OCMP1_BASE_ADDRESS                      0xBF803000
#define _OCMP2_BASE_ADDRESS                      0xBF803200
#define _OCMP3_BASE_ADDRESS                      0xBF803400
#define _OCMP4_BASE_ADDRESS                      0xBF803600
#define _OCMP5_BASE_ADDRESS                      0xBF803800
#define _OSC_BASE_ADDRESS                        0xBF80F000
#define _PCACHE_BASE_ADDRESS                     0xBF884000
#define _PMP_BASE_ADDRESS                        0xBF807000
#define _PORTB_BASE_ADDRESS                      0xBF886040
#define _PORTC_BASE_ADDRESS                      0xBF886080
#define _PORTD_BASE_ADDRESS                      0xBF8860C0
#define _PORTE_BASE_ADDRESS                      0xBF886100
#define _PORTF_BASE_ADDRESS                      0xBF886140
#define _PORTG_BASE_ADDRESS                      0xBF886180
#define _RCON_BASE_ADDRESS                       0xBF80F600
#define _RTCC_BASE_ADDRESS                       0xBF800200
#define _SPI1A_BASE_ADDRESS                      0xBF805800
#define _SPI2_BASE_ADDRESS                       0xBF805A00
#define _SPI2A_BASE_ADDRESS                      0xBF805A00
#define _SPI3_BASE_ADDRESS                       0xBF805800
#define _SPI3A_BASE_ADDRESS                      0xBF805C00
#define _SPI4_BASE_ADDRESS                       0xBF805C00
#define __STRO_BASE_ADDRESS                      0xBF880170
#define _TMR1_BASE_ADDRESS                       0xBF800600
#define _TMR2_BASE_ADDRESS                       0xBF800800
#define _TMR23_BASE_ADDRESS                      0xBF800800
#define _TMR3_BASE_ADDRESS                       0xBF800A00
#define _TMR4_BASE_ADDRESS                       0xBF800C00
#define _TMR45_BASE_ADDRESS                      0xBF800C00
#define _TMR5_BASE_ADDRESS                       0xBF800E00
#define _UART1_BASE_ADDRESS                      0xBF806000
#define _UART1A_BASE_ADDRESS                     0xBF806000
#define _UART1B_BASE_ADDRESS                     0xBF806200
#define _UART2_BASE_ADDRESS                      0xBF806800
#define _UART2A_BASE_ADDRESS                     0xBF806400
#define _UART2B_BASE_ADDRESS                     0xBF806600
#define _UART3_BASE_ADDRESS                      0xBF806400
#define _UART3A_BASE_ADDRESS                     0xBF806800
#define _UART3B_BASE_ADDRESS                     0xBF806A00
#define _UART4_BASE_ADDRESS                      0xBF806200
#define _UART5_BASE_ADDRESS                      0xBF806A00
#define _UART6_BASE_ADDRESS                      0xBF806600
#define _USB_BASE_ADDRESS                        0xBF885000
#define _WDT_BASE_ADDRESS                        0xBF800000

/* include generic header file */
#include "ppic32mx.h"


#endif
