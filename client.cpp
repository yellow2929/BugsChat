#define _CRT_SECURE_NO_WARNINGS 1
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <cstdio>
#include <urlmon.h>
#include <atlstr.h>

#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib, "urlmon.lib")
#define BUF_SIZE 1024


std::string sendmsg;
char recvmsg[BUF_SIZE];
bool flag;
std::string filepath;

static char* WIP = new char[MAX_PATH];
void GetInternetIP()
{
    //  CoInitialize(NULL);
    char buf[MAX_PATH] = { 0 };    //把网页中读出的数据放在此处
    char chTempIp[128] = { 0 };
    char chIP[64] = { 0 };        //最终存放IP在此
                                  //将网页数据写入c:\i.ini文件中
    printf("正在加载------\n");

    CString pURL = "https://api.ip.sb/ip";
    CString pFileName = "D:\\a.html";
    //  DeleteUrlCacheEntry((LPCWSTR)pURL);
    if (URLDownloadToFile(0, pURL, pFileName, 0, NULL) == S_OK)
    {
        printf("URLDownloadToFile OK\n");
    }
    else
    {
        printf("URLDownloadToFile Fail,Error:%d\n", GetLastError());
    }
    FILE* fp = fopen("D:\\a.html", "r");
    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_SET);
        fread(buf, 1, MAX_PATH, fp);
        fclose(fp);
    }
    memcpy(WIP, buf, strlen(buf) - 1);
    WIP[strlen(buf) - 1] = 0;
    //  WIP = buf;

    printf("当前主机公网IP: %s\n", WIP);
    remove("D:\\a.html");
    //  CoUninitialize();
    return;
}


#pragma once

/*
函数描述: 接收指定的字节数
函数参数:
    - fd: 通信的文件描述符(套接字)
    - buf: 存储待接收数据的内存的起始地址
    - size: 指定要接收的字节数
函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
*/
int readn(int fd, char* buf, int size)
{
    char* pt = buf;
    int count = size;
    while (count > 0)
    {
        int len = recv(fd, pt, count, 0);
        if (len == -1)
        {
            return -1;
        }
        else if (len == 0)
        {
            return size - count;
        }
        pt += len;
        count -= len;
    }
    return size;
}

/*
函数描述: 接收带数据头的数据包
函数参数:
    - cfd: 通信的文件描述符(套接字)
    - msg: 一级指针的地址，函数内部会给这个指针分配内存，用于存储待接收的数据，这块内存需要使用者释放
函数返回值: 函数调用成功返回接收的字节数, 发送失败返回-1
*/
int recvMsg(int cfd, char* msg)
{
    // 接收数据
    // 1. 读数据头
    int len = 0;
    readn(cfd, (char*)&len, 4);
    len = ntohl(len);

    // 根据读出的长度分配内存，+1 -> 这个字节存储\0
    char* buf = (char*)malloc(len);
    int ret = readn(cfd, buf, len);
    if (ret != len)
    {
        closesocket(cfd);
        free(buf);
        return -1;
    }
    
    memcpy(msg, buf, len);
    free(buf);
    return ret;
}

/*
函数描述: 发送指定的字节数
函数参数:
    - fd: 通信的文件描述符(套接字)
    - msg: 待发送的原始数据
    - size: 待发送的原始数据的总字节数
函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
*/
int writen(int fd, const char* msg, int size)
{
    const char* buf = msg;
    int count = size;
    while (count > 0)
    {
        int len = send(fd, buf, count, 0);
        if (len == -1)
        {
            closesocket(fd);
            return -1;
        }
        else if (len == 0)
        {
            continue;
        }
        buf += len;
        count -= len;
    }
    return size;
}

/*
函数描述: 发送带有数据头的数据包
函数参数:
    - cfd: 通信的文件描述符(套接字)
    - msg: 待发送的原始数据
    - len: 待发送的原始数据的总字节数
函数返回值: 函数调用成功返回发送的字节数, 发送失败返回-1
*/
int sendMsg(int cfd,const char* msg, int len)
{
    if (msg == NULL || len <= 0 || cfd <= 0)
    {
        return -1;
    }
    // 申请内存空间: 数据长度 + 包头4字节(存储数据长度)
    char* data = (char*)malloc(len + 4);
    int bigLen = htonl(len);
    memcpy(data, &bigLen, 4);
    memcpy(data + 4, msg, len);
    // 发送数据
    int ret = writen(cfd, data, len + 4);
    // 释放内存
    free(data);
    return ret;
}



unsigned Send_Msg(void* args)
{
    SOCKET sock = (SOCKET)args;
    while (true)
    {
        getline(std::cin,sendmsg);
        if (sendmsg == "quit")
        {
            printf("**已退出虫虫聊天室，欢迎下次使用**");
            closesocket(sock);
            exit(0);
        }
        
        std::vector<std::string> tokens;
        std::istringstream iss(sendmsg);

        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.empty()) continue;
        if (tokens[0] == "1")
        {
            if (tokens.size() >= 2)
            {
                printf("[我 to <public>]: %s \n", tokens[1].c_str());
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
            }
            else
                printf("发送消息为空！\n");
        }
        else if(tokens[0] == "2")
        {
            if (tokens.size() >= 3)
            {
                printf("[我 to <id=%s>]: %s \n", tokens[1].c_str(), tokens[2].c_str());
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
            }
            else
                printf("发送消息为空！\n");
        }
        else if(tokens[0] == "3")
        {
            sendMsg(sock, sendmsg.c_str(), sendmsg.size());
        }
        else if (tokens[0] == "4")
        {
            if (tokens.size() < 3)
            {
                printf("指令输入格式错误！\n");
            }
            else
            {
                std::string filename = tokens[1];
                FILE* read = fopen(filename.c_str(), "rb");
                if (!read)
                {
                    perror("file open failed:\n");//输出描述性错误信息
                    continue;
                }
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
                //获取文件大小
                fseek(read, 0, SEEK_END);	//将文件位置指针移动到最后
                std::cout << "文件大小:" << ftell(read) << "Byte" << "\n";
                int filesize = ftell(read);
                fseek(read, 0, SEEK_SET);	//将文件位置指针移动到开头


                //把文件读到内存中来
                char buffer[1024];

                int nCount;
                int ret = 0;
                Sleep(500);
                while (true)
                {
                    nCount = fread(buffer, 1, sizeof(buffer), read);//循环读取文件进行传送
                    if (nCount == 0)
                    {
                        Sleep(20);
                        sendMsg(sock, "y", 1);
                        break;
                    }
                    sendMsg(sock, "n", 1);
                    Sleep(20);
                    ret += sendMsg(sock, buffer, nCount);
                    std::cout << "已发送文件大小:" << " Byte:" << ret << "\n";
                    if (ret == SOCKET_ERROR)
                    {
                        std::perror("sendFile error!\n");
                        break;
                    }
                }
                fclose(read);
                std::cout << "文件发送成功!" << " Byte:" << ret << "\n";
            }
        }
        else if (tokens[0] == "5")
        {
            if (tokens.size() < 2)
            {
                printf("指令输入格式错误！\n");
            }
            else
            {
                flag = true;
                filepath = tokens[1];
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
            }
        }
        else
        {
            printf("!!未知指令!!\n");
        }
    }

    return 0;
}

unsigned Recv_Msg(void* args)
{
    SOCKET sock = (SOCKET)args;
    while (true)
    {
        
            int len = recvMsg(sock, recvmsg);
            if (len == -1)
            {
                printf("服务器接受消息失败\n");
                return 0;
            }
            recvmsg[len] = '\0';

            if (!flag)
            {
                printf("%s\n", recvmsg);
            }
            else
            {
                printf("%s\n", recvmsg);
                if (strcmp(recvmsg, "no") == 0)
                    printf("服务器该文件不存在\n");
                else
                {
                    printf("服务器该文件存在，文件开始下载\n");
                    FILE* write = fopen(filepath.c_str(), "wb");
                    if (!write)
                    {
                        perror("file write failed:\n");
                        return false;
                    }
                    char buffer[1024];
                    int ret = 0;
                    while (true)
                    {
                        recvMsg(sock, buffer);
                        if (buffer[0] == 'y')
                        {
                            break;
                        }
                        int nCount = recvMsg(sock, buffer);
                        ret += fwrite(buffer, 1, nCount, write);
                        if (ret == 0)
                        {
                            printf("服务器掉线!\n");
                        }
                        else if (ret < 0)
                        {
                            std::perror("recv error!\n");
                            return false;
                        }

                        std::cout << "已接受文件大小:" << " Byte:" << ret << "\n";
                    }

                    
                    printf("文件接受成功！\n");

                    fclose(write);
                    printf("文件大小为 Byte:%d\n", ret);
                }
                flag = false;
                
            }
    }


    return 0; 
}

int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return -1;
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        return -1;
    }

    /* The WinSock DLL is acceptable. Proceed. */

    SOCKET cSock;
    cSock = socket(AF_INET, SOCK_STREAM, 0);
    if (cSock < 0)
    {
        perror("socket create error !\n");
        return -1;
    }

    SOCKADDR_IN serv_adr;
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_port = htons(5555);
    inet_pton(AF_INET, "59.110.162.233", &serv_adr.sin_addr);

    //GetInternetIP();
    printf("客户端正在连接服务器(ip[59.110.162.233])中......0.0\n");
    if (connect(cSock, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
    {
        printf("connect error !\n%d", GetLastError());
        return -1;
    }
    else
    {
        printf("服务器连接成功0v0！\n");
        while (true)
        {
            printf("请输入数字进行您的操作：1 登陆 2 注册 3 注销\n");
            int flag;
            scanf("%d", &flag);
            std::string s = "";
            std::string tmp;
            if (flag == 1)
            {
                printf("请输入您的账号ID\n");
                std::string tmp;
                std::cin >> tmp;
                s += "1 " + tmp + " ";
                printf("请输入您的密码\n");
                std::cin >> tmp;
                s += tmp;

                sendMsg(cSock, s.c_str(),s.size());

                int len = recvMsg(cSock, recvmsg);
                if (len == -1)
                {
                    printf("服务器接受消息失败\n");
                    return 0;
                }
                recvmsg[len] = '\0';
                
                if (strcmp(recvmsg, "yes")==0)
                {
                    printf("登录成功!%s\n", recvmsg);
                    break;
                }
                else
                {
                    printf("登录失败：账号不存在或者密码错误!%s\n", recvmsg);
                    continue;
                }

            }
            else if (flag == 2)
            {
                printf("请输入您要注册的账号昵称\n");
                std::string tmp;
                std::cin >> tmp;
                s += "2 " + tmp + " ";
                printf("请输入您要注册的密码\n");
                std::cin >> tmp;
                s += tmp;

                send(cSock, s.c_str(), s.size(), 0);

                int len = recvMsg(cSock, recvmsg);
                if (len == -1)
                {
                    printf("服务器接受消息失败\n");
                    return 0;
                }
                recvmsg[len] = '\0';

                    printf("注册成功!\n");
                    printf("您的账号ID是 %s \n", recvmsg);
                    break;
            }
            else if (flag == 3)
            {
                printf("请输入您要注销的账号ID\n");
                std::string tmp;
                std::cin >> tmp;
                s += "3 " + tmp + " ";
                printf("请输入您要注销的账号密码\n");
                std::cin >> tmp;
                s += tmp;

                send(cSock, s.c_str(), s.size(), 0);

                int len = recvMsg(cSock, recvmsg);
                if (len == -1)
                {
                    printf("服务器接受消息失败\n");
                    return 0;
                }
                recvmsg[len] = '\0';

                if(strcmp(recvmsg, "yes") == 0)
                {
                    printf("注销成功!\n");
                }
                else
                {
                    printf("注销失败!没有此账号或密码错误!\n");
                }

                continue;
            }
            else
            {
                printf("未知符号！重新输入！");
                continue;
            }
        }
    }

    printf("----! 欢迎进入虫虫聊天室 !----\n");
    printf("----<开发者：H29>----\n");
    printf("输入 '1 <聊天内容>' 发送公聊消息\n");
    printf("输入 '2 <私聊的ID> <聊天内容>' 发送私聊消息\n");
    printf("输入 '3' 查看服务器可下载文件\n");
    printf("输入 '4 <本地文件路径> <上传后文件名称>' 上传文件\n");
    printf("输入 '5 <服务器文件名称>' 下载文件\n");
    printf("例如输入'2 1 helloworld'表示向id为1的用户发送私聊消息\n");
    printf("输入quit退出聊天室\n");
    printf("注意要加空格哦,发送的消息不能包含空格o.o\n");

    HANDLE hsend = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Send_Msg, (void *)cSock,0,NULL);
    HANDLE hrecv = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Recv_Msg, (void *)cSock,0,NULL);

    WaitForSingleObject(hsend, INFINITE);
    WaitForSingleObject(hrecv, INFINITE);

    closesocket(cSock);

    WSACleanup();
	return 0;
}