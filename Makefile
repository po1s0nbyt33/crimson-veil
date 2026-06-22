# ---------------------------------------------------------------------------
# Makefile alternativo (sem CMake) para VitaSDK.
# Uso:  make            -> gera CrimsonVeil.vpk
#       make clean      -> limpa artefatos
# Requer: a variavel de ambiente VITASDK definida e $(VITASDK)/bin no PATH.
# ---------------------------------------------------------------------------
TITLE_ID = CRMV00001
APP_NAME = Crimson Veil
TARGET   = CrimsonVeil

SOURCES  = src
INCLUDES = src

CFILES   := $(wildcard $(SOURCES)/*.c)
OBJS     := $(CFILES:.c=.o)

PREFIX   = arm-vita-eabi
CC       = $(PREFIX)-gcc
CFLAGS   = -Wl,-q -Wall -Wextra -Wno-unused-parameter -O3 -std=c11 $(addprefix -I,$(INCLUDES))

LIBS = -lvita2d -lSceDisplay_stub -lSceGxm_stub -lSceSysmodule_stub \
       -lSceCtrl_stub -lSceTouch_stub -lSceAppMgr_stub \
       -lSceCommonDialog_stub -lSceLibKernel_stub -lSceKernelThreadMgr_stub -lSceAudio_stub -lScePgf_stub \
       -Wl,--start-group -lfreetype -lharfbuzz -lpng -ljpeg -lz -lbz2 -Wl,--end-group \
       -lm -lc

all: $(TARGET).vpk

$(TARGET).vpk: eboot.bin
	vita-mksfoex -s TITLE_ID="$(TITLE_ID)" "$(APP_NAME)" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin \
		--add sce_sys/icon0.png=sce_sys/icon0.png \
		--add sce_sys/livearea/contents/bg.png=sce_sys/livearea/contents/bg.png \
		--add sce_sys/livearea/contents/startup.png=sce_sys/livearea/contents/startup.png \
		--add sce_sys/livearea/contents/template.xml=sce_sys/livearea/contents/template.xml \
		$(TARGET).vpk

eboot.bin: $(TARGET).velf
	vita-make-fself -s $(TARGET).velf eboot.bin

$(TARGET).velf: $(TARGET).elf
	vita-elf-create $(TARGET).elf $(TARGET).velf

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $(TARGET).elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) $(TARGET).velf $(TARGET).elf $(TARGET).vpk \
		eboot.bin param.sfo $(TARGET).self

.PHONY: all clean
