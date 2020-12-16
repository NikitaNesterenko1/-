#include <iostream>

#include "include/cpp-httplib/httplib.h"
#include "include/nlohmann/json.hpp"

using json = nlohmann::json;
using namespace httplib;

enum voice_mode
{
	silent_mode,
	speak_mode
};
enum skill_mode
{
	default_mode,
	help_mode
};

/*[
	{
		"user_id": "F812E27KDY",
		"voice_mode": 0,
		"skill_mode": 1,
		"cart": [
			{
				"item": "�������",
				"price": 25973
			}
		]
	}
]*/
json session_list = json::array();
json default_state_buttons =
{
	{
		{"title", "������"},
		{"hide", true}
	},
};
json speak_mode_button =
{
	{"title", "��������"},
	{"hide", true}
};
json silent_mode_button =
{
	{"title", "�������"},
	{"hide", true}
};

json help_state_buttons =
{
	{
		{"title", "�������"},
		{"hide", true}
	},
	{
		{"title", "��������"},
		{"hide", true}
	},
	{
		{"title", "������"},
		{"hide", true}
	},
	{
		{"title", "�������"},
		{"hide", true}
	},
	{
		{"title", "����� �� ������"},
		{"hide", true}
	},
	{
		{"title", "������� ���������"},
		{"hide", true}
	},
};

json get_config(); 
void replace_all(std::string& data,
	const std::string& to_replace,
	const std::string& replace_with); 

json gen_response(const std::string& text,
	const std::string& tts,
	const json& buttons,
	const json* current_session = nullptr,
	const bool end_session = false)
{
	json response = {
		{"response", {
			{"buttons", buttons},
			{"end_session", end_session}
		}},
		{"version", "1.0"}
	};
	if (text != "")
	{
		response["response"]["text"] = text;
	}
	if (current_session != nullptr && (*current_session)["voice_mode"] == speak_mode)
	{
		if (tts != "")
		{
			response["response"]["tts"] = tts;
		}
		response["response"]["buttons"].push_back(silent_mode_button);
	}
	else if (current_session != nullptr && (*current_session)["voice_mode"] == silent_mode)
	{
		response["response"]["buttons"].push_back(speak_mode_button);
		response["response"]["tts"] = "sil <[500]>";
	}
	return response;
}

void yandex_hook(const Request& req, Response& res)
{
	json req_json = json::parse(req.body);

	std::string user_id = req_json["session"]["application"]["application_id"];
	json response;
	json* cur_session = nullptr;

	for (auto& session : session_list)
	{
		if (session["user_id"] == user_id)
		{
			cur_session = &session;
			break;
		}
	}

	if (req_json["session"]["new"].get<bool>())
	{
		if (cur_session == nullptr)
		{
			json session =
			{
				{"user_id", user_id},
				{"skill_mode", default_mode},
				{"voice_mode", silent_mode},
				{"cart", json::array()}
			};
			session_list.push_back(session);
			cur_session = &session_list[session_list.size() - 1];
		}
		else
		{
			(*cur_session)["skill_mode"] = default_mode;
			(*cur_session)["voice_mode"] = silent_mode;
		}

		json response = gen_response(
			"������������! � ������ ��� � ���������.",
			"���+���������! � �����+� ��� � ���+������.",
			default_state_buttons,
			cur_session);

		res.set_content(response.dump(2), "text/json; charset=UTF-8");
		return;
	}

	if (cur_session == nullptr)
	{
		json response = gen_response(
			"��������, ��������� ������",
			"�����+���, ��������+� ��+����",
			default_state_buttons,
			cur_session,
			true);
		res.set_content(response.dump(2), "text/json; charset=UTF-8");
		return;
	}

	std::string command = req_json["request"]["command"];
	if ((*cur_session)["skill_mode"] == help_mode)
	{
		
		std::string text;
		std::string tts;

		if (command == "�������")
		{
			text = "������� ��� ��� ����� � � �� ���� ���������� ���� ���������.";
			tts = "������� ��� ��� ����� � � �� ���� ���������� ���� ���������.";
		}
		else if (command == "��������")
		{
			text = "������� ��� �������� � � ���� ���������� ��� ���� �������";
			tts = "������� ��� �������� � � ���� ���������� ��� ���� �������";
		}
		else if (command == "������")
		{
			text = "��� ��, ��� �� ������ ����������. "
				"� ���� ������ � �������� ��� �������� � ����� ������������.";
			tts = "��� ��, ��� �� ������ ����������. "
				"� ���� ������ � �������� ��� �������� � ����� ������������.";
		}
		else if (command == "�������")
		{
			text = "� �������, ��� �� ������ ��������� ��� ������� �� �������.\n"
				"����� �������� ���-��, ������� \"�������� � ������� ��� 5 ������\".\n"
				"����� ������� ���-��, ������� \"������� �� ������� ���\".";
			tts = "� �������, ��� �� ������ ��������� ��� ������� �� �������.\n"
				"����� �������� ���-��, ������� \"�������� � ������� ��� 5 ������\".\n"
				"����� ������� ���-��, ������� \"������� �� ������� ���\".";
		}
		else if (command == "�����")
		{
			text = "� ��������� ��������� ���� ������� � ����� ������� � ������ �� ����� ���������.";
			tts = "� ��������� ��������� ���� ������� � ����� ������� � ������ �� ����� ���������.";
		}
		else if (command == "������� ���������")
		{
			text = "� ������� ������ ����� ������� ���-�����, ����� ���� ����� �������.";
			tts = "� ������� ������ ����� ������� ���-�����, ����� ���� ����� �������.";
		}
		else if (command == "����� �� ������")
		{
			text = "������. ����� ����� ������ - �����������.";
			tts = "������. ����� ����� ������ - �����������.";
			(*cur_session)["skill_mode"] = default_mode;
		}
		else
		{
			text = "� ���� � �� ���� ��� ������.";
			tts = "� +���� � �� ���+� ��� ���+���.";
		}

		json response;
		if ((*cur_session)["skill_mode"] == help_mode)
		{
			response = gen_response(text, tts, help_state_buttons, cur_session);
		}
		else
		{
			response = gen_response(text, tts, default_state_buttons, cur_session);
		}
		res.set_content(response.dump(2), "text/json; charset=UTF-8");
	}
	else
	{
		if (command == "�������")
		{
			std::string text = "�����, �����";
			std::string tts;
			(*cur_session)["voice_mode"] = silent_mode;
			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command == "��������")
		{
			std::string text = "������.";
			std::string tts = "�����+�.";
			(*cur_session)["voice_mode"] = speak_mode;
			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command == "������")
		{
			std::string text =
				"�������� ��� �������. �������� � ��������� ��������� �����.\n"
				"�������. ��������� ����� ������ �������, � ��� �� �� ���������.\n"
				"������. ������������ � ������������ ����� ������.\n"
				"��������� �������. ������� ������� � ��������� ������ � ������� ������.\n"
				"�����. ������� ����� ������� � �������� � ���.\n"
				"� ��� ���������� ���������?";
			std::string tts =
				"�������� ��� �������. �������� � ��������� ��������� �����.\n"
				"�������. ��������� ����� ������ �������, � ��� �� �� ���������.\n"
				"������. ������������ � ������������ ����� ������.\n"
				"��������� �������. ������� ������� � ��������� ������ � ������� ������.\n"
				"�����. ������� ����� ������� � �������� � ���.\n"
				"� ��� ���������� ���������?";
			json response = gen_response(
				text,
				tts,
				help_state_buttons,
				cur_session);
			(*cur_session)["skill_mode"] = help_mode;
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command == "�������� �������")
		{
			std::string text = "������� �����.";
			std::string tts = "�����+��� ����+�.";
			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);
			(*cur_session).erase("cart");
			(*cur_session)["cart"] = json::array();
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command == "��� � �������")
		{
			std::cout << "cart: " << (*cur_session)["cart"] << std::endl;

			std::string text;
			std::string tts;

			if ((*cur_session)["cart"].empty())
			{
				text = "�� ������ ������ ���� ������� �����.";
				tts = "�� ������ ������ ���� ������� �����.";
			}
			else
			{
				text = "�� ������ ������ � ����� �������:";
				for (auto& elem : (*cur_session)["cart"])
				{
					int price = elem["price"].get<int>();

					text += "\n"
						+ elem["item"].get<std::string>()
						+ " ����� "
						+ std::to_string(price);

					if (price % 10 == 0)
					{
						text += " ������,";
					}
					else if (price % 10 == 1)
					{
						text += " �����,";
					}
					else if (price % 10 < 5 && price % 10 > 0)
					{
						text += " �����,";
					}
					else
					{
						text += " ������,";
					}
				}
				text.pop_back();
				tts = text;
			}

			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command == "������� ���������")
		{
			std::string text = "�������� ���!";
			std::string tts = "�����+��� ��+�!";

			json output =
			{
				{"user_id", user_id},
				{"cart", (*cur_session)["cart"]}
			};

			json config = get_config(); 

			for (std::string link : config["webhooks"])
			{
				
				replace_all(link, "https://", "http://");

				if (link.find("http://") != 0)
				{
					std::cout << "bad link" << std::endl;
					continue;
				}

				const int http_protocol_size = 7;

				int index = link.find('/', http_protocol_size);
				if (index == std::string::npos)
				{
					link.push_back('/');
					index = link.length() - 1;
				}

				std::cout << "test: " << link.substr(0, index) << std::endl;
				std::cout << "2nd : " << link.substr(index, std::string::npos).c_str() << std::endl;

				Client cli(link.substr(0, index).c_str());
				cli.Post(link.substr(index, std::string::npos).c_str(), output.dump(2), "application/json; charset=UTF-8");
			}

			(*cur_session).erase("cart");
			(*cur_session)["cart"] = json::array();

			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session,
				true);
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command == "�����")
		{
			std::string text = "";
			std::string tts = "";

			size_t size = req_json["request"]["nlu"]["tokens"].size();
			int sum = 0;
			for (auto& cart_item : (*cur_session)["cart"])
			{
				sum += cart_item["price"].get<int>();
			}
			if (sum == 0)
			{
				text = "� ��� � ������� ��� �������.";
				tts = "� ��� � ����+��� ��� ���+����.";
			}
			else
			{
				text = "� ����� ����� � ��� ������� �� " + std::to_string(sum);
				tts = "� +����� �+���� � ��� ���+���� �� " + std::to_string(sum);
				if (sum % 10 == 0)
				{
					text += " ������.";
					tts += " ����+��.";
				}
				else if (sum % 10 == 1)
				{
					text += " �����.";
					tts += " �����.";
				}
				else if (sum % 10 < 5 && sum % 10 > 0)
				{
					text += " �����.";
					tts += " ����+�.";
				}
				else
				{
					text += " ������.";
					tts += " ����+��.";
				}
			}

			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command.find("�������� � �������") == 0 || command.find("������ � �������") == 0)
		{
			size_t size = req_json["request"]["nlu"]["tokens"].size();
			std::string text = "��.";
			std::string tts = "��+��.";
			std::string item_name;
			int			item_price = 0;
			int			number_index = 0;
			bool			number_index_set = false;

			for (auto entity : req_json["request"]["nlu"]["entities"])
			{
				if (entity["type"].get<std::string>() == "YANDEX.NUMBER")
				{
					number_index = entity["tokens"]["start"];
					std::cout << entity["value"].type_name() << std::endl;
					int val = entity["value"];
					std::cout << "����: ���� " << val << std::endl;
					if (val < 0)
					{
						text = "���� �� ����� ���� �������������.";
						tts = "���+� �� �+���� �+��� �����+��������.";
					}
					else if (val == 0)
					{
						text = "�� ��� �� ��� ������� ���-�� �� ���������?";
						tts = "�� ��� �� ��� ����+��� ��+� �� �� �����+����?";
					}
					else
					{
						item_price = val;
					}
					//}
					number_index_set = true;
					break;
				}
			}
			if (size == 3)
			{
				text = "����������, ����������, ��� �������� � �������.";
				tts = "���+�������, �������+���, ��� ���+����� � ����+���.";
			}
			else if (!number_index_set)
			{
				text = "����������, ������� ���� ������.";
				tts = "���+�������, ����+��� �+��� ���+���.";
			}
			else if (number_index == 3)
			{
				text = "����������, ������� �������� ������.";
				tts = "���+�������, ����+��� ����+���� ���+���.";
			}
			else
			{
				for (int i = 3; i < number_index; ++i)
				{
					item_name += req_json["request"]["nlu"]["tokens"][i].get<std::string>();
					item_name += " ";
				}
				item_name.pop_back();
				json item = {
					{"item",  item_name},
					{"price", item_price}
				};
				(*cur_session)["cart"].push_back(item);
			}

			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);

			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else if (command.find("������� �� �������") == 0 || command.find("����� �� �������") == 0
			|| command.find("������ �� �������") == 0 || command.find("����� �� �������") == 0)
		{
			std::cout << (*cur_session)["cart"] << std::endl;
			size_t size = req_json["request"]["nlu"]["tokens"].size();

			std::string text;
			std::string tts;
			std::string item_name = "";

			for (int i = 3; i < size; ++i)
			{
				std::cout << req_json["request"]["nlu"]["tokens"][i].get<std::string>() << std::endl;
				item_name += req_json["request"]["nlu"]["tokens"][i].get<std::string>();
				item_name += " ";
			}
			bool found_item = false;
			int	item_index = 0;

			if (item_name == "")
			{
				text = "�� �� ���� �������, ��� ����������� �������.";
				tts = "�� �� ���� ����+���, ��� �����+������ ����+���.";
			}
			else
			{
				item_name.pop_back();
				for (auto& cart_item : (*cur_session)["cart"])
				{
					if (cart_item["item"].get<std::string>() == item_name)
					{
						found_item = true;
						break;
					}
					++item_index;
				}
				if (!found_item)
				{
					std::cout << "����: ����� ������� �� ��� ������" << std::endl;
					text = "������ � ����� ������� ����";
					tts = "���+��� � �+���� ����+��� �+���.";
				}
				else
				{
					std::cout << "����: ����� ����� �������" << std::endl;
					text = "�������.";
					tts = "����+���";
					(*cur_session)["cart"].erase((*cur_session)["cart"].begin() + item_index);
					std::cout << "����: ������� �������" << std::endl;
				}
			}
			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);
			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
		else
		{
			std::string text = "� �� ���� ����� �������.";
			std::string tts = "� �� ��+�� ���+�� ���+����.";

			json response = gen_response(
				text,
				tts,
				default_state_buttons,
				cur_session);

			res.set_content(response.dump(2), "text/json; charset=UTF-8");
		}
	}


	std::cout << std::endl;
}