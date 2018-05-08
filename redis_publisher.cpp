#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <iostream>

#include "redis_publisher.h"

redis_publisher::redis_publisher() : _event_base(0), _event_thread(0),
                                     _redis_context(0) {
}

redis_publisher::~redis_publisher() {
}

bool redis_publisher::init() {
    // initialize the event
    _event_base = event_base_new();    // 创建libevent对象
    if (NULL == _event_base) {
        printf(": Create redis event failed.\n");
        return false;
    }

    memset(&_event_sem, 0, sizeof(_event_sem));
    int ret = sem_init(&_event_sem, 0, 0);
    if (ret != 0) {
        printf(": Init sem failed.\n");
        return false;
    }

    return true;
}

bool redis_publisher::uninit() {
    _event_base = NULL;

    sem_destroy(&_event_sem);
    return true;
}

bool redis_publisher::connect() {
    // 异步连接到redis服务器上，使用默认端口
    _redis_context = redisAsyncConnect("127.0.0.1", 6379);
    if (NULL == _redis_context) {
        printf(": Connect redis failed.\n");
        return false;
    }

    if (_redis_context->err) {
        printf(": Connect redis error: %d, %s\n",
               _redis_context->err, _redis_context->errstr);    // 输出错误信息
        return false;
    }

    // 将事件绑定到redis context上，使设置给redis的回调跟事件关联
    redisLibeventAttach(_redis_context, _event_base);

    // 创建事件处理线程
    int ret = pthread_create(&_event_thread, 0, &redis_publisher::event_thread, this);
    if (ret != 0) {
        printf(": create event thread failed.\n");
        disconnect();
        return false;
    }

    // 设置连接回调，当异步调用连接后，服务器处理连接请求结束后调用，通知调用者连接的状态
    redisAsyncSetConnectCallback(_redis_context,
                                 &redis_publisher::connect_callback);

    // 设置断开连接回调，当服务器断开连接后，通知调用者连接断开，调用者可以利用这个函数实现重连
    redisAsyncSetDisconnectCallback(_redis_context,
                                    &redis_publisher::disconnect_callback);

    sem_post(&_event_sem);
    return true;
}

bool redis_publisher::disconnect() {
    if (_redis_context) {
        redisAsyncDisconnect(_redis_context);
        redisAsyncFree(_redis_context);
        _redis_context = NULL;
    }

    return true;
}

bool redis_publisher::publish(const std::string &channel_name,
                              const std::string &message) {
    int ret = redisAsyncCommand(_redis_context,
                                &redis_publisher::command_callback, this, "PUBLISH %s %s",
                                channel_name.c_str(), message.c_str());
    if (REDIS_ERR == ret) {
        printf("Publish command failed: %d\n", ret);
        return false;
    }

    return true;
}

void redis_publisher::connect_callback(const redisAsyncContext *redis_context,
                                       int status) {
    if (status != REDIS_OK) {
        printf(": Error: %s\n", redis_context->errstr);
    } else {
        printf(": Redis connected!\n");
    }
}

void redis_publisher::disconnect_callback(
        const redisAsyncContext *redis_context, int status) {
    if (status != REDIS_OK) {
        // 这里异常退出，可以尝试重连
        printf(": Error: %s\n", redis_context->errstr);
    }
}

// 消息接收回调函数
void redis_publisher::command_callback(redisAsyncContext *redis_context,
                                       void *reply, void *privdata) {
    printf("command callback.\n");
    // 这里不执行任何操作
}

void *redis_publisher::event_thread(void *data) {
    if (NULL == data) {
        printf(": Error!\n");
        assert(false);
        return NULL;
    }

    redis_publisher *self_this = reinterpret_cast<redis_publisher *>(data);
    return self_this->event_proc();
}

void *redis_publisher::event_proc() {
    sem_wait(&_event_sem);

    // 开启事件分发，event_base_dispatch会阻塞
    event_base_dispatch(_event_base);

    return NULL;
}
