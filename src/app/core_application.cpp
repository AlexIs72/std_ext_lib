#include <string.h>

#include <iostream>
#include <thread>
#include <regex>
//#include <cxxabi.h>

//#include <types/typedefs.h>
#include <sys/logger.h>
#include <utils/string.h>

#include "core_application.h"
#include "app_handler.h"

#define MQTT_DEFAULT_HOST   "localhost"
#define MQTT_DEFAULT_PORT   1883

/*
https://gitlab.labs.nic.cz/turris/domoticz-turris-gadgets/blob/master/MQTT/mosquittopp.cpp
*/

static void on_connect_wrapper(struct mosquitto *mosq, void *userdata, int rc)
{
    UNUSED(mosq);
    class core_application *ca = (class core_application *)userdata;
    ca->on_mqtt_connect(rc);
//    mosquitto_subscribe(mosq, nullptr, _configuration_topic.c_str(), /*qos*/0);
}

static void on_disconnect_wrapper(struct mosquitto *mosq, void *userdata, int rc)
{
    UNUSED(mosq);
    class core_application *ca = (class core_application *)userdata;
    ca->on_mqtt_disconnect(rc);
//    mosquitto_unsubscribe(mosq, nullptr, _configuration_topic.c_str());
}

static void on_message_wrapper(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
    UNUSED(mosq);
    class core_application *ca = (class core_application *)userdata;

/*    if(_configuration_topic == message->topic)
    {
        ca->on_configuration_update((const char *)message->payload);
    }
    else
    {
*/
        ca->on_mqtt_message(message);
//    }
}

core_application::core_application() :
    m_future_obj(m_exit_signal.get_future()),    // TODO need to replace by condition variable
    m_mosq(nullptr)
{
    
}

core_application::~core_application()
{
}

// cppcheck-suppress unusedFunction
int core_application::run()
{
//    UNISED(ah);
    while(m_future_obj.wait_for(std::chrono::milliseconds(1000)) == std::future_status::timeout)
    {
        std::this_thread::yield();
    }

//    mosquitto_loop_forever(m_mosq, /*timeout*/-1, /*max_packets*/1);

    return 0;
}

// cppcheck-suppress unusedFunction
int core_application::attach(const class app_handler *ah)
{
    int result = 0;

    mosquitto_lib_init();
    m_mosq = mosquitto_new(ah->app_name(), /*clean_session*/true, this);
    
    mosquitto_connect_callback_set(m_mosq, on_connect_wrapper);
    mosquitto_disconnect_callback_set(m_mosq, on_disconnect_wrapper);
    mosquitto_message_callback_set(m_mosq, on_message_wrapper);

/*    _configuration_topic += ah->app_name();
    std::ext::string::toupper(_configuration_topic);

std::cout << "_configuration_topic = " << _configuration_topic << std::endl;   
*/
    result = on_attach();
    if(result != 0)
    {
        logger::error("Unable to start controller");
        return result;
    }
    
    int keepalive = 60;
    int res = mosquitto_connect_async(m_mosq, MQTT_DEFAULT_HOST, MQTT_DEFAULT_PORT, keepalive);
// TODO process connection error
    if(res == MOSQ_ERR_SUCCESS)
    {
	    logger::info("MQTT connected");
    }
    else
    {
	    logger::error("MQTT connection error: [%d] %s", res, mosquitto_strerror(res));
    }
    mosquitto_loop_start(m_mosq);

    logger::info("Controller started"/*, typeid(this).name()*/);

    return result;
}

// cppcheck-suppress unusedFunction
int core_application::detach(const class app_handler *ah)
{
    int result = 0;

    UNUSED(ah);

    mosquitto_disconnect(m_mosq);
    mosquitto_loop_stop(m_mosq, false);

    result = on_detach();

//    mosqpp::lib_cleanup();
    mosquitto_destroy(m_mosq);
    mosquitto_lib_cleanup();

    logger::info("Stopped controller"/*, typeid(this).name()*/);

    return result;
}

// cppcheck-suppress unusedFunction
void core_application::terminate()
{
//    std::cout << "----> core_application::on_terminate" << std::endl;
    on_terminate();
//    std::cout << "----> core_application::terminate" << std::endl;
    m_exit_signal.set_value();
}

// cppcheck-suppress unusedFunction
int core_application::publish(const mqtt_topic &topic, bool retain)
{
//    const std::string data = topic.c_str();
    std::vector<uint8_t> data = topic.data();
    return mosquitto_publish(m_mosq, 
                            nullptr, 
                            topic.get_name().data(), 
                            data.size(), 
                            data.data(), /*qos*/0, retain);
}

int core_application::publish(const char *topic, const char *value, bool retain)
{
    return mosquitto_publish(m_mosq, nullptr, topic, strlen(value), value, /*qos*/0, retain);
}

// cppcheck-suppress unusedFunction
int core_application::subscribe(const char *topic)
{
//    mosquitto_publish(m_mosq, nullptr, topic, strlen(value), value, /*qos*/0, /*retain*/false);
    return mosquitto_subscribe(m_mosq, nullptr, topic, /*qos*/0);
}

int core_application::unsubscribe(const char *topic)
{
    return mosquitto_unsubscribe(m_mosq, nullptr, topic);
}

void core_application::on_mqtt_message(const struct mosquitto_message *message)
{
    auto it = std::find_if(m_topic_names.begin(), m_topic_names.end(),
                    [=](const std::string &s){
                        bool res = false;
                        if(mosquitto_topic_matches_sub(s.c_str(), 
                            message->topic, &res) == MOSQ_ERR_SUCCESS)
                        {
                            return res;
                        }   
                        return false;
                    });
    if (it != m_topic_names.end() )
    {
        mqtt_topic *topic = m_topics_data.at(*it);
        if(topic->init((uint8_t *)message->payload, message->payloadlen) == 0)
        {
            topic->emit(message->topic);
            return;
        }
    }
    
    on_mqtt_message_received(message);
}

int core_application::subscribe(mqtt_topic *topic)
{
    std::string name = topic->get_name();
    if ( std::find(m_topic_names.begin(),
                m_topic_names.end(),
                name) == m_topic_names.end() )
    {
        int ret = mosquitto_subscribe(m_mosq, /*&mid*/nullptr, name.c_str(), /*qos*/0);

        if(ret == MOSQ_ERR_SUCCESS)
        {
            m_topics_data[name] = topic;
            m_topic_names.push_back(name);
            return 0;
        }
    }
    return -1;
}

int core_application::unsubscribe(const mqtt_topic &topic)
{
    return unsubscribe(topic.get_name().c_str());
}
