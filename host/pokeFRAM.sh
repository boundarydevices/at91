#!/bin/sh
while [ 1 ] ; do
   l=`readLong -d /dev/fram 0` ;
   if [ $? -ne 0 ]; then echo "readLong failed" ; break ; fi ;
   let "l += 1" ;
   echo "set l to $l" ;
   old=`writeLong -d /dev/fram 0 $l` ;
   if [ $? -ne 0 ]; then echo "writeLong failed" ; break ; fi ;
   sleep 1 ;
done

