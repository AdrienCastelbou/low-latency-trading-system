#pragma once
#include <iostream>
#include <atomic>
#include <thread>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <mach/thread_policy.h>

namespace threading
{
    inline auto setThreadCore(int coreId) noexcept
    {
        return true; // 
        thread_affinity_policy_data_t policy = { coreId };
        thread_port_t mach_thread = pthread_mach_thread_np(pthread_self());
    
        kern_return_t ret = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
        if (ret == KERN_SUCCESS) std::cout << "success" << std::endl;
        else std::cout << "fail" << std::endl;
        return (ret == KERN_SUCCESS);
    }


    template<typename T, typename ...A>
    inline auto createAndStartThread(int coreId, const std::string& name, T&& func, A&& ... args) noexcept
    {
        std::atomic_bool running(false), failed(false);
        auto threadBody = [&]
        {
            if (coreId >= 0 && !setThreadCore(coreId))
            {
                std::cerr << "Failed to set core affinity for " << name << " " << pthread_self() << " to " << coreId << std::endl;
                failed = true;
                return;
            }

            std::cout << "Set core affinity for " << name << " " << pthread_self() << " to " << coreId << std::endl;
            running = true;
            std::forward<T>(func)((std::forward<A>(args))...);
        };

        auto thread = new std::thread(threadBody);

        while (!running && !failed)
        {
            using namespace std::literals::chrono_literals;
            std::this_thread::sleep_for(1s);
        }

        if (failed)
        {
            thread->join();
            delete thread;
            thread = nullptr;
        }
        return thread;
    }
} // namespace threading
