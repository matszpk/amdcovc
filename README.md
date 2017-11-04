## AMDCOVC

This program is for controlling AMD Overdrive settings for graphics cards in Linux console-mode. 
This program has been written to replace the `amdconfig` utility and allow to overclocking of AMD
Radeon GPUs when not running the X11 server. 

The `amdconfig` utility requires a running X11 server and X11 configuration to be working correctly.
Thus, this program eases the overclocking of AMD Radeon GPUs in the console environment (in SSH or 
the Linux console).
However, this program also works under the X11 environment.

### WARNING

This program changes graphics card clock and voltage values.
This program can easily damage your hardware if used without care.

Before setting any of the AMD Overdrive parameters (clocks, voltages, fan speeds), please make sure 
any GPU computations and rendering processes have been stopped.

It goes without saying, but use this program at your own risk. There is no warranty expressly given
or implied in the use of this program. Please read the license before using this program.

### Preliminary requirements

Program to work requires following things:

* C++ environment compliant with C++11 standard (libraries)
* OpenCL environment (to force initializing of devices)
* libadlxx.so library (AMD ADL library)
* pciutils library (libpci).

## Building:

To build program you need:

* A compiler compliant with the C++11 standard
* The OpenCL headers
* The AMD ADL SDK (in developer.amd.com site)
* The pciutils developer package (includes)

## Installing prerequisites (Ubuntu)

apt-get install g++
apt-get install ocl-icd-opencl-dev
apt-get install opencl
apt-get install libpci-dev
install the adl sdk from here: http://developer.amd.com/display-library-adl-sdk/
clone this repo into the base path of the adl-sdk

### Building program

To build program, just type:

```
make
```

### Invoking program

NOTE: If no X11 server is running, this program requires root privileges.

To run the program and get the current GPU settings, type:

```
./amdcovc
```

To print more information about the current GPU adapters, type:

```
./amdcovc -v
```

To print help, type:

```
./amdcovc -?
```

To set the core clock and memory clock for first adapter (0) to 1000MHz with a 
memory clock of 1200MHz, type:

```
./amdcovc coreclk:0=1000 memclk:0=1200
```

### Understanding info printed by program

The AMDCOVC by default prints following information about a graphics card:

```
Adapter 1: Pitcairn PRO [Radeon HD 7850 / R7 265 / R9 270 1024SP]
  Core: 1010 MHz, Mem: 1235 MHz, Vddc: 1.21 V, Load: 98%, Temp: 67 C, Fan: 54%
  Max Ranges: Core: 150 - 1050 MHz, Mem: 75 - 1450 MHz, Vddc: 0.8 - 1.225 V
  PerfLevels: Core: 300 - 1010 MHz, Mem: 150 - 1235 MHz, Vddc: 0.825 - 1.21 V
```

The first line below the adapter is the current state of the graphics card (core clock,
memory clock, voltage, load, temperature and fan speed).

The `PerfLevels` are the current performance level settings from lowest to highest.
The highest performance levels are used when there are computations/renderings occurring.
The first level will be used in idle mode (when there is no work).

The `Max Ranges` are minimal and maximal possible settings for graphics card.

The verbose information contains:

* current state of the graphics adapter (Current CoreClock, MemoreClock, etc. )
* minimum and maximum fan speed settings
* minimum and maximum clock and voltage settings
* current performance level settings
* default performance level settings

### List of parameters

List of parameters that can be set:

* coreclk[:[ADAPTERS][:LEVEL]]=CLOCK - set core clock in MHz
* memclk[:[ADAPTERS][:LEVEL]]=CLOCK - set memory clock in MHz
* coreod[:[ADAPTERS][:LEVEL]]=PERCENT - set core clock in percents (AMDGPU)
* memod[:[ADAPTERS][:LEVEL]]=PERCENT - set memory clock in perecents (AMDGPU)
* vcore[:[ADAPTERS][:LEVEL]]=VOLTAGE - set Vddc voltage in Volts
* icoreclk[:ADAPTERS]=CLOCK - set core clock in MHz for idle level
* imemclk[:ADAPTERS]=CLOCK - set memory clock in MHz for idle level
* ivcore[:ADAPTERS]=VOLTAGE - set Vddc voltage  in Volts for idle level
* fanspeed[:[ADAPTERS][:THID]]=PERCENT -  set fanspeed in percents

Extra specifiers in parameters:

* ADAPTERS - adapter (device) index list (default is 0)
* LEVEL - performance level (typically 0 or 1, default is last)
* THID - thermal controller index (must be 0)

You can use the 'default' value in plce of a value to set the default value.
For fanspeed the 'default' value forces an automatic speed setting.

To overclock a graphics card on an AMD GPU(-PRO) driver, use the `coreod` and `memod`
parameters. 

Parameters `coreclk` and `memclk` are available in AMD GPU(-PRO) driver mode.

### List of options

List of options:

* -a, --adapters=LIST - print information all adapters present.
* -v, --verbose - print verbose information about current adapters.
* --version - print version of this application.
* -?, --help - print the help options.

