#pragma once
#include <fstream>

char endNull = '\0';

const char* data_id[] = {"abc1", "abc2", "abc3", "abc4", "abc5", "abc6"}; //아이디
const char* data_password[] = {"12341", "12342", "12343", "12344", "12345", "12346"}; //비밀번호
int total_account = sizeof(data_id) / sizeof(data_id[0]); // 계정 개수

bool check_account(const char* id, const char* password)
{
	for (int i = 0; i < total_account; i++)
	{
		if (strcmp(id, data_id[i]) == 0 && strcmp(password, data_password[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

void append_log(const char* log)
{
	std::ofstream logfile;
	logfile.open("Log.txt", std::ios_base::out | std::ios_base::app);
	logfile << log;
	logfile.close();
}