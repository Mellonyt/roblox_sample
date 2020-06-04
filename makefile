SHELL=/bin/sh

# OS_TYPE is used to set the os flags
ifndef os
   os      = linux
endif

CORE_DIR =.
IFLAGS	= -I. 

############################## LINUX FLAGS ###################################
ifeq ($(os),linux)
OS_DEFINES      = -DUNIX -DLINUX -D_REENTRANT -DUSE_STL 
OS_FLAGS        = -g -Wall -std=c++14

EXCEPTIONS	= 

LIBDIRS		= -L. 
LIBS		= -lpthread 
OS_DEFINED=true
endif

############################# Handle Errors ##############################
ifndef OS_DEFINED
	ERROR="Error: Undefined os: <$(os)>."
	all: error
endif

# implicit definitions
#---------------------
.SUFFIXES: .cpp .c

.c.o:
	@echo "Compiling " $<
	@$(cc) -c $(CFLAGS) $<

.cpp.o:
	@echo "Compiling " $<
	@$(CC) -c $(CFLAGS) $<

# For CC 
cc		= gcc
CC		= g++

CFLAGS		= $(IFLAGS) $(OS_FLAGS) $(OS_DEFINES) 
LDFLAGS		= $(LIBDIRS) $(LIBS) 

TARGETS		= rbclient rbserver

all	: $(TARGETS)

rbclient: 
	cd client && $(MAKE)

rbserver: 
	cd server && $(MAKE)

clean	:
	cd client && $(MAKE) clean
	cd server && $(MAKE) clean

error:
	@echo $(ERROR)
	@exit 1
