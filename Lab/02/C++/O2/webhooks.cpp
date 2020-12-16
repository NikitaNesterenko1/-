#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

#include "include/cpp-httplib/httplib.h"
#include "include/nlohmann/json.hpp"

using json = nlohmann::json;
using namespace httplib;
namespace fs = std::filesystem;

// �������� � ���������� 'data' ������ 'to_replace' ������� 'replace_with'
void replace_all(std::string& data,
	const std::string& to_replace,
	const std::string& replace_with)
{
	size_t pos = data.find(to_replace);
	while (pos != std::string::npos)
	{
		data.replace(pos, to_replace.size(), replace_with);
		pos = data.find(to_replace, pos + replace_with.size());
	}
}

const std::string config_file_name = "config.json";
const std::string webpage_file_name = "webhooks.html";
std::string webhook_template = R"(
<div class="form-row align-items-center">
    <div class="col">
        <input type="text" value="{Webhook URL}" class="form-control mb-2" disabled>
    </div>
    <div class="col">
        <button type="submit" name="del" value="{Webhook URL}" class="btn btn-danger mb-2">�������</button>
    </div>
</div>)";

json config;

json get_config()
{
	std::ifstream config_file(config_file_name);
	json config;
	if (config_file.is_open())
	{
		std::cout << "����: ���� �������������� json. ���� ��������� �� �������� ���������� �� ���������, "
			<< "��������, �������� � ������ �������." << std::endl;

		config_file >> config;

		std::cout << "����: �������������� json ������� ���������." << std::endl;
		config_file.close();
	}
	else
	{
		fs::path config_path{ config_file_name };
		if (fs::exists(config_path))
		{
			std::cout << "������: ���� " << fs::absolute(config_path) << " ����������, �� ������� ��� �� �������. "
				<< "��������� ����� ������� � ����� �����." << std::endl;
		}
		else
		{
			std::cout << "������: ���� " << fs::absolute(config_path) << " �� ����������." << std::endl;
		}
		json return_json;
		return_json["webhooks"] = json::array();
		return return_json;
	}
	return config;
}


void save_config(json config)
{
	fs::path config_path{ config_file_name };
	if (!fs::exists(config_path))
	{
		std::cout << "����: ���� " << fs::absolute(config_path) << " �� ����������. ������ �����..." << std::endl;
	}

	std::ofstream config_file(config_file_name);
	if (config_file.is_open())
	{
		std::cout << "����: ����� ���� ������������..." << std::endl;
		config_file << config.dump(4);
		config_file.close();
	}
	else
	{
		std::cout << "�� ������� ������� ���� �� ���� " << fs::absolute(config_path) << "!"
			<< "��������� ����� ������� �� ����� " << fs::current_path()
			<< " ��� �� ���� ������������." << std::endl;
	}
}


std::string gen_webhook_page()
{
	std::ifstream webpage_file(webpage_file_name);
	std::string webpage;
	if (webpage_file.is_open())
	{
		std::getline(webpage_file, webpage, '\0');
		webpage_file.close();
	}
	else
	{
		fs::path config_path{ webpage_file_name };
		if (fs::exists(config_path))
		{
			std::cout << "������: ���� " << fs::absolute(config_path) << " ����������, �� ������� ��� �� �������. "
				<< "��������� ����� ������� � ����� �����." << std::endl;
		}
		else
		{
			std::cout << "������: ���� " << fs::absolute(config_path) << " �� ����������." << std::endl;
		}
		return "";
	}
	if (config.empty())
	{
		config = get_config();
	}
	std::string webhook_list;
	if (!config["webhooks"].empty())
	{
		for (auto webhook : config["webhooks"])
		{
			if (webhook.is_string())
			{
				std::string templ = webhook_template;
				replace_all(templ, "{Webhook URL}", webhook.get<std::string>());
				webhook_list += templ;
			}
			else
			{
				std::cout << "������: ��������� ���������������� ����. "
					<< "��� ��������� " << webhook << " � ����� ������ '" << webhook.type_name()
					<< "'. ��������� ������." << std::endl;
			}
		}
	}
	replace_all(webpage, "{webhooks_list}", webhook_list);
	return webpage;
}


void webhooks_get(const Request& req, Response& res)
{
	res.set_content(gen_webhook_page(), "text/html; charset=UTF-8");
	std::cout << std::endl;
}


void webhooks_post(const Request& req, Response& res)
{
	if (config.empty())
	{
		config = get_config();
	}
	if (req.has_param("del"))
	{
		std::string value = req.get_param_value("del");
		if (value == "")
		{
			std::cout << "����: ������ ������ �� �������� �������." << std::endl;
		}
		else
		{
			std::cout << "����: � POST ������� �������� del=" << value << std::endl;

			if (!config["webhooks"].empty())
			{
				for (auto ptr = config["webhooks"].begin();
					ptr != config["webhooks"].end();
					++ptr)
				{
					if (ptr->is_string() && ptr->get<std::string>() == value)
					{
						std::cout << "����: ������ " << ptr->get<std::string>()
							<< " ������ � ������������ � ��� �����." << std::endl;
						config["webhooks"].erase(ptr);
						break;
					}
				}
			}
			else
			{
				std::cout << "����: ������� �����������. ������ �������!" << std::endl;
			}
		}
	}
	else if (req.has_param("set"))
	{
		std::string value = req.get_param_value("set");
		if (value == "")
		{
			std::cout << "����: ������ ������ �� �������� �������." << std::endl;
		}
		else
		{
			std::cout << "����: � POST ������� �������� set=" << value << std::endl;

			if (!config["webhooks"].empty())
			{
				bool webhook_exist = false;
				for (auto ptr = config["webhooks"].begin();
					ptr != config["webhooks"].end();
					++ptr)
				{
					if (ptr->is_string() && ptr->get<std::string>() == value)
					{
						std::cout << "����: ������ " << ptr->get<std::string>()
							<< " ��� ���������� � ������������." << std::endl;
						webhook_exist = true;
						break;
					}
				}
				if (!webhook_exist) {
					std::cout << "����: �������� ������ " << value << "." << std::endl;
					config["webhooks"].push_back(value);
				}
			}
			else
			{
				std::cout << "����: �������� ������ " << value << "." << std::endl;
				config["webhooks"].push_back(value);
			}
		}
	}
	save_config(config);

	res.set_content(gen_webhook_page(), "text/html; charset=UTF-8");
	std::cout << std::endl;
}