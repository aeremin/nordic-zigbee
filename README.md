# Example of ZigBee light bulb based on nRF52840 SoC

## Environment setup
- Install [chocolatey](https://chocolatey.org/) - Windows package manager.
- Install make: `choco install make`.
- Check that `make` is available from command line
- Install ARM GCC: `choco install gcc-arm-embedded`.
- Download [nRF5 SDK for Thread and Zigbee v2.0.0](https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5-SDK-for-Thread/nRF5-SDK-for-Thread-and-Zigbee/nRF5SDKforThreadandZigbee20029775ac.zip).
- Extract it to the `../ZigBee_2.0.0` folder.
- Change the content of `../components/toolchain/gcc/Makefile.windows`:
```bash
GNU_INSTALL_ROOT := C:/ProgramData/chocolatey/lib/gcc-arm-embedded/tools/bin/
GNU_VERSION := 7.3.1
GNU_PREFIX := arm-none-eabi
```
- Change `D0 D1 D2 ... DF` in the `../external/zboss/lib/gcc` libzboss files (both of them) to the ZLL Commissioning trust centre link key.

## Compiling and flashing
- Run `make` from `light_bulb/pca10056/blank/armgcc` to build the firmware.
- Run `make flash` to flash it to the board.

## Notes
- Running `make erase` can be required so board will forget that it has already joined some ZigBee network.
- Double check that kIeeeChannelMask includes ZigBee channel your network coordinator uses.