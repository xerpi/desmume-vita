#include <string>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stack>

#include "../types.h"
#include "../debug.h"
#include "../emufile.h"

#include "utils/emufat.h"
#include "utils/vfat.h"

bool VFAT::build(const char* path, int extra_MB)
{

	return true;
}

VFAT::VFAT()
	: file(NULL)
{
}

VFAT::~VFAT()
{
	delete file;
}

EMUFILE* VFAT::detach()
{
	EMUFILE* ret = file;
	file = NULL;
	return ret;
}
