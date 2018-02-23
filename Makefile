CONTIKI = ../..

all: base_station sender

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
