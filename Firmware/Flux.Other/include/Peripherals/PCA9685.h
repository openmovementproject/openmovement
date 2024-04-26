// PCA9685 LED controller driver header
// Karim Ladha 2014

// Globals
typedef struct {
	unsigned short led_on;
	unsigned short led_off;
}ledVal_t;

typedef struct exLedState {
	unsigned char present;
	unsigned char asleep;
	ledVal_t ledVal[16];
	unsigned char led[16];
	unsigned char gScale;
} exLedState_t;

extern exLedState_t exLedState;

// Prototypes
unsigned char ExLedVerifyDeviceId(void);

void ExLedStartup(unsigned char doSleep);

void ExLedUpdate(void);

void ExLedUpdateRaw(void);

void ExLedSleep(void);

void ExLedWake(void);

//EOF
