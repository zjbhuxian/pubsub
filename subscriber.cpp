#include "redis_subscriber.h"

void recieve_message(const char *channel_name,
                     const char *message, int len) {
    printf("Recieve message:\n    channel name: %s\n    message: %s\n",
           channel_name, message);
}

/**
 * 实现消息的订阅
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    CRedisSubscriber subscriber;
    CRedisSubscriber::NotifyMessageFn fn =
            bind(recieve_message, std::tr1::placeholders::_1,
                 std::tr1::placeholders::_2, std::tr1::placeholders::_3);

    bool ret = subscriber.init(fn);
    if (!ret) {
        printf("Init failed.\n");
        return 0;
    }

    ret = subscriber.connect();
    if (!ret) {
        printf("Connect failed.\n");
        return 0;
    }

    subscriber.subscribe("test-channel");

    while (true) {
        sleep(1);
    }

    subscriber.disconnect();
    subscriber.uninit();

    return 0;
}