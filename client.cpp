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
    char buf[MAX_PATH] = { 0 };    //����ҳ�ж��������ݷ��ڴ˴�
    char chTempIp[128] = { 0 };
    char chIP[64] = { 0 };        //���մ��IP�ڴ�
                                  //����ҳ����д��c:\i.ini�ļ���
    printf("���ڼ���------\n");

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

    printf("��ǰ��������IP: %s\n", WIP);
    remove("D:\\a.html");
    //  CoUninitialize();
    return;
}


#pragma once

/*
��������: ����ָ�����ֽ���
��������:
    - fd: ͨ�ŵ��ļ�������(�׽���)
    - buf: �洢���������ݵ��ڴ����ʼ��ַ
    - size: ָ��Ҫ���յ��ֽ���
��������ֵ: �������óɹ����ط��͵��ֽ���, ����ʧ�ܷ���-1
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
��������: ���մ�����ͷ�����ݰ�
��������:
    - cfd: ͨ�ŵ��ļ�������(�׽���)
    - msg: һ��ָ��ĵ�ַ�������ڲ�������ָ������ڴ棬���ڴ洢�����յ����ݣ�����ڴ���Ҫʹ�����ͷ�
��������ֵ: �������óɹ����ؽ��յ��ֽ���, ����ʧ�ܷ���-1
*/
int recvMsg(int cfd, char* msg)
{
    // ��������
    // 1. ������ͷ
    int len = 0;
    readn(cfd, (char*)&len, 4);
    len = ntohl(len);

    // ���ݶ����ĳ��ȷ����ڴ棬+1 -> ����ֽڴ洢\0
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
��������: ����ָ�����ֽ���
��������:
    - fd: ͨ�ŵ��ļ�������(�׽���)
    - msg: �����͵�ԭʼ����
    - size: �����͵�ԭʼ���ݵ����ֽ���
��������ֵ: �������óɹ����ط��͵��ֽ���, ����ʧ�ܷ���-1
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
��������: ���ʹ�������ͷ�����ݰ�
��������:
    - cfd: ͨ�ŵ��ļ�������(�׽���)
    - msg: �����͵�ԭʼ����
    - len: �����͵�ԭʼ���ݵ����ֽ���
��������ֵ: �������óɹ����ط��͵��ֽ���, ����ʧ�ܷ���-1
*/
int sendMsg(int cfd,const char* msg, int len)
{
    if (msg == NULL || len <= 0 || cfd <= 0)
    {
        return -1;
    }
    // �����ڴ�ռ�: ���ݳ��� + ��ͷ4�ֽ�(�洢���ݳ���)
    char* data = (char*)malloc(len + 4);
    int bigLen = htonl(len);
    memcpy(data, &bigLen, 4);
    memcpy(data + 4, msg, len);
    // ��������
    int ret = writen(cfd, data, len + 4);
    // �ͷ��ڴ�
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
            printf("**���˳���������ң���ӭ�´�ʹ��**");
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
                printf("[�� to <public>]: %s \n", tokens[1].c_str());
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
            }
            else
                printf("������ϢΪ�գ�\n");
        }
        else if(tokens[0] == "2")
        {
            if (tokens.size() >= 3)
            {
                printf("[�� to <id=%s>]: %s \n", tokens[1].c_str(), tokens[2].c_str());
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
            }
            else
                printf("������ϢΪ�գ�\n");
        }
        else if(tokens[0] == "3")
        {
            sendMsg(sock, sendmsg.c_str(), sendmsg.size());
        }
        else if (tokens[0] == "4")
        {
            if (tokens.size() < 3)
            {
                printf("ָ�������ʽ����\n");
            }
            else
            {
                std::string filename = tokens[1];
                FILE* read = fopen(filename.c_str(), "rb");
                if (!read)
                {
                    perror("file open failed:\n");//��������Դ�����Ϣ
                    continue;
                }
                sendMsg(sock, sendmsg.c_str(), sendmsg.size());
                //��ȡ�ļ���С
                fseek(read, 0, SEEK_END);	//���ļ�λ��ָ���ƶ������
                std::cout << "�ļ���С:" << ftell(read) << "Byte" << "\n";
                int filesize = ftell(read);
                fseek(read, 0, SEEK_SET);	//���ļ�λ��ָ���ƶ�����ͷ


                //���ļ������ڴ�����
                char buffer[1024];

                int nCount;
                int ret = 0;
                Sleep(500);
                while (true)
                {
                    nCount = fread(buffer, 1, sizeof(buffer), read);//ѭ����ȡ�ļ����д���
                    if (nCount == 0)
                    {
                        Sleep(20);
                        sendMsg(sock, "y", 1);
                        break;
                    }
                    sendMsg(sock, "n", 1);
                    Sleep(20);
                    ret += sendMsg(sock, buffer, nCount);
                    std::cout << "�ѷ����ļ���С:" << " Byte:" << ret << "\n";
                    if (ret == SOCKET_ERROR)
                    {
                        std::perror("sendFile error!\n");
                        break;
                    }
                }
                fclose(read);
                std::cout << "�ļ����ͳɹ�!" << " Byte:" << ret << "\n";
            }
        }
        else if (tokens[0] == "5")
        {
            if (tokens.size() < 2)
            {
                printf("ָ�������ʽ����\n");
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
            printf("!!δָ֪��!!\n");
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
                printf("������������Ϣʧ��\n");
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
                    printf("���������ļ�������\n");
                else
                {
                    printf("���������ļ����ڣ��ļ���ʼ����\n");
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
                            printf("����������!\n");
                        }
                        else if (ret < 0)
                        {
                            std::perror("recv error!\n");
                            return false;
                        }

                        std::cout << "�ѽ����ļ���С:" << " Byte:" << ret << "\n";
                    }

                    
                    printf("�ļ����ܳɹ���\n");

                    fclose(write);
                    printf("�ļ���СΪ Byte:%d\n", ret);
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
    printf("�ͻ����������ӷ�����(ip[59.110.162.233])��......0.0\n");
    if (connect(cSock, (SOCKADDR*)&serv_adr, sizeof(serv_adr)) == SOCKET_ERROR)
    {
        printf("connect error !\n%d", GetLastError());
        return -1;
    }
    else
    {
        printf("���������ӳɹ�0v0��\n");
        while (true)
        {
            printf("���������ֽ������Ĳ�����1 ��½ 2 ע�� 3 ע��\n");
            int flag;
            scanf("%d", &flag);
            std::string s = "";
            std::string tmp;
            if (flag == 1)
            {
                printf("�����������˺�ID\n");
                std::string tmp;
                std::cin >> tmp;
                s += "1 " + tmp + " ";
                printf("��������������\n");
                std::cin >> tmp;
                s += tmp;

                sendMsg(cSock, s.c_str(),s.size());

                int len = recvMsg(cSock, recvmsg);
                if (len == -1)
                {
                    printf("������������Ϣʧ��\n");
                    return 0;
                }
                recvmsg[len] = '\0';
                
                if (strcmp(recvmsg, "yes")==0)
                {
                    printf("��¼�ɹ�!%s\n", recvmsg);
                    break;
                }
                else
                {
                    printf("��¼ʧ�ܣ��˺Ų����ڻ����������!%s\n", recvmsg);
                    continue;
                }

            }
            else if (flag == 2)
            {
                printf("��������Ҫע����˺��ǳ�\n");
                std::string tmp;
                std::cin >> tmp;
                s += "2 " + tmp + " ";
                printf("��������Ҫע�������\n");
                std::cin >> tmp;
                s += tmp;

                send(cSock, s.c_str(), s.size(), 0);

                int len = recvMsg(cSock, recvmsg);
                if (len == -1)
                {
                    printf("������������Ϣʧ��\n");
                    return 0;
                }
                recvmsg[len] = '\0';

                    printf("ע��ɹ�!\n");
                    printf("�����˺�ID�� %s \n", recvmsg);
                    break;
            }
            else if (flag == 3)
            {
                printf("��������Ҫע�����˺�ID\n");
                std::string tmp;
                std::cin >> tmp;
                s += "3 " + tmp + " ";
                printf("��������Ҫע�����˺�����\n");
                std::cin >> tmp;
                s += tmp;

                send(cSock, s.c_str(), s.size(), 0);

                int len = recvMsg(cSock, recvmsg);
                if (len == -1)
                {
                    printf("������������Ϣʧ��\n");
                    return 0;
                }
                recvmsg[len] = '\0';

                if(strcmp(recvmsg, "yes") == 0)
                {
                    printf("ע���ɹ�!\n");
                }
                else
                {
                    printf("ע��ʧ��!û�д��˺Ż��������!\n");
                }

                continue;
            }
            else
            {
                printf("δ֪���ţ��������룡");
                continue;
            }
        }
    }

    printf("----! ��ӭ������������ !----\n");
    printf("----<�����ߣ�H29>----\n");
    printf("���� '1 <��������>' ���͹�����Ϣ\n");
    printf("���� '2 <˽�ĵ�ID> <��������>' ����˽����Ϣ\n");
    printf("���� '3' �鿴�������������ļ�\n");
    printf("���� '4 <�����ļ�·��> <�ϴ����ļ�����>' �ϴ��ļ�\n");
    printf("���� '5 <�������ļ�����>' �����ļ�\n");
    printf("��������'2 1 helloworld'��ʾ��idΪ1���û�����˽����Ϣ\n");
    printf("����quit�˳�������\n");
    printf("ע��Ҫ�ӿո�Ŷ,���͵���Ϣ���ܰ����ո�o.o\n");

    HANDLE hsend = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Send_Msg, (void *)cSock,0,NULL);
    HANDLE hrecv = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Recv_Msg, (void *)cSock,0,NULL);

    WaitForSingleObject(hsend, INFINITE);
    WaitForSingleObject(hrecv, INFINITE);

    closesocket(cSock);

    WSACleanup();
	return 0;
}