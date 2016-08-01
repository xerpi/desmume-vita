/*
	Copyright (C) 2009-2015 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <psp2/kernel/threadmgr.h>

#include <stdio.h>

#include "../utils/task.h"
#include "debug.h"

int getOnlineCores (void)
{
	return 4;
}

class Task::Impl {
private:
	SceUID _thread;
	bool _isThreadRunning;

public:
	Impl();
	~Impl();

	void start(bool spinlock);
	void execute(const TWork &work, void *param);
	void* finish();
	void shutdown();

	//slock_t *mutex;
	SceUID condWork;
	SceUID condEndWork;
	TWork workFunc;
	void *workFuncParam;
	void *ret;
	bool exitThread;
};

static int taskProc(SceSize args, void *argp)
{
	Task::Impl *ctx = *(Task::Impl **)argp;
	do {

		while (ctx->workFunc == NULL && !ctx->exitThread) {
			sceKernelWaitEventFlag(ctx->condWork, 1,
				SCE_EVENT_WAITAND, NULL, NULL);
		}

		sceKernelClearEventFlag(ctx->condWork, ~1);

		if (ctx->workFunc != NULL) {
			ctx->ret = ctx->workFunc(ctx->workFuncParam);
		} else {
			ctx->ret = NULL;
		}

		ctx->workFunc = NULL;
		sceKernelSetEventFlag(ctx->condEndWork, 1);
	} while(!ctx->exitThread);

	return 0;
}

Task::Impl::Impl()
{
	_isThreadRunning = false;
	workFunc = NULL;
	workFuncParam = NULL;
	ret = NULL;
	exitThread = false;

	condWork = sceKernelCreateEventFlag("desmume_cond_work", 0, 0, NULL);
	condEndWork = sceKernelCreateEventFlag("desmume_cond_end_work", 0, 0, NULL);
}

Task::Impl::~Impl()
{
	shutdown();
	sceKernelDeleteEventFlag(condWork);
	sceKernelDeleteEventFlag(condEndWork);
}

void Task::Impl::start(bool spinlock)
{
	if (this->_isThreadRunning) {
		return;
	}

	this->workFunc = NULL;
	this->workFuncParam = NULL;
	this->ret = NULL;
	this->exitThread = false;
	this->_thread = sceKernelCreateThread("desmume_task", taskProc,
		0x10000100, 0x10000, 0, 0, NULL);

	Task::Impl *_this = this;
	sceKernelStartThread(this->_thread, sizeof(_this), &_this);

	this->_isThreadRunning = true;
}

void Task::Impl::execute(const TWork &work, void *param)
{
	if (work == NULL || !this->_isThreadRunning) {
		return;
	}

	this->workFunc = work;
	this->workFuncParam = param;

	sceKernelSetEventFlag(condWork, 1);
}

void* Task::Impl::finish()
{
	void *returnValue = NULL;

	if (!this->_isThreadRunning) {
		return returnValue;
	}

	while (this->workFunc != NULL) {
		sceKernelWaitEventFlag(condEndWork, 1, SCE_EVENT_WAITAND, NULL, NULL);
	}

	sceKernelClearEventFlag(condEndWork, ~1);

	returnValue = this->ret;

	return returnValue;
}

void Task::Impl::shutdown()
{

	if (!this->_isThreadRunning) {
		return;
	}

	this->workFunc = NULL;
	this->exitThread = true;

	sceKernelSetEventFlag(condWork, 1);
	sceKernelWaitThreadEnd(_thread, NULL, NULL);

	this->_isThreadRunning = false;
}

void Task::start(bool spinlock) { impl->start(spinlock); }
void Task::shutdown() { impl->shutdown(); }
Task::Task() : impl(new Task::Impl()) {}
Task::~Task() { delete impl; }
void Task::execute(const TWork &work, void* param) { impl->execute(work,param); }
void* Task::finish() { return impl->finish(); }

