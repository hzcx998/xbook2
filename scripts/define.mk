MKDIR		:=	mkdir -p
RMDIR		:=	rmdir -p
CP			:=	cp -af
RM			:=	rm -rf
CD			:=	cd
MV			:=	mv
FIND		:=	find

# System environment variable.
ifeq ($(OS),Windows_NT)
	HOSTOS		:= windows
else
	ifeq ($(shell uname),Darwin)
		HOSTOS		:= macos
	else
		ifeq ($(shell uname),Linux)
			HOSTOS		:= linux
		else
			HOSTOS		:= unix-like
		endif
	endif
endif

ifeq ($(HOSTOS),windows)
	SUFFIX	:= .exe
	SHARED_SUFFIX	:= dll
else
	SUFFIX	:=
	SHARED_SUFFIX	:= so
endif

ifeq ($(HOSTOS),linux)
ECHO		:= /bin/echo -e
else
ECHO		:= echo
endif

export RM CP CD MV FIND MKDIR HOSTOS SUFFIX SHARED_SUFFIX ECHO

ifneq ($(HOSTOS),linux)
ECHO_RM		:=RM
ECHO_CC		:=CC
ECHO_CXX	:=CXX
ECHO_AS		:=AS
ECHO_LD		:=LD
ECHO_AR		:=AR
ECHO_HOSTCC	:=HOSTCC
ECHO_OUTPUT	:=OUTPUT
else
ECHO_RM		:=\e[32mRM\e[0m
ECHO_CC		:=\e[32mCC\e[0m
ECHO_CXX	:=\e[32mCXX\e[0m
ECHO_AS		:=\e[32mAS\e[0m
ECHO_LD		:=\e[32mLD\e[0m
ECHO_AR		:=\e[35mAR\e[0m
ECHO_HOSTCC	:=\e[33mHOSTCC\e[0m
ECHO_OUTPUT	:=\e[35mOUTPUT\e[0m
endif

export ECHO_RM ECHO_CC ECHO_CXX ECHO_AS ECHO_LD ECHO_AR ECHO_OUTPUT ECHO_HOSTCC

CROSS_COMPILE	?=

# Make variables (CC, etc...)
AS			:=	$(CROSS_COMPILE)gcc -x assembler-with-cpp
CC			:=	$(CROSS_COMPILE)gcc
CPP			:=	$(CROSS_COMPILE)gcc -E
CXX			:=	$(CROSS_COMPILE)g++
LD			:=	$(CROSS_COMPILE)ld
AR			:=	$(CROSS_COMPILE)ar
OC			:=	$(CROSS_COMPILE)objcopy
OD			:=	$(CROSS_COMPILE)objdump
NM			:=	$(CROSS_COMPILE)nm

HOSTCC		:=	gcc

export AS AR CC LD CPP CXX OC OD NM HOSTCC