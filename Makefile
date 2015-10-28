#********************************* MAKEFILE ***********************************
#**                            *******************                           **
#**                                                                          **
#** Project : 10086 (common lib)                                             **
#** Filename : Makefile                                                      **
#** Version : 1.0                                                            **
#** Date : NOV 24, 2011                                                      **
#**                                                                          **
#******************************************************************************
#**                                                                          **
#** Copyright (c) 2011 GreenPoint Communications Inc.                        **
#** All rights reserved.                                                     **
#**                                                                          **
#******************************************************************************
#Version History:
#----------------
#Version : 1.0
#Date : NOV 24, 2011   
#Revised By : kernel.qu
#Description : This is the makefile template .the HEAD


export BUILD_TYPE=sharelib staticlib #target

HEADDIR := $(shell pwd)
#LIBDIR	:= ../../../lib
LIBDIR	:= ./lib
CC := $(CROSS_COMPILE)gcc
CXX:= $(CROSS_COMPILE)g++
STRIP := $(CROSS_COMPILE)strip
AR :=$(CROSS_COMPILE)ar

SYSTEM_PLATFORM := -DLINUX -D_OS_LINUX
SUPPORT_BISINESS_TYPE :=  
APP_MACRO :=  $(SYSTEM_PLATFORM) $(SUPPORT_BISINESS_TYPE)
INCLUDE_TOP_DIR := ..
INCLUDE_FALAG := -I$(HEADDIR)/              
								 
CFLAG := $(APP_MACRO)  $(INCLUDE_FALAG) -g -Wall
CFLAGS := $(CFLAG) -Wstrict-prototypes -Wimplicit-function-declaration -fPIC
CXXFLAGS := $(CFLAGS) 
LDFLAGS  := -L$(HEADDIR)        
LIBS := -lpthread -ldl 
#STATICLIBS :=  -los
#OBJS_DIR := ../../../build/linux/superbus/thread_pool/obj
#DEPS_DIR :=../../../build/linux/superbus/thread_pool/deps

OBJS_DIR := ./build/obj
DEPS_DIR := ./build/deps


SUBDIRS = 


DEPS_FILTER := sed  '$$s,$$, Makefile,g'

FORMAT   := c cpp cxx
TARGET   := thread_pool

IGNORE_SOURCE := main.c main.o

#
#export the vars for the makefile in the sub dir
#
export CROSS_COMPILE CFLAGS CXXFLAGS LDFLAGS 


FORMAT_MODE = $(foreach format,$(FORMAT),%.$(format) )
LOCAL_SOURCE :=  $(filter-out $(IGNORE_SOURCE),$(foreach format,$(FORMAT),$(wildcard *.$(format)))) #method 2


#SOURCE_FILES := $(foreach dir, $(SUBDIRS),$(foreach format,$(FORMAT),$(wildcard $(dir)/*.$(format))) )
SOURCE_FILES += $(LOCAL_SOURCE)
SOURCE_FILES := $(filter-out $(IGNORE_SOURCE), $(SOURCE_FILES) )


OBJ_FILES := $(foreach format,$(FORMAT_MODE),$(filter %.o,$(patsubst $(format),$(OBJS_DIR)/%.o,$(SOURCE_FILES))) ) 

LOCAL_DEPS := $(foreach format,$(FORMAT_MODE),$(filter %.d,$(patsubst $(format),$(DEPS_DIR)/%.d,$(LOCAL_SOURCE))) )
#
#*************we define  how to create the dependence file********************************************
#
$(DEPS_DIR)/%.d : %.c
	$(CC) $(CFLAGS)  -MM  $< > $@.tmp
	$(DEPS_FILTER)  < $@.tmp >$@
	rm $@.tmp


$(OBJS_DIR)/%.o : %.c   
	$(CC) $(CFLAGS) -o $@   -c $<  

#*****************************************************************************************************
all: sub $(BUILD_TYPE)
include $(LOCAL_DEPS)
sub:
	for x in $(SUBDIRS); do\
              (cd $$x && $(MAKE) ); \
	done

release: target sharelib staticlib

target:$(TARGET) 
sharelib:lib$(TARGET).so
staticlib:lib$(TARGET).a

$(TARGET):$(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^  $(LDFLAGS)  $(LIBS)  $(STATICLIBS)
	#$(STRIP) $@

#
#next is  compile mothod for  a .so/.a library,if you want do it :)
#
lib$(TARGET).so: $(OBJ_FILES)
	$(CXX) $(CFLAGS) $(LDFLAGS) -shared -o $@ -Wl,-soname,$@ -lc $^ $(STATICLIBS)
	cp lib$(TARGET).so $(LIBDIR)/ -f 

#
#next is compile mothod for a .a library
#
lib$(TARGET).a:$(OBJ_FILES)
	$(AR) -rf $@ $^
	mv lib$(TARGET).a $(LIBDIR)/ -f
	mv lib$(TARGET).so $(LIBDIR)/ -f
install:
	
clean:
	for x in $(SUBDIRS); do (cd $$x && $(MAKE) clean ); done
	-rm -f  $(OBJ_FILES)
	-rm -f  $(HEADDIR)/lib$(TARGET).a 
	-rm -f  $(LIBDIR)/lib$(TARGET).a  
	-rm -f  $(HEADDIR)/lib$(TARGET).so 
	-rm -f  $(LIBDIR)/lib$(TARGET).so
	-rm -f  $(TARGET)  
	-rm -f  $(DEPS_DIR)/*.d

