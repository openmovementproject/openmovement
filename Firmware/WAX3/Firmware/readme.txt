WAX Firmware


Demo device settings:

TARGET=0
ACTTHRES=4
INACTTHRES=2
ACTTIME=15
TRANINT=5000
JITTERMASK=2047
RATE=25
WATERMARK=10
DEVICE




The commands you should need to adjust are (here showing what should be their current values):

  ACTTHRES=4       -- wake-up activity threshold (0-255; * 62.5 mg), default 4 = 250 mg.
  INACTTHRES=2     -- go-to-sleep inactivity threshold (0-255; * 62.5 mg), default 2 = 125 mg.
  ACTTIME=4        -- go-to-sleep inactivity time (0-255, * 1 second), default 4 = 4 seconds.
  TRANINT=31250    -- transmission interval (* 4 us), default 31250 = 125 ms.
  JITTERMASK=8191  -- transmission jitter mask (* 4 us, must be a value (2^n)-1;), default 8191 (=0x1fff) = 32 ms.


Other commands (that you shouldn't need) are:

  DEVICE=1         -- device id (0-65535), default 1+.
  TARGET=0         -- target address for the data (0-65535), default 0.
  CHANNEL=15       -- transmission channel (11-26), default 15.
  PAN=4660         -- network id (0-65535), default 4660 = 0x1234.
  WATERMARK=10     -- number of samples stored on accelerometer before waking processor (0-31), default 10.
  RATE=25          -- sample rate code (various coded values), default 25: 50 Hz low-power +- 16g. 
                      ...add 1 for 100 Hz, 2 for 200 Hz, etc.
                      ...subtract 16 for non-low-power mode (less noise).
                      ...add 64 for +- 8g sensitivity, 128 for +- 4g sensitivity, 256 for +- 2g sensitivity.


For this application, where power isn't an issue, I would suggest trying these settings:

  TRANINT=3125
  JITTERMASK=0
  ACTTIME=255


...these settings should attempt to transmit a sample every 12.5 msec (the actual rate will end up being around 20 msec as there won't be samples ready at each transmission time), and the last setting will cause the device to transmit for around 4.25 minutes after any activity.  You could also reduce the activity threshold for wake-up if required. 






TODO:





Compile-time parameters:
•	device address (sender RFD / receiver FFD)
•	default settings (channel, PAN ID, parent address, data settings)

[Attached]
Boot-loader: Timeout to main program.

Connected: Charging. Data connection to rewrite config.

[Detatched] - disable auto-sleep.
Hibernate: Lowest power deep sleep config - wake on USB connect only.

Standby: Processor sleeping. Wake on accelerometer activity interrupt or USB attach.

Sampling: Setup accelerometer to sample to FIFO with watermark interrupt, unlinked activity/inactivity detection. Sleep until timer/FIFO. At time-jittered intervals, flush the accelerometer FIFO to buffers, transmit the current buffer (include offset from end of buffer)
•	First packet is special metadata packet.
•	Calculate new jitter. If not sent metadata, increasing interval (remember interval between wakes). Otherwise, halved interval if >1 packets to transmit.
•	If RAM buffer overflow (or n buffers dropped), assume out of range and give up for a while (forced sleep delay?)











  // Called before main()
  void __init(void);


  // Startup
  void _startup (void)
  {
    _asm lfsr 1, _stack _endasm
    _asm lfsr 2, _stack _endasm
    _asm clrf TBLPTRU, 0 _endasm
    // ...
  }





#define SPI1_PUT(_v) { PIR1bits.SSPIF  = 0; do { SSPCON1bits.WCOL  = 0; SSPBUF  = (_v); } while (SSPCON1bits.WCOL);  while (!PIR1bits.SSPIF);  }
#define SPI2_PUT(_v) { PIR3bits.SSP2IF = 0; do { SSP2CON1bits.WCOL = 0; SSP2BUF = (_v); } while (SSP2CON1bits.WCOL); while (!PIR3bits.SSP2IF); }


	// This needs to run very fast - the MRF module trys to grab 25mA on a POR and it must be stopped quickly
	CLOCK_INTOSC_1M(); 
        
	// This code reduces the MRF chip current to ~3mA but it is still in its powered state
        ACCEL_CS        = 1;
        ACCEL_CS_PIN    = 0;
        PHY_CS          = 1;
        PHY_CS_TRIS     = 0;
        PHY_RESETn      = 0;
        PHY_RESETn_TRIS = 0;
        LED_PIN         = 0;
        LED             = 0;
        
	// Remap the pins to talk to the MRF chip (this assumes RF_USE_SPI==2, and ENABLE_PA_LNA power amplifier not defined)
	REMAP_PINS();
	
	// Init the pins to talk to the MRF chip
	RF_INIT_PINS();                                 // Init the pins to talk to the MRF chip, sets up the SPI interface
        RFIE = 0;                                       // Disable interrupts

        // Perform an MRF hardware reset
        PHY_RESETn = 0;
        { short j; for (j = 0; j < (short)300; j++); }
        PHY_RESETn = 1;
        { short j; for (j = 0; j < (short)300; j++); }
        
	// Put MRF to sleep mode (~2uA)
        PHY_WAKE = 0;                                   // Clear the WAKE pin in order to allow the device to go to sleep
        PHY_CS = 0; SPI2_PUT(0x55); SPI2_PUT(0x04); PHY_CS = 1;  // (WRITE_SOFTRST = 0x04)   Make a power management reset to ensure device goes to sleep 
        PHY_CS = 0; SPI2_PUT(0x45); SPI2_PUT(0x80); PHY_CS = 1;  // (WRITE_TXBCNINTL = 0x80) Write the registers required to place the device in sleep
        PHY_CS = 0; SPI2_PUT(0x1B); SPI2_PUT(0x60); PHY_CS = 1;  // (WRITE_RXFLUSH = 0x60)   ...
        PHY_CS = 0; SPI2_PUT(0x6B); SPI2_PUT(0x80); PHY_CS = 1;  // (WRITE_SLPACK = 0x80)    ...

        // Place the accelerometer into standby (this assumes ACCEL_USE_SPI==1)
        ACCEL_INIT_PINS();                              // Ensure pins are correct
        ACCEL_INT1_IE = 0;                              // Disable accelerometer interrupts 
        SSP1STAT &= 0x3F;                               // SPI1 - Sample at middle, transmit on clock idle to active
        SSP1CON1 = 0x31;                                // SPI1 - Config (enabled, clock idle high, master mode Fosc/4)
        ACCEL_CS = 0;                                   // (active low)
        Nop(); Nop(); Nop(); Nop(); Nop(); Nop(); Nop();// Delay >3.4us (6.8 Tcy @ 8 MHz)
	SPI1_PUT(0x2D); SPI1_PUT(0x00);                 // (ADDR_POWER_CTL = 0x00) b3=0: measurement making off (~0.1uA)
	ACCEL_CS = 1;                                   // (active low)
        SSP1CON1.SSPEN = 0;                             // Close SPI1 (disable synchronous serial port)


// LATER: Re-enable RF interrupts
        RFIE = 1;

