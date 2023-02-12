# Overview
For a general overview as well as automatically updated documentation, please refer to the [Ultramote website](https://etrommer.github.io/ultramote/)

# Firmware
## Remove Lora-E5 readout protection
The Seeedstudio Lora-E5 comes with a preloaded firmware and readout protection enabled. This needs to be disabled before other firmware can be flashed. Run:
```bash
openocd -f interface/stlink.cfg -f target/stm32wlx.cfg
```
then connect to the OpenOCD session from another terminal, using: `nc localhost 4444`. In the telnet session, run:
```
init
reset halt
smt32wlx unlock 0
reset halt
exit
```

# Hardware
## Documentation
Schematics, BoM, Gerber files etc. are generated using [KiBot](https://github.com/INTI-CMNB/KiBot). To automatically generate documentation, install KiBot [as described here](https://github.com/INTI-CMNB/KiBot#installation).

With kibot installed, change to the `pcb` directory and run KiBot:
```bash
cd pcb
kibot
```
This generates all documentation in the local subfolder `generated`. If you think that an important piece of documentation is missing, please open an issue.
