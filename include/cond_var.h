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

void greenify_set_async_(void *(*factory)(), void (*callback)(void *));

typedef void *(*async_factory_t)();
typedef void (*async_callback_t)(void *);
