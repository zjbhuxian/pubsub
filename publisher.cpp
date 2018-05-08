#include "redis_publisher.h"

/**
 * 实现发布消息
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[]) {
    redis_publisher publisher;

    bool ret = publisher.init();
    if (!ret) {
        printf("Init failed.\n");
        return 0;
    }

    ret = publisher.connect();
    if (!ret) {
        printf("connect failed.");
        return 0;
    }

    while (true) {
        publisher.publish("test-channel", "Hello World!");
        sleep(1);
    }

    publisher.disconnect();
    publisher.uninit();
    return 0;
}
