#include <iostream>
#include "timer.h"

timer::timer() :
    m_ms_interval(0),
    m_exit_flag(false)
    /*,
    m_future_obj(m_exit_signal.get_future())*/
{
    
}

timer::~timer()
{
    stop();
}

int timer::start_once()
{
    if(m_ms_interval <= 0 || m_started_mutex.try_lock() == false)
    {
        return -1;
    }

    m_timer_thread = std::thread([=]() {
        std::mutex cvm;
        std::unique_lock<std::mutex> lock(cvm);

        bool status = m_cv_exit_flag.wait_for(lock, 
                        std::chrono::milliseconds(m_ms_interval),
                        [=]{return m_exit_flag == true; });

        m_started_mutex.unlock();
        
        if( status == false)
        {
            m_callback(m_cb_param);
        }
        
    });
    
    return 0;
}

int timer::start()
{
    if(m_ms_interval <= 0 || m_started_mutex.try_lock() == false)
    {
        return -1;
    }

    m_timer_thread = std::thread([=]() {
        std::mutex cvm;
        std::unique_lock<std::mutex> lock(cvm);

        while(m_exit_flag == false)
        {
            bool status = m_cv_exit_flag.wait_for(lock, 
                        std::chrono::milliseconds(m_ms_interval),
                        [=]{return m_exit_flag == true; });
            if( status == false)
            {
                m_callback(m_cb_param);
            }
        }
        m_started_mutex.unlock();
//std::cout  << "Thread finished" << std::endl;
    });

    return 0;
}

void timer::stop()
{
//    m_exit_signal.set_value();
    m_exit_flag = true;
    m_cv_exit_flag.notify_all();
    if(m_timer_thread.joinable())
    {
        m_timer_thread.join();
    }
}


