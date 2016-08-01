/*
 * Copyright (c) 2015 Sergi Granell (xerpi)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <psp2/net/net.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/kernel/error.h>

#include "console.h"

#define NET_INIT_SIZE 16*1024

static int console_initialzed = 0;
static SceUID console_mtx;
static void *net_memory = NULL;
static SceNetSockaddrIn target;
static int sock;

void console_init()
{
	SceNetInitParam initparam;

	if (console_initialzed) {
		return;
	}

	console_mtx = sceKernelCreateMutex("console_mutex", 0, 0, NULL);

	if (sceSysmoduleIsLoaded(SCE_SYSMODULE_NET) != SCE_SYSMODULE_LOADED)
		sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

	if (sceNetShowNetstat() == SCE_NET_ERROR_ENOTINIT) {
		net_memory = malloc(NET_INIT_SIZE);

		initparam.memory = net_memory;
		initparam.size = NET_INIT_SIZE;
		initparam.flags = 0;

		sceNetInit(&initparam);
	}

	sock = sceNetSocket("netdbg", SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM, 0);
	memset(&target, 0, sizeof(target));
	target.sin_family = SCE_NET_AF_INET;
	target.sin_port   = sceNetHtons(3490);

	sceNetInetPton(SCE_NET_AF_INET, "192.168.1.104", &target.sin_addr);

	console_initialzed = 1;
}

void console_fini()
{
	if (console_initialzed) {
		sceKernelDeleteMutex(console_mtx);
		sceNetSocketClose(sock);
		sceNetTerm();
		if (net_memory) {
			free(net_memory);
			net_memory = NULL;
		}
	}
}

static inline void netprintf(const char *str)
{
	sceNetSendto(sock, str, strlen(str), SCE_NET_MSG_DONTWAIT,
		(SceNetSockaddr *)&target, sizeof(target));
}

void console_print(const char *s)
{
	int mtx_err = sceKernelTryLockMutex(console_mtx, 1);

	netprintf(s);

	if (mtx_err == SCE_KERNEL_OK) {
		sceKernelUnlockMutex(console_mtx, 1);
	}
}

void console_printf(const char *s, ...)
{
	unsigned int mtx_timeout = 0xFFFFFFFF;
	sceKernelLockMutex(console_mtx, 1, &mtx_timeout);

	char buf[256];
	va_list argptr;
	va_start(argptr, s);
	vsnprintf(buf, sizeof(buf), s, argptr);
	va_end(argptr);
	console_print(buf);

	sceKernelUnlockMutex(console_mtx, 1);
}
