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

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CBCentralManager *centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
    self.centralManager = centralManager;
}

- (void)didReceiveMemoryWarning {
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
    NSString *localName = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
    if ([localName length] > 0)
    {
        // Need to either check the name is WAX or use service matching
        NSLog(@"Found a device - Wax 9?: %@", localName);
        if( [localName isEqualToString:@"WAX9-A962"] )
        {
            [self.centralManager stopScan];
            self.wax9Peripheral = peripheral;
            peripheral.delegate = self;
            [self.centralManager connectPeripheral:peripheral options:nil];
        }
    }
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
        NSLog( @"%+d %+d %+d | %+d %+d %+d | %+d %+d %+d", ax, ay, az, gx, gy, gz, mx, my, mz );
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic error:(NSError *) error
{
    //NSLog( @"didUpdateNotificationStateForCharacteristic from %@", characteristic.UUID );
    if ( characteristic.isNotifying )
    {
        [peripheral readValueForCharacteristic:characteristic];
    }
}

@end
