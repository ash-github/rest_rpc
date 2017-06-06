#include <rest_rpc/server.hpp>

uint16_t port = 9000;
size_t pool_size = std::thread::hardware_concurrency();

struct person
{
	int age;
	std::string name;
};

REFLECTION(person, age, name);

namespace client
{
	int test(person const& p)
	{
		return p.age;
	}

	int add(int a, int b)
	{
		return a + b;
	}

	void dummy()
	{
		std::cout << "dummy" << std::endl;
	}

	void some_task_takes_a_lot_of_time(double, int)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(5s);
	}

	struct foo
	{
		template <typename T>
		T add_impl(T a, T b)
		{
			return a + b;
		}

		int add(int a, int b)
		{
			return add_impl(a, b);
		}
	};
}

struct test
{
	void compose(int i, const std::string& str, const timax::rpc::blob_t& bl, double d)
	{
		std::cout << i << " " << str << " " << bl.data() << " " << bl.size() <<" "<<d<< std::endl;
	}
};


template <size_t ... Is>
void print(std::index_sequence<Is...>)
{
	bool swallow[] = { (printf("%d\n", Is), true)... };
}

int main()
{
	timax::log::get().init("rest_rpc_server.lg");
	using server_t = timax::rpc::server<timax::rpc::msgpack_codec>;
	server_t server{ port, pool_size, std::chrono::seconds{ 2 } };
	client::foo foo{};

	server.register_handler("add", client::add);
	server.register_handler("test", client::test);
	server.register_handler("add_pub", client::add, [&server](auto conn, int r) { server.pub("sub_add", r); });
	server.register_handler("foo_add", timax::bind(&client::foo::add, &foo));
	server.register_handler("dummy", client::dummy);
	server.register_handler("add_with_conn", []
		(timax::rpc::connection_ptr conn, int a, int b)
	{
		auto result = a + b;
		if (result < 1)
			conn->close();
		return result;
	});
	
	server.async_register_handler("time_consuming", client::some_task_takes_a_lot_of_time, [](auto conn) { std::cout << "acomplished!" << std::endl; });

	test t;
	server.register_handler("compose", timax::bind(&test::compose, &t));

	server.start();
	std::getchar();
	server.stop();
	return 0;
}