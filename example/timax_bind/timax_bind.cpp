#include <rest_rpc/rpc.hpp>

namespace client
{
	int add(int a, int b)
	{
		return a + b;
	}

	int apply_add(std::function<int()> add_result, int rhs)
	{
		return add_result() + rhs;
	}

	struct foo
	{
		double wtf(int a, std::string const& b) const
		{
			return a * static_cast<double>(b.size());
		}
	};
}

int main()
{
	using namespace std::string_literals;
	auto string = "127.0.0.1:9000, 127.0.0.1:4000"s;
	auto endpoints = timax::rpc::get_tcp_endpoints(string);
	for (auto const& endpoint : endpoints)
	{
		std::cout << endpoint << std::endl;
	}

	//using namespace std::placeholders;
	client::foo foo;
	auto bind1_with_boost_placeholders = timax::bind(&client::foo::wtf, foo, 1, _1);
	auto bind1_with_std_placeholders = timax::bind(&client::foo::wtf, foo,
		std::placeholders::_1, std::placeholders::_2);
	auto bind2 = timax::bind(client::add, 1, _1);
	auto bind3 = timax::bind(&client::foo::wtf, &foo);
	
	auto foo_ptr = std::make_shared<client::foo>();
	auto bind4 = timax::bind(&client::foo::wtf, foo_ptr);
	auto bind_test = timax::bind(client::apply_add, timax::bind(client::add, 1, 1), _1);

	bind1_with_boost_placeholders("boost::placeholders");
	bind1_with_std_placeholders(2, "std::placehodlers");
	bind2(2);
	bind3(3, "WTF");
	bind4(4, "shared_ptr");
	bind_test(1);

	return 0;
}