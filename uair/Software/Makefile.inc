BUILDTYPE:=Debug

ifeq ($(HOSTMODE),1)
BUILDDIR=$(BUILDROOT)/build/host/$(BUILDTYPE)
else
BUILDDIR=$(BUILDROOT)/build/STM32WL55xx/$(BUILDTYPE)
endif

ifneq (, $(shell which wslpath))
	WSL=true
else
	WSL=false
endif


OUTHEX=$(BUILDDIR)/mais/uair/Software/app/$(APP)/$(APP).hex

all: $(OUTHEX)
	echo $(OUTHEX)
	echo Build dir $(BUILDDIR)

.PHONY: $(OUTHEX)

$(OUTHEX):  $(BUILDDIR)/Makefile
	make -C $(BUILDDIR) -j8

ifeq ($(HOSTMODE),1)
$(BUILDDIR)/Makefile:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake ../../.. -B. \
        -DTOOLCHAIN_PREFIX=/usr/ \
        -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
        -DTARGET_APP=$(APP) \
        -DAPP_LIST="$(APP)" \
        -DHOSTMODE=1 \
        -DMCU_DIR=mais/uair/hostmode/target \
        -DLINKER_SCRIPT="" \
        -DOBJECT_GEN_FLAGS="-m32 -DHOSTMODE=1 -Werror -Wall -ffunction-sections -fdata-sections -fno-builtin -fomit-frame-pointer" \
        -DAPP_BUILD_DIR=mais/uair/Software/app/$(APP)

else
$(BUILDDIR)/Makefile:
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR) && cmake ../../.. -B. \
        -DTOOLCHAIN_PREFIX=/usr/ \
        -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
        -DCMAKE_TOOLCHAIN_FILE=cross.cmake \
        -DTARGET_APP=$(APP) \
        -DAPP_LIST="$(APP)" \
        -DLINKER_SCRIPT="mais/uair/Software/target/stm32wl55xx_uair_flash.ld" \
        -DOBJECT_GEN_FLAGS="-Werror -Wall -ffunction-sections -fdata-sections -fno-builtin -fomit-frame-pointer" \
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
