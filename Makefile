###
# Makefile
# Mateusz Szpakowski
###

ADLSDKDIR = ./dependencies/ADL_SDK_V10.2
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
LDFLAGS = -Wall -O3 -std=c++11
INCDIRS = -I$(ADLSDKDIR)/include
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
