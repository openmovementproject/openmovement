/*
 * Copyright (c) 2013-2014, Newcastle University, UK.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
// ViewController.m
// Gavin Wood, 2014

// http://stackoverflow.com/questions/19928623/read-data-from-ble-device

#import "ViewController.h"

@interface ViewController ()
@end

@implementation ViewController

#define WAX9_UUID            @"00000000-0008-A8BA-E311-F48C90364D99"
#define WAX9_UUID_COMMAND    @"00000001-0008-A8BA-E311-F48C90364D99"
#define WAX9_UUID_DATAOUTPUT @"00000002-0008-A8BA-E311-F48C90364D99"

const int MaxDevicesToPair = 2;
NSString *devices[MaxDevicesToPair] = { @"WAX9-DF96", @"WAX9-DE0B" };
//NSString *devices[MaxDevicesToPair] = { @"WAX9-DF96", @"" };
//NSString *devices[MaxDevicesToPair] = { @"WAX9-DE0B", @"" };

NSMutableDictionary *peripheralDictionary;
NSMutableDictionary *indexDictionary;
float mag[MaxDevicesToPair][3];
float acc[MaxDevicesToPair][3];
float rot[MaxDevicesToPair][3];

int bytesReceived = 0;
bool isRecording = false;

- (void)viewDidLoad {
    [super viewDidLoad];
    
    //private/var/folders/vg/2hltpvqj4_932fmp3mb_myzc0000gp/T/temp.zKEJCc/WAX9WAX9-DF96.txt/ Setup
    _status.text = @"Waiting on name";
    _gyroscope.text = @"1234";
    _magnetometer.text = @"5678";
    _accelerometer.text = @"91011";
    _device.text = [devices[0] stringByAppendingString:devices[1]];
    peripheralDictionary = [[NSMutableDictionary alloc] init];
    indexDictionary = [[NSMutableDictionary alloc] init];

    // Start scanning for the devices after the GUI has loaded
    [self startScanningForDevice];
}

-(void)startScanningForDevice
{
    // Stop the current version
    self.centralManager = NULL;
    isRecording = false;
    
    // Start the manager again
    CBCentralManager *centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
    self.centralManager = centralManager;
    
    NSLog( @"Started scanning for devices" );
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    return NO;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - CBCentralManagerDelegate

// method called whenever you have successfully connected to the BLE peripheral
- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
    NSLog(@"didConnectPeripheral=");
    
    // Set the delegate to itself
    peripheral.delegate = self;
    
    // Discover the services
    [peripheral discoverServices:nil];
}

// method called whenever the device state changes.
- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    NSLog(@"centralManagerDidUpdateState=");
    
    // Determine the state of the peripheral
    if ([central state] == CBCentralManagerStatePoweredOff)
    {
        NSLog(@"CoreBluetooth BLE hardware is powered off");
    }
    else if ([central state] == CBCentralManagerStatePoweredOn)
    {
        NSLog(@"CoreBluetooth BLE hardware is powered on and ready");
        
        NSLog(@"scanForPeripheralsWithServices");
        
        NSArray *services = @[[CBUUID UUIDWithString:WAX9_UUID_DATAOUTPUT]];
        (void)services;
        
        //[self.centralManager scanForPeripheralsWithServices:services options:nil];
        [self.centralManager scanForPeripheralsWithServices:nil options:nil];
    }
    else if ([central state] == CBCentralManagerStateUnauthorized)
    {
        NSLog(@"CoreBluetooth BLE state is unauthorized");
    }
    else if ([central state] == CBCentralManagerStateUnknown)
    {
        NSLog(@"CoreBluetooth BLE state is unknown");
    }
    else if ([central state] == CBCentralManagerStateUnsupported)
    {
        NSLog(@"CoreBluetooth BLE hardware is unsupported on this platform");
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI
{
    //NSString *localName = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
    NSString *localName = [peripheral name];
    if ([localName length] > 0)
    {
        // Need to either check the name is WAX or use service matching
        
        if( [localName hasPrefix:@"WAX9"] )
        {
            NSLog(@"Found WAX9(%@)", localName);
            
            bool paired = false;
            
            for( int i=0; i<MaxDevicesToPair; i++ )
            {
                NSString *device = devices[i];
                
                if( [localName isEqualToString:device ] )
                {
                     NSLog( @"Pairing to device %@", device );
                    
                    // Keep track of this peripheral by its name
                    [peripheralDictionary setObject:peripheral forKey:localName];
                    
                    // Make a record of our index for this peripheral
                    NSNumber* myAutoreleasedNumber = nil;
                    myAutoreleasedNumber = [NSNumber numberWithInt:i];
                    [indexDictionary setObject:myAutoreleasedNumber forKey:localName];
                    
                    // Connect to this peripheral
                    peripheral.delegate = self;
                    [self.centralManager connectPeripheral:peripheral options:nil];
                    _status.text = @"Found";
                    
                    // Flag that we paired against this device
                    paired= true;
                }
            }
            if( !paired )
            {
                NSLog( @"Did not pair to WAX9(%@)", localName );
            }
            
            if( [indexDictionary count] == 2 )
            {
                [self.centralManager stopScan];
            }
        }
    }
}
-(IBAction) startRecording: (id) sender
{
    isRecording = true;
    bytesReceived = 0;
    
    _status.text = @"Recording";
}

-(IBAction) endRecording: (id) sender
{
    isRecording = false;
    
    _status.text = @"Not recording";
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error
{
    NSLog(@"didDiscoverServices");

    for (CBService *service in peripheral.services)
    {
        NSLog(@"Discovered service: %@", service.UUID);
        
        NSLog(@"peripheral discoverCharacteristics for %@", service.UUID);
        [peripheral discoverCharacteristics:nil forService:service];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error
{
    //int count = (int)[service.characteristics count];
    //NSLog( @"Number of service characteristics = %d", count );
    
    // https://github.com/timburks/iOSHeartRateMonitor/blob/master/HRM/HeartRateViewController.m
    if ([service.UUID isEqual:[CBUUID UUIDWithString:WAX9_UUID]])
    {
        // We must set the notify byte before the write command byte
        for (CBCharacteristic *aChar in service.characteristics)
        {
            if([aChar.UUID isEqual:[CBUUID UUIDWithString:WAX9_UUID_DATAOUTPUT]])
            {
                [peripheral setNotifyValue:true forCharacteristic:aChar];
            }
        }
        // Notify is ready to receive - now stream the data
        for (CBCharacteristic *aChar in service.characteristics)
        {
            if([aChar.UUID isEqual:[CBUUID UUIDWithString:WAX9_UUID_COMMAND]])
            {
                NSData *data = [NSData dataWithBytes:(unsigned char[]){0x1} length:1];
                [peripheral writeValue:data forCharacteristic:aChar type:CBCharacteristicWriteWithoutResponse];
            }
        }
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *) error
{
    if ((characteristic.value) != nil)
    {
        // Read the data in
        NSData *data = [characteristic value];      // 1
        
        // Check the data length is 20. i
        int length = (int)[data length];
        (void)length;
        assert( length == 20 );
        
        // Get at the bytes
        const uint8_t *sensorData = [data bytes];
        
        short ax = ((short)sensorData[3] << 8) | sensorData[2];
        short ay = ((short)sensorData[5] << 8) | sensorData[4];
        short az = ((short)sensorData[7] << 8) | sensorData[6];
        
        short gx = ((short)sensorData[9] << 8) | sensorData[8];
        short gy = ((short)sensorData[11] << 8) | sensorData[10];
        short gz = ((short)sensorData[13] << 8) | sensorData[12];
        
        short mx = ((short)sensorData[15] << 8) | sensorData[14];
        short my = ((short)sensorData[17] << 8) | sensorData[16];
        short mz = ((short)sensorData[19] << 8) | sensorData[18];
       
      //  short batt = ((short)sensorData[21] << 8) | sensorData[20];
       
        float fax = (float) ax;
        float fay = (float) ay;
        float faz = (float) az;
        
        fax = fax / 4096.0f;
        fay = fay / 4096.0f;
        faz = faz / 4096.0f;
        
        float fgx = (float) gx;
        float fgy = (float) gy;
        float fgz = (float) gz;
        
        // Linear scale of mapping to degrees
        // 2000 degrees/2 0.07
        //  500 degrees/2 0.0175
        //  250 degrees/2  0.00875
        
        // Temperature you would divide by 10 from its signed integer
        
        // Change in height
        // Take a reading - p0
        //                - p
        //  delta h = k(ln(p/p0)) where k is typically -8434.6678
        
        fgx = fgx * 0.07f;
        fgy = fgy * 0.07f;
        fgz = fgz * 0.07f;
        
        float fmx = (float) mx;
        float fmy = (float) my;
        float fmz = (float) mz;
        
        // Cache the device name
        NSString *deviceName = [peripheral name];
        
        // Get our index for the device
        NSNumber *deviceIndexKeyPair = [indexDictionary objectForKey:deviceName];
        int deviceIndex = [deviceIndexKeyPair intValue];
        
        // Setup our global variables
        acc[deviceIndex][0] = fax; acc[deviceIndex][1] = fay; acc[deviceIndex][2] = faz;
        mag[deviceIndex][0] = fmx; mag[deviceIndex][1] = fmy; mag[deviceIndex][2] = fmz;
        rot[deviceIndex][0] = fgx; rot[deviceIndex][1] = fgy; rot[deviceIndex][2] = fgz;
        
        // Create a filename for this info
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsDirectory = [paths objectAtIndex:0];
        
        //make a file name to write the data to using the documents directory:
        NSString *fileName = [NSString stringWithFormat:@"%@/WAX9%@.txt", documentsDirectory, deviceName ];
        
        // Get the timestamp
        NSDate *myDate = [[NSDate alloc] init];
        NSDateFormatter *dateFormat = [[NSDateFormatter alloc] init];
        [dateFormat setDateFormat:@"yyyy-MM-dd HH:mm:ss.SSS"];
        NSString *formattedDate = [dateFormat stringFromDate:myDate];
        const char *strDate = [formattedDate cStringUsingEncoding:NSASCIIStringEncoding];
        if( isRecording )
        {
            const char *strFilename = [fileName cStringUsingEncoding:NSASCIIStringEncoding];
            FILE *f = fopen( strFilename, "a" );
            if( f )
            {
                fprintf(f, "%s,", strDate );
                fprintf(f, "%.5f,%.5f,%.5f,", fax, fay, faz );
                fprintf(f, "%.5f,%.5f,%.5f,", fgx, fgy, fgz );
                fprintf(f, "%.5f,%.5f,%.5f\n", fmx, fmy, fmz );
                //    fprintf(f, "%d\n", batt );
                fclose(f);
                
                bytesReceived += length;
            }
        }
        
        // Update the screen
        _status.text         = [NSString stringWithFormat:@"Bytes recorded %d", bytesReceived];
        _accelerometer.text  = [NSString stringWithFormat:@"%.2f %.2f %.2f\n%.2f %.2f %.2f",
                                            acc[0][0], acc[0][1], acc[0][2],
                                            acc[1][0], acc[1][1], acc[1][2]
                                ];
        
        _magnetometer.text  = [NSString stringWithFormat:@"%.2f %.2f %.2f\n%.2f %.2f %.2f",
                                mag[0][0], mag[0][1], mag[0][2],
                                mag[1][0], mag[1][1], mag[1][2]
                               ];
        
        _gyroscope.text  = [NSString stringWithFormat:@"%.2f %.2f %.2f\n%.2f %.2f %.2f",
                               rot[0][0], rot[0][1], rot[0][2],
                               rot[1][0], rot[1][1], rot[1][2]
                               ];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *) error
{
    //NSLog( @"didUpdateNotificationStateForCharacteristic from %@", characteristic.UUID );
    if ( characteristic.isNotifying )
    {
        // Code to check if our devices are notifying
        
        // Cache the device name
        NSString *deviceName = [peripheral name];
        
        // Get our index for the device
        NSNumber *deviceIndexKeyPair = [indexDictionary objectForKey:deviceName];
        int deviceIndex = [deviceIndexKeyPair intValue];
        (void)deviceIndex;
        
        //[peripheral readValueForCharacteristic:characteristic];
    }
}

@end
