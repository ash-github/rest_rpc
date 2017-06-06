#include <rest_rpc/client.hpp>

namespace client
{
	struct person
	{
		int age;
		std::string name;
	};
	REFLECTION(person, age, name);

	struct configure
	{
		std::string hostname;
		std::string port;
	};
	REFLECTION(configure, hostname, port);

	configure get_config()
	{
		std::ifstream in("client.cfg");
		std::stringstream ss;
		ss << in.rdbuf();
	
		configure cfg = { "127.0.0.1", "9000" };
		try
		{
			auto file_content = ss.str();
			iguana::json::from_json(cfg, file_content.data(), file_content.size());
		}
		catch (const std::exception& e)
		{
			timax::SPD_LOG_ERROR(e.what());
		}
	
		return cfg;
	}
}

using sync_client = timax::rpc::sync_client<timax::rpc::msgpack_codec>;

namespace client
{
	TIMAX_DEFINE_PROTOCOL(add, int(int, int));
	TIMAX_DEFINE_PROTOCOL(test, int(person));
}

int main(void)
{
	timax::log::get().init("rest_rpc_client.lg");
	//auto cfg = client::get_config();

	auto endpoint = timax::rpc::get_tcp_endpoint("127.0.0.1", 9000);

	sync_client client;
	
	try
	{
		auto result = client.call(endpoint, client::add, 1, 2);
		assert(result == 3);
	
		//client.call(endpoint, client::madoka, 2.0, 8);
	}
	catch (timax::rpc::exception const& e)
	{
		std::cout << e.get_error_message() << std::endl;
	}

	return 0;
}