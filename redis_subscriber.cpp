#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "redis_subscriber.h"

CRedisSubscriber::CRedisSubscriber() : _event_base(0), _event_thread(0),
                                       _redis_context(0) {
}

CRedisSubscriber::~CRedisSubscriber() {
}

bool CRedisSubscriber::init(const NotifyMessageFn &fn) {
    // initialize the event
    _notify_message_fn = fn;
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

bool CRedisSubscriber::uninit() {
    _event_base = NULL;

    sem_destroy(&_event_sem);
    return true;
}

bool CRedisSubscriber::connect() {
    // connect redis
    _redis_context = redisAsyncConnect("127.0.0.1", 6379);    // 异步连接到redis服务器上，使用默认端口
    if (NULL == _redis_context) {
        printf(": Connect redis failed.\n");
        return false;
    }

    if (_redis_context->err) {
        printf(": Connect redis error: %d, %s\n",
               _redis_context->err, _redis_context->errstr);    // 输出错误信息
        return false;
    }

    // attach the event
    redisLibeventAttach(_redis_context, _event_base);    // 将事件绑定到redis context上，使设置给redis的回调跟事件关联

    // 创建事件处理线程
    int ret = pthread_create(&_event_thread, 0, &CRedisSubscriber::event_thread, this);
    if (ret != 0) {
        printf(": create event thread failed.\n");
        disconnect();
        return false;
    }

    // 设置连接回调，当异步调用连接后，服务器处理连接请求结束后调用，通知调用者连接的状态
    redisAsyncSetConnectCallback(_redis_context,
                                 &CRedisSubscriber::connect_callback);

    // 设置断开连接回调，当服务器断开连接后，通知调用者连接断开，调用者可以利用这个函数实现重连
    redisAsyncSetDisconnectCallback(_redis_context,
                                    &CRedisSubscriber::disconnect_callback);

    // 启动事件线程
    sem_post(&_event_sem);
    return true;
}

bool CRedisSubscriber::disconnect() {
    if (_redis_context) {
        redisAsyncDisconnect(_redis_context);
        redisAsyncFree(_redis_context);
        _redis_context = NULL;
    }

    return true;
}

bool CRedisSubscriber::subscribe(const std::string &channel_name) {
    int ret = redisAsyncCommand(_redis_context,
                                &CRedisSubscriber::command_callback, this, "SUBSCRIBE %s",
                                channel_name.c_str());
    if (REDIS_ERR == ret) {
        printf("Subscribe command failed: %d\n", ret);
        return false;
    }

    printf(": Subscribe success: %s\n", channel_name.c_str());
    return true;
}

void CRedisSubscriber::connect_callback(const redisAsyncContext *redis_context,
                                        int status) {
    if (status != REDIS_OK) {
        printf(": Error: %s\n", redis_context->errstr);
    } else {
        printf(": Redis connected!");
    }
}

void CRedisSubscriber::disconnect_callback(
        const redisAsyncContext *redis_context, int status) {
    if (status != REDIS_OK) {
        // 这里异常退出，可以尝试重连
        printf(": Error: %s\n", redis_context->errstr);
    }
}

// 消息接收回调函数
void CRedisSubscriber::command_callback(redisAsyncContext *redis_context,
                                        void *reply, void *privdata) {
    if (NULL == reply || NULL == privdata) {
        return;
    }

    // 静态函数中，要使用类的成员变量，把当前的this指针传进来，用this指针间接访问
    CRedisSubscriber *self_this = reinterpret_cast<CRedisSubscriber *>(privdata);
    redisReply *redis_reply = reinterpret_cast<redisReply *>(reply);

    // 订阅接收到的消息是一个带三元素的数组
    if (redis_reply->type == REDIS_REPLY_ARRAY &&
        redis_reply->elements == 3) {
        printf(": Recieve message:%s:%d:%s:%d:%s:%d\n",
               redis_reply->element[0]->str, redis_reply->element[0]->len,
               redis_reply->element[1]->str, redis_reply->element[1]->len,
               redis_reply->element[2]->str, redis_reply->element[2]->len);

        // 调用函数对象把消息通知给外层
        self_this->_notify_message_fn(redis_reply->element[1]->str,
                                      redis_reply->element[2]->str, redis_reply->element[2]->len);
    }
}

void *CRedisSubscriber::event_thread(void *data) {
    if (NULL == data) {
        printf(": Error!\n");
        assert(false);
        return NULL;
    }

    CRedisSubscriber *self_this = reinterpret_cast<CRedisSubscriber *>(data);
    return self_this->event_proc();
}

void *CRedisSubscriber::event_proc() {
    sem_wait(&_event_sem);

    // 开启事件分发，event_base_dispatch会阻塞
    event_base_dispatch(_event_base);

    return NULL;
}