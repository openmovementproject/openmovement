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
ifeq "$(wildcard nbproject/Makefile-local-Release.mk)" "nbproject/Makefile-local-Release.mk"
include nbproject/Makefile-local-Release.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=Release
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
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS
SUB_IMAGE_ADDRESS_COMMAND=--image-address $(SUB_IMAGE_ADDRESS)
else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../../Flux/src/Ftl/Ecc.c ../../../../../Flux/src/Ftl/FsFtl.c ../../../../../Flux/src/Ftl/Ftl.c ../../../../../Flux/src/Usb/USB_CDC_MSD.c ../../../../../Flux/src/Utils/Fifo.c ../../../../../Flux/src/Utils/Util.c ../../../../../Flux/src/Utils/FSutils.c C:/microchip_solutions_v2013-06-15/Microchip/Common/TimeDelay.c C:/microchip_solutions_v2013-06-15/Microchip/USB/usb_device.c "C:/microchip_solutions_v2013-06-15/Microchip/USB/CDC Device Driver/usb_function_cdc.c" "C:/microchip_solutions_v2013-06-15/Microchip/USB/MSD Device Driver/usb_function_msd.c" ../Logger.c ../Settings.c ../main.c ../usb_descriptors.c "../AX3 SelfTest.c" ../HardwareProfile-CWA.c ../FSIO.c ../bmi160.c ../rtcc.c ../bmm150.c ../../../../../Flux/src/Peripherals/Rtc.c ../IMU-BMI160.c ../Analogue.c ../NandMulti.c ../NandMulti1.c ../NandMulti2.c ../../Common/BootCheck.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1232478524/Ecc.o ${OBJECTDIR}/_ext/1232478524/FsFtl.o ${OBJECTDIR}/_ext/1232478524/Ftl.o ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o ${OBJECTDIR}/_ext/985258737/Fifo.o ${OBJECTDIR}/_ext/985258737/Util.o ${OBJECTDIR}/_ext/985258737/FSutils.o ${OBJECTDIR}/_ext/916655611/TimeDelay.o ${OBJECTDIR}/_ext/1961590870/usb_device.o ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o" ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o ${OBJECTDIR}/_ext/1472/FSIO.o ${OBJECTDIR}/_ext/1472/bmi160.o ${OBJECTDIR}/_ext/1472/rtcc.o ${OBJECTDIR}/_ext/1472/bmm150.o ${OBJECTDIR}/_ext/2002583637/Rtc.o ${OBJECTDIR}/_ext/1472/IMU-BMI160.o ${OBJECTDIR}/_ext/1472/Analogue.o ${OBJECTDIR}/_ext/1472/NandMulti.o ${OBJECTDIR}/_ext/1472/NandMulti1.o ${OBJECTDIR}/_ext/1472/NandMulti2.o ${OBJECTDIR}/_ext/2049610667/BootCheck.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1232478524/Ecc.o.d ${OBJECTDIR}/_ext/1232478524/FsFtl.o.d ${OBJECTDIR}/_ext/1232478524/Ftl.o.d ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d ${OBJECTDIR}/_ext/985258737/Fifo.o.d ${OBJECTDIR}/_ext/985258737/Util.o.d ${OBJECTDIR}/_ext/985258737/FSutils.o.d ${OBJECTDIR}/_ext/916655611/TimeDelay.o.d ${OBJECTDIR}/_ext/1961590870/usb_device.o.d ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d ${OBJECTDIR}/_ext/1472/Logger.o.d ${OBJECTDIR}/_ext/1472/Settings.o.d ${OBJECTDIR}/_ext/1472/main.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o.d" ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d ${OBJECTDIR}/_ext/1472/FSIO.o.d ${OBJECTDIR}/_ext/1472/bmi160.o.d ${OBJECTDIR}/_ext/1472/rtcc.o.d ${OBJECTDIR}/_ext/1472/bmm150.o.d ${OBJECTDIR}/_ext/2002583637/Rtc.o.d ${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d ${OBJECTDIR}/_ext/1472/Analogue.o.d ${OBJECTDIR}/_ext/1472/NandMulti.o.d ${OBJECTDIR}/_ext/1472/NandMulti1.o.d ${OBJECTDIR}/_ext/1472/NandMulti2.o.d ${OBJECTDIR}/_ext/2049610667/BootCheck.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1232478524/Ecc.o ${OBJECTDIR}/_ext/1232478524/FsFtl.o ${OBJECTDIR}/_ext/1232478524/Ftl.o ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o ${OBJECTDIR}/_ext/985258737/Fifo.o ${OBJECTDIR}/_ext/985258737/Util.o ${OBJECTDIR}/_ext/985258737/FSutils.o ${OBJECTDIR}/_ext/916655611/TimeDelay.o ${OBJECTDIR}/_ext/1961590870/usb_device.o ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/AX3\ SelfTest.o ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o ${OBJECTDIR}/_ext/1472/FSIO.o ${OBJECTDIR}/_ext/1472/bmi160.o ${OBJECTDIR}/_ext/1472/rtcc.o ${OBJECTDIR}/_ext/1472/bmm150.o ${OBJECTDIR}/_ext/2002583637/Rtc.o ${OBJECTDIR}/_ext/1472/IMU-BMI160.o ${OBJECTDIR}/_ext/1472/Analogue.o ${OBJECTDIR}/_ext/1472/NandMulti.o ${OBJECTDIR}/_ext/1472/NandMulti1.o ${OBJECTDIR}/_ext/1472/NandMulti2.o ${OBJECTDIR}/_ext/2049610667/BootCheck.o

# Source Files
SOURCEFILES=../../../../../Flux/src/Ftl/Ecc.c ../../../../../Flux/src/Ftl/FsFtl.c ../../../../../Flux/src/Ftl/Ftl.c ../../../../../Flux/src/Usb/USB_CDC_MSD.c ../../../../../Flux/src/Utils/Fifo.c ../../../../../Flux/src/Utils/Util.c ../../../../../Flux/src/Utils/FSutils.c C:/microchip_solutions_v2013-06-15/Microchip/Common/TimeDelay.c C:/microchip_solutions_v2013-06-15/Microchip/USB/usb_device.c C:/microchip_solutions_v2013-06-15/Microchip/USB/CDC Device Driver/usb_function_cdc.c C:/microchip_solutions_v2013-06-15/Microchip/USB/MSD Device Driver/usb_function_msd.c ../Logger.c ../Settings.c ../main.c ../usb_descriptors.c ../AX3 SelfTest.c ../HardwareProfile-CWA.c ../FSIO.c ../bmi160.c ../rtcc.c ../bmm150.c ../../../../../Flux/src/Peripherals/Rtc.c ../IMU-BMI160.c ../Analogue.c ../NandMulti.c ../NandMulti1.c ../NandMulti2.c ../../Common/BootCheck.c


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
	${MAKE}  -f nbproject/Makefile-Release.mk dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=24FJ1024GB606
MP_LINKER_FILE_OPTION=,--script="..\..\Common\hid_boot_p24FJ1024GB606.gld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1232478524/Ecc.o: ../../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232478524" 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ecc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Ftl/Ecc.c  -o ${OBJECTDIR}/_ext/1232478524/Ecc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232478524/Ecc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232478524/Ecc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1232478524/FsFtl.o: ../../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232478524" 
	@${RM} ${OBJECTDIR}/_ext/1232478524/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232478524/FsFtl.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Ftl/FsFtl.c  -o ${OBJECTDIR}/_ext/1232478524/FsFtl.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232478524/FsFtl.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232478524/FsFtl.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1232478524/Ftl.o: ../../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232478524" 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ftl.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Ftl/Ftl.c  -o ${OBJECTDIR}/_ext/1232478524/Ftl.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232478524/Ftl.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232478524/Ftl.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o: ../../../../../Flux/src/Usb/USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232492898" 
	@${RM} ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Usb/USB_CDC_MSD.c  -o ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/985258737/Fifo.o: ../../../../../Flux/src/Utils/Fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/985258737" 
	@${RM} ${OBJECTDIR}/_ext/985258737/Fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/985258737/Fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Utils/Fifo.c  -o ${OBJECTDIR}/_ext/985258737/Fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/985258737/Fifo.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/985258737/Fifo.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/985258737/Util.o: ../../../../../Flux/src/Utils/Util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/985258737" 
	@${RM} ${OBJECTDIR}/_ext/985258737/Util.o.d 
	@${RM} ${OBJECTDIR}/_ext/985258737/Util.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Utils/Util.c  -o ${OBJECTDIR}/_ext/985258737/Util.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/985258737/Util.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/985258737/Util.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/985258737/FSutils.o: ../../../../../Flux/src/Utils/FSutils.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/985258737" 
	@${RM} ${OBJECTDIR}/_ext/985258737/FSutils.o.d 
	@${RM} ${OBJECTDIR}/_ext/985258737/FSutils.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Utils/FSutils.c  -o ${OBJECTDIR}/_ext/985258737/FSutils.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/985258737/FSutils.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/985258737/FSutils.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/916655611/TimeDelay.o: C:/microchip_solutions_v2013-06-15/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/916655611" 
	@${RM} ${OBJECTDIR}/_ext/916655611/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/916655611/TimeDelay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  C:/microchip_solutions_v2013-06-15/Microchip/Common/TimeDelay.c  -o ${OBJECTDIR}/_ext/916655611/TimeDelay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/916655611/TimeDelay.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/916655611/TimeDelay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1961590870/usb_device.o: C:/microchip_solutions_v2013-06-15/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1961590870" 
	@${RM} ${OBJECTDIR}/_ext/1961590870/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1961590870/usb_device.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  C:/microchip_solutions_v2013-06-15/Microchip/USB/usb_device.c  -o ${OBJECTDIR}/_ext/1961590870/usb_device.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1961590870/usb_device.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1961590870/usb_device.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/115247727/usb_function_cdc.o: C:/microchip_solutions_v2013-06-15/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/115247727" 
	@${RM} ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  "C:/microchip_solutions_v2013-06-15/Microchip/USB/CDC Device Driver/usb_function_cdc.c"  -o ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1092050741/usb_function_msd.o: C:/microchip_solutions_v2013-06-15/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1092050741" 
	@${RM} ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  "C:/microchip_solutions_v2013-06-15/Microchip/USB/MSD Device Driver/usb_function_msd.c"  -o ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Logger.c  -o ${OBJECTDIR}/_ext/1472/Logger.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Logger.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Logger.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Settings.c  -o ${OBJECTDIR}/_ext/1472/Settings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Settings.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Settings.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../main.c  -o ${OBJECTDIR}/_ext/1472/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../usb_descriptors.c  -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/AX3\ SelfTest.o: ../AX3\ SelfTest.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o".d 
	@${RM} "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o" 
	${MP_CC} $(MP_EXTRA_CC_PRE)  "../AX3 SelfTest.c"  -o "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o"  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o: ../HardwareProfile-CWA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../HardwareProfile-CWA.c  -o ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/FSIO.o: ../FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/FSIO.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../FSIO.c  -o ${OBJECTDIR}/_ext/1472/FSIO.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/FSIO.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/FSIO.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/bmi160.o: ../bmi160.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/bmi160.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/bmi160.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../bmi160.c  -o ${OBJECTDIR}/_ext/1472/bmi160.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/bmi160.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bmi160.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/rtcc.o: ../rtcc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/rtcc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/rtcc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../rtcc.c  -o ${OBJECTDIR}/_ext/1472/rtcc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/rtcc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/rtcc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/bmm150.o: ../bmm150.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/bmm150.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/bmm150.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../bmm150.c  -o ${OBJECTDIR}/_ext/1472/bmm150.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/bmm150.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bmm150.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2002583637/Rtc.o: ../../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2002583637" 
	@${RM} ${OBJECTDIR}/_ext/2002583637/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/2002583637/Rtc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Peripherals/Rtc.c  -o ${OBJECTDIR}/_ext/2002583637/Rtc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2002583637/Rtc.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2002583637/Rtc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/IMU-BMI160.o: ../IMU-BMI160.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/IMU-BMI160.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../IMU-BMI160.c  -o ${OBJECTDIR}/_ext/1472/IMU-BMI160.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Analogue.o: ../Analogue.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Analogue.c  -o ${OBJECTDIR}/_ext/1472/Analogue.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Analogue.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Analogue.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/NandMulti.o: ../NandMulti.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../NandMulti.c  -o ${OBJECTDIR}/_ext/1472/NandMulti.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/NandMulti.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/NandMulti.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/NandMulti1.o: ../NandMulti1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../NandMulti1.c  -o ${OBJECTDIR}/_ext/1472/NandMulti1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/NandMulti1.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/NandMulti1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/NandMulti2.o: ../NandMulti2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti2.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../NandMulti2.c  -o ${OBJECTDIR}/_ext/1472/NandMulti2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/NandMulti2.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/NandMulti2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2049610667/BootCheck.o: ../../Common/BootCheck.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2049610667" 
	@${RM} ${OBJECTDIR}/_ext/2049610667/BootCheck.o.d 
	@${RM} ${OBJECTDIR}/_ext/2049610667/BootCheck.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Common/BootCheck.c  -o ${OBJECTDIR}/_ext/2049610667/BootCheck.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2049610667/BootCheck.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -mno-eds-warn  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2049610667/BootCheck.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/_ext/1232478524/Ecc.o: ../../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232478524" 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ecc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Ftl/Ecc.c  -o ${OBJECTDIR}/_ext/1232478524/Ecc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232478524/Ecc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232478524/Ecc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1232478524/FsFtl.o: ../../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232478524" 
	@${RM} ${OBJECTDIR}/_ext/1232478524/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232478524/FsFtl.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Ftl/FsFtl.c  -o ${OBJECTDIR}/_ext/1232478524/FsFtl.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232478524/FsFtl.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232478524/FsFtl.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1232478524/Ftl.o: ../../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232478524" 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232478524/Ftl.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Ftl/Ftl.c  -o ${OBJECTDIR}/_ext/1232478524/Ftl.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232478524/Ftl.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232478524/Ftl.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o: ../../../../../Flux/src/Usb/USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1232492898" 
	@${RM} ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Usb/USB_CDC_MSD.c  -o ${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1232492898/USB_CDC_MSD.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/985258737/Fifo.o: ../../../../../Flux/src/Utils/Fifo.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/985258737" 
	@${RM} ${OBJECTDIR}/_ext/985258737/Fifo.o.d 
	@${RM} ${OBJECTDIR}/_ext/985258737/Fifo.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Utils/Fifo.c  -o ${OBJECTDIR}/_ext/985258737/Fifo.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/985258737/Fifo.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/985258737/Fifo.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/985258737/Util.o: ../../../../../Flux/src/Utils/Util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/985258737" 
	@${RM} ${OBJECTDIR}/_ext/985258737/Util.o.d 
	@${RM} ${OBJECTDIR}/_ext/985258737/Util.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Utils/Util.c  -o ${OBJECTDIR}/_ext/985258737/Util.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/985258737/Util.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/985258737/Util.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/985258737/FSutils.o: ../../../../../Flux/src/Utils/FSutils.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/985258737" 
	@${RM} ${OBJECTDIR}/_ext/985258737/FSutils.o.d 
	@${RM} ${OBJECTDIR}/_ext/985258737/FSutils.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Utils/FSutils.c  -o ${OBJECTDIR}/_ext/985258737/FSutils.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/985258737/FSutils.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/985258737/FSutils.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/916655611/TimeDelay.o: C:/microchip_solutions_v2013-06-15/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/916655611" 
	@${RM} ${OBJECTDIR}/_ext/916655611/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/916655611/TimeDelay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  C:/microchip_solutions_v2013-06-15/Microchip/Common/TimeDelay.c  -o ${OBJECTDIR}/_ext/916655611/TimeDelay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/916655611/TimeDelay.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/916655611/TimeDelay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1961590870/usb_device.o: C:/microchip_solutions_v2013-06-15/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1961590870" 
	@${RM} ${OBJECTDIR}/_ext/1961590870/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1961590870/usb_device.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  C:/microchip_solutions_v2013-06-15/Microchip/USB/usb_device.c  -o ${OBJECTDIR}/_ext/1961590870/usb_device.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1961590870/usb_device.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1961590870/usb_device.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/115247727/usb_function_cdc.o: C:/microchip_solutions_v2013-06-15/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/115247727" 
	@${RM} ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  "C:/microchip_solutions_v2013-06-15/Microchip/USB/CDC Device Driver/usb_function_cdc.c"  -o ${OBJECTDIR}/_ext/115247727/usb_function_cdc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/115247727/usb_function_cdc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1092050741/usb_function_msd.o: C:/microchip_solutions_v2013-06-15/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1092050741" 
	@${RM} ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  "C:/microchip_solutions_v2013-06-15/Microchip/USB/MSD Device Driver/usb_function_msd.c"  -o ${OBJECTDIR}/_ext/1092050741/usb_function_msd.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1092050741/usb_function_msd.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Logger.c  -o ${OBJECTDIR}/_ext/1472/Logger.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Logger.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Logger.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Settings.c  -o ${OBJECTDIR}/_ext/1472/Settings.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Settings.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Settings.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../main.c  -o ${OBJECTDIR}/_ext/1472/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/main.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../usb_descriptors.c  -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/usb_descriptors.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/AX3\ SelfTest.o: ../AX3\ SelfTest.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o".d 
	@${RM} "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o" 
	${MP_CC} $(MP_EXTRA_CC_PRE)  "../AX3 SelfTest.c"  -o "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o"  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/AX3 SelfTest.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o: ../HardwareProfile-CWA.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../HardwareProfile-CWA.c  -o ${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/HardwareProfile-CWA.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/FSIO.o: ../FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/FSIO.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../FSIO.c  -o ${OBJECTDIR}/_ext/1472/FSIO.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/FSIO.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/FSIO.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/bmi160.o: ../bmi160.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/bmi160.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/bmi160.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../bmi160.c  -o ${OBJECTDIR}/_ext/1472/bmi160.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/bmi160.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bmi160.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/rtcc.o: ../rtcc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/rtcc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/rtcc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../rtcc.c  -o ${OBJECTDIR}/_ext/1472/rtcc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/rtcc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/rtcc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/bmm150.o: ../bmm150.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/bmm150.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/bmm150.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../bmm150.c  -o ${OBJECTDIR}/_ext/1472/bmm150.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/bmm150.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/bmm150.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2002583637/Rtc.o: ../../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2002583637" 
	@${RM} ${OBJECTDIR}/_ext/2002583637/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/2002583637/Rtc.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../../../../Flux/src/Peripherals/Rtc.c  -o ${OBJECTDIR}/_ext/2002583637/Rtc.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2002583637/Rtc.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2002583637/Rtc.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/IMU-BMI160.o: ../IMU-BMI160.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/IMU-BMI160.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../IMU-BMI160.c  -o ${OBJECTDIR}/_ext/1472/IMU-BMI160.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/IMU-BMI160.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/Analogue.o: ../Analogue.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../Analogue.c  -o ${OBJECTDIR}/_ext/1472/Analogue.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/Analogue.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/Analogue.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/NandMulti.o: ../NandMulti.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../NandMulti.c  -o ${OBJECTDIR}/_ext/1472/NandMulti.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/NandMulti.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/NandMulti.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/NandMulti1.o: ../NandMulti1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti1.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti1.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../NandMulti1.c  -o ${OBJECTDIR}/_ext/1472/NandMulti1.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/NandMulti1.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/NandMulti1.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/1472/NandMulti2.o: ../NandMulti2.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1472" 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti2.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/NandMulti2.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../NandMulti2.c  -o ${OBJECTDIR}/_ext/1472/NandMulti2.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/1472/NandMulti2.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/1472/NandMulti2.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/_ext/2049610667/BootCheck.o: ../../Common/BootCheck.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/2049610667" 
	@${RM} ${OBJECTDIR}/_ext/2049610667/BootCheck.o.d 
	@${RM} ${OBJECTDIR}/_ext/2049610667/BootCheck.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  ../../Common/BootCheck.c  -o ${OBJECTDIR}/_ext/2049610667/BootCheck.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/_ext/2049610667/BootCheck.o.d"      -mno-eds-warn  -g -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -mlarge-code -mlarge-data -O3 -I"." -I".." -I"../../../../../Flux/include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -I"../../Common" -msmart-io=1 -msfr-warn=off  
	@${FIXDEPS} "${OBJECTDIR}/_ext/2049610667/BootCheck.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../../Common/hid_boot_p24FJ1024GB606.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1  -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include"  -mreserve=data@0x800:0x81B -mreserve=data@0x81C:0x81D -mreserve=data@0x81E:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x827 -mreserve=data@0x82A:0x84F   -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1,$(MP_LINKER_FILE_OPTION),--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="../OutputMap.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../../Common/hid_boot_p24FJ1024GB606.gld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_Release=$(CND_CONF)  -no-legacy-libc  $(COMPARISON_BUILD)  -I"." -I".." -I"../../../../../Flux/Include" -I"C:/microchip_solutions_v2013-06-15/Microchip/Include" -Wl,--local-stack,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="../OutputMap.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release
	${RM} -r dist/Release

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
