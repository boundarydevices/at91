#!/bin/sh
if [ "at91sam7x" = "$PHYSDEVDRIVER" ] ; then
   productFile=/tmp/at91.product ;
   if [ "xadd" = "x$ACTION" ] ; then
      product=`cat /sys$PHYSDEVPATH/../product` ;
      snstring=`cat /sys$PHYSDEVPATH/../serial` ;
      # save last product added
      cp /sys$PHYSDEVPATH/../product $productFile ;
      case "$product" in
         ( *Flash ) 
            echo "Flash programmer application"  > /dev/ttyS0 ;
            insmod /mmc/at91sam7x_flash.ko
            devnum=`grep at91_flash /proc/devices | sed "s/\([0-9]*\).*/\\1/"` ;
            mknod /dev/atmel_flash c $devnum 0 ;
            sleep 1 ;
            /mmc/at91run 0x00100000 /mmc/usbPeriph.rom.bin ;
            ;;
         
         ( *Periph ) 
            echo "USB Peripheral application"  > /dev/ttyS0 ;
            insmod /mmc/at91sam7x_fram.ko ;
            insmod /mmc/at91sam7x_serial.ko ;
            devnum=`grep at91_fram /proc/devices | sed "s/\([0-9]*\).*/\\1/"` ;
            mknod /dev/fram c $devnum 0 ;
            devnum=`grep at91_serial /proc/devices | sed "s/\([0-9]*\).*/\\1/"` ;
            mknod /dev/ttyAT0 c $devnum 0 ;
            mknod /dev/ttyAT1 c $devnum 1 ;
            /mmc/at91run 0x00100000 /mmc/usbPeriph.rom.bin ;
            insmod /mmc/at91sam7x_ether.ko ;
            ifconfig ateth0 up ;
            udhcpc -f -q -n -s /mmc/dhcp -i ateth0 ;
            ;; 

         ( * ) 
            echo "SAM-BA application" > /dev/ttyS0 ;
            sleep 1 ;
            /mmc/at91run 0x202000 /mmc/usbFlash.ram.bin ;
            ;;
      esac
   else
      # what was last product added?
      product=`cat $productFile` ;
      case "$product" in
         ( *Periph ) 
            echo "Shutdown USB Peripheral application"  > /dev/ttyS0 ;
            rm -f /dev/fram ;
            rm -f /dev/ttyAT0 ;
            rm -f /dev/ttyAT1 ;
            ifconfig ateth0 down ;
            rmmod at91sam7x_fram ;
            rmmod at91sam7x_serial ;
            rmmod at91sam7x_ether ;
            ;;
         ( *Flash ) 
            echo "Shutdown USB Flash application"  > /dev/ttyS0 ;
            rm -f /dev/atmel_flash ;
            rmmod at91sam7x_flash ;
            ;;
      esac
      echo "remove $product" > /dev/ttyS0 ;
      rm -f $productFile ;
   fi
   /mmc/udpSendto 127.0.0.1 0xa0a0 "$ACTION.$product" ;
fi

