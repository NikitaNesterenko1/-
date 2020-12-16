#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include "include/cpp-httplib/httplib.h"
using namespace httplib;

void webhooks_get(const Request& req, Response& res);
void webhooks_post(const Request& req, Response& res);

void yandex_hook(const Request& req, Response& res);

int main()
{
	// https://stackoverflow.com/a/57134096

	Server srv;

	// ��� �������� ���������� https://webhook.site/
	srv.Get("/webhooks", webhooks_get);
	srv.Post("/webhooks", webhooks_post);

	srv.Post("/yandex_hook", yandex_hook);

	std::cout << "������ ������� ����������!\n"
		<< "������ ����� �� ������ localhost:1234\n\n"
		<< "�������� http://localhost:1234/webhooks � ���-��������, "
		<< "����� �������� ������ � ������ ���������� ���-������.\n"
		<< "������ ��� ������� http://localhost:1234/yandex_hook."
		<< std::endl;

	srv.listen("localhost", 1234);
}
