// KL 06-01-2012
// Humidicon device driver


// Functions
char Humidicon_init(void); 
char Humidicon_measurement_request(void); 
void Humidicon_read(void);

short Humidicon_Convert_centiC(unsigned short value);
unsigned short Humidicon_Convert_percentage(unsigned short value);

// Typedef
typedef struct
{
	char status;			// (0 = normal, 1 = stale, 2 = command mode, 3 = diagnostic, -1 = not present)
 	unsigned short humidity;
	unsigned short temperature; 
} humidicon_t;

// Globals
extern char humPresent;
extern humidicon_t gHumResults;


//EOF



