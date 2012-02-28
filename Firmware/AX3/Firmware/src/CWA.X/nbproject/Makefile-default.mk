#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
include Makefile

# Environment
SHELL=cmd.exe
# Adding MPLAB X bin directory to path
PATH:=C:/Program Files (x86)/Microchip/MPLABX/mplab_ide/mplab_ide/modules/../../bin/:$(PATH)
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1660554189/Ecc.o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ${OBJECTDIR}/_ext/1660554189/Ftl.o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ${OBJECTDIR}/_ext/434517530/Analog.o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ${OBJECTDIR}/_ext/434517530/Nand.o ${OBJECTDIR}/_ext/434517530/Rtc.o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ${OBJECTDIR}/_ext/1921405088/Fifo.o ${OBJECTDIR}/_ext/1921405088/Util.o ${OBJECTDIR}/_ext/1380151971/TimeDelay.o ${OBJECTDIR}/_ext/276258160/FSIO.o ${OBJECTDIR}/_ext/209253324/usb_device.o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1660554189/Ecc.o.d ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d ${OBJECTDIR}/_ext/1660554189/Ftl.o.d ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d ${OBJECTDIR}/_ext/434517530/Analog.o.d ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d ${OBJECTDIR}/_ext/434517530/Nand.o.d ${OBJECTDIR}/_ext/434517530/Rtc.o.d ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d ${OBJECTDIR}/_ext/1921405088/Fifo.o.d ${OBJECTDIR}/_ext/1921405088/Util.o.d ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d ${OBJECTDIR}/_ext/276258160/FSIO.o.d ${OBJECTDIR}/_ext/209253324/usb_device.o.d ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d ${OBJECTDIR}/_ext/1472/Logger.o.d ${OBJECTDIR}/_ext/1472/Settings.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1660554189/Ecc.o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ${OBJECTDIR}/_ext/1660554189/Ftl.o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ${OBJECTDIR}/_ext/434517530/Analog.o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ${OBJECTDIR}/_ext/434517530/Nand.o ${OBJECTDIR}/_ext/434517530/Rtc.o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ${OBJECTDIR}/_ext/1921405088/Fifo.o ${OBJECTDIR}/_ext/1921405088/Util.o ${OBJECTDIR}/_ext/1380151971/TimeDelay.o ${OBJECTDIR}/_ext/276258160/FSIO.o ${OBJECTDIR}/_ext/209253324/usb_device.o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

# Path to java used to run MPLAB X when this makefile was created
MP_JAVA_PATH="C:\Program Files\Java\jre6/bin/"
OS_CURRENT="$(shell uname -s)"
############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
MP_CC="C:\Program Files (x86)\Microchip\MPLAB C30\bin\pic30-gcc.exe"
# MP_BC is not defined
MP_AS="C:\Program Files (x86)\Microchip\MPLAB C30\bin\pic30-as.exe"
MP_LD="C:\Program Files (x86)\Microchip\MPLAB C30\bin\pic30-ld.exe"
MP_AR="C:\Program Files (x86)\Microchip\MPLAB C30\bin\pic30-ar.exe"
DEP_GEN=${MP_JAVA_PATH}java -jar "C:/Program Files (x86)/Microchip/MPLABX/mplab_ide/mplab_ide/modules/../../bin/extractobjectdependencies.jar" 
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps
MP_CC_DIR="C:\Program Files (x86)\Microchip\MPLAB C30\bin"
# MP_BC_DIR is not defined
MP_AS_DIR="C:\Program Files (x86)\Microchip\MPLAB C30\bin"
MP_LD_DIR="C:\Program Files (x86)\Microchip\MPLAB C30\bin"
MP_AR_DIR="C:\Program Files (x86)\Microchip\MPLAB C30\bin"
# MP_BC_DIR is not defined

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ256GB106
MP_LINKER_FILE_OPTION=,--script="..\app_hid_boot_p24FJ256GB106.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1660554189/Ecc.o: ../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c  
	
${OBJECTDIR}/_ext/1660554189/FsFtl.o: ../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c  
	
${OBJECTDIR}/_ext/1660554189/Ftl.o: ../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c  
	
${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o: ../../../../Flux/src/Hardware/HardwareProfile-CWA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2131236057 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.ok ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" -o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ../../../../Flux/src/Hardware/HardwareProfile-CWA.c  
	
${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o: ../../../../Flux/src/Peripherals/Accel-ADXL345.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.ok ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" -o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ../../../../Flux/src/Peripherals/Accel-ADXL345.c  
	
${OBJECTDIR}/_ext/434517530/Analog.o: ../../../../Flux/src/Peripherals/Analog.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.ok ${OBJECTDIR}/_ext/434517530/Analog.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Analog.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Analog.o.d" -o ${OBJECTDIR}/_ext/434517530/Analog.o ../../../../Flux/src/Peripherals/Analog.c  
	
${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o: ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.ok ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" -o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c  
	
${OBJECTDIR}/_ext/434517530/Nand.o: ../../../../Flux/src/Peripherals/Nand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.ok ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Nand.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Nand.o.d" -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c  
	
${OBJECTDIR}/_ext/434517530/Rtc.o: ../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Rtc.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Rtc.o.d" -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c  
	
${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o: ../../../../Flux/src/Usb/USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660568563 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.ok ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" -o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ../../../../Flux/src/Usb/USB_CDC_MSD.c  
	
${OBJECTDIR}/_ext/1921405088/Fifo.o: ../../../../Flux/src/Utils/Fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1921405088 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.ok ${OBJECTDIR}/_ext/1921405088/Fifo.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" -o ${OBJECTDIR}/_ext/1921405088/Fifo.o ../../../../Flux/src/Utils/Fifo.c  
	
${OBJECTDIR}/_ext/1921405088/Util.o: ../../../../Flux/src/Utils/Util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1921405088 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.ok ${OBJECTDIR}/_ext/1921405088/Util.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Util.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Util.o.d" -o ${OBJECTDIR}/_ext/1921405088/Util.o ../../../../Flux/src/Utils/Util.c  
	
${OBJECTDIR}/_ext/1380151971/TimeDelay.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1380151971 
	@${RM} ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.ok ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/1380151971/TimeDelay.o "C:/Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c"  
	
${OBJECTDIR}/_ext/276258160/FSIO.o: ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/276258160 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/276258160/FSIO.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/276258160/FSIO.o.d" -o ${OBJECTDIR}/_ext/276258160/FSIO.o "../../../../Flux/src/Microchip/MDD File System/FSIO.c"  
	
${OBJECTDIR}/_ext/209253324/usb_device.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/209253324 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.ok ${OBJECTDIR}/_ext/209253324/usb_device.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/209253324/usb_device.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/209253324/usb_device.o.d" -o ${OBJECTDIR}/_ext/209253324/usb_device.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c"  
	
${OBJECTDIR}/_ext/64030191/usb_function_cdc.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/64030191 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.ok ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" -o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c"  
	
${OBJECTDIR}/_ext/1271328659/usb_function_msd.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1271328659 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.ok ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" -o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/MSD Device Driver/usb_function_msd.c"  
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.ok ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Logger.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Logger.o.d" -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c  
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.ok ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Settings.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Settings.o.d" -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c  
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.ok ${OBJECTDIR}/_ext/1472/main.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c  
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c  
	
else
${OBJECTDIR}/_ext/1660554189/Ecc.o: ../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c  
	
${OBJECTDIR}/_ext/1660554189/FsFtl.o: ../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c  
	
${OBJECTDIR}/_ext/1660554189/Ftl.o: ../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c  
	
${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o: ../../../../Flux/src/Hardware/HardwareProfile-CWA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2131236057 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.ok ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" -o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ../../../../Flux/src/Hardware/HardwareProfile-CWA.c  
	
${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o: ../../../../Flux/src/Peripherals/Accel-ADXL345.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.ok ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" -o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ../../../../Flux/src/Peripherals/Accel-ADXL345.c  
	
${OBJECTDIR}/_ext/434517530/Analog.o: ../../../../Flux/src/Peripherals/Analog.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.ok ${OBJECTDIR}/_ext/434517530/Analog.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Analog.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Analog.o.d" -o ${OBJECTDIR}/_ext/434517530/Analog.o ../../../../Flux/src/Peripherals/Analog.c  
	
${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o: ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.ok ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" -o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c  
	
${OBJECTDIR}/_ext/434517530/Nand.o: ../../../../Flux/src/Peripherals/Nand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.ok ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Nand.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Nand.o.d" -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c  
	
${OBJECTDIR}/_ext/434517530/Rtc.o: ../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Rtc.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Rtc.o.d" -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c  
	
${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o: ../../../../Flux/src/Usb/USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660568563 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.ok ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" -o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ../../../../Flux/src/Usb/USB_CDC_MSD.c  
	
${OBJECTDIR}/_ext/1921405088/Fifo.o: ../../../../Flux/src/Utils/Fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1921405088 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.ok ${OBJECTDIR}/_ext/1921405088/Fifo.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" -o ${OBJECTDIR}/_ext/1921405088/Fifo.o ../../../../Flux/src/Utils/Fifo.c  
	
${OBJECTDIR}/_ext/1921405088/Util.o: ../../../../Flux/src/Utils/Util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1921405088 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.ok ${OBJECTDIR}/_ext/1921405088/Util.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Util.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Util.o.d" -o ${OBJECTDIR}/_ext/1921405088/Util.o ../../../../Flux/src/Utils/Util.c  
	
${OBJECTDIR}/_ext/1380151971/TimeDelay.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1380151971 
	@${RM} ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.ok ${OBJECTDIR}/_ext/1380151971/TimeDelay.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1380151971/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/1380151971/TimeDelay.o "C:/Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c"  
	
${OBJECTDIR}/_ext/276258160/FSIO.o: ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/276258160 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/276258160/FSIO.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/276258160/FSIO.o.d" -o ${OBJECTDIR}/_ext/276258160/FSIO.o "../../../../Flux/src/Microchip/MDD File System/FSIO.c"  
	
${OBJECTDIR}/_ext/209253324/usb_device.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/209253324 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.ok ${OBJECTDIR}/_ext/209253324/usb_device.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/209253324/usb_device.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/209253324/usb_device.o.d" -o ${OBJECTDIR}/_ext/209253324/usb_device.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c"  
	
${OBJECTDIR}/_ext/64030191/usb_function_cdc.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/64030191 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.ok ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" -o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c"  
	
${OBJECTDIR}/_ext/1271328659/usb_function_msd.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1271328659 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.ok ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" -o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/MSD Device Driver/usb_function_msd.c"  
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.ok ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Logger.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Logger.o.d" -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c  
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.ok ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Settings.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Settings.o.d" -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c  
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.ok ${OBJECTDIR}/_ext/1472/main.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c  
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c  
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}        -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__ICD2RAM=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1
else
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}        -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\pic30-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -omf=elf
endif


# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
