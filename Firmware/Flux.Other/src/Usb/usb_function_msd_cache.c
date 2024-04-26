/*********************************************************************
  File Information:
    FileName:        usb_function_msd_multi_sector.c
    Dependencies:    See INCLUDES section below
    Processor:       PIC18, PIC24, or PIC32
    Compiler:        C18, C30, or C32
    Company:         Microchip Technology, Inc.

    Software License Agreement

    The software supplied herewith by Microchip Technology Incorporated
    (the "Company") for its PIC(r) Microcontroller is intended and
    supplied to you, the Company's customer, for use solely and
    exclusively on Microchip PICmicro Microcontroller products. The
    software is owned by the Company and/or its supplier, and is
    protected under applicable copyright laws. All rights are reserved.
    Any use in violation of the foregoing restrictions may subject the
    user to criminal sanctions under applicable laws, as well as to
    civil liability for the breach of the terms and conditions of this
    license.

    THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
    WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
    TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
    IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

  Summary:
    This file contains functions, macros, definitions, variables,
    datatypes, etc. that are required for use of the MSD function
    driver. This file should be included in projects that use the MSD
    \function driver.
    
    
    
    This file is located in the "\<Install Directory\>\\Microchip\\USB\\MSD
    Device Driver" directory.

  Description:
    USB MSD Function Driver File
    
    This file contains functions, macros, definitions, variables,
    datatypes, etc. that are required for use of the MSD function
    driver. This file should be included in projects that use the MSD
    \function driver.
    
    This file is located in the "\<Install Directory\>\\Microchip\\USB\\MSD
    Device Driver" directory.
    
    When including this file in a new project, this file can either be
    referenced from the directory in which it was installed or copied
    directly into the user application folder. If the first method is
    chosen to keep the file located in the folder in which it is installed
    then include paths need to be added so that the library and the
    application both know where to reference each others files. If the
    application folder is located in the same folder as the Microchip
    folder (like the current demo folders), then the following include
    paths need to be added to the application's project:
    
    ..\\..\\Microchip\\Include
    .
    
    If a different directory structure is used, modify the paths as
    required. An example using absolute paths instead of relative paths
    would be the following:
    
    C:\\Microchip Solutions\\Microchip\\Include
    
    C:\\Microchip Solutions\\My Demo Application

	Karim Ladha 2015, Notes: 
	Originally forked from usb_function_msd_multi_sector.c revision 2.9h.
	This msd driver supports only one LUN for simplicity and speed
	Changed to use same disk API as FatFs through the ftl lun shim
	Changed to use more sophisticated DiskCache.c/h driver with less blocking

	TODO:
	Make the sector read case use endpoint re-load like the write case
	Try to remove the requirement for the response buffer

*/
// Include
#include "USB/usb.h"
#include "USB/usb_function_msd.h"

#ifndef USB_USE_MSD
	// Check correct usb config
	#warning "MSD driver is disabled"
#else

#include "FatFs/DiskCache.h"
//#include "FSconfig.h"

#define DEBUG_LEVEL_LOCAL	DEBUG_USB
#define DBG_FILE			"dcusbmsd.c"
#include "utils/debug.h"

#ifndef USB_USE_ASYNC_MSD
	#error "Must define this symbol for USB_CDC_MSD.c to work"
#endif 

// This diver is for one LUN disk only
#define LUN_INDEX 0

// Prototypes: Functionality provided by DiskCache.c
extern MEDIA_INFORMATION* LUNMediaInitialize(void);
extern DWORD LUNReadCapacity(void);
extern WORD LUNReadSectorSize(void);
extern BYTE LUNMediaDetect(void);
extern BYTE LUNWriteProtectState(void);

// Definitions
// Missing state machine states - KL
#define MSD_WRITE10_ERROR					0x05
#define MSD_WRITE10_AWAITING_COMPLETION		0x06
//Adjustable user options
#define MSD_FAILED_READ_MAX_ATTEMPTS  (BYTE)100u    //Used for error case handling
#define MSD_FAILED_WRITE_MAX_ATTEMPTS (BYTE)100u    //Used for error case handling

// Checks
#if !defined(__C30__) && !defined(__C32__) && !defined(__XC16__)
	#error "This driver is not for this hardware."
#endif
#if MAX_LUN != 0 /* 0 = one disk */
	#error "This driver does not use the LUN API, only one disk possible."
#endif

// Global variables
//State machine variables
BYTE MSD_State;			// Takes values MSD_WAIT, MSD_DATA_IN or MSD_DATA_OUT
BYTE MSDCommandState;
BYTE MSDReadState;
BYTE MSDWriteState;
BYTE MSDRetryAttempt;
//Other variables
USB_MSD_CBW gblCBW;	
BYTE gblCBWLength;
RequestSenseResponse gblSenseData[MAX_LUN + 1];
BYTE *ptrNextData;
USB_HANDLE USBMSDOutHandle;
USB_HANDLE USBMSDInHandle;
WORD MSBBufferIndex;
WORD gblMediaPresent; 
BOOL SoftDetach[MAX_LUN + 1];
BOOL MSDHostNoData;
BOOL MSDCBWValid;

static WORD_VAL TransferLength;
static DWORD_VAL LBA;
BYTE fetchStatus;
unsigned char respBuffer[MSD_OUT_EP_SIZE]; //KL
/* 
 * Number of Blocks and Block Length are global because 
 * for every READ_10 and WRITE_10 command need to verify if the last LBA 
 * is less than gblNumBLKS	
 */	
DWORD_VAL gblNumBLKS,gblBLKLen;	
extern const ROM InquiryResponse inq_resp;

/** P R I V A T E  P R O T O T Y P E S ***************************************/
BYTE MSDProcessCommand(void);
void MSDProcessCommandMediaAbsent(void);
void MSDProcessCommandMediaPresent(void);
BYTE MSDReadHandler(void);
BYTE MSDWriteHandler(void);
void ResetSenseData(void);
BYTE MSDCheckForErrorCases(DWORD);
void MSDErrorHandler(BYTE);
static void MSDComputeDeviceInAndResidue(WORD);


// Source
// USB MSDTasks daisy chain - triggered software interrupt
void __attribute__((interrupt,auto_psv)) MsdInterrupt(void)
{
	MSDTasks();
	USB_MSD_TASKS_CLEAR();
}

/******************************************************************************
  Function:
    void USBMSDInit(void)
    
  Summary:
    This routine initializes the MSD class packet handles, prepares to
    receive a MSD packet, and initializes the MSD state machine. This
    \function should be called once after the device is enumerated.

  Description:
    This routine initializes the MSD class packet handles, prepares to
    receive a MSD packet, and initializes the MSD state machine. This
    \function should be called once after the device is enumerated.
    
    Typical Usage:
    <code>
    void USBCBInitEP(void)
    {
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        USBMSDInit();
    }
    </code>
  Conditions:
    The device should already be enumerated with a configuration that
    supports MSD before calling this function.
    
  Paramters: None

  Remarks:
    None                                                                                                          
  ****************************************************************************/	
void USBMSDInit(void)
{
    //Prepare to receive the first CBW
    USBMSDInHandle = 0;    
    USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP,(BYTE*)&msd_cbw,MSD_OUT_EP_SIZE);
    //Initialize IN handle to point to first available IN MSD bulk endpoint entry
    USBMSDInHandle = USBGetNextHandle(MSD_DATA_IN_EP, IN_TO_HOST);
    MSD_State = MSD_WAIT;
    MSDCommandState = MSD_COMMAND_WAIT;
    MSDReadState = MSD_READ10_WAIT;
    MSDWriteState = MSD_WRITE10_WAIT;
	MSDHostNoData = FALSE;
    gblNumBLKS.Val = 0;
    gblBLKLen.Val = 0;
    MSDCBWValid = TRUE;

	/* 
		Using the DiskCache.c/h driver
	*/
	if(!(gDiskCache.state & DC_OPERATIONAL))
		DiskCacheStartup(DISK_CACHE_DEFAULT);

	// Clear all of the soft detach variables
	gblMediaPresent = 0;
	gblCBW.bCBWLUN=0;
	SoftDetach[gblCBW.bCBWLUN] =  FALSE;
	
	//see if the media is attached
	if(LUNMediaDetect() && LUNMediaInitialize())
	{
		if(gDiskCache.state & DC_OPERATIONAL)	
			gblMediaPresent = 1;
	}

	ResetSenseData();

#if 0 // Old logic. Remember, one disk only
	gblMediaPresent = 0;
    //For each of the possible logical units
    for(gblCBW.bCBWLUN=0;gblCBW.bCBWLUN<(MAX_LUN + 1);gblCBW.bCBWLUN++)
    {
        //clear all of the soft detach variables
        SoftDetach[gblCBW.bCBWLUN] =  FALSE;

        //see if the media is attached
        if(LUNMediaDetect())
        {
            //initialize the media
            if(LUNMediaInitialize())
            {
                //if the media was present and successfully initialized
                //  then mark and indicator that the media is ready
                gblMediaPresent |= ((WORD)1<<gblCBW.bCBWLUN);
            }
        }
        ResetSenseData();
    }
#endif


	// KL: Initialize called in USB initialize
	USB_MSD_TASKS_INT_ENABLE();
}

/******************************************************************************
 	Function:
 		void USBCheckMSDRequest(void)

 	Summary:
 		This routine handles MSD specific request that happen on EP0.  
        This function should be called from the USBCBCheckOtherReq() call back 
        function whenever implementing an MSD device.

 	Description:
 		This routine handles MSD specific request that happen on EP0.  These
        include, but are not limited to, the standard RESET and GET_MAX_LUN 
 		command requests.  This function should be called from the 
        USBCBCheckOtherReq() call back function whenever using an MSD device.	

        Typical Usage:
        <code>
        void USBCBCheckOtherReq(void)
        {
            //Since the stack didn't handle the request I need to check
            //  my class drivers to see if it is for them
            USBCheckMSDRequest();
        }
        </code>

 	PreCondition:
 		None
 		
 	Parameters:
 		None
 	
 	Return Values:
 		None
 		
 	Remarks:
 		None
 
 *****************************************************************************/	
void USBCheckMSDRequest(void)
{
    if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;
    if(SetupPkt.bIntfID != MSD_INTF_ID) return;

	switch(SetupPkt.bRequest)
    {
	    case MSD_RESET:
	        //First make sure all request parameters are correct:
	        //MSD BOT specs require wValue to be == 0x0000 and wLength == 0x0000
	        if((SetupPkt.wValue != 0) || (SetupPkt.wLength != 0)) 
	        {
    	        return; //Return without handling the request (results in STALL)
    	    }    
	        
	        //Host would typically issue this after a STALL event on an MSD
	        //bulk endpoint.  The MSD reset should re-initialize status
	        //so as to prepare for a new CBW.  Any currently ongoing command
	        //block should be aborted, but the STALL and DTS states need to be
	        //maintained (host will re-initialize these separately using 
	        //CLEAR_FEATURE, endpoint halt).
            MSD_State = MSD_WAIT;
            MSDCommandState = MSD_COMMAND_WAIT;
            MSDReadState = MSD_READ10_WAIT;
            MSDWriteState = MSD_WRITE10_WAIT;
            MSDCBWValid = TRUE;
            //Need to re-arm MSD bulk OUT endpoint, if it isn't currently armed, 
            //to be able to receive next CBW.  If it is already armed, don't need
            //to do anything, since we can already receive the next CBW (or we are 
            //STALLed, and the host will issue clear halt first).
            if(!USBHandleBusy(USBGetNextHandle(MSD_DATA_OUT_EP, OUT_FROM_HOST)))
            {        
                USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP,(BYTE*)&msd_cbw,MSD_OUT_EP_SIZE);
            }                

            //Let USB stack know we took care of handling the EP0 SETUP request.
            //Allow zero byte status stage to proceed normally now.
            USBEP0Transmit(USB_EP0_NO_DATA);    
	    	break;
	    case GET_MAX_LUN:
            //First make sure all request parameters are correct:
            //MSD BOT specs require wValue to be == 0x0000, and wLengh == 1
            if((SetupPkt.wValue != 0) || (SetupPkt.wLength != 1))   
            {
                break;  //Return without handling the request (results in STALL)
            }    
            
            //If the host asks for the maximum number of logical units
            //  then send out a packet with that information
	    	CtrlTrfData[0] = MAX_LUN;
            USBEP0SendRAMPtr((BYTE*)&CtrlTrfData[0],1,USB_EP0_INCLUDE_ZERO);
	    	break;
    }	//end switch(SetupPkt.bRequest)
}

/*********************************************************************************
  Function:
        BYTE MSDTasks(void)
    
  Summary:
    This function runs the MSD class state machines and all of its
    sub-systems. This function should be called periodically once the
    device is in the configured state in order to keep the MSD state
    machine going.
  Description:
    This function runs the MSD class state machines and all of its
    sub-systems. This function should be called periodically once the
    device is in the configured state in order to keep the MSD state
    machine going.
    
    Typical Usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == TRUE))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Keep the MSD state machine going
                MSDTasks();
    
                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    None
  Return Values:
    BYTE -  the current state of the MSD state machine the valid values are
            defined in MSD.h under the MSDTasks state machine declaration section.
            The possible values are the following\:
            * MSD_WAIT
            * MSD_DATA_IN
            * MSD_DATA_OUT
            * MSD_SEND_CSW
  Remarks:
    None                                                                          
  *********************************************************************************/	
BYTE MSDTasks(void)
{
    BYTE i;
    
    //Error check to make sure we have are in the CONFIGURED_STATE, prior to
    //performing MSDTasks().  Some of the MSDTasks require that the device be
    //configured first.
    if(USBGetDeviceState() != CONFIGURED_STATE)
    {
        return MSD_WAIT;
    }
    
    //Note: Both the USB stack code (usb_device.c) and this MSD handler code 
    //have the ability to modify the BDT values for the MSD bulk endpoints.  If the 
    //USB stack operates in USB_INTERRUPT mode (user option in usb_config.h), we
    //should temporarily disable USB interrupts, to avoid any possibility of both 
    //the USB stack and this MSD handler from modifying the same BDT entry, or
    //MSD state machine variables (ex: in the case of MSD_RESET) at the same time.
    USBMaskInterrupts();
    
    //Main MSD task dispatcher.  Receives MSD Command Block Wrappers (CBW) and
    //dispatches appropriate lower level handlers to service the requests.
    switch(MSD_State)
    {
        case MSD_WAIT: //idle state, when we are waiting for a command from the host
        {
            //Check if we have received a new command block wrapper (CBW)
            if(!USBHandleBusy(USBMSDOutHandle))
            {
        		//If we are in the MSD_WAIT state, and we received an OUT transaction
        		//on the MSD OUT endpoint, then we must have just received an MSD
        		//Command Block Wrapper (CBW).
        		//First copy the the received data to to the gblCBW structure, so
        		//that we keep track of the command, but free up the MSD OUT endpoint
        		//buffer for fulfilling whatever request may have been received.
                //gblCBW = msd_cbw; //we are doing this, but below method can yeild smaller code size
            	for(i = 0; i < MSD_CBW_SIZE; i++)
                {
                	*((BYTE*)&gblCBW.dCBWSignature + i) = *((BYTE*)&msd_cbw.dCBWSignature + i);
                }   	

        	    //If this CBW is valid?
        		if((USBHandleGetLength(USBMSDOutHandle) == MSD_CBW_SIZE) && (gblCBW.dCBWSignature == MSD_VALID_CBW_SIGNATURE)) 
            	{
                	//The CBW was valid, set flag meaning any stalls after this point 
                	//should not be "persistent" (as in the case of non-valid CBWs).
                	MSDCBWValid = TRUE; 

                    //Is this CBW meaningful?	
       				if((gblCBW.bCBWLUN <= MAX_LUN)                                      //Verify the command is addressed to a supported LUN
               		&&(gblCBW.bCBWCBLength <= MSD_MAX_CB_SIZE)                          //Verify the claimed CB length is reasonable/valid
               		&&(gblCBW.bCBWCBLength >= 0x01)                                     //Verify the claimed CB length is reasonable/valid
               		&&((gblCBW.bCBWFlags & MSD_CBWFLAGS_RESERVED_BITS_MASK) == 0x00))   //Verify reserved bits are clear
            		{
                		
                		//The CBW was both valid and meaningful.
                		//Begin preparing a valid Command Status Wrapper (CSW), 
                		//in anticipation of completing the request successfully.
                		//If an error detected is later, we will change the status 
                		//before sending the CSW.
                    	msd_csw.dCSWSignature = MSD_VALID_CSW_SIGNATURE;
                    	msd_csw.dCSWTag = gblCBW.dCBWTag;
                    	msd_csw.dCSWDataResidue = 0x0;
                    	msd_csw.bCSWStatus = MSD_CSW_COMMAND_PASSED;	
                    	
                    	//Since a new CBW just arrived, we should re-init the
                    	//lower level state machines to their default states.  
                    	//Even if the prior operation didn't fully complete 
                    	//normally, we should abandon the prior operation, when
                    	//a new CBW arrives.
                    	MSDCommandState = MSD_COMMAND_WAIT;
                    	MSDReadState = MSD_READ10_WAIT;
                    	MSDWriteState = MSD_WRITE10_WAIT;

                    	//Keep track of retry attempts, in case of temporary 
                    	//failures during read or write of the media.
                    	MSDRetryAttempt = 0;
                    	
                    	//Check the command.  With the exception of the REQUEST_SENSE
                    	//command, we should reset the sense key info for each new command block.
                    	//Assume the command will get processed successfully (and hence "NO SENSE" 
                    	//response, which is used for success cases), unless handler code
                    	//later on detects some kind of error.  If it does, it should
                    	//update the sense keys to reflect the type of error detected,
                    	//prior to sending the CSW.
                    	if(gblCBW.CBWCB[0] != MSD_REQUEST_SENSE)
                    	{
                      		gblSenseData[LUN_INDEX].SenseKey=S_NO_SENSE;
        			        gblSenseData[LUN_INDEX].ASC=ASC_NO_ADDITIONAL_SENSE_INFORMATION;
        			        gblSenseData[LUN_INDEX].ASCQ=ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
    			        }

                    	//Isolate the data direction bit.  The direction bit is bit 7 of the bCBWFlags byte.
	                   	//Then, based on the direction of the data transfer, prepare the MSD state machine
                    	//so it knows how to proceed with processing the request.
                        //If bit7 = 0, then direction is OUT from host.  If bit7 = 1, direction is IN to host
        				if (gblCBW.bCBWFlags & MSD_CBW_DIRECTION_BITMASK)
        				{
        					MSD_State = MSD_DATA_IN;
        				}
        				else //else direction must be OUT from host
            			{
            			    MSD_State = MSD_DATA_OUT;
        				}  

        				//Determine if the host is expecting there to be data transfer or not.
        				//Doing this now will make for quicker error checking later.
        				if(gblCBW.dCBWDataTransferLength != 0)		
                    	{
	                    	MSDHostNoData = FALSE;
	                    }
	                    else
	                    {
		                    MSDHostNoData = TRUE;
		                }    	
		                
                        //Copy the received command to the lower level command 
                        //state machine, so it knows what to do.
                        MSDCommandState = gblCBW.CBWCB[0];  
        			}
        			else
        			{
            			//else the CBW wasn't meaningful.  Section 6.4 of BOT specs v1.0 says,
            			//"The response of a device to a CBW that is not meaningful is not specified."
            			//Lets STALL the bulk endpoints, so as to promote the possibility of recovery.
            			USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);
              			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);  
                    } 
        		}//end of: if((USBHandleGetLength(USBMSDOutHandle) == MSD_CBW_SIZE) && (gblCBW.dCBWSignature == MSD_VALID_CBW_SIGNATURE))
        		else  //The CBW was not valid. 
        		{
            		//Section 6.6.1 of the BOT specifications rev. 1.0 says the device shall STALL bulk IN and OUT
	        		//endpoints (or should discard OUT data if not stalled), and should stay in this state until a 
	        		//"Reset Recovery" (MSD Reset + clear endpoint halt commands on EP0, see section 5.3.4)
          			USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);
          			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);  
          			MSDCBWValid = FALSE;    //Flag so as to enable a "persistent" 
          			//stall (cannot be cleared by clear endpoint halt, unless preceded
          			//by an MSD reset).
        		}
            }//if(!USBHandleBusy(USBMSDOutHandle))
            break;
        }//end of: case MSD_WAIT:
        case MSD_DATA_IN:
            if(MSDProcessCommand() == MSD_COMMAND_WAIT)
            {
                // Done processing the command, send the status
                MSD_State = MSD_SEND_CSW;
            }
            break;
        case MSD_DATA_OUT:
            if(MSDProcessCommand() == MSD_COMMAND_WAIT)
            {
    			/* Finished receiving the data prepare and send the status */
    		  	if ((msd_csw.bCSWStatus == MSD_CSW_COMMAND_PASSED)&&(msd_csw.dCSWDataResidue!=0)) 
    		  	{
    		  		msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
    		    }
                MSD_State = MSD_SEND_CSW;
            }
            break;
        case MSD_SEND_CSW:
            //Check to make sure the bulk IN endpoint is available before sending CSW.
            //The endpoint might still be busy sending the last packet on the IN endpoint.
            if(USBHandleBusy(USBGetNextHandle(MSD_DATA_IN_EP, IN_TO_HOST)) == TRUE)
            {
                break;  //Not available yet.  Just stay in this state and try again later.
            }
            
            //Send the Command Status Wrapper (CSW) packet            
            USBMSDInHandle = USBTxOnePacket(MSD_DATA_IN_EP,(BYTE*)&msd_csw,MSD_CSW_SIZE);
            //If the bulk OUT endpoint isn't already armed, make sure to do so 
            //now so we can receive the next CBW packet from the host.
            if(!USBHandleBusy(USBMSDOutHandle))
            {
                USBMSDOutHandle = USBRxOnePacket(MSD_DATA_OUT_EP,(BYTE*)&msd_cbw,MSD_OUT_EP_SIZE);
            }
           	MSD_State=MSD_WAIT;
            break;
        default:
            //Illegal condition that should not happen, but might occur if the
            //device firmware incorrectly calls MSDTasks() prior to calling
            //USBMSDInit() during the set-configuration portion of enumeration.
			DBG_ERROR("unknown state");
            MSD_State = MSD_WAIT;
			msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);          
    }//switch(MSD_State)
    
    //Safe to re-enable USB interrupts now.
    USBUnmaskInterrupts();
    
    return MSD_State;
}


/******************************************************************************
 	Function:
 		BYTE MSDProcessCommand(void)
 		
 	Description:
 		This function processes a command received through the MSD
 		class driver
 		
 	PreCondition:
 		None
 		
 	Parameters:
 		None
 		
 	Return Values:
 		BYTE - the current state of the MSDProcessCommand state
 		machine.  The valid values are defined in MSD.h under the
 		MSDProcessCommand state machine declaration section
 		
 	Remarks:
 		None
 
 *****************************************************************************/	
BYTE MSDProcessCommand(void)
{   
  	//Check if the media is either not present, or has been flagged by firmware
  	//to pretend to be non-present (ex: SoftDetached).
  	if((LUNMediaDetect() == FALSE) || (SoftDetach[gblCBW.bCBWLUN] == TRUE) || (!(gDiskCache.state & DC_OPERATIONAL)))
    {
        //Clear flag so we know the media need initialization, if it becomes 
        //present in the future.
        gblMediaPresent &= ~((WORD)1<<gblCBW.bCBWLUN);
        MSDProcessCommandMediaAbsent();
   	}
    else
    {
        //Check if the media is present and hasn't been already flagged as initialized.
        if((gblMediaPresent & ((WORD)1<<gblCBW.bCBWLUN)) == 0)
        {
            //Try to initialize the media
            if(LUNMediaInitialize())
            {
                //The media initialized successfully.  Set flag letting software
                //know that it doesn't need re-initialization again (unless the 
                //media is removable and is subsequently removed and re-inserted). 
                gblMediaPresent |= ((WORD)1<<gblCBW.bCBWLUN);

                //The media is present and has initialized successfully.  However,
                //we should still notify the host that the media may have changed,
                //from the host's perspective, since we just initialized it for 
                //the first time.         
        		gblSenseData[LUN_INDEX].SenseKey = S_UNIT_ATTENTION;
        		gblSenseData[LUN_INDEX].ASC = ASC_NOT_READY_TO_READY_CHANGE;
        		gblSenseData[LUN_INDEX].ASCQ = ASCQ_MEDIUM_MAY_HAVE_CHANGED;
                //Signify a soft error to the host, so it knows to check the 
                //sense keys to learn that the media just changed.
                msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED; //No real "error" per se has occurred
                //Process the command now.
                MSDProcessCommandMediaPresent();
            }
            else
            {
                //The media failed to initialize for some reason.
				DBG_ERROR("media error");
                MSDProcessCommandMediaAbsent();
            }
        }
        else
        {
            //The media was present and was already initialized/ready to process
            //the host's command.
            MSDProcessCommandMediaPresent();
        }
    }

    return MSDCommandState;
}

/******************************************************************************
 	Function:
 		void MSDProcessCommandMediaAbsent(void)
 		
 	Description:
 		This funtion processes a command received through the MSD
 		class driver, when the removable MSD media (ex: MMC/SD card) is not 
 		present, or has been "soft detached" deliberately by the application
 		firmware.
 		
 	PreCondition:
 		The MSD function should have already been initialized (the media isn't
 		required to be initalized however).  Additionally, a valid MSD Command 
 		Block Wrapper (CBW) should have been received and partially parsed 
 		prior to calling this function.
 		
 	Parameters:
 		None
 	
 	Return Values:
 		BYTE - the current state of the MSDProcessCommand state
 		machine.  The valid values are defined in usb_function_msd.h under the 
 		MSDProcessCommand state machine declaration section
 		
 	Remarks:
 		None
 
  *****************************************************************************/	
void MSDProcessCommandMediaAbsent(void)
{
    //Check what command we are currently processing, to decide how to handle it.
    switch(MSDCommandState)
    {
        case MSD_REQUEST_SENSE:
            //The host sends this request when it wants to check the status of 
            //the device, and/or identify the reason for the last error that was 
            //reported by the device.
            //Set the sense keys to let the host know that the reason the last
            //command failed was because the media was not present.
            ResetSenseData();
            gblSenseData[LUN_INDEX].SenseKey=S_NOT_READY;
    		gblSenseData[LUN_INDEX].ASC=ASC_MEDIUM_NOT_PRESENT;
    		gblSenseData[LUN_INDEX].ASCQ=ASCQ_MEDIUM_NOT_PRESENT;

            //After initializing the sense keys above, the subsequent handling 
            //code for this state is the same with or without media.
            //Therefore, to save code size, we just call the media present handler.
            MSDProcessCommandMediaPresent();
            break;
        case MSD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        case MSD_TEST_UNIT_READY:
            //The host will typically periodically poll the device by sending this
            //request.  Since this is a removable media device, and the media isn't
            //present, we need to indicate an error to let the host know (to 
            //check the sense keys, which will tell it the media isn't present).
    		msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;
            MSDCommandState = MSD_COMMAND_WAIT;
            break;
        case MSD_INQUIRY:
            //The handling code for this state is the same with or without media.
            //Therefore, to save code size, we just call the media present handler.
            MSDProcessCommandMediaPresent();
            break;
        case MSD_COMMAND_RESPONSE:
            //The handling code for this state is the same with or without media.
            //Therefore, to save code size, we just call the media present handler.
            MSDProcessCommandMediaPresent();
            break;            
        default:
		    //An unsupported command was received.  Since we are uncertain how 
		    //many bytes we should send/or receive, we should set sense key data 
		    //and then STALL, to force the host to perform error recovery.
			DBG_INFO("recovery attempt, stall");
		    MSDErrorHandler(MSD_ERROR_UNSUPPORTED_COMMAND);
            break;
    }
}//void MSDProcessCommandMediaAbsent(void)


/******************************************************************************
 	Function:
 		void MSDProcessCommandMediaPresent(void)
 		
 	Description:
 		This funtion processes a command received through the MSD
 		class driver
 		
 	PreCondition:
 		None
 		
 	Paramters:
 		None
 	
 	Return Values:
 		BYTE - the current state of the MSDProcessCommand state
 		machine.  The valid values are defined in MSD.h under the 
 		MSDProcessCommand state machine declaration section
 		
 	Remarks:
 		None
 
 *****************************************************************************/	
void MSDProcessCommandMediaPresent(void)
{
    BYTE i; 
    BYTE NumBytesInPacket;
	// KL: Using response buffer
	unsigned char* resp = respBuffer;

    //Check what command we are currently processing, to decide how to handle it.
    switch(MSDCommandState)
    {
		case MSD_READ_10:
		    //The host issues a "Read 10" request when it wants to read some number
		    //of 10-bit length blocks (512 byte blocks) of data from the media.
		    //Since this is a common request and is part of the "critical path"
		    //performance wise, we put this at the top of the state machine checks.
        	if(MSDReadHandler() == MSD_READ10_WAIT)
        	{
			    MSDCommandState = MSD_COMMAND_WAIT;
            }
            break;
    	case MSD_WRITE_10:
		    //The host issues a "Write 10" request when it wants to write some number
		    //of 10-bit length blocks (512 byte blocks) of data to the media.
		    //Since this is a common request and is part of the "critical path"
		    //performance wise, we put this near the top of the state machine checks.
        	if(MSDWriteHandler() == MSD_WRITE10_WAIT)
        	{
			    MSDCommandState = MSD_COMMAND_WAIT;
            }
		    break;
    	case MSD_INQUIRY:
    	{
        	//The host wants to learn more about our MSD device (spec version, 
        	//supported abilities, etc.)
        	
            //Error check: If host doesn't want any data, then just advance to CSW phase.
            if(MSDHostNoData == TRUE)
            {
                MSDCommandState = MSD_COMMAND_WAIT;
                break;
            }    
            
            //Get the 16-bit "Allocation Length" (Number of bytes to respond 
            //with.  Note: Value provided in CBWCB is in big endian format)
            TransferLength.byte.HB = gblCBW.CBWCB[3]; //MSB
            TransferLength.byte.LB = gblCBW.CBWCB[4]; //LSB
        	//Check for possible errors.  
            if(MSDCheckForErrorCases(TransferLength.Val) != MSD_ERROR_CASE_NO_ERROR)
            {
                break;
            }

          	//Compute and load proper csw residue and device in number of byte.
            MSDComputeDeviceInAndResidue(sizeof(InquiryResponse));

            //If we get to here, this implies no errors were found and the command is legit.

            //copy the inquiry results from the defined ROM buffer 
            //  into the USB buffer so that it can be transmitted
        	memcpypgm2ram((void *)resp, (ROM void*)&inq_resp, sizeof(InquiryResponse));   //Inquiry response is 36 bytes total
        	MSDCommandState = MSD_COMMAND_RESPONSE;
            break;
        }
        case MSD_READ_CAPACITY:
        {
            //The host asked for the total capacity of the device.  The response
            //packet is 8-bytes (32-bits for last LBA implemented, 32-bits for block size).
            DWORD_VAL sectorSize;
            DWORD_VAL capacity;

            //get the information from the physical media
            capacity.Val = LUNReadCapacity();
            sectorSize.Val = LUNReadSectorSize();
			DBG_INFO("report lba count: %s",DBG_ultoa(capacity.Val));
            
            //Copy the data to the buffer.  Host expects the response in big endian format.
        	resp[0]=capacity.v[3];
        	resp[1]=capacity.v[2];
        	resp[2]=capacity.v[1];
        	resp[3]=capacity.v[0];
        	
        	resp[4]=sectorSize.v[3];
        	resp[5]=sectorSize.v[2];
        	resp[6]=sectorSize.v[1];
        	resp[7]=sectorSize.v[0];

          	//Compute and load proper csw residue and device in number of byte.
            TransferLength.Val = 0x08;      //READ_CAPACITY always has an 8-byte response.
            MSDComputeDeviceInAndResidue(0x08);
        
        	MSDCommandState = MSD_COMMAND_RESPONSE;
            break;
        }
        case MSD_REQUEST_SENSE:    
            //The host normally sends this request after a CSW completed, where
            //the device indicated some kind of error on the previous transfer.
            //In this case, the host will typically issue this request, so it can
            //learn more details about the cause/source of the error condition.
            
            //Error check: if the host doesn't want any data, just advance to CSW phase.
            if(MSDHostNoData == TRUE)
            {
                MSDCommandState = MSD_COMMAND_WAIT;
                break;
            }    

          	//Compute and load proper csw residue and device in number of byte.
            TransferLength.Val = sizeof(RequestSenseResponse);      //REQUEST_SENSE has an 18-byte response.
            MSDComputeDeviceInAndResidue(sizeof(RequestSenseResponse));
             
            //Copy the requested response data from flash to the USB ram buffer.
          	for(i=0;i<sizeof(RequestSenseResponse);i++)
          	{
          		resp[i]=gblSenseData[LUN_INDEX]._byte[i];
            }
          	MSDCommandState = MSD_COMMAND_RESPONSE;
            break;
	    case MSD_MODE_SENSE:
        	resp[0]=0x03;
        	resp[1]=0x00;
        	resp[2]=(LUNWriteProtectState()) ? 0x80 : 0x00;
        	resp[3]= 0x00;
 
           	//Compute and load proper csw residue and device in number of byte.
            TransferLength.Val = 0x04;      
            MSDComputeDeviceInAndResidue(0x04);
        	MSDCommandState = MSD_COMMAND_RESPONSE;
    	    break;
		case MSD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        	gblSenseData[LUN_INDEX].SenseKey=S_ILLEGAL_REQUEST;
        	gblSenseData[LUN_INDEX].ASC=ASC_INVALID_COMMAND_OPCODE;
        	gblSenseData[LUN_INDEX].ASCQ=ASCQ_INVALID_COMMAND_OPCODE;
        	msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;
        	msd_csw.dCSWDataResidue = 0x00;
			MSDCommandState = MSD_COMMAND_WAIT;
            break;
		case MSD_TEST_UNIT_READY:
            //The host will typically send this command periodically to check if
            //it is ready to be used and to obtain polled notification of changes
            //in status (ex: user removed media from a removable media MSD volume).
            //There is no data stage for this request.  The information we send to
            //the host in response to this request is entirely contained in the CSW.
            
            //First check for possible errors.
            if(MSDCheckForErrorCases(0) != MSD_ERROR_CASE_NO_ERROR)
            {
                break;
            }    
            //The stack sets this condition when the status of the removable media
            //has just changed (ex: the user just plugged in the removable media,
            //in which case we want to notify the host of the changed status, by
            //sending a deliberate "error" notification).  This doesn't mean any 
            //real error has occurred.
            if((gblSenseData[LUN_INDEX].SenseKey==S_UNIT_ATTENTION) && (msd_csw.bCSWStatus==MSD_CSW_COMMAND_FAILED))
            {
                MSDCommandState = MSD_COMMAND_WAIT;
            }
            else
            {
            	ResetSenseData();
            	msd_csw.dCSWDataResidue=0x00;
    			MSDCommandState = MSD_COMMAND_WAIT;
            }
            break;
		case MSD_VERIFY:
            //Fall through to STOP_START
		case MSD_STOP_START:
        	msd_csw.dCSWDataResidue=0x00;
			MSDCommandState = MSD_COMMAND_WAIT;
            break;
        case MSD_COMMAND_RESPONSE:
            //This command state didn't originate from the host.  This state was
            //set by the firmware (for one of the other handlers) when it was 
            //finished preparing the data to send to the host, and it is now time
            //to transmit the data over the bulk IN endpoint.
            if(USBHandleBusy(USBMSDInHandle) == FALSE)
            {
                //We still have more bytes needing to be sent.  Compute how many 
                //bytes should be in the next IN packet that we send.
                if(gblCBW.dCBWDataTransferLength >= MSD_IN_EP_SIZE)
                {
                    NumBytesInPacket = MSD_IN_EP_SIZE;
                    gblCBW.dCBWDataTransferLength -= MSD_IN_EP_SIZE;
                }   
                else
                {
                    //This is a short packet and will be our last IN packet sent
                    //in the transfer.
                    NumBytesInPacket = gblCBW.dCBWDataTransferLength;
                    gblCBW.dCBWDataTransferLength = 0;
                } 
                
                //We still have more bytes needing to be sent.  Check if we have
                //already fulfilled the device input expected quantity of bytes.
                //If so, we need to keep sending IN packets, but pad the extra
                //bytes with value = 0x00 (see error case 5 MSD device BOT v1.0 
                //spec handling).
                if(TransferLength.Val >= NumBytesInPacket)
                {
                    //No problem, just send the requested data and keep track of remaining count.
                    TransferLength.Val -= NumBytesInPacket;
                }    
                else
                {
                    //The host is reading more bytes than the device has to send.
                    //In this case, we still need to send the quantity of bytes requested,
                    //but we have to fill the pad bytes with 0x00.  The below for loop
                    //is execution speed inefficient, but performance isn't important 
                    //since this code only executes in the case of a host error 
                    //anyway (Hi > Di).
					DBG_INFO("host over read!");
                    for(i = 0; i < NumBytesInPacket; i++)
                    {
                        if(TransferLength.Val != 0)
                        {
                            TransferLength.Val--;     
                        }    
                        else
                        {
                            resp[i] = 0x00;
                        }    
                    }    
                }    
                
                //We are now ready to send the packet to the host.                   
                USBMSDInHandle = USBTxOnePacket(MSD_DATA_IN_EP,(BYTE*)resp, NumBytesInPacket);
                
                //Check to see if we are done sending all requested bytes of data
                if(gblCBW.dCBWDataTransferLength == 0)
                {
                    //We have sent all the requested bytes.  Go ahead and
                    //advance state so as to send the CSW.
                    MSDCommandState = MSD_COMMAND_WAIT;
                    break;                    
                }                    
            }
            break;
        case MSD_COMMAND_ERROR:
			DBG_INFO("un-supported cmd");
			// Fall through
		default:
		    //An unsupported command was received.  Since we are uncertain how many
		    //bytes we should send/or receive, we should set sense key data and then
		    //STALL, to force the host to perform error recovery.
			if(MSDCommandState != MSD_COMMAND_ERROR){DBG_INFO("unsupported cmd");}
		    MSDErrorHandler(MSD_ERROR_UNSUPPORTED_COMMAND);
 		    break;
	} // end switch	
}//void MSDProcessCommandMediaPresent(void)


/******************************************************************************
 	Function:
 		static void MSDComputeDeviceInAndResidue(WORD DiExpected)
 		
 	Description:
 		This is a private function that performs Hi > Di data size checking
 		and handling.  This function also computes the proper CSW data residue
 		and updates the global variable.
 		
 	PreCondition:
 		Should only be called in the context of the 
 		MSDProcessCommandMediaPresent() handler function, after receiving a new
 		command that needs processing.  Before calling this function, make sure
 		the gblCBW.dCBWDataTransferLength and TransferLength.Val variables have
 		been pre-loaded with the expected host and device data size values.
 		
 	Parameters:
 		WORD DiExpected - Input: Firmware can specify an addional value that 
 		might be smaller than the TransferLength.Val value.  The function will
 		update TransferLength.Val with the smaller of the original value, or
 		DiExpected.
 		
 	Return Values:
 		None
 		
 	Remarks:
 		None
 
  *****************************************************************************/
static void MSDComputeDeviceInAndResidue(WORD DiExpected)
{
  	//Error check number of bytes to send.  Check for Hi < Di
  	if(gblCBW.dCBWDataTransferLength < DiExpected)
  	{
      	//The host has requested less data than the entire reponse.  We
      	//send only the host requested quantity of bytes.
      	msd_csw.dCSWDataResidue = 0;
      	TransferLength.Val = gblCBW.dCBWDataTransferLength;
    }   	
    else
    {
        //The host requested greater than or equal to the number of bytes expected.
        if(DiExpected < TransferLength.Val)
        {
            TransferLength.Val = DiExpected;
        }    
        msd_csw.dCSWDataResidue = gblCBW.dCBWDataTransferLength - TransferLength.Val;
    }     
}    

////////////////////////////////////////////////////////////////////////////////////
//KL: Direction: logical disk -> remote device
// 
BYTE MSDReadHandler(void)
{
	// External reader state
	static DiskCacheSector_t *sector_done = NULL, *sector = NULL;
	// Internal state
	static unsigned char* readPtr = NULL;
	static unsigned long nextLba = 0;
	static unsigned short remaining = 0;

	// Endpoint clear and sector flagged complete, close sector
	if((sector_done != NULL)&&(!USBHandleBusy(USBMSDInHandle)))
	{
		DBG_VERBOSE("rd-off:%s",DBG_ultoa(sector_done->lba));
		// Sector can be returned to the pool
		DiskCacheReadClose(sector_done);
		// Clear pointer flag
		sector_done = NULL;
	}

  	switch(MSDReadState)
    {
        case MSD_READ10_WAIT:
            // Extract the BE LBA from the CBW
        	LBA.v[3]=gblCBW.CBWCB[2];
        	LBA.v[2]=gblCBW.CBWCB[3];
        	LBA.v[1]=gblCBW.CBWCB[4];
        	LBA.v[0]=gblCBW.CBWCB[5];
        	// Sector count
        	TransferLength.byte.HB = gblCBW.CBWCB[7];   
        	TransferLength.byte.LB = gblCBW.CBWCB[8];   

            // Check for possible error cases before proceeding
            if(MSDCheckForErrorCases(TransferLength.Val * (DWORD)LUNReadSectorSize()) != MSD_ERROR_CASE_NO_ERROR)
            {
				// Error checking function handles correct error responses
                break;
            }    

            // Assume success initially, msd_csw.bCSWStatus will get set to 0x01 
            // or 0x02 later if an error is detected during the actual read sequence.        	
        	msd_csw.bCSWStatus=0x0;
        	msd_csw.dCSWDataResidue=gblCBW.dCBWDataTransferLength;
        	
        	// Set up read
			nextLba = LBA.Val;
			readPtr = NULL; // Set read next state by nulling read ptr
			remaining = LUNReadSectorSize();

            //Advance state machine to begin fetching data
            MSDReadState = MSD_READ10_XMITING_DATA;

            //Fall through...
        case MSD_READ10_XMITING_DATA:

			// Check read pointer
			if(readPtr == NULL)
			{
				// Sector is not ready yet
				readPtr = DiskCacheReadOpen(nextLba, &sector);
				if(readPtr == NULL)
					break;
				DBG_VERBOSE("rd-in:%s",DBG_ultoa(nextLba));
			}

			// Test endpoint availability to send data
			if(!USBHandleBusy(USBMSDInHandle)) 
			{
				// Queue the transfer on the endpoint (IN = to host)
				USBMSDInHandle = USBTxOnePacket(MSD_DATA_IN_EP, readPtr, MSD_IN_EP_SIZE); 
				// Decrement the remaining bytes in the transaction
				msd_csw.dCSWDataResidue -= MSD_IN_EP_SIZE;
				// Adjust the position in the sector 
				remaining -= MSD_IN_EP_SIZE; 
				readPtr += MSD_IN_EP_SIZE; 
				//Check if there is any remaining data in the transaction    
				if(msd_csw.dCSWDataResidue == 0)
				{
					// Flag sector for closing on next transaction
					sector_done = sector;
					sector = NULL;
					readPtr = NULL;
					// Finished
					MSDReadState = MSD_READ10_AWAITING_COMPLETION;
					break;
				}
				// If there are no bytes left in the sector
				if(remaining == 0)
				{
					// Flag sector for closing on next transaction
					sector_done = sector;
					// Try to get next sector now
					nextLba++;
					readPtr = DiskCacheReadOpen(nextLba, &sector);
					if(readPtr){DBG_VERBOSE("rd-in:%s",DBG_ultoa(nextLba));}
					remaining = LUNReadSectorSize(); 	// Reset remaining
				}
			}
            break;
        case MSD_READ10_AWAITING_COMPLETION:
            //If the old data isn't completely sent over USB yet, need to stay
            //in this state and return, until the endpoint becomes available again.
            if(USBHandleBusy(USBMSDInHandle))
            {
                break;
            }
            else
            {
                MSDReadState = MSD_READ10_WAIT;
            }    
            break;
        case MSD_READ10_ERROR:
			// Close open sectors
			if(sector != NULL)
			{
				DiskCacheReadClose(sector);
				DBG_VERBOSE("rd-off:%s",DBG_ultoa(sector->lba));
			}
			sector = NULL;
			if(sector_done != NULL)
			{
				DiskCacheReadClose(sector_done);
				DBG_VERBOSE("rd-off:%s",DBG_ultoa(sector_done->lba));
			}
			sector_done = NULL;
			readPtr = NULL;
        default:
            //A read error occurred.  Notify the host.
			if(MSDReadState != MSD_READ10_ERROR){DBG_ERROR("read state err");}
            msd_csw.bCSWStatus=0x01;  //indicate error
            //Set error status sense keys, so the host can check them later
            //to determine how to proceed.
            gblSenseData[LUN_INDEX].SenseKey=S_MEDIUM_ERROR;
	        gblSenseData[LUN_INDEX].ASC=ASC_NO_ADDITIONAL_SENSE_INFORMATION;
	        gblSenseData[LUN_INDEX].ASCQ=ASCQ_NO_ADDITIONAL_SENSE_INFORMATION;
            //Make sure the IN endpoint is available before advancing the state machine.
            //Host will expect CSW next.
            if(USBHandleBusy(USBMSDInHandle) != 0)
            {
                break;
            }
            //Stall data endpoint and advance state machine
            USBStallEndpoint(MSD_DATA_IN_EP,1); //Will cause host to perform clear endpoint halt, then request CSW
            MSDReadState = MSD_READ10_WAIT;
    }
    return MSDReadState;
}

////////////////////////////////////////////////////////////////////////////////////
//KL: Direction: logical disk <- remote device
// 
BYTE MSDWriteHandler(void)
{
	// External reader state
	static DiskCacheSector_t* sector = NULL;	
	// Internal state
	static unsigned char* writePtr = NULL;
	static unsigned long nextLba = 0;
	static unsigned short remaining = 0;

    static USB_HANDLE outHandle[2];
    static unsigned char nextHandle;

    switch(MSDWriteState)
    {
        case MSD_WRITE10_WAIT:
            // Extract the BE LBA from the CBW
        	LBA.v[3]=gblCBW.CBWCB[2];
        	LBA.v[2]=gblCBW.CBWCB[3];
        	LBA.v[1]=gblCBW.CBWCB[4];
        	LBA.v[0]=gblCBW.CBWCB[5];
        	// Sector count
        	TransferLength.byte.HB = gblCBW.CBWCB[7];   
        	TransferLength.byte.LB = gblCBW.CBWCB[8];   
            msd_csw.dCSWDataResidue = gblCBW.dCBWDataTransferLength;

     		//Check if the media is write protected before deciding what
      		//to do with the data.
      		if(LUNWriteProtectState()) 
            {
				DBG_INFO("write, but protected!");
                //The media appears to be write protected.
          	    //Let host know error occurred.  The bCSWStatus flag is also used by
          	    //the write handler, to know not to even attempt the write sequence.
          	    msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;    

                //Set sense keys so the host knows what caused the error.
          	    gblSenseData[LUN_INDEX].SenseKey=S_DATA_PROTECT;
          	    gblSenseData[LUN_INDEX].ASC=ASC_WRITE_PROTECTED;
          	    gblSenseData[LUN_INDEX].ASCQ=ASCQ_WRITE_PROTECTED;

                //Stall the OUT endpoint, so as to promptly inform the host
                //that the data cannot be accepted, due to write protected media.
          		USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
          		MSDWriteState = MSD_WRITE10_WAIT;
          	    return MSDWriteState;
          	}

        	// Setup transfer
			nextLba = LBA.Val;
			// If running, returns valid write ptr at start of write.
			writePtr = DiskCacheWriteOpen(nextLba, &sector); 
			DBG_VERBOSE("wt_on:%s",DBG_ultoa(nextLba));
			remaining = LUNReadSectorSize();
        	
            // Arm both MSD bulk OUT endpoints (even and odd) to begin receiving
            // the data to write to the media, from the host.
            if((writePtr != NULL) && (!USBHandleBusy(USBMSDOutHandle)) && (!USBHandleBusy(USBGetNextHandle(MSD_DATA_OUT_EP, OUT_FROM_HOST))))
            {
				// Either fresh write or whole sector was written, load both endpoints
                outHandle[0] = USBRxOnePacket(MSD_DATA_OUT_EP, writePtr, MSD_OUT_EP_SIZE);
                outHandle[1] = USBRxOnePacket(MSD_DATA_OUT_EP, writePtr + MSD_OUT_EP_SIZE, MSD_OUT_EP_SIZE);
				writePtr += (2*MSD_OUT_EP_SIZE);
				nextHandle = 0;
				USBMSDOutHandle = outHandle[0];
            }
            else
            {
                // Something is wrong.  The endpoints should have been free at 
                // this point in the code. Inform host of error and wait.
				DBG_ERROR("lock-up suspect");
                msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
                USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
          		MSDWriteState = MSD_WRITE10_WAIT;
				// Release sector
				if(sector != NULL)
				{
					DiskCacheWriteAbort(sector);
					DBG_VERBOSE("wt-off:%s",DBG_ultoa(sector->lba));
					sector = NULL;
				}
          		break;                
            }        

        	MSDWriteState = MSD_WRITE10_RX_SECTOR;
            //Fall through to MSD_WRITE10_RX_SECTOR
			
        case MSD_WRITE10_RX_SECTOR:
        {
			// Check we can receive new data
			if(writePtr == NULL)
			{
				// Sector is not ready yet. Still waiting for last write to complete.
				if(!DiskCacheWriteReady(nextLba))
				{
					// Not ready - keep waiting.
					DBG_INFO("write held up");	
					break;
				}
				// Now get next write buffer
				writePtr = DiskCacheWriteOpen(nextLba, &sector);
				DBG_VERBOSE("wt_on:%s",DBG_ultoa(nextLba));
				if(writePtr == NULL)
				{
					// Not ready - This is an error since we already checked it would not block
					DBG_INFO("error");
					break;
				}
				// Check phase is correct. Always zero for first part of every sector.
				if(nextHandle != 0)
				{	
					MSDWriteState = MSD_WRITE10_ERROR;
					break;
				}
				// Another sector to be written, load both endpoints and set next ptr to mid-sector.
                outHandle[0] = USBRxOnePacket(MSD_DATA_OUT_EP, writePtr, MSD_OUT_EP_SIZE);
                outHandle[1] = USBRxOnePacket(MSD_DATA_OUT_EP, writePtr + MSD_OUT_EP_SIZE, MSD_OUT_EP_SIZE);
				writePtr += (2*MSD_OUT_EP_SIZE);
			}

			// Check for data input
			if(!USBHandleBusy(outHandle[nextHandle]))
			{
				// Packet received - check size ok first
				if(USBHandleGetLength(outHandle[nextHandle]) != MSD_OUT_EP_SIZE)
				{
					// All transfers should be a full endpoint, error if not
					DBG_ERROR("write ep err");
					MSDWriteState = MSD_WRITE10_ERROR;
					break;
				}
				// Decrement residue
				msd_csw.dCSWDataResidue -= MSD_OUT_EP_SIZE;
				if(msd_csw.dCSWDataResidue == 0)
				{
	          	    // All of write request received
	          	    MSDWriteState = MSD_WRITE10_AWAITING_COMPLETION;
					// Commit the last sector of the write
					DiskCacheWriteClose(sector);
					DBG_VERBOSE("wt-off:%s",DBG_ultoa(sector->lba));
					sector = NULL;
					writePtr = NULL;
					// Set global next endpoint ptr (where next CBW arrives at)
					USBMSDOutHandle = USBGetNextHandle(MSD_DATA_OUT_EP, OUT_FROM_HOST);
					break;
				}
				// Check for remaining bytes in current sector, zero indicates end of sector
				remaining -= MSD_OUT_EP_SIZE;
				if(remaining == 0)
				{
					DiskCacheWriteClose(sector); // Indicate sector is complete
					DBG_VERBOSE("wt-off:%s",DBG_ultoa(sector->lba));
					sector = NULL;
					writePtr = NULL;				// Null write position
					nextLba++;						// Increment to next sector
					remaining = LUNReadSectorSize();// Reset remaining (at least one more sector to go)
					// New empty sector buffer will be loaded next call
				}
				else if (remaining <= MSD_OUT_EP_SIZE)
				{
					// Second to last transfer in the sector hits this condition.
					// We will already have the other endpoint armed so do nothing.
				}
				else
				{
					// Re-arm *this* end point again. *Next* endpoint already armed.
					outHandle[nextHandle] = USBRxOnePacket(MSD_DATA_OUT_EP, writePtr, MSD_OUT_EP_SIZE);
					writePtr += MSD_OUT_EP_SIZE;
				}
				// Next time, check other endpoint for data
				if(nextHandle==0) 	nextHandle = 1;
				else 				nextHandle = 0;
			}
			break;
        }//case MSD_WRITE10_RX_SECTOR:

		case MSD_WRITE10_AWAITING_COMPLETION : 
			// It is important to wait for the writer to return to idle before allowing the write to fully complete. 
			// Otherwise, the first sector write could stall after telling the host we were ready.
			if(DiskCacheWriteReady(sector->lba))
				MSDWriteState = MSD_WRITE10_WAIT; // Inform host we are ready for the next write
			break;

        case MSD_WRITE10_ERROR :
			// Close last sector
			DiskCacheWriteAbort(sector);
			DBG_VERBOSE("wt-off:%s",DBG_ultoa(sector->lba));
			sector = NULL;
			writePtr = NULL;
		default :
            //Illegal condition which should not occur.  If for some reason it
            //does, try to let the host know know an error has occurred.
			// All transfers should be a full endpoint, error if not
			if(MSDWriteState != MSD_WRITE10_ERROR){DBG_ERROR("write state err");}
			msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;
			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
			// Abort this transfer
			MSDWriteState = MSD_WRITE10_WAIT;
			// Error out write
			if(sector != NULL)
			{
				DiskCacheWriteAbort(sector);
				DBG_VERBOSE("wt-off:%s",DBG_ultoa(sector->lba));
				sector = NULL;
			}
			break;           
    }
    
    return MSDWriteState;
}

/******************************************************************************
 	Function:
 		void ResetSenseData(void)
 		
 	Description:
 		This routine resets the Sense Data, initializing the
 		structure RequestSenseResponse gblSenseData.
 		
 	PreCondition:
 		None 
 		
 	Parameters:
 		None
 		
 	Return Values:
 		None
 		
 	Remarks:
 		None
 			
  *****************************************************************************/
void ResetSenseData(void) 
{
	gblSenseData[LUN_INDEX].ResponseCode=S_CURRENT;
	gblSenseData[LUN_INDEX].VALID=0;			// no data in the information field
	gblSenseData[LUN_INDEX].Obsolete=0x0;
	gblSenseData[LUN_INDEX].SenseKey=S_NO_SENSE;
	//gblSenseData.Resv;
	gblSenseData[LUN_INDEX].ILI=0;
	gblSenseData[LUN_INDEX].EOM=0;
	gblSenseData[LUN_INDEX].FILEMARK=0;
	gblSenseData[LUN_INDEX].InformationB0=0x00;
	gblSenseData[LUN_INDEX].InformationB1=0x00;
	gblSenseData[LUN_INDEX].InformationB2=0x00;
	gblSenseData[LUN_INDEX].InformationB3=0x00;
	gblSenseData[LUN_INDEX].AddSenseLen=0x0a;	// n-7 (n=17 (0..17))
	gblSenseData[LUN_INDEX].CmdSpecificInfo.Val=0x0;
	gblSenseData[LUN_INDEX].ASC=0x0;
	gblSenseData[LUN_INDEX].ASCQ=0x0;
	gblSenseData[LUN_INDEX].FRUC=0x0;
	gblSenseData[LUN_INDEX].SenseKeySpecific[0]=0x0;
	gblSenseData[LUN_INDEX].SenseKeySpecific[1]=0x0;
	gblSenseData[LUN_INDEX].SenseKeySpecific[2]=0x0;
}



/******************************************************************************
 	Function:
 		BYTE MSDCheckForErrorCases(DWORD DeviceBytes)
 		
 	Description:
 	   This function can be called to check for various error cases, primarily 
 	   the "Thirteen Cases" errors described in the MSD BOT v1.0 specs.  If an
 	   error is detected, the function internally calls the MSDErrorHandler()
 	   handler function, to take care of appropriately responding to the host, 
 	   based on the error condition.
 	PreCondition:
 	    None
 	     		
 	Parameters:
 		DWORD DeviceBytes - Input: This is the total number of bytes the MSD 
 		            device firmware is expecting in the MSD transfer.  
 	Return Values:
 		BYTE - Returns a byte containing the error code.  The possible error
 		    cases that can be detected and reported are:
            MSD_ERROR_CASE_NO_ERROR - None of the "Thirteen cases" errors were detected
            MSD_ERROR_CASE_2 	            
            MSD_ERROR_CASE_3 	            
            MSD_ERROR_CASE_4 	            
            MSD_ERROR_CASE_5 	            
            MSD_ERROR_CASE_7 	            
            MSD_ERROR_CASE_8 	            
            MSD_ERROR_CASE_9 	            
            MSD_ERROR_CASE_11               
            MSD_ERROR_CASE_10               
            MSD_ERROR_CASE_13               
 		
 	Remarks:
 		None
 			
  *****************************************************************************/
BYTE MSDCheckForErrorCases(DWORD DeviceBytes)
{
    BYTE MSDErrorCase;
    BOOL HostMoreDataThanDevice;
    BOOL DeviceNoData;
  
    //Check if device is expecting no data (Dn)
    if(DeviceBytes == 0)
    {
        DeviceNoData = TRUE;
    }    
    else
    {
        DeviceNoData = FALSE;
    }     
    
    //First check for the three good/non-error cases
    
    //Check for good case: Hn = Dn (Case 1)
    if((MSDHostNoData == TRUE) && (DeviceNoData == TRUE))
    {
        return MSD_ERROR_CASE_NO_ERROR;
    }    

    //Check for good cases where the data sizes between host and device match
    if(gblCBW.dCBWDataTransferLength == DeviceBytes)
    {
        //Check for good case: Hi = Di (Case 6)
        if(MSD_State == MSD_DATA_IN)
        {
            //Make sure Hi = Di, instead of Hi = Do
            if(MSDCommandState != MSD_WRITE_10)
            {
                return MSD_ERROR_CASE_NO_ERROR;
            }    
        }
        else //if(MSD_State == MSD_DATA_OUT)  
        {
            //Check for good case: Ho = Do (Case 12)
            //Make sure Ho = Do, instead of Ho = Di
            if(MSDCommandState == MSD_WRITE_10)
            {
                return MSD_ERROR_CASE_NO_ERROR;
            }             
        }      
    }    

    //If we get to here, this implies some kind of error is occuring.  Do some
    //checks to find out which error occurred, so we know how to handle it.

    //Check if the host is expecting to transfer more bytes than the device. (Hx > Dx)
    if(gblCBW.dCBWDataTransferLength > DeviceBytes)
    {
        HostMoreDataThanDevice = TRUE;
    }   
    else
    {
        HostMoreDataThanDevice = FALSE;
    } 
 
    //Check host's expected data direction
	if(MSD_State == MSD_DATA_OUT)
	{
    	//First check for Ho <> Di (Case 10)
    	if((MSDCommandState != MSD_WRITE_10) && (DeviceNoData == FALSE))
    	    MSDErrorCase = MSD_ERROR_CASE_10;
   	   	//Check for Hn < Do  (Case 3)
    	else if(MSDHostNoData == TRUE)  
    	    MSDErrorCase = MSD_ERROR_CASE_3;
    	//Check for Ho > Dn  (Case 9)
    	else if(DeviceNoData == TRUE)
    	    MSDErrorCase = MSD_ERROR_CASE_9;
    	//Check for Ho > Do  (Case 11)
    	else if(HostMoreDataThanDevice == TRUE)
    	    MSDErrorCase = MSD_ERROR_CASE_11;
    	//Check for Ho < Do  (Case 13)
    	else //if(gblCBW.dCBWDataTransferLength < DeviceBytes)
    	    MSDErrorCase = MSD_ERROR_CASE_13;
    }    
    else //else the MSD_State must be == MSD_DATA_IN
    {
    	//First check for Hi <> Do (Case 8)
    	if(MSDCommandState == MSD_WRITE_10)
    	    MSDErrorCase = MSD_ERROR_CASE_8;    	
    	//Check for Hn < Di  (Case 2)
    	else if(MSDHostNoData == TRUE)  
    	    MSDErrorCase = MSD_ERROR_CASE_2;
    	//Check for Hi > Dn  (Case 4)
    	else if(DeviceNoData == TRUE)
    	    MSDErrorCase = MSD_ERROR_CASE_4;
    	//Check for Hi > Di  (Case 5)
    	else if(HostMoreDataThanDevice == TRUE)
    	    MSDErrorCase = MSD_ERROR_CASE_5;
        //Check for Hi < Di  (Case 7)
    	else //if(gblCBW.dCBWDataTransferLength < DeviceBytes)
    	    MSDErrorCase = MSD_ERROR_CASE_7;
    }        
    //Now call the MSDErrorHandler(), based on the error that was detected.
    MSDErrorHandler(MSDErrorCase);
    return MSDErrorCase;
}    


/******************************************************************************
 	Function:
 		void MSDErrorHandler(BYTE ErrorCase)
 		
 	Description:
 	    Once an error condition has been detected, this function can be called
 	    to set the proper states and perform the proper tasks needed to let the
 	    host know about the error.
 	PreCondition:
 		Firmware should have already determined an error occurred, and it should
 		know what the error code was before calling this handler.
 		
 	Parameters:
 		BYTE ErrorCase - Input: This is the error code that the firmware 
 		                    detected.  This error code will determine how the
 		                    handler will behave (ex: what status to send to host,
 		                    what endpoint(s) should be stalled, etc.).
 		                    The implemented error case possibilities are (suffix
 		                    numbers correspond to the "Thirteen cases" numbers 
 		                    described in the MSD BOT specs v1.0):
 		                    
                            MSD_ERROR_CASE_2 	            
                            MSD_ERROR_CASE_3 	            
                            MSD_ERROR_CASE_4 	            
                            MSD_ERROR_CASE_5 	            
                            MSD_ERROR_CASE_7 	            
                            MSD_ERROR_CASE_8 	            
                            MSD_ERROR_CASE_9 	            
                            MSD_ERROR_CASE_11               
                            MSD_ERROR_CASE_10               
                            MSD_ERROR_CASE_13               
                            MSD_ERROR_UNSUPPORTED_COMMAND   

 	Return Values:
 		None
 		
 	Remarks:
 		None
 			
  *****************************************************************************/
void MSDErrorHandler(BYTE ErrorCase)
{
    BYTE OldMSD_State;
    
	//Both MSD bulk IN and OUT endpoints should not be busy when these error cases are detected
	//If for some reason this isn't true, then we should preserve the state machines states for now.
    if((USBHandleBusy(USBMSDInHandle)) || (USBHandleBusy(USBMSDOutHandle)))
    {
    	return;	
    }
	
	DBG_INFO("handling err: %u",(unsigned short)ErrorCase);
    //Save the old state before we change it.  The old state is needed to determine
    //the proper handling behavior in the case of receiving unsupported commands.
    OldMSD_State = MSD_State;

	//Reset main state machines back to idle values.
	MSDCommandState = MSD_COMMAND_WAIT;
	MSDReadState = MSD_READ10_WAIT;
	MSDWriteState = MSD_WRITE10_WAIT;
	//After the conventional 13 test cases failures, the host still expects a valid CSW packet
    msd_csw.dCSWDataResidue = gblCBW.dCBWDataTransferLength; //Indicate the unconsumed/unsent data
   	msd_csw.bCSWStatus = MSD_CSW_COMMAND_FAILED;    //Gets changed later to phase error for errors that user phase error
	MSD_State = MSD_SEND_CSW;	        

    //Now do other error related handling tasks, which depend on the specific 
    //error	type that was detected.
	switch(ErrorCase)
	{
		case MSD_ERROR_CASE_2://Also CASE_3
			msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;	
        	break;	
		case MSD_ERROR_CASE_4://Also CASE_5
       		USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);	//STALL the bulk IN MSD endpoint
			break;
		case MSD_ERROR_CASE_7://Also CASE_8
       		msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;	
       		USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);	//STALL the bulk IN MSD endpoint
       		break;		
		case MSD_ERROR_CASE_9://Also CASE_11
			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST); //Stall the bulk OUT endpoint
			break;
		case MSD_ERROR_CASE_10://Also CASE_13
	        msd_csw.bCSWStatus = MSD_CSW_PHASE_ERROR;	
			USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST);
			break;
			
        case MSD_ERROR_UNSUPPORTED_COMMAND:
        	ResetSenseData();
			gblSenseData[LUN_INDEX].SenseKey=S_ILLEGAL_REQUEST;
			gblSenseData[LUN_INDEX].ASC=ASC_INVALID_COMMAND_OPCODE;
			gblSenseData[LUN_INDEX].ASCQ=ASCQ_INVALID_COMMAND_OPCODE;
            if((OldMSD_State == MSD_DATA_OUT) && (gblCBW.dCBWDataTransferLength != 0))
            {
			    USBStallEndpoint(MSD_DATA_OUT_EP, OUT_FROM_HOST); 
			}
			else
			{
        	    USBStallEndpoint(MSD_DATA_IN_EP, IN_TO_HOST);
            }     	    
            break;
		default:	//Shouldn't get hit, don't call MSDErrorHandler() if there is no error
			break;
	}//switch(ErrorCase)			
}	



//-----------------------------------------------------------------------------------------
#endif //end of #ifdef USB_USE_MSD
//End of file usb_function_msd.c

