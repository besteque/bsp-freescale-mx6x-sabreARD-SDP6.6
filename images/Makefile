ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

SRC := $(CURDIR)/../src
BOARD_ROOT := $(SRC)/hardware/startup/boards
SAMPLEDIR := $(SRC)/hardware/support/appsmedia-sample

HOST_MKIFS := mkifs

SUFFIXES := .build .ifs .raw

.PHONY: all clean

all: ipl-mx6x-sabreARD.bin ifs-mx6x-sabreARD.raw

clean: 
	$(RM_HOST) ifs-mx6x-sabreARD.raw ipl-mx6x-sabreARD.bin 
	$(RM_HOST) appsmedia-sample-mx6x-sabreARD.build

ifs-mx6x-sabreARD.raw: sabreARD.build
	$(HOST_MKIFS) -vvv -r../install $(MKIFSFLAGS) $^ $@
	
appsmedia-sample: ifs-appsmedia-sample-mx6x-sabreARD.bin

appsmedia-sample-mx6x-sabreARD.build:
	cat $(BOARD_ROOT)/imx6x/sabreARD/build $(SAMPLEDIR)/appsmedia-sample-imx6.build $(SAMPLEDIR)/appsmedia-sample-common.build > $(CURDIR)/$@

ifs-appsmedia-sample-mx6x-sabreARD.bin: appsmedia-sample-mx6x-sabreARD.build
	$(HOST_MKIFS) -r../install -v $(MKIFSFLAGS) $^ $@
	
ipl-mx6x-sabreARD.bin:
	./mkflashimage
	