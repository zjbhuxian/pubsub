#ifndef REDIS_PUBLISH_SUBSCRIBE_CREDISPUBLISHER_H
#define REDIS_PUBLISH_SUBSCRIBE_CREDISPUBLISHER_H

#include <stdlib.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <boost/functional.hpp>

class redis_publisher {
public:
    redis_publisher();

    ~redis_publisher();

    bool init();

    bool uninit();

    bool connect();

    bool disconnect();

    bool publish(const std::string &channel_name,
                 const std::string &message);

private:
    static void connect_callback(const redisAsyncContext *redis_context,
                                 int status);

    static void disconnect_callback(const redisAsyncContext *redis_context,
                                    int status);

    static void command_callback(redisAsyncContext *redis_context,
                                 void *reply,
                                 void *privdata);

    static void *event_thread(void *data);

    void *event_proc();

private:
    event_base *_event_base;            // libevent事件对象
    pthread_t _event_thread;            // 事件线程ID
    sem_t _event_sem;                   // 事件线程的信号量
    redisAsyncContext *_redis_context;  // hiredis异步对象

};


#endif //REDIS_PUBLISH_SUBSCRIBE_CREDISPUBLISHER_H
