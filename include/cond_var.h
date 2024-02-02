#pragma once
#include "hook_greenify.h"

#ifdef __cplusplus
#include <condition_variable>

const int async_hook = 1;

const char* wait_sym();
const char* notify_sym();

void green_wait(std::condition_variable *const self, std::unique_lock<std::mutex> &lock);
void green_notify(std::condition_variable *const self);
#else
const int async_hook = 0;
#endif

typedef int (*async_factory_t)();
typedef void (*async_callback_t)(int);

void greenify_set_async_(async_factory_t, async_callback_t);
