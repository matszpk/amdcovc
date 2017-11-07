###
# Makefile
# Mateusz Szpakowski
###

ADLSDKDIR = ./dependencies/ADL_SDK_V10.2
CXX = g++
CXXFLAGS = -Wall -O3 -std=c++11 -Iincludes
LDFLAGS = -Wall -O3 -std=c++11
SRC_DIR = ./source
OBJ_DIR = ./obj
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))
INCDIRS = -I$(ADLSDKDIR)/include
LIBDIRS =
LIBS = -ldl -lpci -lm -lOpenCL -pthread

.PHONY: all clean

all: amdcovc

amdcovc: $(OBJ_FILES)
	$(CXX) $(LDFLAGS) $(LIBDIRS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f ./obj/*.o ./obj/*.d amdcovc

CXXFLAGS += -MMD
-include $(OBJ_FILES:.o=.d)
