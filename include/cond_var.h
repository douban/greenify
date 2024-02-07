#ifndef _COND_VAR
#define _COND_VAR

#ifdef __cplusplus
#include <condition_variable>

const long async_hook = 1;

const char* wait_sym();
const char* notify_sym();

void green_wait(std::condition_variable *const self, std::unique_lock<std::mutex> &lock);
void green_notify(std::condition_variable *const self);
#else
const long async_hook = 0;
#endif

void greenify_set_async_(long (*factory)(), void (*waiter)(long), void (*callback)(long));

#endif
