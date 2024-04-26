// Karim Ladha 24-06-2015
// Driver for combining multiple nand memories in plane wise fashion to make a larger device
// ONFI parameter page is ignored since all devices need checking manually anyway; Uses ID bytes
// The Control lines are assumed to be all common but with separate CE and RB pins per device

/* Assumptions:
The NAND devices are ONFI 1.0 compiant and have up to 128 pages per block
The devices all support page copy back and can perform odd to even page copy
Odd to even block copy is not possible because they are on separate planes  
*/

/* Requirements:
The following variables are required to allow access to the different memory 
chips. The example below shows 2 chips with a common RB pin:
// Include for pin type
#include "Peripherals/GenericIO.h" 
// Definition of device count
#define NUMBER_OF_NANDS		Number of chip selects 
#define NAND_TIMEOUT 		Timeout of longest operation 
// NAND device pin definitions
const IO_pin_t nandPins[TOTAL_NAND_PINS] = {
	{(void*)&NAND_CLE_PORT, (1<<NAND_CLE_PORT_PIN_NUM)},
	{(void*)&NAND_ALE_PORT, (1<<NAND_ALE_PORT_PIN_NUM)},
	{(void*)NULL, 0 Write protect pin unused (placeholder)},
	{(void*)&NAND_DEVICE1_CE_PORT, (1<<NAND_DEVICE1_CE_PIN_NUM)},
	{(void*)&NAND_DEVICE2_CE_PORT, (1<<NAND_DEVICE2_CE_PIN_NUM)},
	{(void*)&NAND_DEVICE1_RB_PORT, (1<<NAND_DEVICE1_RB_PIN_NUM)},
	{(void*)&NAND_DEVICE2_RB_PORT, (1<<NAND_DEVICE2_RB_PIN_NUM)}};
*/

/* Limitations:
Using devices with different sized blocks and pages will aggregate using the smallest value 
*/

// Definitions 
#define MAX_NAND_ID_LENGTH		7		// Max nand id len (must be >= 5) 
#define NAND_SUCCESS			1		// Return value, i.e. true
#define NAND_ERROR				0		// Return value, i.e. false

// NAND device pin indexes in pin array
#define PIN_NAND_CLE			0		// Index of CLE in the nandPins array
#define PIN_NAND_ALE			1		// Index of ALE in the nandPins array
#define PIN_NAND_WP				2		// Index of WP in the nandPins array (currently unused)
#define PIN_NAND_CE1			3		// Index offset of CE pins in the nandPins array
#define PIN_NAND_RB1			(PIN_NAND_CE1 + NUMBER_OF_NANDS) // Index offset of RB pins 
#define TOTAL_NAND_PINS			(3 + 2*NUMBER_OF_NANDS)

// Types
/* Device information struct */
typedef struct NandDeviceInfo_tag{ /* 0 = not present for any entry */
	unsigned short blocks;		// Block count 
	unsigned char blockPlanes;	// Number of planes
	unsigned char blockPages;	// Page count
	unsigned short pageBytes;	// Page length
} NandDeviceInfo_t;
/* Device types and their information */
typedef struct {
	const unsigned char devIdLen;
	const unsigned char devId[MAX_NAND_ID_LENGTH];
	const NandDeviceInfo_t info;
} NandType_t;

// Globals
/* If compatible device(s) found, nandPresent is != 0 */
extern char nandPresent;  
/* List of additional supported devices, optional */
extern const NandType_t nandTypesList[];
/* List of discovered devices populated during initialize */
extern NandDeviceInfo_t nandDevices[];

// Prototypes 
// Initialize the NAND device
char NandInitialize(void);

// Shut down the NAND device
char NandShutdown(void);

// Verify the device id
unsigned char NandVerifyDeviceId(void);

// Get the nand parameters for the FTL
NandDeviceInfo_t* NandDeviceInfo(void);

// Erase a block
char NandEraseBlock(unsigned short block);

// Write page directly
char NandWritePage(unsigned short destBlock, unsigned char destPage, unsigned char *buffer);

// Read page directly
char NandReadPage(unsigned short srcBlock, unsigned char srcPage, unsigned char *buffer);

// Copy a page
char NandCopyPage(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage);

// Load a page in to the buffer for reading
char NandLoadPageRead(unsigned short block, unsigned char page);

// Read in from the page buffer (optional crc and checksums)
char NandReadBuffer(unsigned short offset, unsigned char *buffer, unsigned short length);
char NandReadBuffer512WordSummed(unsigned short offset, unsigned short *buffer, unsigned short *outSum);
char NandReadBuffer512WordSummedCrc(unsigned short offset, unsigned short *buffer, unsigned short *outSum, unsigned short *crc);

// Load a page in to the buffer for writing to the specified location
char NandLoadPageWrite(unsigned short srcBlock, unsigned char srcPage, unsigned short destBlock, unsigned char destPage);

// Write in to the loaded page buffer
char NandWriteBuffer(unsigned short offset, const unsigned char *buffer, unsigned short length);

// Commit the loaded page buffer
char NandStorePage(void);

// Commit the loaded page buffer
char NandStorePageRepeat(unsigned short block, unsigned char page);

// Get a devices information page (not all ONFI 1.0 devices support this)
char NandReadDeviceInfoPage(unsigned char chip, unsigned char *buffer, unsigned short length);

// Debug functions for emulated NAND
#ifdef _WIN32
void NandDebugRead(unsigned short block, unsigned char page, unsigned short offset, unsigned char *buffer, unsigned short length);
void NandDebugCorrupt(unsigned short block, unsigned char page);
void NandDebugFail(unsigned short block, unsigned char page);
#endif

//EOF


