#include "ClientManager.h"

ClientManager::ClientManager()
{
	con = mysql_init(NULL);

	if (!mysql_real_connect(con, host, user, pw, database_name, 0, NULL, 0))
	{
		std::cout << "Failed to conncet" << std::endl;
		exit(1);
	}
	
}

ClientManager::~ClientManager()
{
	mysql_close(con);
}

int ClientManager::insert_client(Client& cli)
{
	char sql[1024];
	sprintf(sql, "insert into users (name,password) values('%s','%s')",
		 cli.name.c_str(), cli.password.c_str());

	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to insert data : Error:%s\n", mysql_error(con));
		return -1;
	}
	mysql_query(con, "SELECT LAST_INSERT_ID()");
	MYSQL_RES* res = mysql_store_result(con);
    MYSQL_ROW row = mysql_fetch_row(res);
	return atoi(row[0]);
}

bool ClientManager::update_client(Client& cli)
{
	char sql[1024];
	sprintf(sql, "UPDATE users SET name ='%s',password = '%s'"
		"where id = %d", cli.name.c_str(), cli.password.c_str(),
		cli.id);
	
	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to update data : Error:%s\n", mysql_error(con));
		return false;
	}

	return true;
}

bool ClientManager::delete_client(int client_id, std::string password)
{
	char sql[1024];
	sprintf(sql, "SELECT * FROM users WHERE id=%d AND password='%s'", client_id, password);

	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to select data : Error:%s\n", mysql_error(con));
		return false;
	}

	MYSQL_RES* res = mysql_store_result(con);

	if (res == NULL) return false;
	mysql_free_result(res);
	res = NULL;

	char sql2[1024];
	sprintf(sql2, "DELETE FROM users WHERE id=%d", client_id);
	if (mysql_query(con, sql2))
	{
		fprintf(stderr, "Failed to delete data : Error:%s\n", mysql_error(con));
		return false;
	}

	return true;
}

vector<Client> ClientManager::get_clients(string condition)
{
	vector<Client> cliList;

	char sql[1024];
	sprintf(sql, "SELECT * FROM users %s ", condition.c_str());

	if (mysql_query(con, sql))
	{
		fprintf(stderr, "Failed to selete data : Error:%s\n", mysql_error(con));
		return {};
	}
	MYSQL_RES* res = mysql_store_result(con);

	MYSQL_ROW row;
	while ((row = mysql_fetch_row(res)))
	{
		Client cli;
		cli.id = atoi(row[0]);
		cli.name = row[1];
		cli.password = row[2];

		cliList.push_back(cli);

	}
	return cliList;
}
