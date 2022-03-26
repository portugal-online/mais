BUILDTYPE:=Debug

ifeq ($(OS),Windows_NT)     
    HOST_OS := Windows
else
    HOST_OS := $(shell uname)
endif

ifeq ($(HOSTMODE),x86_64)
	EXTRA_FLAGS="-m64"
        HOST_ARCH="x86_64"
	BUILDDIR=$(BUILDROOT)/build/$(HOST_ARCH)/$(BUILDTYPE)
else ifeq ($(HOSTMODE),1)
	EXTRA_FLAGS="-m32"
        HOST_ARCH="i686"
	BUILDDIR=$(BUILDROOT)/build/$(HOST_ARCH)/$(BUILDTYPE)
else ifeq ($(HOSTMODE),i686)
	EXTRA_FLAGS="-m32"
        HOST_ARCH="i686"
	BUILDDIR=$(BUILDROOT)/build/$(HOST_ARCH)/$(BUILDTYPE)
else
BUILDDIR=$(BUILDROOT)/build/STM32WL55xx/$(BUILDTYPE)
endif

ifneq (, $(shell which wslpath))
	WSL=true
else
	WSL=false
endif

OUTAPP=$(BUILDDIR)/mais/uair/Software/app/$(APP)/$(APP)

OUTHEX=$(OUTAPP).hex
OUTBIN=$(OUTAPP).elf

all: $(OUTHEX)
	@echo Building $(OUTHEX)
	@echo Build dir $(BUILDDIR)

.PHONY: $(OUTHEX)

$(OUTHEX):  $(BUILDDIR)/Makefile
	make -C $(BUILDDIR) -j8

ifndef HOSTMODE
$(BUILDDIR)/Makefile:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake ../../.. -B. \
        -DTOOLCHAIN_PREFIX=/usr/ \
        -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
        -DCMAKE_TOOLCHAIN_FILE=cross.cmake \
        -DTARGET_APP=$(APP) \
        -DHOST_OS="" \
        -DAPP_LIST="$(APP)" \
        -DLINKER_SCRIPT="mais/uair/Software/target/stm32wl55xx_uair_flash.ld" \
        -DOBJECT_GEN_FLAGS="-Werror -Wall -ffunction-sections -fdata-sections -fno-builtin -fomit-frame-pointer" \
        -DAPP_BUILD_DIR=mais/uair/Software/app/$(APP)
else

$(BUILDDIR)/Makefile:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake ../../.. -B. \
        -DTOOLCHAIN_PREFIX=/usr/ \
        -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
        -DHOST_OS=$(HOST_OS) \
        -DTARGET_APP=$(APP) \
        -DAPP_LIST="$(APP)" \
        -DHOSTMODE=1 \
        -DHOST_ARCH=$(HOST_ARCH) \
        -DMCU_DIR=mais/uair/hostmode/target \
        -DUAIR_UNIT_TESTS="$(UAIR_UNIT_TESTS)" \
        -DLINKER_SCRIPT="" \
        -DOBJECT_GEN_FLAGS="$(EXTRA_FLAGS) -DHOSTMODE -fPIE -DPIE -Werror -Wall -ffunction-sections -fdata-sections -fno-builtin -fomit-frame-pointer" \
        -DAPP_BUILD_DIR=mais/uair/Software/app/$(APP)

endif

$(BUILDDIR):
	mkdir -p build/debug 

define check_programmer
	@if [ "x$(STMPROGRAMMER)" = "x" ]; then \
		echo "*********** ERROR ***********"; \
		echo "You need to define STMPROGRAMMER environment variable!"; \
		echo; \
		exit -1; \
		fi
endef

ifeq (true, $(WSL))
flash: $(OUTHEX)
	$(call check_programmer)
	$(STMPROGRAMMER) -c port=SWD reset=HWrst -w "$(shell wslpath -w $(OUTHEX))"
	$(STMPROGRAMMER) -c port=SWD reset=HWrst -hardRst
else

flash: $(OUTHEX)
	$(call check_programmer)
	$(STMPROGRAMMER) -c port=SWD reset=HWrst -w $(OUTHEX)
	$(STMPROGRAMMER) -c port=SWD reset=HWrst -hardRst
endif

clean:
	if [ -e $(BUILDDIR)/Makefile ]; then make -C $(BUILDDIR) clean; fi
	rm -rf $(BUILDDIR)/app $(BUILDDIR)/Makefile $(BUILDDIR)/CMakeFiles $(BUILDDIR)/CMakeCache.txt

ifdef HOSTMODE

$(OUTBIN): $(OUTHEX)

run: $(OUTBIN)
	$(OUTBIN)
endif
