SHELL=/bin/sh

# OS_TYPE is used to set the os flags
ifndef os
   os      = linux
endif

CORE_DIR =.
IFLAGS	= -I. -I..

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

SRC		= rbserver.cpp args.cpp utils.cpp rbmessage.cpp server.cpp
OBJS	= $(SRC:.cpp=.o) 

TARGETS		= rbserver 

all	: $(TARGETS)

rbserver: $(OBJS)
	@echo "Building " $@
	$(CC) -o $@ $(OBJS) $(LIBDIRS) $(LIBS)
	@echo "build complete - " $@

clean	:
	rm -f *.o *~
	rm -f $(TARGETS)
	rm -f *.log

error:
	@echo $(ERROR)
	@exit 1

# DO NOT DELETE THIS LINE -- make depend depends on it.