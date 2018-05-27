#ifndef REDIS_SUBSCRIBER_H
#define REDIS_SUBSCRIBER_H

#include <stdlib.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <functional>
#include <iostream>

/**
 * 封装 hiredis， 实现消息订阅 redis 功能
 */
class RedisSubscriber {
public:
    typedef std::function<void(const char *, const char *, int)> NotifyMessageFn;    // 回调函数对象类型，当接收到消息后调用回调把消息发送出去

    RedisSubscriber();

    ~RedisSubscriber();

    bool init(const NotifyMessageFn &fn);    // 传入回调对象
    bool uninit();

    bool connect();

    bool disconnect();

    // 可以多次调用，订阅多个频道
    bool subscribe(const std::string &channel_name);

private:
    // 下面三个回调函数供redis服务调用
    // 连接回调
    static void connect_callback(const redisAsyncContext *redis_context, int status);

    // 断开连接的回调
    static void disconnect_callback(const redisAsyncContext *redis_context, int status);

    // 执行命令回调
    static void command_callback(redisAsyncContext *redis_context, void *reply, void *privdata);

    // 事件分发线程函数
    static void *event_thread(void *data);

    void *event_proc();

private:
    // libevent事件对象
    event_base *eventBase;
    // 事件线程ID
    pthread_t eventThread;
    // 事件线程的信号量
    sem_t eventSem;
    // hiredis异步对象
    redisAsyncContext *context;

    // 通知外层的回调函数对象
    NotifyMessageFn notifyMessageFn;
};

#endif