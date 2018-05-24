#include "redis_publisher.h"

/**
 * 实现发布消息
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    RedisPublisher publisher;

    bool ret = publisher.init();
    if (!ret) {
        std::cout << "Init failed." << std::endl;
        return 0;
    }

    ret = publisher.connect();
    if (!ret) {
        std::cout << "connect failed." << std::endl;
        return 0;
    }

    while (true) {
        //设置频道名称，频道内容
        publisher.publish("每日报道", "欢迎订阅每日报道的新闻!");
        sleep(1);
    }

    publisher.disconnect();
    publisher.uninit();
    return 0;
}