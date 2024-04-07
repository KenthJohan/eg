# demo_mks1




## slcan

### Check for CAN device permission
```
ls -l /dev/ttyACM*
```

### Change permission
```
sudo chmod a+rw /dev/ttyACM0
```

### can-utils
```bash
sudo slcand -o -c -s0 /dev/ttyACM0 can0
sudo ifconfig can0 up
sudo ifconfig can0 txqueuelen 1000
cansend can0 999#DEADBEEF   # Send a frame to 0x999 with payload 0xdeadbeef
candump can0                # Show all traffic received by can0
canbusload can0 500000      # Calculate bus loading percentage on can0 
cansniffer can0             # Display top-style view of can traffic
cangen can0 -D 11223344DEADBEEF -L 8    # Generate fixed-data CAN messages
```














## candlelight
https://canable.io/getting-started.html#cantactapp-windows

```bash
sudo code /etc/udev/rules.d/99-candlelight.rules
```

#### 99-candlelight.rules
```
SUBSYSTEM=="net", ATTRS{idVendor}=="1d50", ATTRS{idProduct}=="606f", ATTRS{serial}=="0047005F4D4D501320383834", NAME="can5"
```

### Automatically configure can bus on 
https://github.com/woj76/gs_usb_leonardo/blob/master/80-can.network

Put this file in: `/etc/systemd/network/` and then run `$ sudo systemctl enable systemd-networkd` to enable `systemd-networkd`
Now start `systemd-networkd`: `sudo systemctl start systemd-networkd`
Credit: https://github.com/linux-can/can-utils/issues/68#issuecomment-584505426
#### /etc/systemd/network/80-can.network
```
# For Ubuntu 20.04

[Match]
Name=can*

[CAN]
BitRate=500000

# Put this file in: "/etc/systemd/network/" and then run "sudo systemctl enable systemd-networkd" to enable systemd-networkd
# Now start systemd-networkd: "sudo systemctl start systemd-networkd"
# Credit: https://github.com/linux-can/can-utils/issues/68#issuecomment-584505426
```



### Enable can interface
```bash
sudo ip link set can5 up type can bitrate 500000
```



## Code
https://github.com/linux-can/can-utils/blob/master/candump.c


## References
* https://www.youtube.com/watch?v=GdnKQclBvP4
* https://github.com/makerbase-motor/MKS-SERVO57D/blob/master/Example%20Code/Arduino%20Example%20Code/arduino_11_Serial_Speed%20Mode/arduino_11_speedMode/arduino_11_speedMode.ino
* https://www.kernel.org/doc/html/next/networking/can.html