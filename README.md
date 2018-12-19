Change content of ${SDK_ROOT}/components/toolchain/gcc/Makefile.posix:

GNU_INSTALL_ROOT ?= /usr/bin/
GNU_VERSION ?= 7.3.1
GNU_PREFIX ?= arm-none-eabi


Some casts (void* <--> something*) are needed in zb_zcl_level_control.h and zb_zcl_on_off.h.