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
OUTPUT_SUFFIX=cof
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=cof
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1472/accel.o ${OBJECTDIR}/_ext/1472/data.o ${OBJECTDIR}/_ext/1472/usb_cdc.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/util.o ${OBJECTDIR}/_ext/1472/spi.o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ${OBJECTDIR}/_ext/1472/main-receiver.o ${OBJECTDIR}/_ext/1472/main-transmitter.o ${OBJECTDIR}/_ext/1472/c018i.o ${OBJECTDIR}/_ext/1453245926/TimeDelay.o ${OBJECTDIR}/_ext/828323842/Console.o ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o ${OBJECTDIR}/_ext/686890857/usb_device.o ${OBJECTDIR}/_ext/1736990619/MiWi.o ${OBJECTDIR}/_ext/828323842/NVM.o ${OBJECTDIR}/_ext/828323842/SymbolTime.o ${OBJECTDIR}/_ext/2134243105/P2P.o ${OBJECTDIR}/_ext/1999247862/crc.o ${OBJECTDIR}/_ext/1999247862/security.o ${OBJECTDIR}/_ext/1472/MRF24J40.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1472/accel.o.d ${OBJECTDIR}/_ext/1472/data.o.d ${OBJECTDIR}/_ext/1472/usb_cdc.o.d ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d ${OBJECTDIR}/_ext/1472/util.o.d ${OBJECTDIR}/_ext/1472/spi.o.d ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d ${OBJECTDIR}/_ext/1472/main-receiver.o.d ${OBJECTDIR}/_ext/1472/main-transmitter.o.d ${OBJECTDIR}/_ext/1472/c018i.o.d ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d ${OBJECTDIR}/_ext/828323842/Console.o.d ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o.d ${OBJECTDIR}/_ext/686890857/usb_device.o.d ${OBJECTDIR}/_ext/1736990619/MiWi.o.d ${OBJECTDIR}/_ext/828323842/NVM.o.d ${OBJECTDIR}/_ext/828323842/SymbolTime.o.d ${OBJECTDIR}/_ext/2134243105/P2P.o.d ${OBJECTDIR}/_ext/1999247862/crc.o.d ${OBJECTDIR}/_ext/1999247862/security.o.d ${OBJECTDIR}/_ext/1472/MRF24J40.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1472/accel.o ${OBJECTDIR}/_ext/1472/data.o ${OBJECTDIR}/_ext/1472/usb_cdc.o ${OBJECTDIR}/_ext/1472/usb_descriptors.o ${OBJECTDIR}/_ext/1472/util.o ${OBJECTDIR}/_ext/1472/spi.o ${OBJECTDIR}/_ext/1472/HardwareProfile.o ${OBJECTDIR}/_ext/1472/main-receiver.o ${OBJECTDIR}/_ext/1472/main-transmitter.o ${OBJECTDIR}/_ext/1472/c018i.o ${OBJECTDIR}/_ext/1453245926/TimeDelay.o ${OBJECTDIR}/_ext/828323842/Console.o ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o ${OBJECTDIR}/_ext/686890857/usb_device.o ${OBJECTDIR}/_ext/1736990619/MiWi.o ${OBJECTDIR}/_ext/828323842/NVM.o ${OBJECTDIR}/_ext/828323842/SymbolTime.o ${OBJECTDIR}/_ext/2134243105/P2P.o ${OBJECTDIR}/_ext/1999247862/crc.o ${OBJECTDIR}/_ext/1999247862/security.o ${OBJECTDIR}/_ext/1472/MRF24J40.o


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
MP_CC="C:\Program Files (x86)\Microchip\mcc18\bin\mcc18.exe"
# MP_BC is not defined
MP_AS="C:\Program Files (x86)\Microchip\mcc18\bin\..\mpasm\MPASMWIN.exe"
MP_LD="C:\Program Files (x86)\Microchip\mcc18\bin\mplink.exe"
MP_AR="C:\Program Files (x86)\Microchip\mcc18\bin\mplib.exe"
DEP_GEN=${MP_JAVA_PATH}java -jar "C:/Program Files (x86)/Microchip/MPLABX/mplab_ide/mplab_ide/modules/../../bin/extractobjectdependencies.jar" 
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps
MP_CC_DIR="C:\Program Files (x86)\Microchip\mcc18\bin"
# MP_BC_DIR is not defined
MP_AS_DIR="C:\Program Files (x86)\Microchip\mcc18\bin\..\mpasm"
MP_LD_DIR="C:\Program Files (x86)\Microchip\mcc18\bin"
MP_AR_DIR="C:\Program Files (x86)\Microchip\mcc18\bin"
# MP_BC_DIR is not defined

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=18F26J50
MP_PROCESSOR_OPTION_LD=18f26j50
MP_LINKER_DEBUG_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1472/accel.o: ../accel.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/accel.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/accel.o   ../accel.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/accel.o 
	
${OBJECTDIR}/_ext/1472/data.o: ../data.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/data.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/data.o   ../data.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/data.o 
	
${OBJECTDIR}/_ext/1472/usb_cdc.o: ../usb_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_cdc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/usb_cdc.o   ../usb_cdc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/usb_cdc.o 
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/usb_descriptors.o   ../usb_descriptors.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	
${OBJECTDIR}/_ext/1472/util.o: ../util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/util.o   ../util.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/util.o 
	
${OBJECTDIR}/_ext/1472/spi.o: ../spi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/spi.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/spi.o   ../spi.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/spi.o 
	
${OBJECTDIR}/_ext/1472/HardwareProfile.o: ../HardwareProfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/HardwareProfile.o   ../HardwareProfile.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/HardwareProfile.o 
	
${OBJECTDIR}/_ext/1472/main-receiver.o: ../main-receiver.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main-receiver.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/main-receiver.o   ../main-receiver.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main-receiver.o 
	
${OBJECTDIR}/_ext/1472/main-transmitter.o: ../main-transmitter.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main-transmitter.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/main-transmitter.o   ../main-transmitter.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main-transmitter.o 
	
${OBJECTDIR}/_ext/1472/c018i.o: ../c018i.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/c018i.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/c018i.o   ../c018i.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/c018i.o 
	
${OBJECTDIR}/_ext/1453245926/TimeDelay.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1453245926 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1453245926/TimeDelay.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1453245926/TimeDelay.o 
	
${OBJECTDIR}/_ext/828323842/Console.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/Console.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/828323842 
	@${RM} ${OBJECTDIR}/_ext/828323842/Console.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/828323842/Console.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/Console.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/828323842/Console.o 
	
${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1433380754 
	@${RM} ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o 
	
${OBJECTDIR}/_ext/686890857/usb_device.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/686890857 
	@${RM} ${OBJECTDIR}/_ext/686890857/usb_device.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/686890857/usb_device.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/686890857/usb_device.o 
	
${OBJECTDIR}/_ext/1736990619/MiWi.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/MiWi/MiWi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1736990619 
	@${RM} ${OBJECTDIR}/_ext/1736990619/MiWi.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1736990619/MiWi.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/MiWi/MiWi.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1736990619/MiWi.o 
	
${OBJECTDIR}/_ext/828323842/NVM.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/NVM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/828323842 
	@${RM} ${OBJECTDIR}/_ext/828323842/NVM.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/828323842/NVM.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/NVM.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/828323842/NVM.o 
	
${OBJECTDIR}/_ext/828323842/SymbolTime.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/SymbolTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/828323842 
	@${RM} ${OBJECTDIR}/_ext/828323842/SymbolTime.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/828323842/SymbolTime.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/SymbolTime.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/828323842/SymbolTime.o 
	
${OBJECTDIR}/_ext/2134243105/P2P.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/P2P/P2P.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2134243105 
	@${RM} ${OBJECTDIR}/_ext/2134243105/P2P.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/2134243105/P2P.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/P2P/P2P.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/2134243105/P2P.o 
	
${OBJECTDIR}/_ext/1999247862/crc.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Transceivers/crc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1999247862 
	@${RM} ${OBJECTDIR}/_ext/1999247862/crc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1999247862/crc.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Transceivers/crc.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1999247862/crc.o 
	
${OBJECTDIR}/_ext/1999247862/security.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Transceivers/security.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1999247862 
	@${RM} ${OBJECTDIR}/_ext/1999247862/security.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1999247862/security.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Transceivers/security.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1999247862/security.o 
	
${OBJECTDIR}/_ext/1472/MRF24J40.o: ../MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/MRF24J40.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/MRF24J40.o   ../MRF24J40.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/MRF24J40.o 
	
else
${OBJECTDIR}/_ext/1472/accel.o: ../accel.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/accel.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/accel.o   ../accel.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/accel.o 
	
${OBJECTDIR}/_ext/1472/data.o: ../data.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/data.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/data.o   ../data.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/data.o 
	
${OBJECTDIR}/_ext/1472/usb_cdc.o: ../usb_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_cdc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/usb_cdc.o   ../usb_cdc.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/usb_cdc.o 
	
${OBJECTDIR}/_ext/1472/usb_descriptors.o: ../usb_descriptors.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/usb_descriptors.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/usb_descriptors.o   ../usb_descriptors.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/usb_descriptors.o 
	
${OBJECTDIR}/_ext/1472/util.o: ../util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/util.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/util.o   ../util.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/util.o 
	
${OBJECTDIR}/_ext/1472/spi.o: ../spi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/spi.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/spi.o   ../spi.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/spi.o 
	
${OBJECTDIR}/_ext/1472/HardwareProfile.o: ../HardwareProfile.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/HardwareProfile.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/HardwareProfile.o   ../HardwareProfile.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/HardwareProfile.o 
	
${OBJECTDIR}/_ext/1472/main-receiver.o: ../main-receiver.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main-receiver.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/main-receiver.o   ../main-receiver.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main-receiver.o 
	
${OBJECTDIR}/_ext/1472/main-transmitter.o: ../main-transmitter.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/main-transmitter.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/main-transmitter.o   ../main-transmitter.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/main-transmitter.o 
	
${OBJECTDIR}/_ext/1472/c018i.o: ../c018i.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/c018i.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/c018i.o   ../c018i.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/c018i.o 
	
${OBJECTDIR}/_ext/1453245926/TimeDelay.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Common/TimeDelay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1453245926 
	@${RM} ${OBJECTDIR}/_ext/1453245926/TimeDelay.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1453245926/TimeDelay.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Common/TimeDelay.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1453245926/TimeDelay.o 
	
${OBJECTDIR}/_ext/828323842/Console.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/Console.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/828323842 
	@${RM} ${OBJECTDIR}/_ext/828323842/Console.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/828323842/Console.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/Console.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/828323842/Console.o 
	
${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/USB/CDC\ Device\ Driver/usb_function_cdc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1433380754 
	@${RM} ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/USB/CDC Device Driver/usb_function_cdc.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1433380754/usb_function_cdc.o 
	
${OBJECTDIR}/_ext/686890857/usb_device.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/USB/usb_device.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/686890857 
	@${RM} ${OBJECTDIR}/_ext/686890857/usb_device.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/686890857/usb_device.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/USB/usb_device.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/686890857/usb_device.o 
	
${OBJECTDIR}/_ext/1736990619/MiWi.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/MiWi/MiWi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1736990619 
	@${RM} ${OBJECTDIR}/_ext/1736990619/MiWi.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1736990619/MiWi.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/MiWi/MiWi.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1736990619/MiWi.o 
	
${OBJECTDIR}/_ext/828323842/NVM.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/NVM.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/828323842 
	@${RM} ${OBJECTDIR}/_ext/828323842/NVM.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/828323842/NVM.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/NVM.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/828323842/NVM.o 
	
${OBJECTDIR}/_ext/828323842/SymbolTime.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/SymbolTime.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/828323842 
	@${RM} ${OBJECTDIR}/_ext/828323842/SymbolTime.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/828323842/SymbolTime.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/SymbolTime.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/828323842/SymbolTime.o 
	
${OBJECTDIR}/_ext/2134243105/P2P.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/WirelessProtocols/P2P/P2P.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2134243105 
	@${RM} ${OBJECTDIR}/_ext/2134243105/P2P.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/2134243105/P2P.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/WirelessProtocols/P2P/P2P.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/2134243105/P2P.o 
	
${OBJECTDIR}/_ext/1999247862/crc.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Transceivers/crc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1999247862 
	@${RM} ${OBJECTDIR}/_ext/1999247862/crc.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1999247862/crc.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Transceivers/crc.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1999247862/crc.o 
	
${OBJECTDIR}/_ext/1999247862/security.o: ../../../../../../../Microchip\ Solutions\ v2011-12-05/Microchip/Transceivers/security.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1999247862 
	@${RM} ${OBJECTDIR}/_ext/1999247862/security.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1999247862/security.o   "../../../../../../../Microchip Solutions v2011-12-05/Microchip/Transceivers/security.c" 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1999247862/security.o 
	
${OBJECTDIR}/_ext/1472/MRF24J40.o: ../MRF24J40.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1472 
	@${RM} ${OBJECTDIR}/_ext/1472/MRF24J40.o.d 
	${MP_CC} $(MP_EXTRA_CC_PRE) -p$(MP_PROCESSOR_OPTION) -I".." -I"/c:/mcc18/h" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/USB" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/MiWi" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/WirelessProtocols/P2P" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers/MRF24J40" -I"../../../../../../../Microchip Solutions v2011-12-05/Microchip/Include/Transceivers" -mL -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-  -I ${MP_CC_DIR}\\..\\h  -fo ${OBJECTDIR}/_ext/1472/MRF24J40.o   ../MRF24J40.c 
	@${DEP_GEN} -d ${OBJECTDIR}/_ext/1472/MRF24J40.o 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "..\18f26j50.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w -x -u_DEBUG -m"$(BINDIR_)$(TARGETBASE).map" -w -l"../../../../../../../mcc18/lib"  -z__MPLAB_BUILD=1  -u_CRUNTIME -z__MPLAB_DEBUG=1 -z__MPLAB_DEBUGGER_ICD3=1 $(MP_LINKER_DEBUG_OPTION) -l ${MP_CC_DIR}\\..\\lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
else
dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_LD} $(MP_EXTRA_LD_PRE) "..\18f26j50.lkr"  -p$(MP_PROCESSOR_OPTION_LD)  -w  -m"$(BINDIR_)$(TARGETBASE).map" -w -l"../../../../../../../mcc18/lib"  -z__MPLAB_BUILD=1  -u_CRUNTIME -l ${MP_CC_DIR}\\..\\lib  -o dist/${CND_CONF}/${IMAGE_TYPE}/WAX.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}   
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
