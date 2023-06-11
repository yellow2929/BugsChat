#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sstream>
#include <map>
#include <dirent.h>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include "ClientManager.h"

const int MAX_CONN = 20;
int epid;
std::map<int, Client> client_info;
std::map<int, bool> client_online;
std::map<std::string, int> client_id_fd;
std::map<int, std::string> recv_fd_s;
std::map<int, std::string> send_fd_s;

std::vector<std::string> split_string(std::string s)
{
	std::vector<std::string> tokens;
	std::istringstream iss(s);

	std::string token;
	while (iss >> token) {
		tokens.push_back(token);
	}

	return tokens;
}

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

int recvMsg(int cfd, char* msg)
{

	int len = 0;
	readn(cfd, (char*)&len, 4);
	len = ntohl(len);

	char* buf = (char*)malloc(len);
	int ret = readn(cfd, buf, len);
	if (ret != len)
	{
		close(cfd);
		free(buf);
		return -1;
	}

	memcpy(msg, buf, len);
	free(buf);
	return ret;
}

int writen(int fd, const char* msg, int size)
{
	const char* buf = msg;
	int count = size;
	while (count > 0)
	{
		int len = send(fd, buf, count, 0);
		if (len == -1)
		{
			close(fd);
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

int sendMsg(int cfd,const char* msg, int len)
{
	if (msg == NULL || len <= 0 || cfd <= 0)
	{
		return -1;
	}
	char* data = (char*)malloc(len + 4);
	int bigLen = htonl(len);
	memcpy(data, &bigLen, 4);
	memcpy(data + 4, msg, len);
	int ret = writen(cfd, data, len + 4);
	free(data);
	return ret;
}

void client_login(int fd,std::string s, ClientManager* CM)
{
	std::vector<std::string> tokens = split_string(s);

	std::string id = tokens[1], password = tokens[2];
	vector<Client> ret = CM->get_clients("WHERE id=" + id + " AND password='" + password + "'");
	if (ret.size() > 0)
	{
		client_info[fd] = ret[0];
		client_online[fd] = true;
		client_id_fd[id] = fd;
		printf("%d login success ! id = %d", fd,ret[0].id);
		std::cout << " name = " << ret[0].name << '\n';
		sendMsg(fd, "yes", 3);
	}
	else
	{
		sendMsg(fd, "no", 2);
	}
}

void client_register(int fd,std::string s, ClientManager* CM)
{
	std::vector<std::string> tokens = split_string(s);

	std::string name = tokens[1], password = tokens[2];
	Client tmp;
	tmp.name = name;
	tmp.password = password;

	int tmp_id = CM->insert_client(tmp);

	if (tmp_id < 0)
	{
		printf("%d register failed !", fd);
	}
	else
	{
		printf("%d register success ! id = %d\n", fd, tmp_id);
		tmp.id = tmp_id;
		client_online[fd] = true;
		client_info[fd] = tmp;
		std::string tmp_id_s = std::to_string(tmp_id);
		client_id_fd[tmp_id_s] = fd;
		sendMsg(fd, tmp_id_s.c_str(), tmp_id_s.size());
	}

}

void client_delete(int fd, std::string s, ClientManager* CM)
{
	std::vector<std::string> tokens = split_string(s);

	int id = atoi(tokens[1].c_str());
	std::string password = tokens[2];

	if (CM->delete_client(id, password))
	{
		sendMsg(fd, "yes", 3);
	}
	else
	{
		sendMsg(fd, "no", 2);
	}
}

void public_send(int fd, std::string s)
{
	std::vector<std::string> tokens = split_string(s);
	string name = client_info[fd].name;
	for (auto& iter : client_info)
	{
		if (iter.first != fd && send_fd_s.find(iter.first) == send_fd_s.end())
		{
			sendMsg(iter.first, ("[<public>" + name + "]" + " : " + tokens[1]).c_str(), name.size() + tokens[1].size() + 13);
		}
	}
}

void private_send(int fd, std::string s)
{
	std::vector<std::string> tokens = split_string(s);


	if (client_id_fd.find(tokens[1]) == client_id_fd.end())
	{
		sendMsg(fd, "The user is not present or online!", 34);
	}
	else if (client_info[fd].id == atoi(tokens[1].c_str()))
	{
		sendMsg(fd, "Do not send message to yourself!", 32);
	}
	else
	{
		int send_fd = client_id_fd[tokens[1]];
		if (send_fd_s.find(send_fd) != send_fd_s.end())
		{
			sendMsg(fd, "The user is receiving file!", 27);
			return;
		}
		else
		{
			sendMsg(send_fd, ("[<private>" + client_info[fd].name + "]" + " : " + tokens[2]).c_str(), client_info[send_fd].name.size() + tokens[2].size() + 14);
		}
	}
}

void send_filesname(int fd)
{
	DIR* pDir;   
	struct dirent* ptr;
	if (!(pDir = opendir("/root/projects/server/files")))
	{
		sendMsg(fd, "server error!",13);
		return;
	}
	else
		sendMsg(fd, "get file_name success!", 22);
	while ((ptr = readdir(pDir)) != 0) {
		if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
			sendMsg(fd, ptr->d_name, strlen(ptr->d_name));
		}
	}
	closedir(pDir);
}

void change_encoding(std::string s)
{
	std::vector<std::string> tokens = split_string(s);

	int result = system(("./change_encoding.sh /root/projects/server/files/" + tokens[2]).c_str());

	if (result == -1)
		perror("µ÷ÓÃ½Å±¾Ê§°Ü");

}

void *recv_file(void *args)
{
	pthread_detach(pthread_self());
	int fd = (int)(long)args;
	std::string str = recv_fd_s[fd];
	std::vector<std::string> tokens = split_string(str);
	std::cout<<fd<<" send : "<< tokens[2]<< '\n';


	std::string filename = tokens[2];

	FILE* file = fopen(("/root/projects/server/files/"+filename).c_str(), "wb");
	if (file == nullptr) {
		std::cout << "Failed to open file." << std::endl;
		return NULL;
	}
	char buffer[1024];
	int ret = 0;
	ssize_t bytesRead;
	while (true)
	{
		recvMsg(fd, buffer);
		if (buffer[0] == 'y')
		{
			break;
		}
		bytesRead = recvMsg(fd, buffer);
		ret += bytesRead;
		fwrite(buffer, 1, bytesRead, file);
	}
	printf("file is received!size = %d Byte\n",ret);
	fclose(file);
	recv_fd_s.erase(fd);


	struct epoll_event ev_client;
	ev_client.events = EPOLLIN;
	ev_client.data.fd = fd;

	epoll_ctl(epid, EPOLL_CTL_ADD, fd, &ev_client);


	return NULL;
}

void *send_file(void* args)
{
	pthread_detach(pthread_self());
	int fd = (int)(long)args;
	std::string str = send_fd_s[fd];
	std::vector<std::string> tokens = split_string(str);
	std::cout << fd << " send : " << tokens[1] << '\n';

	std::string filepath = "/root/projects/server/files/" + tokens[1];

	FILE* file = fopen(filepath.c_str(), "rb");
	if (file == nullptr) {
		std::cout << "Failed to open file." << std::endl;
		sendMsg(fd, "no", 2);
		return NULL;
	}
	else
	{
		usleep(5000);
		sendMsg(fd, "yes", 3);
		char buffer[1024];

		int nCount;
		int ret = 0;
		usleep(50000);
		while (true)
		{
			nCount = fread(buffer, 1, sizeof(buffer), file);
			if (nCount == 0)
			{
				sendMsg(fd, "y", 1);
				break;
			}
			sendMsg(fd, "n", 1);
			usleep(500);
			ret += sendMsg(fd, buffer, nCount);
			usleep(500);
		}
		fclose(file);
		std::cout << "send file!" << " Byte:" << ret << "\n";
	}
	send_fd_s.erase(fd);
	return NULL;
}

int main()
{
	client_id_fd.clear();
	ClientManager* CM = ClientManager::GetInstance();
	epid = epoll_create1(0);
	if (epid < 0)
	{
		perror("epoll create error!\n");
		return -1;
	}

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		perror("socket create error!\n");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(5555);

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		perror("socket create error!\n");
		return -1;
	}

	if (listen(sockfd, 1024) < 0)
	{
		perror("listen create error!\n");
		return -1;
	}
	printf("server is listening\n");
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	if (epoll_ctl(epid, EPOLL_CTL_ADD, sockfd, &ev) < 0)
	{
		printf("server epoll_ctl error!\n");
		return -1;
	}
	while (true)
	{
		struct epoll_event evs[MAX_CONN];
		int n = epoll_wait(epid, evs, MAX_CONN, -1);

		if (n < 0)
		{
			printf("epoll_wait error!\n");
			break;
		}
		for (int i = 0; i < n; i++)
		{
			int fd = evs[i].data.fd;
			if (fd == sockfd)
			{
				struct sockaddr_in client_addr;
				socklen_t client_addr_len = sizeof(client_addr);
				int client_sockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);

				struct epoll_event ev_client;
				ev_client.events = EPOLLIN;
				ev_client.data.fd = client_sockfd;

				if (epoll_ctl(epid, EPOLL_CTL_ADD, client_sockfd, &ev_client) < 0)
				{
					printf("client epoll_ctl error!\n");
					return -1;
				}
				printf("%d is connected\n", client_sockfd);

				client_online[fd] = false;
			}
			else
			{
				char buffer[1024];
				int n = recvMsg(fd, buffer);

				if (n < 0) {
					printf("%d disconnect!\n",fd);
					close(fd);
					epoll_ctl(epid, EPOLL_CTL_DEL, fd, 0);
					std::string id = std::to_string(client_info[fd].id);
					client_id_fd.erase(id);
					client_info.erase(fd);
					client_online.erase(fd);
					break;
				}
				else if (n == 0)
				{
					printf("%d disconnect!\n", fd);
					close(fd);
					epoll_ctl(epid, EPOLL_CTL_DEL, fd, 0);
					std::string id = std::to_string(client_info[fd].id);
					client_id_fd.erase(id);
					client_info.erase(fd);
					client_online.erase(fd);
					break;
				}
				else
				{
					std::string msg(buffer, n);
					printf("%d sends a message! \n", fd);
					std::cout << msg << '\n';
					if (!client_online[fd])
					{
						switch (msg[0])
						{
							case '1':
								client_login(fd,msg,CM);
								break;
							case '2':
								client_register(fd,msg,CM);
								break;
							case '3':
								client_delete(fd,msg,CM);
								break;
							default:
								break;
						}
					}
					else
					{
						std::string msg(buffer, n);
						switch (msg[0])
						{
						case '1':
							public_send(fd, msg);
							break;
						case '2':
							private_send(fd, msg);
							break;
						case '3':
							send_filesname(fd);
							break;
						case '4':
							recv_fd_s[fd] = msg;
							epoll_ctl(epid, EPOLL_CTL_DEL, fd, 0);
							pthread_t pthid0;
							pthread_create(&pthid0, NULL, recv_file, (void*)(long)fd);
							change_encoding(msg);
							break;
						case '5':
							send_fd_s[fd] = msg;
							pthread_t pthid1;
							pthread_create(&pthid1, NULL, send_file, (void*)(long)fd);
							break;
						default:
							break;
						}

					
					}
				}
			}
		}
	}


	close(sockfd);
	close(epid);
	return 0;

}
