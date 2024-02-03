long (*async_factory)();
void (*async_waiter)(long);
void (*async_callback)(long);

void greenify_set_async_(long (*factory)(), void (*waiter)(long), void (*callback)(long)) {
    async_factory = factory;
    async_waiter = waiter;
    async_callback = callback;
}

#ifdef __cplusplus
#include <condition_variable>
#include <dlfcn.h>
#include <unordered_map>
#include <queue>
#include <cstdio>

const char* ref_symbol(void *ref) {
    Dl_info res;
    dladdr(ref, &res);
    return res.dli_sname;
}

std::unordered_map<std::condition_variable*, std::queue<long>> waiters;

void green_wait(std::condition_variable *const self, std::unique_lock<std::mutex> &lock) {
    auto ref = async_factory();
    printf("creating waiter %ld for %p releasing %p\n", ref, self, &lock);
    waiters[self].push(ref);
    lock.unlock();
    async_waiter(ref);
    printf("signal %p waiting on %p\n", self, &lock);
    lock.lock();
    printf("signal %p acquired %p\n", self, &lock);
}

void green_notify(std::condition_variable *const self) {
    auto q = waiters[self];
    printf("releasing waiter %ld for %p\n", q.front(), self);
    for (; !q.empty(); q.pop()) async_callback(q.front());
    waiters.erase(self);
}

const char* wait_sym() {
    void (std::condition_variable::*wait)(std::unique_lock<std::mutex>&) =
        &std::condition_variable::wait;
    return ref_symbol((void *) wait);
}

const char* notify_sym() {
    return ref_symbol((void *) &std::condition_variable::notify_all);
}
#endif
