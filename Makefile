###
# Makefile
# Mateusz Szpakowski
###

ADLSDKDIR = ./dependencies/ADL_SDK_V10.2
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11
LDFLAGS = -Wall -O3 -std=c++11
SRC_DIR = ./
OBJ_DIR = ./
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
INCDIRS = -I$(ADLSDKDIR)/include -I ./
LIBDIRS =
LIBS = -ldl -lpci -lm -lOpenCL -pthread

.PHONY: all clean

all: amdcovc

amdcovc: amdcovc.o error.o
	$(CXX) $(LDFLAGS) $(LIBDIRS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.d amdcovc

CXXFLAGS += -MMD
-include $(OBJ_FILES:.o=.d)
