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
MKDIR=mkdir -p
RM=rm -f 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf
else
IMAGE_TYPE=production
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1139140331/usb_function_msd.o ${OBJECTDIR}/_ext/1302440332/usb_device.o ${OBJECTDIR}/_ext/1472/Analogue.o ${OBJECTDIR}/_ext/1472/Data.o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ${OBJECTDIR}/_ext/1472/Logger.o ${OBJECTDIR}/_ext/1472/Settings.o ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o ${OBJECTDIR}/_ext/1472/main.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/util.o ${OBJECTDIR}/_ext/1660554189/Ecc.o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ${OBJECTDIR}/_ext/1660554189/Ftl.o ${OBJECTDIR}/_ext/276258160/FSIO.o ${OBJECTDIR}/_ext/434517530/Accel.o ${OBJECTDIR}/_ext/434517530/Gyro.o ${OBJECTDIR}/_ext/434517530/Nand.o ${OBJECTDIR}/_ext/434517530/Rtc.o ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o ${OBJECTDIR}/_ext/753455365/TimeDelay.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

# Path to java used to run MPLAB X when this makefile was created
MP_JAVA_PATH=C:\\Program\ Files\ \(x86\)\\Java\\jre6/bin/
OS_CURRENT="$(shell uname -s)"
############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
MP_CC=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin\\pic30-gcc.exe
# MP_BC is not defined
MP_AS=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin\\pic30-as.exe
MP_LD=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin\\pic30-ld.exe
MP_AR=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin\\pic30-ar.exe
# MP_BC is not defined
MP_CC_DIR=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin
# MP_BC_DIR is not defined
MP_AS_DIR=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin
MP_LD_DIR=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin
MP_AR_DIR=C:\\Program\ Files\ \(x86\)\\Microchip\\MPLAB\ C30\\bin
# MP_BC_DIR is not defined

.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf

MP_PROCESSOR_OPTION=24FJ256GB106
MP_LINKER_FILE_OPTION=,--script=../app_hid_boot_p24FJ256GB106.gld
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
${OBJECTDIR}/_ext/1472/HardwareProfile.o: ../HardwareProfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d -o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ../HardwareProfile.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d -o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ../HardwareProfile.c    2>&1  > ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d > ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d > ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err 
	@cat ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Accel.o: ../../../../Flux/src/Peripherals/Accel.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.ok ${OBJECTDIR}/_ext/434517530/Accel.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Accel.o.d -o ${OBJECTDIR}/_ext/434517530/Accel.o ../../../../Flux/src/Peripherals/Accel.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Accel.o.d -o ${OBJECTDIR}/_ext/434517530/Accel.o ../../../../Flux/src/Peripherals/Accel.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Accel.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Accel.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Accel.o.d > ${OBJECTDIR}/_ext/434517530/Accel.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Accel.o.d > ${OBJECTDIR}/_ext/434517530/Accel.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Accel.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Accel.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Accel.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Accel.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/68158137/usb_function_cdc.o: /C/Microchip\ Solutions/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/68158137 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d -o ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o /C/Microchip\ Solutions/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d -o ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o /C/Microchip\ Solutions/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c    2>&1  > ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d > ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${CP} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d > ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${CP} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err 
	@cat ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err 
	@if [ -f ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.ok ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Settings.o.d -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Settings.o.d -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c    2>&1  > ${OBJECTDIR}/_ext/1472/Settings.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Settings.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Settings.o.d > ${OBJECTDIR}/_ext/1472/Settings.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Settings.o.tmp ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Settings.o.d > ${OBJECTDIR}/_ext/1472/Settings.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Settings.o.tmp ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Settings.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Settings.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Analogue.o: ../Analogue.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.ok ${OBJECTDIR}/_ext/1472/Analogue.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Analogue.o.d -o ${OBJECTDIR}/_ext/1472/Analogue.o ../Analogue.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Analogue.o.d -o ${OBJECTDIR}/_ext/1472/Analogue.o ../Analogue.c    2>&1  > ${OBJECTDIR}/_ext/1472/Analogue.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Analogue.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Analogue.o.d > ${OBJECTDIR}/_ext/1472/Analogue.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Analogue.o.d > ${OBJECTDIR}/_ext/1472/Analogue.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Analogue.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Analogue.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Analogue.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Analogue.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c    2>&1  > ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d > ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d > ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@cat ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/util.o: ../util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.ok ${OBJECTDIR}/_ext/1472/util.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/util.o.d -o ${OBJECTDIR}/_ext/1472/util.o ../util.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/util.o.d -o ${OBJECTDIR}/_ext/1472/util.o ../util.c    2>&1  > ${OBJECTDIR}/_ext/1472/util.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/util.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/util.o.d > ${OBJECTDIR}/_ext/1472/util.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/util.o.tmp ${OBJECTDIR}/_ext/1472/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/util.o.d > ${OBJECTDIR}/_ext/1472/util.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/util.o.tmp ${OBJECTDIR}/_ext/1472/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/util.o.err 
	@cat ${OBJECTDIR}/_ext/1472/util.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/util.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/util.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/276258160/FSIO.o: ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/276258160 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/276258160/FSIO.o.d -o ${OBJECTDIR}/_ext/276258160/FSIO.o ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/276258160/FSIO.o.d -o ${OBJECTDIR}/_ext/276258160/FSIO.o ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c    2>&1  > ${OBJECTDIR}/_ext/276258160/FSIO.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/276258160/FSIO.o.d > ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${CP} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/276258160/FSIO.o.d > ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${CP} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@cat ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@if [ -f ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/276258160/FSIO.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.ok ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Logger.o.d -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Logger.o.d -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c    2>&1  > ${OBJECTDIR}/_ext/1472/Logger.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Logger.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Logger.o.d > ${OBJECTDIR}/_ext/1472/Logger.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Logger.o.tmp ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Logger.o.d > ${OBJECTDIR}/_ext/1472/Logger.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Logger.o.tmp ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Logger.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Logger.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1139140331/usb_function_msd.o: /C/Microchip\ Solutions/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1139140331 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d -o ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o /C/Microchip\ Solutions/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d -o ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o /C/Microchip\ Solutions/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c    2>&1  > ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d > ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${CP} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d > ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${CP} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err 
	@cat ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Nand.o: ../../../../Flux/src/Peripherals/Nand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.ok ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Nand.o.d -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Nand.o.d -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Nand.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Nand.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Nand.o.d > ${OBJECTDIR}/_ext/434517530/Nand.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Nand.o.d > ${OBJECTDIR}/_ext/434517530/Nand.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Nand.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Nand.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1660554189/Ecc.o: ../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ecc.o.d -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ecc.o.d -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c    2>&1  > ${OBJECTDIR}/_ext/1660554189/Ecc.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1660554189/Ecc.o.d > ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1660554189/Ecc.o.d > ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@cat ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Data.o: ../Data.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.ok ${OBJECTDIR}/_ext/1472/Data.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Data.o.d -o ${OBJECTDIR}/_ext/1472/Data.o ../Data.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Data.o.d -o ${OBJECTDIR}/_ext/1472/Data.o ../Data.c    2>&1  > ${OBJECTDIR}/_ext/1472/Data.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Data.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Data.o.d > ${OBJECTDIR}/_ext/1472/Data.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Data.o.tmp ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Data.o.d > ${OBJECTDIR}/_ext/1472/Data.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Data.o.tmp ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Data.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Data.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Data.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Data.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1660554189/Ftl.o: ../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ftl.o.d -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ftl.o.d -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c    2>&1  > ${OBJECTDIR}/_ext/1660554189/Ftl.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1660554189/Ftl.o.d > ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1660554189/Ftl.o.d > ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@cat ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Rtc.o: ../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Rtc.o.d -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Rtc.o.d -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Rtc.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Rtc.o.d > ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Rtc.o.d > ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Rtc.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.ok ${OBJECTDIR}/_ext/1472/main.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/main.o.d -o ${OBJECTDIR}/_ext/1472/main.o ../main.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/main.o.d -o ${OBJECTDIR}/_ext/1472/main.o ../main.c    2>&1  > ${OBJECTDIR}/_ext/1472/main.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/main.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/main.o.d > ${OBJECTDIR}/_ext/1472/main.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/main.o.tmp ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/main.o.d > ${OBJECTDIR}/_ext/1472/main.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/main.o.tmp ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/main.o.err 
	@cat ${OBJECTDIR}/_ext/1472/main.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/main.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/main.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1660554189/FsFtl.o: ../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c    2>&1  > ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d > ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d > ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@cat ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1302440332/usb_device.o: /C/Microchip\ Solutions/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1302440332 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok ${OBJECTDIR}/_ext/1302440332/usb_device.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1302440332/usb_device.o.d -o ${OBJECTDIR}/_ext/1302440332/usb_device.o /C/Microchip\ Solutions/Microchip/USB/usb_device.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1302440332/usb_device.o.d -o ${OBJECTDIR}/_ext/1302440332/usb_device.o /C/Microchip\ Solutions/Microchip/USB/usb_device.c    2>&1  > ${OBJECTDIR}/_ext/1302440332/usb_device.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1302440332/usb_device.o.d > ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${CP} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1302440332/usb_device.o.d > ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${CP} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1302440332/usb_device.o.err 
	@cat ${OBJECTDIR}/_ext/1302440332/usb_device.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Gyro.o: ../../../../Flux/src/Peripherals/Gyro.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.ok ${OBJECTDIR}/_ext/434517530/Gyro.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Gyro.o.d -o ${OBJECTDIR}/_ext/434517530/Gyro.o ../../../../Flux/src/Peripherals/Gyro.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Gyro.o.d -o ${OBJECTDIR}/_ext/434517530/Gyro.o ../../../../Flux/src/Peripherals/Gyro.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Gyro.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Gyro.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Gyro.o.d > ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Gyro.o.d > ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Gyro.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Gyro.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Gyro.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Gyro.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/753455365/TimeDelay.o: /C/Microchip\ Solutions/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/753455365 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d -o ${OBJECTDIR}/_ext/753455365/TimeDelay.o /C/Microchip\ Solutions/Microchip/Common/TimeDelay.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d -o ${OBJECTDIR}/_ext/753455365/TimeDelay.o /C/Microchip\ Solutions/Microchip/Common/TimeDelay.c    2>&1  > ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d > ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${CP} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d > ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${CP} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err 
	@cat ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err 
	@if [ -f ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o: ../USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d -o ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o ../USB_CDC_MSD.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d -o ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o ../USB_CDC_MSD.c    2>&1  > ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d > ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d > ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err 
	@cat ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok; else exit 1; fi
	
else
${OBJECTDIR}/_ext/1472/HardwareProfile.o: ../HardwareProfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d -o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ../HardwareProfile.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d -o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ../HardwareProfile.c    2>&1  > ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d > ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d > ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err 
	@cat ${OBJECTDIR}/_ext/1472/HardwareProfile.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/HardwareProfile.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Accel.o: ../../../../Flux/src/Peripherals/Accel.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.ok ${OBJECTDIR}/_ext/434517530/Accel.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Accel.o.d -o ${OBJECTDIR}/_ext/434517530/Accel.o ../../../../Flux/src/Peripherals/Accel.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Accel.o.d -o ${OBJECTDIR}/_ext/434517530/Accel.o ../../../../Flux/src/Peripherals/Accel.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Accel.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Accel.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Accel.o.d > ${OBJECTDIR}/_ext/434517530/Accel.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Accel.o.d > ${OBJECTDIR}/_ext/434517530/Accel.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp ${OBJECTDIR}/_ext/434517530/Accel.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Accel.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Accel.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Accel.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Accel.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Accel.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/68158137/usb_function_cdc.o: /C/Microchip\ Solutions/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/68158137 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d -o ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o /C/Microchip\ Solutions/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d -o ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o /C/Microchip\ Solutions/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c    2>&1  > ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d > ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${CP} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d > ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${CP} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.d 
	@${RM} ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err 
	@cat ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.err 
	@if [ -f ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/68158137/usb_function_cdc.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Settings.o: ../Settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.ok ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Settings.o.d -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Settings.o.d -o ${OBJECTDIR}/_ext/1472/Settings.o ../Settings.c    2>&1  > ${OBJECTDIR}/_ext/1472/Settings.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Settings.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Settings.o.d > ${OBJECTDIR}/_ext/1472/Settings.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Settings.o.tmp ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Settings.o.d > ${OBJECTDIR}/_ext/1472/Settings.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Settings.o.tmp ${OBJECTDIR}/_ext/1472/Settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Settings.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Settings.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Settings.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Settings.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Analogue.o: ../Analogue.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.ok ${OBJECTDIR}/_ext/1472/Analogue.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Analogue.o.d -o ${OBJECTDIR}/_ext/1472/Analogue.o ../Analogue.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Analogue.o.d -o ${OBJECTDIR}/_ext/1472/Analogue.o ../Analogue.c    2>&1  > ${OBJECTDIR}/_ext/1472/Analogue.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Analogue.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Analogue.o.d > ${OBJECTDIR}/_ext/1472/Analogue.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Analogue.o.d > ${OBJECTDIR}/_ext/1472/Analogue.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp ${OBJECTDIR}/_ext/1472/Analogue.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Analogue.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Analogue.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Analogue.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Analogue.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Analogue.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d -o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ../usb_descriptors.c    2>&1  > ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d > ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d > ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@cat ${OBJECTDIR}/_ext/1472/usb_descriptors.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/usb_descriptors.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/util.o: ../util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.ok ${OBJECTDIR}/_ext/1472/util.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/util.o.d -o ${OBJECTDIR}/_ext/1472/util.o ../util.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/util.o.d -o ${OBJECTDIR}/_ext/1472/util.o ../util.c    2>&1  > ${OBJECTDIR}/_ext/1472/util.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/util.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/util.o.d > ${OBJECTDIR}/_ext/1472/util.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/util.o.tmp ${OBJECTDIR}/_ext/1472/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/util.o.d > ${OBJECTDIR}/_ext/1472/util.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/util.o.tmp ${OBJECTDIR}/_ext/1472/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/util.o.err 
	@cat ${OBJECTDIR}/_ext/1472/util.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/util.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/util.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/276258160/FSIO.o: ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/276258160 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/276258160/FSIO.o.d -o ${OBJECTDIR}/_ext/276258160/FSIO.o ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/276258160/FSIO.o.d -o ${OBJECTDIR}/_ext/276258160/FSIO.o ../../../../Flux/src/Microchip/MDD\ File\ System/FSIO.c    2>&1  > ${OBJECTDIR}/_ext/276258160/FSIO.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/276258160/FSIO.o.d > ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${CP} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/276258160/FSIO.o.d > ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${CP} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp ${OBJECTDIR}/_ext/276258160/FSIO.o.d 
	@${RM} ${OBJECTDIR}/_ext/276258160/FSIO.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@cat ${OBJECTDIR}/_ext/276258160/FSIO.o.err 
	@if [ -f ${OBJECTDIR}/_ext/276258160/FSIO.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/276258160/FSIO.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Logger.o: ../Logger.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.ok ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Logger.o.d -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Logger.o.d -o ${OBJECTDIR}/_ext/1472/Logger.o ../Logger.c    2>&1  > ${OBJECTDIR}/_ext/1472/Logger.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Logger.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Logger.o.d > ${OBJECTDIR}/_ext/1472/Logger.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Logger.o.tmp ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Logger.o.d > ${OBJECTDIR}/_ext/1472/Logger.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Logger.o.tmp ${OBJECTDIR}/_ext/1472/Logger.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Logger.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Logger.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Logger.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Logger.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1139140331/usb_function_msd.o: /C/Microchip\ Solutions/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1139140331 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d -o ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o /C/Microchip\ Solutions/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d -o ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o /C/Microchip\ Solutions/Microchip/USB/MSD\ Device\ Driver/usb_function_msd.c    2>&1  > ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d > ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${CP} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d > ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${CP} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.d 
	@${RM} ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err 
	@cat ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1139140331/usb_function_msd.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Nand.o: ../../../../Flux/src/Peripherals/Nand.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.ok ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Nand.o.d -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Nand.o.d -o ${OBJECTDIR}/_ext/434517530/Nand.o ../../../../Flux/src/Peripherals/Nand.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Nand.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Nand.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Nand.o.d > ${OBJECTDIR}/_ext/434517530/Nand.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Nand.o.d > ${OBJECTDIR}/_ext/434517530/Nand.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp ${OBJECTDIR}/_ext/434517530/Nand.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Nand.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Nand.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Nand.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Nand.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1660554189/Ecc.o: ../../../../Flux/src/Ftl/Ecc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ecc.o.d -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ecc.o.d -o ${OBJECTDIR}/_ext/1660554189/Ecc.o ../../../../Flux/src/Ftl/Ecc.c    2>&1  > ${OBJECTDIR}/_ext/1660554189/Ecc.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1660554189/Ecc.o.d > ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1660554189/Ecc.o.d > ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp ${OBJECTDIR}/_ext/1660554189/Ecc.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ecc.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@cat ${OBJECTDIR}/_ext/1660554189/Ecc.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1660554189/Ecc.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/Data.o: ../Data.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.ok ${OBJECTDIR}/_ext/1472/Data.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Data.o.d -o ${OBJECTDIR}/_ext/1472/Data.o ../Data.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/Data.o.d -o ${OBJECTDIR}/_ext/1472/Data.o ../Data.c    2>&1  > ${OBJECTDIR}/_ext/1472/Data.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/Data.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/Data.o.d > ${OBJECTDIR}/_ext/1472/Data.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Data.o.tmp ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/Data.o.d > ${OBJECTDIR}/_ext/1472/Data.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/Data.o.tmp ${OBJECTDIR}/_ext/1472/Data.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/Data.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/Data.o.err 
	@cat ${OBJECTDIR}/_ext/1472/Data.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/Data.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/Data.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1660554189/Ftl.o: ../../../../Flux/src/Ftl/Ftl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ftl.o.d -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/Ftl.o.d -o ${OBJECTDIR}/_ext/1660554189/Ftl.o ../../../../Flux/src/Ftl/Ftl.c    2>&1  > ${OBJECTDIR}/_ext/1660554189/Ftl.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1660554189/Ftl.o.d > ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1660554189/Ftl.o.d > ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp ${OBJECTDIR}/_ext/1660554189/Ftl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/Ftl.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@cat ${OBJECTDIR}/_ext/1660554189/Ftl.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1660554189/Ftl.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Rtc.o: ../../../../Flux/src/Peripherals/Rtc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Rtc.o.d -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Rtc.o.d -o ${OBJECTDIR}/_ext/434517530/Rtc.o ../../../../Flux/src/Peripherals/Rtc.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Rtc.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Rtc.o.d > ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Rtc.o.d > ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp ${OBJECTDIR}/_ext/434517530/Rtc.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Rtc.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Rtc.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Rtc.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Rtc.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/main.o: ../main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.ok ${OBJECTDIR}/_ext/1472/main.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/main.o.d -o ${OBJECTDIR}/_ext/1472/main.o ../main.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/main.o.d -o ${OBJECTDIR}/_ext/1472/main.o ../main.c    2>&1  > ${OBJECTDIR}/_ext/1472/main.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/main.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/main.o.d > ${OBJECTDIR}/_ext/1472/main.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/main.o.tmp ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/main.o.d > ${OBJECTDIR}/_ext/1472/main.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/main.o.tmp ${OBJECTDIR}/_ext/1472/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/main.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/main.o.err 
	@cat ${OBJECTDIR}/_ext/1472/main.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/main.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/main.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1660554189/FsFtl.o: ../../../../Flux/src/Ftl/FsFtl.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1660554189 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d -o ${OBJECTDIR}/_ext/1660554189/FsFtl.o ../../../../Flux/src/Ftl/FsFtl.c    2>&1  > ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d > ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d > ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${CP} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp ${OBJECTDIR}/_ext/1660554189/FsFtl.o.d 
	@${RM} ${OBJECTDIR}/_ext/1660554189/FsFtl.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@cat ${OBJECTDIR}/_ext/1660554189/FsFtl.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1660554189/FsFtl.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1302440332/usb_device.o: /C/Microchip\ Solutions/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1302440332 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok ${OBJECTDIR}/_ext/1302440332/usb_device.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1302440332/usb_device.o.d -o ${OBJECTDIR}/_ext/1302440332/usb_device.o /C/Microchip\ Solutions/Microchip/USB/usb_device.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1302440332/usb_device.o.d -o ${OBJECTDIR}/_ext/1302440332/usb_device.o /C/Microchip\ Solutions/Microchip/USB/usb_device.c    2>&1  > ${OBJECTDIR}/_ext/1302440332/usb_device.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1302440332/usb_device.o.d > ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${CP} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1302440332/usb_device.o.d > ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${CP} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp ${OBJECTDIR}/_ext/1302440332/usb_device.o.d 
	@${RM} ${OBJECTDIR}/_ext/1302440332/usb_device.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1302440332/usb_device.o.err 
	@cat ${OBJECTDIR}/_ext/1302440332/usb_device.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1302440332/usb_device.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/434517530/Gyro.o: ../../../../Flux/src/Peripherals/Gyro.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/434517530 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.ok ${OBJECTDIR}/_ext/434517530/Gyro.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Gyro.o.d -o ${OBJECTDIR}/_ext/434517530/Gyro.o ../../../../Flux/src/Peripherals/Gyro.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/434517530/Gyro.o.d -o ${OBJECTDIR}/_ext/434517530/Gyro.o ../../../../Flux/src/Peripherals/Gyro.c    2>&1  > ${OBJECTDIR}/_ext/434517530/Gyro.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/434517530/Gyro.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/434517530/Gyro.o.d > ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/434517530/Gyro.o.d > ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${CP} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp ${OBJECTDIR}/_ext/434517530/Gyro.o.d 
	@${RM} ${OBJECTDIR}/_ext/434517530/Gyro.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/434517530/Gyro.o.err 
	@cat ${OBJECTDIR}/_ext/434517530/Gyro.o.err 
	@if [ -f ${OBJECTDIR}/_ext/434517530/Gyro.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/434517530/Gyro.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/753455365/TimeDelay.o: /C/Microchip\ Solutions/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/753455365 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d -o ${OBJECTDIR}/_ext/753455365/TimeDelay.o /C/Microchip\ Solutions/Microchip/Common/TimeDelay.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d -o ${OBJECTDIR}/_ext/753455365/TimeDelay.o /C/Microchip\ Solutions/Microchip/Common/TimeDelay.c    2>&1  > ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d > ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${CP} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d > ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${CP} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp ${OBJECTDIR}/_ext/753455365/TimeDelay.o.d 
	@${RM} ${OBJECTDIR}/_ext/753455365/TimeDelay.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err 
	@cat ${OBJECTDIR}/_ext/753455365/TimeDelay.o.err 
	@if [ -f ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/753455365/TimeDelay.o.ok; else exit 1; fi
	
${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o: ../USB_CDC_MSD.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err 
	@echo ${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d -o ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o ../USB_CDC_MSD.c  
	@-${MP_CC} $(MP_EXTRA_CC_PRE)  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -I".." -I"C:/Microchip Solutions/Microchip/Include" -I"../../../../Flux/include" -mlarge-code -mlarge-data -MMD -MF ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d -o ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o ../USB_CDC_MSD.c    2>&1  > ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err ; if [ $$? -eq 0 ] ; then touch ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok ; fi 
ifneq (,$(findstring MINGW32,$(OS_CURRENT))) 
	@sed -e 's/\"//g' -e 's/\\$$/__EOL__/g' -e 's/\\ /__ESCAPED_SPACES__/g' -e 's/\\/\//g' -e 's/__ESCAPED_SPACES__/\\ /g' -e 's/__EOL__$$/\\/g' ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d > ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp 
else 
	@sed -e 's/\"//g' ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d > ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${CP} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.d 
	@${RM} ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.tmp
endif
	@touch ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err 
	@cat ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.err 
	@if [ -f ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok ] ; then rm -f ${OBJECTDIR}/_ext/1472/USB_CDC_MSD.o.ok; else exit 1; fi
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf  -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf ${OBJECTFILES}        -Wl,--defsym=__MPLAB_BUILD=1,--report-mem$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__ICD2RAM=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1
else
dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf  -mcpu=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf ${OBJECTFILES}        -Wl,--defsym=__MPLAB_BUILD=1,--report-mem$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\pic30-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/CWA.X.${IMAGE_TYPE}.elf -omf=elf
endif


# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
