#include <hiredis/hiredis.h>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
    struct timeval timeout = {2, 0};

    redisContext *pRedisContext = (redisContext *) redisConnectWithTimeout("127.0.0.1", 6379, timeout);

    if ((NULL == pRedisContext) || (pRedisContext->err)) {
        if (pRedisContext) {
            std::cout << "connect error:" << pRedisContext->errstr << std::endl;
        } else {
            std::cout << "connect error: can't allocate redis context." << std::endl;
        }
        return -1;
    }

    redisReply *pRedisReply = (redisReply *) redisCommand(pRedisContext, "INFO");

    std::cout << pRedisReply->str << std::endl;

    freeReplyObject(pRedisReply);


    return 0;
}