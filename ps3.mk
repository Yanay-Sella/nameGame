# PSL1GHT build rules; runs inside the ps3dev Docker container (use the top-level Makefile instead).
.SUFFIXES:
include $(PSL1GHT)/ppu_rules

TARGET    := nameGame
BUILD     := build
SOURCES   ?= source
INCLUDES  := include

TITLE     := nameGame
APPID     := NMGM00001
CONTENTID := UP0001-$(APPID)_00-0000000000000000

LOG_HOST  ?= 127.0.0.1
LOG_PORT  ?= 18195

CFLAGS    = -O2 -Wall -mcpu=cell $(MACHDEP) $(INCLUDE) -I$(PORTLIBS)/include/SDL2 \
            -DLOG_HOST=\"$(LOG_HOST)\" -DLOG_PORT=$(LOG_PORT)
CXXFLAGS  = $(CFLAGS)
LDFLAGS   = $(MACHDEP) -Wl,-Map,$(notdir $@).map
LIBS      := -lSDL2_mixer -lmikmod -lvorbisfile -lvorbis -logg -lSDL2 -ldebugnet -lnet -lsysmodule -lgcm_sys -lrsx -lsysutil -lio -laudio -lrt -llv2 -lm

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(BUILD)/$(TARGET)
# Copied into the package root: pkgfiles/USRDIR/* lands next to EBOOT.BIN.
export PKGFILES := $(CURDIR)/pkgfiles
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)
export BUILDDIR := $(CURDIR)/$(BUILD)
export LD       := $(CXX)
export OFILES   := $(foreach dir,$(SOURCES),$(patsubst %.cpp,%.o,$(notdir $(wildcard $(dir)/*.cpp))))
export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(LIBPSL1GHT_INC) -I$(PORTLIBS)/include
export LIBPATHS := $(LIBPSL1GHT_LIB) -L$(PORTLIBS)/lib

.PHONY: pkg

pkg:
	@mkdir -p $(BUILD)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/ps3.mk $(OUTPUT).pkg

else

$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(OFILES:.o=.d)

endif
