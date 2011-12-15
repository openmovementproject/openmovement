import usb
import sys
import time

dev = None

while dev == None:
    # find our device
    dev = usb.core.find(idVendor=0x04d8, idProduct=0x0057)
    time.sleep(1)

print "Found device"

for cfg in dev:
    sys.stdout.write(str(cfg.bConfigurationValue) + '\n')
    for intf in cfg:
        sys.stdout.write('\t' + \
                         str(intf.bInterfaceNumber) + \
                         ',' + \
                         str(intf.bAlternateSetting) + \
                         '\n')
        for ep in intf:
            sys.stdout.write('\t\t' + \
                             str(ep.bEndpointAddress) + \
                             '\n')