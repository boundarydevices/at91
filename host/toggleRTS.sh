#!/bin/sh
if [ "1" = "$1" ]; then
   mask=0x00080000 ;
else
   mask=0x00040000 ;
fi
tmp=`at91reg /dev/at91sam7x0 0xfffc0000 $mask` ;
tmp=`at91reg /dev/at91sam7x0 0xfffc4000 $mask` ;
x=`at91reg /dev/at91sam7x0 0xfffc0014` ;
y=`at91reg /dev/at91sam7x0 0xfffc4014` ;
echo $x/$y

