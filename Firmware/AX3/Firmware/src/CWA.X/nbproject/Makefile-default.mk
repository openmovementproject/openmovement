#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
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

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../Flux/src/Ftl/Ecc.c ../../../../Flux/src/Ftl/FsFtl.c ../../../../Flux/src/Ftl/Ftl.c ../../../../Flux/src/Hardware/HardwareProfile-CWA.c ../../../../Flux/src/Peripherals/Accel-ADXL345.c ../../../../Flux/src/Peripherals/Analog.c ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c ../../../../Flux/src/Peripherals/Nand.c ../../../../Flux/src/Peripherals/Rtc.c ../../../../Flux/src/Usb/USB_CDC_MSD.c ../../../../Flux/src/Utils/Fifo.c ../../../../Flux/src/Utils/Util.c "../../../../Flux/src/Microchip/MDD File System/FSIO.c" "C:/Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c" "C:/Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c" "C:/Microchip Solutions v2011-12-05/Microchip/USB/MSD Device Driver/usb_function_msd.c" ../Logger.c ../Settings.c ../main.c ../usb_descriptors.c "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c"

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1660554189/Ecc.o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ${OBJECTDIR}/_ext/1660554189/Ftl.o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ${OBJECTDIR}/_ext/434517530/Analog.o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ${OBJECTDIR}/_ext/434517530/Nand.o ${OBJECTDIR}/_ext/434517530/Rtc.o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ${OBJECTDIR}/_ext/1921405088/Fifo.o ${OBJECTDIR}/_ext/1921405088/Util.o ${OBJECTDIR}/_ext/276258160/FSIO.o ${OBJECTDIR}/_ext/209253324/usb_device.o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1453245926/TimeDelay.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1660554189/Ecc.o.d ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d ${OBJECTDIR}/_ext/1660554189/Ftl.o.d ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d ${OBJECTDIR}/_ext/434517530/Analog.o.d ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d ${OBJECTDIR}/_ext/434517530/Nand.o.d ${OBJECTDIR}/_ext/434517530/Rtc.o.d ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d ${OBJECTDIR}/_ext/1921405088/Fifo.o.d ${OBJECTDIR}/_ext/1921405088/Util.o.d ${OBJECTDIR}/_ext/276258160/FSIO.o.d ${OBJECTDIR}/_ext/209253324/usb_device.o.d ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d ${OBJECTDIR}/_ext/1472/Logger.o.d ${OBJECTDIR}/_ext/1472/Settings.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1660554189/Ecc.o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ${OBJECTDIR}/_ext/1660554189/Ftl.o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ${OBJECTDIR}/_ext/434517530/Analog.o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ${OBJECTDIR}/_ext/434517530/Nand.o ${OBJECTDIR}/_ext/434517530/Rtc.o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ${OBJECTDIR}/_ext/1921405088/Fifo.o ${OBJECTDIR}/_ext/1921405088/Util.o ${OBJECTDIR}/_ext/276258160/FSIO.o ${OBJECTDIR}/_ext/209253324/usb_device.o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1453245926/TimeDelay.o

# Source Files
SOURCEFILES=../../../../Flux/src/Ftl/Ecc.c ../../../../Flux/src/Ftl/FsFtl.c ../../../../Flux/src/Ftl/Ftl.c ../../../../Flux/src/Hardware/HardwareProfile-CWA.c ../../../../Flux/src/Peripherals/Accel-ADXL345.c ../../../../Flux/src/Peripherals/Analog.c ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c ../../../../Flux/src/Peripherals/Nand.c ../../../../Flux/src/Peripherals/Rtc.c ../../../../Flux/src/Usb/USB_CDC_MSD.c ../../../../Flux/src/Utils/Fifo.c ../../../../Flux/src/Utils/Util.c ../../../../Flux/src/Microchip/MDD File System/FSIO.c C:/Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c C:/Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c C:/Microchip Solutions v2011-12-05/Microchip/USB/MSD Device Driver/usb_function_msd.c ../Logger.c ../Settings.c ../main.c ../usb_descriptors.c ../../../../../../../Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
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
	@${MKDIR} "${OBJECTDIR}/_ext/1660554189" 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c    
	
${OBJECTDIR}/_ext/1660554189/FsFtl.o: ../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660554189" 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c    
	
${OBJECTDIR}/_ext/1660554189/Ftl.o: ../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660554189" 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c    
	
${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o: ../../../../Flux/src/Hardware/HardwareProfile-CWA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2131236057" 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.ok ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.err 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" -o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ../../../../Flux/src/Hardware/HardwareProfile-CWA.c    
	
${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o: ../../../../Flux/src/Peripherals/Accel-ADXL345.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.ok ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" -o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ../../../../Flux/src/Peripherals/Accel-ADXL345.c    
	
${OBJECTDIR}/_ext/434517530/Analog.o: ../../../../Flux/src/Peripherals/Analog.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.ok ${OBJECTDIR}/_ext/434517530/Analog.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Analog.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Analog.o.d" -o ${OBJECTDIR}/_ext/434517530/Analog.o ../../../../Flux/src/Peripherals/Analog.c    
	
${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o: ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.ok ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" -o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c    
	
${OBJECTDIR}/_ext/434517530/Nand.o: ../../../../Flux/src/Peripherals/Nand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.ok ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Nand.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Nand.o.d" -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c    
	
${OBJECTDIR}/_ext/434517530/Rtc.o: ../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Rtc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Rtc.o.d" -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c    
	
${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o: ../../../../Flux/src/Usb/USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660568563" 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.ok ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" -o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ../../../../Flux/src/Usb/USB_CDC_MSD.c    
	
${OBJECTDIR}/_ext/1921405088/Fifo.o: ../../../../Flux/src/Utils/Fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1921405088" 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.ok ${OBJECTDIR}/_ext/1921405088/Fifo.o.err 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" -o ${OBJECTDIR}/_ext/1921405088/Fifo.o ../../../../Flux/src/Utils/Fifo.c    
	
${OBJECTDIR}/_ext/1921405088/Util.o: ../../../../Flux/src/Utils/Util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1921405088" 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.ok ${OBJECTDIR}/_ext/1921405088/Util.o.err 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Util.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Util.o.d" -o ${OBJECTDIR}/_ext/1921405088/Util.o ../../../../Flux/src/Utils/Util.c    
	
${OBJECTDIR}/_ext/276258160/FSIO.o: ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/276258160" 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/276258160/FSIO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/276258160/FSIO.o.d" -o ${OBJECTDIR}/_ext/276258160/FSIO.o "../../../../Flux/src/Microchip/MDD File System/FSIO.c"    
	
${OBJECTDIR}/_ext/209253324/usb_device.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/209253324" 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.ok ${OBJECTDIR}/_ext/209253324/usb_device.o.err 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/209253324/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/209253324/usb_device.o.d" -o ${OBJECTDIR}/_ext/209253324/usb_device.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c"    
	
${OBJECTDIR}/_ext/64030191/usb_function_cdc.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/64030191" 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.ok ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.err 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" -o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c"    
	
${OBJECTDIR}/_ext/1271328659/usb_function_msd.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1271328659" 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.ok ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.err 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" -o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/MSD Device Driver/usb_function_msd.c"    
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.ok ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Logger.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Logger.o.d" -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c    
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.ok ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Settings.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Settings.o.d" -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c    
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.ok ${OBJECTDIR}/_ext/1472/main.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c    
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c    
	
${OBJECTDIR}/_ext/1453245926/TimeDelay.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1453245926" 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.ok ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.err 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/1453245926/TimeDelay.o "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c"    
	
else
${OBJECTDIR}/_ext/1660554189/Ecc.o: ../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660554189" 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ecc.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c    
	
${OBJECTDIR}/_ext/1660554189/FsFtl.o: ../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660554189" 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/FsFtl.o.d" -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c    
	
${OBJECTDIR}/_ext/1660554189/Ftl.o: ../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660554189" 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660554189/Ftl.o.d" -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c    
	
${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o: ../../../../Flux/src/Hardware/HardwareProfile-CWA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2131236057" 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.ok ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.err 
	@${RM} ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o.d" -o ${OBJECTDIR}/_ext/2131236057/HardwareProfile-CWA.o ../../../../Flux/src/Hardware/HardwareProfile-CWA.c    
	
${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o: ../../../../Flux/src/Peripherals/Accel-ADXL345.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.ok ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o.d" -o ${OBJECTDIR}/_ext/434517530/Accel-ADXL345.o ../../../../Flux/src/Peripherals/Accel-ADXL345.c    
	
${OBJECTDIR}/_ext/434517530/Analog.o: ../../../../Flux/src/Peripherals/Analog.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o.ok ${OBJECTDIR}/_ext/434517530/Analog.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Analog.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Analog.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Analog.o.d" -o ${OBJECTDIR}/_ext/434517530/Analog.o ../../../../Flux/src/Peripherals/Analog.c    
	
${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o: ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.ok ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o.d" -o ${OBJECTDIR}/_ext/434517530/Gyro-L3G4200D.o ../../../../Flux/src/Peripherals/Gyro-L3G4200D.c    
	
${OBJECTDIR}/_ext/434517530/Nand.o: ../../../../Flux/src/Peripherals/Nand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.ok ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Nand.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Nand.o.d" -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c    
	
${OBJECTDIR}/_ext/434517530/Rtc.o: ../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/434517530" 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/434517530/Rtc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/434517530/Rtc.o.d" -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c    
	
${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o: ../../../../Flux/src/Usb/USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1660568563" 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.ok ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.err 
	@${RM} ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o.d" -o ${OBJECTDIR}/_ext/1660568563/USB_CDC_MSD.o ../../../../Flux/src/Usb/USB_CDC_MSD.c    
	
${OBJECTDIR}/_ext/1921405088/Fifo.o: ../../../../Flux/src/Utils/Fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1921405088" 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o.ok ${OBJECTDIR}/_ext/1921405088/Fifo.o.err 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Fifo.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Fifo.o.d" -o ${OBJECTDIR}/_ext/1921405088/Fifo.o ../../../../Flux/src/Utils/Fifo.c    
	
${OBJECTDIR}/_ext/1921405088/Util.o: ../../../../Flux/src/Utils/Util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1921405088" 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o.ok ${OBJECTDIR}/_ext/1921405088/Util.o.err 
	@${RM} ${OBJECTDIR}/_ext/1921405088/Util.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1921405088/Util.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1921405088/Util.o.d" -o ${OBJECTDIR}/_ext/1921405088/Util.o ../../../../Flux/src/Utils/Util.c    
	
${OBJECTDIR}/_ext/276258160/FSIO.o: ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/276258160" 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/276258160/FSIO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/276258160/FSIO.o.d" -o ${OBJECTDIR}/_ext/276258160/FSIO.o "../../../../Flux/src/Microchip/MDD File System/FSIO.c"    
	
${OBJECTDIR}/_ext/209253324/usb_device.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/209253324" 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o.ok ${OBJECTDIR}/_ext/209253324/usb_device.o.err 
	@${RM} ${OBJECTDIR}/_ext/209253324/usb_device.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/209253324/usb_device.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/209253324/usb_device.o.d" -o ${OBJECTDIR}/_ext/209253324/usb_device.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c"    
	
${OBJECTDIR}/_ext/64030191/usb_function_cdc.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/64030191" 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.ok ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.err 
	@${RM} ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/64030191/usb_function_cdc.o.d" -o ${OBJECTDIR}/_ext/64030191/usb_function_cdc.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c"    
	
${OBJECTDIR}/_ext/1271328659/usb_function_msd.o: C:/Microchip\ Solutions\ v2011-12-05/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1271328659" 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.ok ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.err 
	@${RM} ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1271328659/usb_function_msd.o.d" -o ${OBJECTDIR}/_ext/1271328659/usb_function_msd.o "C:/Microchip Solutions v2011-12-05/Microchip/USB/MSD Device Driver/usb_function_msd.c"    
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.ok ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Logger.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Logger.o.d" -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c    
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.ok ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Settings.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/Settings.o.d" -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c    
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.ok ${OBJECTDIR}/_ext/1472/main.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d" -o ${OBJECTDIR}/_ext/1472/main.o ../main.c    
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c    
	
${OBJECTDIR}/_ext/1453245926/TimeDelay.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1453245926" 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.ok ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.err 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -MMD -MF "${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d" -o ${OBJECTDIR}/_ext/1453245926/TimeDelay.o "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c"    
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../app_hid_boot_p24FJ256GB106.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1
else
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../app_hid_boot_p24FJ256GB106.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\pic30-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -omf=elf
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
