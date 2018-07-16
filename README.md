## AMDCOVC

This is program to control AMD Overdrive settings that is working if graphics cards
are in console-mode. This program has been written to replace `amdconfig` utility
and allow to overclock Radeon GPU's if no running X11 server. An `amdconfig`
utility requires a running X11 server and X11 kept configuration to be working
correctly. Thus, this program ease overclocking of Radeon GPU's under console environment
(in SSH or Linux console). Nevertheless, this program can works under X11 environment.

### WARNING

This program changes graphics card's clocks and voltages. This program can easily
DAMAGE your hardware if it will be used CARELESSLY.

Before any setting of AMD Overdrive parameters (clocks, voltages, fan speeds),
please STOP ANY the GPU computations and the GPU renderings.

YOU USE THIS PROGRAM AT YOUR OWN RISK! WE DO NOT GIVE ANY WARRANTY THAT PROGRAM
WILL BE WORKING CORRECTLY IN ANY CASE! Read license before using this program.

### Preliminary requirements

Program to work requires following things:

* C++ environment compliant with C++11 standard (libraries)
* pciutils library (libpci).

Optional components that can be used by program for AMD Catalyst drivers:

* OpenCL environment (to force initializing of devices)
* libadlxx.so library (AMD ADL library)

To build program you need:

* compiler compliant with C++11 standard
* OpenCL headers (for AMD Catalyst drivers)
* AMD ADL SDK (in developer.amd.com site) (for AMD Catalyst drivers)
* pciutils developer package (includes)

### Building program

If you want to use amdovc with AMD Catalyst drivers, you should enable ADLSDK
by replacing line in Makefile:

```
HAVE_ADLSDK = 0
```

to 

```
HAVE_ADLSDK = 1
```

and you should set correct place of the ADLSDK in Makefile in line:

```
ADLSDKDIR = /home/mat/docs/src/ADL_SDK9
```

by replacing `/home/mat/docs/src/ADL_SDK9` by your correct path.

Now. To build program, just type:

```
make
```

### Invoking program

NOTE: If no X11 server is running, then this program requires root privileges.

To run program, just type:

```
./amdcovc
```

This command prints current status of all adapters (graphics cards).

```
./amdcovc -v
```

Prints more informations about graphics cards.

To print help, type:

```
./amdcovc -?
```

To set core clock and memory clock for first adapter, just type:

```
./amdcovc coreclk:0=1000 memclk:0=1200
```

Sets core clock to 1000 MHz and memory clock to 1200 MHz.

### Understanding info printed by program

The AMDCOVC by default prints following informations about graphics card:

```
Adapter 1: Pitcairn PRO [Radeon HD 7850 / R7 265 / R9 270 1024SP]
  Core: 1010 MHz, Mem: 1235 MHz, Vddc: 1.21 V, Load: 98%, Temp: 67 C, Fan: 54%
  Max Ranges: Core: 150 - 1050 MHz, Mem: 75 - 1450 MHz, Vddc: 0.8 - 1.225 V
  PerfLevels: Core: 300 - 1010 MHz, Mem: 150 - 1235 MHz, Vddc: 0.825 - 1.21 V
```

The first line below adapter is current state of graphics card (core clock,
memory clock, voltage, load, temperature and fan speed). The new version add additonal
info about device topology (for example `PCI 3:0:0`).

The `PerfLevels` is current performance level settings from lowest to highest.
Ofcourse, the highest performance levels is used while some computations/rendering.
First level will be used in idle mode (if no work).

The `Max Ranges` is minimal and maximal possible settings for graphics card.

For AMDGPU mode, the AMDCOVC prints:

```
Adapter 2: Ellesmere [Radeon RX 470/480/570/580]
  Core: 300 MHz, Mem: 300 MHz, CoreOD: 0, MemOD: 0
  PerfCtrl: auto, Temp: 42 C, Fan: 31.7647%
  Core Clocks: 300 608 910 1077 1145 1191 1236 1290
  Memory Clocks: 300 1750
```

Likely as in AMD Catalyst mode, an AMDCOVC prints core and memory clocks,
but also prints core and memory overdrive factors (0-20). The `PerfCtrl` is
current performance control (can be low, manual or high). The line `Core Clocks`
contains all possible clocks for core. The line `Memory Clocks`
contains all possible clocks for memory.

The verbose informations contains:

* current state of graphics (Current CoreClock,...)
* minimal and maximal fan speed settings
* minimal and maximal clocks
* minimal and maximal voltages (only AMD Catalyst mode)
* current performance level settings (only AMD Catalyst mode)
* default performance level settings (only AMD Catalyst mode)

also, in AMDGPU mode:

* list of possible core clocks
* list of possible memory clocks

### List of parameters

List of parameters that can be set:

* coreclk[:[ADAPTERS][:LEVEL]]=CLOCK - set core clock in MHz
* memclk[:[ADAPTERS][:LEVEL]]=CLOCK - set memory clock in MHz
* ccoreclk[:[ADAPTERS][:LEVEL]]=CLOCK - set current core clock in MHz (AMDGPU)
* cmemclk[:[ADAPTERS][:LEVEL]]=CLOCK - set current memory clock in MHz (AMDGPU)
* coreod[:[ADAPTERS][:LEVEL]]=PERCENT - set core clock in percents (AMDGPU)
* memod[:[ADAPTERS][:LEVEL]]=PERCENT - set memory clock in perecents (AMDGPU)
* corepl[:[ADAPTERS][:LEVEL]]=LEVEL - set current core performance level (AMDGPU)
* mempl[:[ADAPTERS][:LEVEL]]=LEVEL - set current memory performance level (AMDGPU)
* vcore[:[ADAPTERS][:LEVEL]]=VOLTAGE - set Vddc voltage in Volts
* icoreclk[:ADAPTERS]=CLOCK - set core clock in MHz for idle level
* imemclk[:ADAPTERS]=CLOCK - set memory clock in MHz for idle level
* ivcore[:ADAPTERS]=VOLTAGE - set Vddc voltage  in Volts for idle level
* fanspeed[:[ADAPTERS][:THID]]=PERCENT -  set fanspeed in percents

Extra specifiers in parameters:

* ADAPTERS - adapter (device) index list (default is 0) or `all` for all adapters
* LEVEL - performance level (typically 0 or 1, default is last)
* THID - thermal controller index (must be 0)

The adapter's list can be a single number (0), a single range (0-1) or list of
numbers and ranges, or `all` specifier (choose all adapters).

You can use 'default' in value place to set default value.
For fanspeed 'default' value force automatic speed setup.

To overclock graphic card on AMDGPU(-PRO) driver, can you use `coreod` and `memod`
parameters. Parameters `coreclk` and `memclk` is available in AMDGPU(-PRO) driver mode.

AMDCOVC under AMDGPU(-PRO) the `coreclk` and `memclk` paramters sets GPU core overdrive
and a memory overdirve parameters which determines overclocking in percent.
These paramaters does not impact on current GPU and memory clocks, but these paramters
impact on their maximal values of clocks.
To underclock core and memory you should use `ccoreclk` and `cmemclk` parameters or
`corepl` and `mempl` parameters. These parameters set current core and memory
performance level. The value default sets default automatic performance control.

### List of options

List of options:

* -a, --adapters=LIST - print informations only for these adapters
* -v, --verbose - print verbose informations
* --version - print version
* -?, --help - print help
