#include "redis_subscriber.h"

void recieve_message(const char *channel_name,
                     const char *message, int len) {
    printf("订阅信息:\n    频道名称: %s\n    来自频道的内容: %s\n",
           channel_name, message);
}

/**
 * 实现消息的订阅
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    RedisSubscriber subscriber;
    RedisSubscriber::NotifyMessageFn fn =
            bind(recieve_message, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3);

    bool ret = subscriber.init(fn);
    if (!ret) {
        std::cout << "Init failed." << std::endl;
        return 0;
    }

    ret = subscriber.connect();
    if (!ret) {
        std::cout << "connect failed." << std::endl;
        return 0;
    }

    subscriber.subscribe("每日报道");

    while (true) {
        sleep(1);
    }

    subscriber.disconnect();
    subscriber.uninit();

    return 0;
}