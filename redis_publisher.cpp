#include <stddef.h>
#include <assert.h>
#include <string.h>
#include "redis_publisher.h"

RedisPublisher::RedisPublisher() : _event_base(0), _event_thread(0),
                                   _redis_context(0) {
}

RedisPublisher::~RedisPublisher() {
}

bool RedisPublisher::init() {
	// 初始化事件处理
	_event_base = event_base_new();     // 初始化事件处理，并获得事件处理的实例
	if (NULL == _event_base) {
		std::cout << "Create redis event failed." << std::endl;
		return false;
	}

	memset(&_event_sem, 0, sizeof(_event_sem));
	int ret = sem_init(&_event_sem, 0, 0);      //初始化信号量
	if (ret != 0) {
		std::cout << "Init sem failed." << std::endl;
		return false;
	}

	return true;
}

bool RedisPublisher::uninit() {
	_event_base = NULL;

	sem_destroy(&_event_sem);
	return true;
}

bool RedisPublisher::connect() {
	// connect redis
	_redis_context = redisAsyncConnect("127.0.0.1", 6379);    // 异步连接到redis服务器上，使用默认端口
	if (NULL == _redis_context) {
		std::cout << "Connect redis failed." << std::endl;
		return false;
	}

	if (_redis_context->err) {
		// 输出错误信息
		std::cout << "Connect redis error: " << _redis_context->err << _redis_context->errstr << std::endl;
		return false;
	}

	// attach the event
	redisLibeventAttach(_redis_context, _event_base);    // 将事件绑定到redis context上，使设置给redis的回调跟事件关联

	// 创建事件处理线程
	int ret = pthread_create(&_event_thread, 0, &RedisPublisher::event_thread, this);
	if (ret != 0) {
		std::cout << "Create event thread failed." << std::endl;
		disconnect();
		return false;
	}

	// 设置连接回调，当异步调用连接后，服务器处理连接请求结束后调用，通知调用者连接的状态
	redisAsyncSetConnectCallback(_redis_context,
	                             &RedisPublisher::connect_callback);

	// 设置断开连接回调，当服务器断开连接后，通知调用者连接断开，调用者可以利用这个函数实现重连
	redisAsyncSetDisconnectCallback(_redis_context,
	                                &RedisPublisher::disconnect_callback);

	// 启动事件线程
	sem_post(&_event_sem);
	return true;
}

bool RedisPublisher::disconnect() {
	if (_redis_context) {
		redisAsyncDisconnect(_redis_context);
		redisAsyncFree(_redis_context);
		_redis_context = NULL;
	}

	return true;
}

bool RedisPublisher::publish(const std::string &channel_name,
                              const std::string &message) {
	int ret = redisAsyncCommand(_redis_context,
	                            &RedisPublisher::command_callback, this, "PUBLISH %s %s",
	                            channel_name.c_str(), message.c_str());
	if (REDIS_ERR == ret) {
		std::cout << "Publish command failed: " << ret << std::endl;
		return false;
	}

	return true;
}

void RedisPublisher::connect_callback(const redisAsyncContext *redis_context,
                                       int status) {
	if (status != REDIS_OK) {
		std::cout << "Error: " << redis_context->errstr << std::endl;
	} else {
		std::cout << "Redis connected!" << std::endl;
	}
}

void RedisPublisher::disconnect_callback(
		const redisAsyncContext *redis_context, int status) {
	if (status != REDIS_OK) {
		// 这里异常退出，可以尝试重连
		std::cout << "Error: " << redis_context->errstr << std::endl;
	}
}

// 消息接收回调函数
void RedisPublisher::command_callback(redisAsyncContext *redis_context,
                                      void *reply, void *privdata) {
	//printf("command callback.\n");
	// 这里不执行任何操作
}

void *RedisPublisher::event_thread(void *data) {
	if (NULL == data) {
		std::cout << "Error!" << std::endl;
		assert(false);
		return NULL;
	}

	//reinterpret_cast运算符是用来处理无关类型之间的转换；它会产生一个新的值，这个值会有与原始参数（expressoin）有完全相同的比特位
	RedisPublisher *self_this = reinterpret_cast<RedisPublisher *>(data);
	return self_this->event_proc();
}

void *RedisPublisher::event_proc() {
	sem_wait(&_event_sem);

	// 开启事件分发，event_base_dispatch会阻塞
	event_base_dispatch(_event_base);

	return NULL;
}