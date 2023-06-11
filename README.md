# BugsChat


注意：

linux服务端文件

ClientManager.cpp

ClientManager.h

server.cpp

change_encoding.sh


windows客户端文件

client.cpp





linux服务端需要安装mysql数据库，具体安装方法自行搜索

linux编译使用命令
g++ main.cpp ClientManager.cpp -o test `mysql_config --cflags --libs`



实现功能

1.登陆/注册/注销账号

2.文件上传和下载

3.公聊和私聊

4.识别txt文件编码自动转换为GB2312,windows系统可上传下载

5.可挂载在云服务器，实现局域网和远程通信

实现方法

1.利用epoll模型，搭建基本通信框架

2.利用linux c++连接mysql数据库，进行增删查改实现用户登陆/注册/注销账号

3.封装recvMsg和sendMsg函数，发送数据格式为数据大小+数据内容，解决tcp分包粘包问题

4.c++调用shell脚本，实现识别txt文件编码自动转换为GB2312

5.服务端文件上传下载利用多线程实现，防止阻塞



总开发时间5天左右，算是一个c++网络编程起步练习，下一步是研读那本非常经典的《 Linux多线程服务端编程：使用muduo C++网络库 》精进自己c++服务器编码水平

PS：啊咧，你不是测开嘛，怎么学起后端了（55555我也不想干测开，但是c++找不到工作了捏55555）
