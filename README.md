# pubsub
### 项目介绍

pubsub是一个基于C++操作 redis 实现异步订阅和发布的项目，它是一个学习性质的Demo项目，用于实现发布/订阅消息范式 。

### 运行效果

![https://github.com/LinHaoo/resources/blob/master/pubsub.png]()

订阅客户端接受来自发布服务端的的内容示意图

### 运行环境

ubuntu，redis，hiredis ，libevent

### 快速教程 

1.下载源码

2.解压

3.进入源码目录，运行make命令，生成publisher、subscriber可执行程序

4.启动redis-server 

5.运行publisher发布者程序

6.运行subscriber订阅者程序

### 项目结构

pubsub

​	--publisher.cpp		//实现发布消息的发布主程序

​	--redis_publisher.cpp	

​	--redis_publisher.h	//封装了 hiredis，实现消息发布给 redis的功能

​	--subscriber.cpp		//实现订阅消息的订阅主程序

​	--redis_subscriber.cpp

​	---redis_subscriber.h	//封装 hiredis， 实现消息订阅 redis 功能

### 版权信息 

暂无









