#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <condition_variable>
#include <functional>
#include <atomic>
#include <thread>
//#include <future>
#include <chrono>
#include <mutex>

class timer
{
    public:
        timer ();
        virtual ~timer ();

        int start();
        int start_once();
        void stop();

        inline void set_interval(int ms_interval)
        {
            m_ms_interval = ms_interval;
        }

        inline void set_function(std::function<void(void *)> callback, void *cb_param = nullptr)
        {
            m_callback = callback;
            m_cb_param = cb_param;
        }

    private:
//        std::chrono::milliseconds m_interval;
        int m_ms_interval;
        std::function<void(void *)> m_callback;
        void *m_cb_param;
//        std::promise<void> m_exit_signal;
//        std::future<void> m_future_obj;
//        bool m_exit_flag;
        std::atomic<bool> m_exit_flag;
        std::condition_variable     m_cv_exit_flag;
//        std::mutex m_stop_mutex;
        std::mutex m_started_mutex;
        std::thread m_timer_thread;
//        bool    m_started;
//        std::chrono::milliseconds   
};

#endif /* end of include guard TIMER_HPP */

