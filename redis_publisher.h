#ifndef REDIS_PUBLISHER_H
#define REDIS_PUBLISHER_H

#include <stdlib.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

/**
 * redis_publisher.h 封装了 hiredis，实现消息发布给 redis的功能
 */
class RedisPublisher {
public:
	RedisPublisher();

	~RedisPublisher();

	bool init();

	bool uninit();

	bool connect();

	bool disconnect();

	bool publish(const std::string &channel_name, const std::string &message);

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
};

#endif