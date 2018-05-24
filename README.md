# redis_publish_subscribe
简介

C++操作 redis 实现异步订阅和发布

功能

实现发布/订阅消息范式 ，消息的发送者（发布者）不是计划发送其消息给特定的接收者（订阅者）。而是发布的消息分为不同的类别，而不需要知道什么样的订阅者订阅。订阅者对一个或多个类别表达兴趣，于是只接收感兴趣的消息，而不需要知道什么样的发布者发布的消息。这种发布者和订阅者的[解耦](https://zh.wikipedia.org/wiki/%E8%80%A6%E5%90%88)可以允许更好的[可扩展性](https://zh.wikipedia.org/wiki/%E5%8F%AF%E6%89%A9%E5%B1%95%E6%80%A7)和更为动态的[网络拓扑](https://zh.wikipedia.org/wiki/%E7%BD%91%E7%BB%9C%E6%8B%93%E6%89%91)。(引自wikipedia)

运行效果

 ![](C:\Users\hao\Desktop\订阅者.png)

环境配置

Linux

Redis[数据库](https://baike.baidu.com/item/%E6%95%B0%E6%8D%AE%E5%BA%93) 

hiredis 库

libevent 库

C++11



快速教程 

1.下载源码

2.解压

3.进入源码目录，运行make命令，生成publisher、subscriber可执行程序

4.启动redis-server 

5.运行publisher发布者程序

6.运行subscriber订阅者程序







