#pragma once
#include<mysql.h>
#include<iostream>
#include<string>
#include<vector>
using namespace std;
typedef struct Client
{
	int id;
	string name;
	string password;
	bool operator==(const struct Client& W)
	{
		return W.id == this->id
			&& W.name == this->name
			&& W.password == this->password;
	}
}Client;

class ClientManager
{
	ClientManager();
	~ClientManager();
public :
	static ClientManager* GetInstance()
	{
		static ClientManager ClientManager_Instance;
		return &ClientManager_Instance;
	}
public:
	int insert_client(Client& t);
	bool update_client(Client& t);
	bool delete_client(int client_id,std::string password);
	vector<Client> get_clients(string condition ="");
private:
	MYSQL* con;
	const char* host = "localhost";
	const char* user = "root";
	const char* pw = "123456";
	const char* database_name = "database_client";
};

