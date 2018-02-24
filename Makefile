CONTIKI = ../..

all: base_station sender

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
