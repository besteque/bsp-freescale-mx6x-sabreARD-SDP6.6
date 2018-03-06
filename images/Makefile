ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

HOST_MKIFS := mkifs

SUFFIXES := .build .ifs .raw

.PHONY: all clean

all: ipl-mx6x-sabreARD.bin ifs-mx6x-sabreARD.raw

clean: 
	$(RM_HOST) ifs-mx6x-sabreARD.raw ipl-mx6x-sabreARD.bin 

ifs-mx6x-sabreARD.raw: sabreARD.build
	$(HOST_MKIFS) -vvv -r../install $(MKIFSFLAGS) $^ $@
	
ipl-mx6x-sabreARD.bin:
	./mkflashimage
	