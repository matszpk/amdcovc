###
# Makefile
# Mateusz Szpakowski
###

# 1 - if you need AMD Catalyst support, 0 - if you won't
HAVE_ADLSDK = 0
HAVE_TERMINFO = 1
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
LDFLAGS = -Wall -O3 -std=c++11

ifeq ($(HAVE_TERMINFO),1)
TINFOLIB = -lncurses
TINFODEFINES = -DHAVE_TERMINFO=1
else
TINFOLIB =
TINFODEFINES =
endif

ifeq ($(HAVE_ADLSDK),1)
ADLSDKDIR = /home/mat/docs/src/ADL_SDK9
INCDIRS = -I$(ADLSDKDIR)/include
DEFINES = -DHAVE_ADLSDK=1 $(TINFODEFINES)
# for AMDGPU-PRO SLES/OpenSUSE
LIBDIRS = -L/opt/amdgpu-pro/lib64
# for AMDGPU-PRO Ubuntu
#LIBDIRS = -L/opt/amdgpu-pro/lib/x86_64-linux-gnu
#LIBDIRS =
LIBS = -ldl -lpci -lm $(TINFOLIB) -lOpenCL -pthread
else
LIBS = -ldl -lpci -lm $(TINFOLIB) -pthread
LIBDIRS = 
INCDIRS = 
DEFINES = $(TINFODEFINES)
endif

.PHONY: all clean

all: amdcovc

amdcovc: amdcovc.o
	$(CXX) $(LDFLAGS) $(LIBDIRS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) $(DEFINES) -c -o $@ $<

clean:
	rm -f *.o amdcovc
