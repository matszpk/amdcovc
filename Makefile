###
# Makefile
# Mateusz Szpakowski
###

ADLSDKDIR = /home/mat/docs/src/ADL_SDK9
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
LDFLAGS = -Wall -O3 -std=c++11
INCDIRS = -I$(ADLSDKDIR)/include
# for AMDGPU-PRO SLES/OpenSUSE
#LIBDIRS = -L/opt/amdgpu-pro/lib64
# for AMDGPU-PRO Ubuntu
#LIBDIRS = -L/opt/amdgpu-pro/lib/x86_64-linux-gnu
LIBDIRS =
LIBS = -ldl -lpci -lm -lOpenCL -pthread

.PHONY: all clean

all: amdcovc

amdcovc: amdcovc.o
	$(CXX) $(LDFLAGS) $(LIBDIRS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c -o $@ $<

clean:
	rm -f *.o amdcovc
