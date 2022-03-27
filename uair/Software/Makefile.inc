BUILDTYPE:=Debug

ifeq ($(OS),Windows_NT)
    HOST_OS := Windows
else
    HOST_OS := $(shell uname)
endif

ifeq ($(HOST_OS),Darwin)
	TOOLCHAIN_PREFIX := /usr/local/
else
	TOOLCHAIN_PREFIX := /usr/
endif

ifdef UAIR_HOST_MODE
ifdef HOSTMODE
$(error The options UAIR_HOST_MODE and HOSTMODE are mutually exclusive.)
else
HOSTMODE=${UAIR_HOST_MODE}
endif
endif

ifeq ($(HOSTMODE),1)
	override HOSTMODE=i686
endif

ifeq ($(HOSTMODE),x86_64)
	BUILDDIR=$(BUILDROOT)/build/x86_64/$(BUILDTYPE)
else ifeq ($(HOSTMODE),i686)
	BUILDDIR=$(BUILDROOT)/build/x86_64/$(BUILDTYPE)
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
        -DTOOLCHAIN_PREFIX=$(TOOLCHAIN_PREFIX) \
        -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
        -DCMAKE_TOOLCHAIN_FILE=cross.cmake \
        -DTARGET_APP=$(APP) \
        -DHOST_OS="" \
        -DAPP_LIST="$(APP)" \
        -DUAIR_HOST_MODE=$(HOSTMODE) \
        -DUAIR_UNIT_TESTS="$(UAIR_UNIT_TESTS)" \
        -DAPP_BUILD_DIR=mais/uair/Software/app/$(APP)
else

$(BUILDDIR)/Makefile:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake ../../.. -B. \
        -DTOOLCHAIN_PREFIX=$(TOOLCHAIN_PREFIX) \
        -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
        -DHOST_OS=$(HOST_OS) \
        -DTARGET_APP=$(APP) \
        -DAPP_LIST="$(APP)" \
        -DUAIR_HOST_MODE=$(HOSTMODE) \
        -DUAIR_UNIT_TESTS="$(UAIR_UNIT_TESTS)" \
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
