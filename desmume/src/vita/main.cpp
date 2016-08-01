/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>

#include <vita2d.h>

#include "utils.h"
#include "console.h"

#include "../MMU.h"
#include "../NDSSystem.h"
#include "../debug.h"
#include "../render3D.h"
#include "../rasterize.h"
#include "../saves.h"
#include "../mic.h"
#include "../SPU.h"
#include "../GPU_osd.h"

static void wait_key_press();

extern "C" {
int ftruncate(int fd, off_t length)
{
	printf("ftruncate\n");
	return 0;
}
char *getcwd(char *buf, size_t size)
{
	printf("getcwd\n");
	return "ux0:data/";
}
int mkdir(const char *pathname, mode_t mode)
{
	printf("mkdir\n");
	return 0;
}
int chdir(const char *path)
{
	printf("chdir\n");
	return 0;
}
}

volatile bool execute = false;

GPU3DInterface *core3DList[] = {
	&gpu3DNull,
	&gpu3DRasterize,
	NULL
};

SoundInterface_struct *SNDCoreList[] = {
	&SNDDummy,
	&SNDDummy,
	&SNDDummy,
	NULL
};

static inline unsigned int ABGR1555toRGBA8(unsigned short c)
{
	const unsigned int a = c&0x8000, b = c&0x7C00, g = c&0x03E0, r = c&0x1F;
	const unsigned int rgb = (r << 27) | (g << 14) | (b << 1);
	return ((a * 0xFF) >> 15) | rgb | ((rgb >> 5) & 0x07070700);
}

static int run = 1;
const float scale = SCREEN_H / (192 * 2.0);
const int fb_x = SCREEN_W/2 - (256/2) * scale;
const int fb_y =  SCREEN_H/2 - ((192*2)/2) * scale;

#define EXIT_MASK (SCE_CTRL_START | SCE_CTRL_LTRIGGER)

static void update_input()
{
	static int last_touch = 0;
	SceCtrlData pad;
	SceTouchData touch;
	int touch_x, touch_y;
	buttonstruct<bool> input = {};

	sceCtrlPeekBufferPositive(0, &pad, 1);
	sceTouchPeek(0, &touch, 1);

	input.E = pad.buttons & SCE_CTRL_RTRIGGER;
	input.W = pad.buttons & SCE_CTRL_LTRIGGER;
	input.X = pad.buttons & SCE_CTRL_SQUARE;
	input.Y = pad.buttons & SCE_CTRL_TRIANGLE;
	input.A = pad.buttons & SCE_CTRL_CIRCLE;
	input.B = pad.buttons & SCE_CTRL_CROSS;
	input.S = pad.buttons & SCE_CTRL_START;
	input.T = pad.buttons & SCE_CTRL_SELECT;
	input.U = pad.buttons & SCE_CTRL_UP;
	input.D = pad.buttons & SCE_CTRL_DOWN;
	input.L = pad.buttons & SCE_CTRL_LEFT;
	input.R = pad.buttons & SCE_CTRL_RIGHT;

	if ((pad.buttons & EXIT_MASK) == EXIT_MASK)
		run = 0;

	if (touch.reportNum > 0) {
		touch_x = lerp(touch.report[0].x, 1920, SCREEN_W);
		touch_y = lerp(touch.report[0].y, 1088, SCREEN_H);

		if (touch_x >= fb_x && touch_x <= (fb_x + 256 * scale) &&
		    touch_y >= (fb_y + 192 * scale) && touch_y <= (fb_y + 2*192 * scale)) {
			NDS_setTouchPos((touch_x - fb_x) / scale, (touch_y - fb_y - 192 * scale) / scale);
			last_touch = 1;
		}
	} else {
		if (last_touch) {
			NDS_releaseTouch();
			last_touch = 0;
		}
	}

	NDS_setPad(
		input.R, input.L, input.D, input.U,
		input.T, input.S, input.B, input.A,
		input.Y, input.X, input.W, input.E,
		input.G, input.F);

	NDS_beginProcessingInput();
	NDS_endProcessingInput();
}

#define FRAMESKIP 0
#define ROM_PATH "ux0:/data/game.nds"

int main()
{
	int i;
	vita2d_texture *fb = NULL;
	void *data;
	uint16_t *src;

	console_init();
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0, 0, 0, 0xFF));
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);

	DBG("DeSmuME Vita by xerpi\n");

	struct NDS_fw_config_data fw_config;
	NDS_FillDefaultFirmwareConfigData(&fw_config);
  	NDS_Init();
	NDS_3D_ChangeCore(1);
	backup_setManualBackupType(0);
	CommonSettings.ConsoleType = NDS_CONSOLE_TYPE_FAT;

	DBG("Loading " ROM_PATH "...\n");
	if (NDS_LoadROM("ux0:/data/game.nds") < 0) {
		printf("Error loading game.nds\n");
		goto exit;
	}

	fb = vita2d_create_empty_texture_format(256, 192*2,
		SCE_GXM_TEXTURE_FORMAT_U1U5U5U5_ABGR);

	data = vita2d_texture_get_datap(fb);

	execute = true;

	while (run) {
		update_input();
		NDS_exec<false>();

		for (i = 0; i < FRAMESKIP; i++) {
			NDS_SkipNextFrame();
			NDS_exec<false>();
		}

		//SPU_Emulate_user();

		src = (uint16_t *)GPU->GetDisplayInfo().masterNativeBuffer;

		vita2d_start_drawing();
		vita2d_clear_screen();

		memcpy(data, src, sizeof(uint16_t) * 256 * 192 * 2);

		vita2d_draw_texture_scale(fb, fb_x, fb_y, scale, scale);

		vita2d_end_drawing();
		vita2d_swap_buffers();
	}

exit:
	vita2d_fini();
	if (fb) {
		vita2d_free_texture(fb);
	}

	sceKernelExitProcess(0);
	return 0;
}
