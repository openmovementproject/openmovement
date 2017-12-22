// Pedometer code
// Karim Ladha 2013-2014
#ifndef PEDOMETER_H
#define PEDOMETER_H

// If on a non-embedded system...
#if defined(_WIN32) || defined(__APPLE__) || defined(__linux__)
#define PEDOMETER_VARIABLE
#endif

#ifdef PEDOMETER_VARIABLE
// Variable pedometer behaviour
typedef struct {
unsigned short samp_rate;				/*Sample rate in Hz*/
unsigned short one_g_shift;				/*shift to make 1g acceleration equal approx 32 (4096/32 = 2^7)*/
unsigned short svm_filter_val;			/*sets number of samples used for svm filtering, power of 2*/
unsigned short smooth_lpf_bw;			/*sets number of samples for signal smoothing, power of 2*/
unsigned short interval_filter_const;	/*sets number of steps to calculate step frequency over, power of 2*/
unsigned short threshold_decay_val;		/*set rate at which threshold decays, fraction of 256*/
unsigned short activity_threshold;		/*threshold of ac signal to count steps, 1g x fraction of 32*/
unsigned short min_step_interval;		/*minimum step interval in samples*/
unsigned short max_step_interval;		/*maximum step interval in samples*/
} pedVars_t;
extern pedVars_t pedVars;				/*Make and set externally*/
#define SAMP_RATE				pedVars.samp_rate	
#define ONE_G_SHIFT				pedVars.one_g_shift
#define SVM_FILTER_VAL			pedVars.svm_filter_val
#define	SMOOTH_LPF_BW			pedVars.smooth_lpf_bw
#define INTERVAL_FILTER_CONST	pedVars.interval_filter_const
#define THRESHOLD_DECAY_VAL		pedVars.threshold_decay_val	
#define ACTIVITY_THRESHOLD		pedVars.activity_threshold	
#define MIN_STEP_INTERVAL		pedVars.min_step_interval	
#define MAX_STEP_INTERVAL		pedVars.max_step_interval		
#else

// For +/-8g (1g = 4096)
#define ONE_G_SHIFT				7		/*Shift to make 1g acceleration equal approx 32 (4096/32 = 2^7)*/

// Const values 100Hz and 50Hz, choose one
//#define SAMP_RATE				100		/*Sample rate in Hz*/
#define SAMP_RATE				50		/*Sample rate in Hz*/

#if (SAMP_RATE == 100)
#define SVM_FILTER_VAL			9		/*Sets number of samples used for svm filtering, power of 2*/
#define	SMOOTH_LPF_BW			5		/*Sets number of samples for signal smoothing, power of 2*/
#define THRESHOLD_DECAY_VAL		16		/*Set rate at which threshold decays, fraction of 256*/
#define MIN_STEP_INTERVAL		20		/*Minimum step interval in samples*/
#define MAX_STEP_INTERVAL		200		/*Maximum step interval in samples*/
#else if(SAMP_RATE==50)
#define SVM_FILTER_VAL			8		/*Sets number of samples used for svm filtering, power of 2*/
#define	SMOOTH_LPF_BW			4		/*Sets number of samples for signal smoothing, power of 2*/
#define THRESHOLD_DECAY_VAL		8		/*Set rate at which threshold decays, fraction of 256*/
#define MIN_STEP_INTERVAL		10		/*Minimum step interval in samples*/
#define MAX_STEP_INTERVAL		100		/*Maximum step interval in samples*/

// For general step counting
#define INTERVAL_FILTER_CONST	3		/*Sets number of steps to calculate step frequency over, power of 2*/
#define ACTIVITY_THRESHOLD		4		/*Threshold of AC signal to count steps, 1g x fraction of 32*/
#endif

#endif

// Types
// State of step counter
typedef enum {A_RISING, A_FALLING, A_LATCHED} pedState_t;
// Main pedometer context
typedef struct {
	// User readable/writable
	unsigned short 	steps;
	unsigned long 	eeSum;
	// Internal only
	pedState_t 		state;
	unsigned short 	tick;
	unsigned short	lastStep;
	unsigned short	lastInterval;	
	unsigned long	iirAveInterval;
	unsigned short	aveInterval;
	signed short 	acc;
	unsigned long 	iirAveSvm;
	unsigned short	aveSvm;
	unsigned long	iirAveAcc;
	unsigned short	aveAcc;
	unsigned short 	peakTracker;
	unsigned short	highThresh;
	unsigned short	lowThresh;
} ped_t;
extern ped_t ped;
// Internal accel type (same as normal accel type)
typedef struct {
	signed short x;	
	signed short y;
	signed short z;
} ped_accel_t;

// Prototypes
// Call init first, it uses the initialiser for approximating svm 
extern void PedInit(ped_accel_t* initialiser);
// Call on samples as required
extern void PedTasks(ped_accel_t* data, unsigned short num);

#endif
//EOF
