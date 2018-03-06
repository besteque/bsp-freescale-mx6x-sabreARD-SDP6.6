#!/bin/sh
# script to build a binary IPL and boot image for i.MX6x Sabre-ARD board. 
# NOTE the image (ipl-ifs-mx6x-sabreARD.bin) must be built as binary, i.e. [virtual=armle,binary] in the buildfile 
set -v

#	Convert IPL into BINARY format
${QNX_HOST}/usr/bin/ntoarmv7-objcopy --input-format=elf32-littlearm --output-format=binary ../install/armle-v7/boot/sys/ipl-mx6x-sabreARD ./tmp-ipl-mx6x-sabreARD.bin

#	Pad BINARY IPL
mkrec  -ffull -r tmp-ipl-mx6x-sabreARD.bin > ipl-mx6x-sabreARD.bin

rm tmp-ipl-mx6x-sabreARD.bin
