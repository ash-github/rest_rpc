#include <rest_rpc/client.hpp>

namespace bench
{
	struct configure
	{
		std::string		hostname;
		std::string		port;
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

	TIMAX_DEFINE_PROTOCOL(add, int(int, int));
	TIMAX_DEFINE_PROTOCOL(bench_connection, void(void));

	std::atomic<uint64_t> count{ 0 };

	void bench_async(boost::asio::ip::tcp::endpoint const& endpoint)
	{
		using client_t = timax::rpc::async_client<timax::rpc::msgpack_codec>;

		auto client = std::make_shared<client_t>();

		std::thread{ []
		{
			while (true)
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1s);
				std::cout << count.load() << std::endl;
				count.store(0);
			}
		} }.detach();


		std::thread{
			[client, &endpoint]
		{
			int a = 0, b = 0;
			while (true)
			{
				client->call(endpoint, bench::add, a, b++).on_ok([](auto)
				{
					++count;
				});
			}

		} }.detach();
	}

	void bench_sync(boost::asio::ip::tcp::endpoint const& endpoint)
	{
		using client_t = timax::rpc::sync_client<timax::rpc::msgpack_codec>;

		std::thread{ []
		{
			while (true)
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1s);
				std::cout << count.load() << std::endl;
				count.store(0);
			}
		} }.detach();

		std::thread{
			[endpoint]
		{
			client_t client;

			int a = 0, b = 0;
			while (true)
			{
				try
				{
					client.call(endpoint, bench::add, a, b++);
					++count;
				}
				catch (...)
				{
					std::cout << "Exception: " << std::endl;
					break;
				}
			}

		} }.detach();
	}

	void bench_conn(boost::asio::ip::tcp::endpoint const& endpoint, int connection_count)
	{
		using namespace std::chrono_literals;
		using client_private_t = timax::rpc::async_client_private<timax::rpc::msgpack_codec>;

		auto pool = std::make_shared<timax::rpc::io_service_pool>(std::thread::hardware_concurrency());
		pool->start();

		std::thread
		{
			[pool, &endpoint, connection_count]
			{	
				std::list<client_private_t> client;
				auto const push_in_one_turn = 2048;
				auto already_pushed = 0;
				auto push_left = connection_count;
				while (true)
				{
					while (already_pushed < push_in_one_turn && push_left > 0)
					{
						client.emplace_back(pool->get_io_service());
						auto ctx = client.back().make_rpc_context(endpoint, bench_connection);
						client.back().call(ctx);
						++already_pushed;
						--push_left;
					}
					already_pushed = 0;
					std::this_thread::sleep_for(20ms);
				}
			} 

		}.detach();

	}
}

int main(int argc, char *argv[])
{
	// first of all, initialize log module
	timax::log::get().init("rest_rpc_client.lg");

	auto config = bench::get_config();
	auto endpoint = timax::rpc::get_tcp_endpoint(
		config.hostname, boost::lexical_cast<int16_t>(config.port));
	int connection_count;

	enum class client_style_t
	{
		UNKNOWN,
		SYCN,
		ASYCN,
		CONN,
	};

	std::string client_style;
	client_style_t style = client_style_t::UNKNOWN;

	if (2 > argc)
	{
		std::cout << "Usage: " << "$ ./bench_client %s(sync, async or conn)" << std::endl;
		return -1;
	}
	else
	{
		client_style = argv[1];
		if ("sync" == client_style)
		{
			style = client_style_t::SYCN;
		}
		else if ("async" == client_style)
		{
			style = client_style_t::ASYCN;
		}
		else if ("conn" == client_style)
		{
			style = client_style_t::CONN;
		}
		if (client_style_t::UNKNOWN == style)
		{
			std::cout << "Usage: " << "$ ./bench_client %s(sync, async or conn)" << std::endl;
			return -1;
		}
	}

	switch (style)
	{
	case client_style_t::SYCN:
		bench::bench_sync(endpoint);
		break;
	case client_style_t::ASYCN:
		bench::bench_async(endpoint);
		break;
	case client_style_t::CONN:
		if (3 != argc)
		{
			std::cout << "Usage: " << "$ ./bench_client conn %d(connection count)" << std::endl;
			return -1;
		}
		connection_count = boost::lexical_cast<int>(argv[2]);
		bench::bench_conn(endpoint, connection_count);
		break;
	default:
		return -1;
	}

	std::getchar();
	return 0;
}
