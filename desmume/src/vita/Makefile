TITLE_ID = DESMUME00
TARGET   = DeSmuME

DESMUME_CXX_SOURCES = armcpu.cpp \
	arm_instructions.cpp \
	bios.cpp \
	cp15.cpp \
	common.cpp \
	debug.cpp \
	Disassembler.cpp \
	driver.cpp \
	emufile.cpp \
	encrypt.cpp \
	firmware.cpp \
	FIFO.cpp \
	GPU.cpp \
	GPU_osd_stub.cpp \
	mc.cpp \
	readwrite.cpp \
	wifi.cpp \
	path.cpp \
	MMU.cpp \
	NDSSystem.cpp \
	ROMReader.cpp \
	render3D.cpp \
	rasterize.cpp \
	rtc.cpp \
	saves.cpp \
	SPU.cpp \
	matrix.cpp \
	gfx3d.cpp \
	texcache.cpp \
	thumb_instructions.cpp \
	movie.cpp \
	mic.cpp \
	cheatSystem.cpp \
	slot1.cpp \
	slot2.cpp \
	version.cpp \
	addons/slot2_auto.cpp addons/slot2_mpcf.cpp addons/slot2_paddle.cpp addons/slot2_gbagame.cpp addons/slot2_none.cpp addons/slot2_rumblepak.cpp addons/slot2_guitarGrip.cpp addons/slot2_expMemory.cpp addons/slot2_piano.cpp addons/slot2_passme.cpp addons/slot1_none.cpp addons/slot1_r4.cpp addons/slot1_retail_nand.cpp addons/slot1_retail_auto.cpp addons/slot1_retail_mcrom.cpp addons/slot1_retail_mcrom_debug.cpp addons/slot1comp_mc.cpp addons/slot1comp_rom.cpp addons/slot1comp_protocol.cpp \
	utils/advanscene.cpp \
	utils/datetime.cpp \
	utils/xstring.cpp \
	utils/fsnitro.cpp \
	utils/dlditool.cpp \
	utils/emufat.cpp \
	utils/guid.cpp \
	utils/decrypt/decrypt.cpp \
	utils/decrypt/header.cpp \
	utils/decrypt/crc.cpp \
	utils/tinyxml/tinyxml.cpp \
	utils/tinyxml/tinystr.cpp \
	utils/tinyxml/tinyxmlerror.cpp \
	utils/tinyxml/tinyxmlparser.cpp \
	utils/libfat/cache.cpp \
	utils/libfat/directory.cpp \
	utils/libfat/disc.cpp \
	utils/libfat/fatdir.cpp \
	utils/libfat/fatfile.cpp \
	utils/libfat/filetime.cpp \
	utils/libfat/file_allocation_table.cpp \
	utils/libfat/libfat.cpp \
	utils/libfat/libfat_public_api.cpp \
	utils/libfat/lock.cpp \
	utils/libfat/partition.cpp \
	metaspu/metaspu.cpp \
	arm-common/arm_gen.cpp \
	arm-common/arm_jit.cpp \
	vita/vita_task.cpp \
	vita/vita_vfat.cpp

DESMUME_C_SOURCES = utils/ConvertUTF.c

DESMUME_OBJS = $(foreach source, $(DESMUME_CXX_SOURCES), ../$(source:.cpp=.o)) \
	$(foreach source, $(DESMUME_C_SOURCES), ../$(source:.c=.o))

DESMUME_FLAGS = -DHAVE_LIBZ -DHAVE_JIT

OBJS     = main.o console.o $(DESMUME_OBJS)

LIBS = -lvita2d -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub -lSceNet_stub\
	-lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub -lScePgf_stub \
	-lSceCommonDialog_stub -lScePower_stub -lfreetype -lpng -ljpeg -lz -lm -lc -lstdc++

PREFIX    = arm-vita-eabi
CC        = $(PREFIX)-gcc
CXX       = $(PREFIX)-g++
CFLAGS    = -Wl,-q -Wall -O3 -I../ $(DESMUME_FLAGS)
CXXFLAGS  = $(CFLAGS) -fno-rtti -fno-exceptions -fpermissive
ASFLAGS   = $(CFLAGS)

all: $(TARGET).vpk

%.vpk: eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "$(TARGET)" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin $@

eboot.bin: $(TARGET).velf
	vita-make-fself $< $@

%.velf: %.elf
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).vpk $(TARGET).velf $(TARGET).elf $(OBJS) \
		eboot.bin param.sfo

vpksend: $(TARGET).vpk
	curl -T $(TARGET).vpk ftp://$(PSVITAIP):1337/ux0:/
	@echo "Sent."

send: eboot.bin
	curl -T eboot.bin ftp://$(PSVITAIP):1337/ux0:/app/$(TITLE_ID)/
	@echo "Sent."
