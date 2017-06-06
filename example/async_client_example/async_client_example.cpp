#include <rest_rpc/rpc.hpp>

namespace client
{
	TIMAX_DEFINE_PROTOCOL(add, int(int, int));
	TIMAX_DEFINE_PROTOCOL(compose, void(int, const std::string&, timax::rpc::blob_t, double));
	TIMAX_DEFINE_FORWARD(sub_add, int);
	TIMAX_DEFINE_PROTOCOL(sub_not_exist, double(int, std::string const&));
}


using tcp = boost::asio::ip::tcp;
using async_client_t = timax::rpc::async_client<timax::rpc::msgpack_codec>;

// create the client
async_client_t asycn_client;

void async_client_rpc_example(tcp::endpoint const& endpoint)
{
	using namespace std::chrono_literals;

	// the interface is type safe and non-connect oriented designed
	asycn_client.call(endpoint, client::add, 1.0, 200.0f);

	// we can set some callbacks to process some specific eventsS
	asycn_client.call(endpoint, client::add, 1, 2).on_ok([](auto r) 
	{ 
		std::cout << r << std::endl; 
	}).on_error([](auto const& error)
	{
		std::cout << error.get_error_message() << std::endl;
	}).timeout(1min);

	// we can also use the asynchronized client in a synchronized way
	try
	{
		auto task = asycn_client.call(endpoint, client::add, 3, 5);
		auto const& result = task.get();
		std::cout << result << std::endl;
	}
	catch (timax::rpc::exception const& e)
	{
		std::cout << e.get_error_message() << std::endl;
	}

	auto backup_endpoint = timax::rpc::get_tcp_endpoint("127.0.0.1", 8999);
	asycn_client.call(endpoint, client::add, 1, 2).on_error([&backup_endpoint](auto const& error)
	{
		if (error.get_error_code() == timax::rpc::error_code::BADCONNECTION)
			asycn_client.call(backup_endpoint, client::add, 1, 2);
	});
}

void async_client_sub_example(tcp::endpoint const& endpoint)
{
	// we can use the sub interface to keep track of some topics we are interested in
	asycn_client.sub(endpoint, client::sub_add, [](auto r)
	{
		std::cout << r << std::endl;
	}, // interface of dealing with error is also supplied;
		[](auto const& error) 
	{
		std::cout << error.get_error_message() << std::endl;
	});
}

void async_compose_example(tcp::endpoint const& endpoint)
{
	timax::rpc::blob_t p = { "it is a test", 13 };
	auto task = asycn_client.call(endpoint, client::compose, 1, "test", p, 2.5);

	task.cancel();

	try
	{
		task.wait();
	}
	catch (timax::rpc::exception const& exception)
	{
		std::cout << exception.get_error_message() << std::endl;
	}
}

void test_timeout(tcp::endpoint const& endpoint)
{
	using namespace std::chrono_literals;
	for (auto loop = 0; loop < 10000; ++loop)
	{
		asycn_client.call(endpoint, client::add, 1, loop).
			on_error([loop](auto const& error)
		{
			std::cout << "Error: " << error.get_error_message() << " loop" << loop << std::endl;
		}).on_ok([loop](auto r)
		{
			std::cout << "OK loop" << loop << std::endl;
		});
		std::this_thread::sleep_for(10s);
	}
}

int main()
{
	timax::log::get().init("async_client_example.lg");

	auto endpoint = timax::rpc::get_tcp_endpoint("127.0.0.1", 9000);

	async_client_rpc_example(endpoint);
	async_client_sub_example(endpoint);
	async_compose_example(endpoint);
	test_timeout(endpoint);
	std::getchar();
	return 0;
}