#ifndef __CORE_APPLICATION_HPP__
#define __CORE_APPLICATION_HPP__

#include <future>
#include <chrono>
#include <unordered_map>

#include <mosquitto.h>  

#include <mqtt/mqtt_topic.h>
#include <types/typedefs.h>

class app_handler;

/*
class mqtt_topic_connector
{
    public:
        mqtt_topic_connector(const T &topic)
        {
        }
        
        const T get_topic() const
        {
            return topic;
        }
        
    private:
        const T topic;        
};
*/

class core_application
{
    public:
        core_application ();
        virtual ~core_application ();

        int attach(const class app_handler *ah);
        int detach(const class app_handler *ah);
        virtual int run();
        virtual void terminate();
        virtual void on_mqtt_connect(int rc) { UNUSED(rc); };
        virtual void on_mqtt_disconnect(int rc) { UNUSED(rc);  };
        void on_mqtt_message(const struct mosquitto_message *message); // {UNUSED(message); return;};
        virtual void on_mqtt_message_received(const struct mosquitto_message *message) { UNUSED(message); };

//        virtual void on_configuration_update(const char *cfg_str) { UNUSED(cfg_str); }

    protected:
        virtual int on_attach() = 0;
        virtual int on_detach() = 0;
        virtual void on_terminate() {};

        int publish(const mqtt_topic &topic, bool retain = false);
        int publish(const char *topic_name, const char *value, bool retain = false);
        int subscribe(const char *topic_name);
        int subscribe(mqtt_topic *topic); 
        int unsubscribe(const char *topic_name);
        int unsubscribe(const mqtt_topic &topic); 

    private:
        std::promise<void>  m_exit_signal;
        std::future<void>   m_future_obj;
        std::vector<std::string>  m_topic_names;
        std::unordered_map<std::string, /*std::any*/ mqtt_topic *>  m_topics_data;
        struct mosquitto *m_mosq;
};

#endif /* end of include guard CORE_APPLICATION_HPP */

