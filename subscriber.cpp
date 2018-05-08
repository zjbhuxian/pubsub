#include "redis_subscriber.h"

void recieve_message(const char *channel_name,
                     const char *message, int len) {
    printf("Recieve message:\n    channel name: %s\n    message: %s\n",
           channel_name, message);
}

int main(int argc, char *argv[]) {
    redis_subscriber subscriber;
    redis_subscriber::NotifyMessageFn fn =
            bind(recieve_message, std::placeholders::_1,
                 std::placeholders::_2, std::placeholders::_3);

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