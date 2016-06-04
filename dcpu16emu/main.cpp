#include "stdafx.h"
#include "cpu.h"
#include "main.h"

volatile bool running = false;
std::atomic_int cycles_available = 0;
std::shared_ptr<std::promise<void>> cycle_promise;

void run_worker()
{
	while (running)
	{
		if (cycles_available <= 0)
		{
			cycle_promise = std::make_shared<std::promise<void>>();
			cycle_promise->get_future().get();
			cycle_promise.reset();
		}
		int cycle = cpu.step();
		if (cycle < 0)
		{
			std::cout << "  ^ Error" << std::endl;
			running = false;
			return;
		}
		cycles_available -= cycle;
	}
}

void timer()
{
	auto time = std::chrono::high_resolution_clock::now();

	const size_t cycles_per_sec = 100000;
	const intmax_t lcm = std::_Lcm<cycles_per_sec, std::chrono::high_resolution_clock::period::den>::value;
	const size_t cycles_per_tick = std::chrono::high_resolution_clock::period::num * (lcm / std::chrono::high_resolution_clock::period::den);
	const std::chrono::high_resolution_clock::duration interval(lcm / cycles_per_sec);
	
	while (running)
	{
		cycles_available += cycles_per_tick;
		if (cycle_promise)
			cycle_promise->set_value();
		time += interval;
		std::this_thread::sleep_until(time);
	}
}

void dump(const std::string& arg)
{
	static uint32_t add = 0;
	if (arg != "")
		add = static_cast<uint32_t>(std::stoul(arg));
	int i, j;
	for (i = 0; i < 8; i++)
	{
		printf("%04X:", add);
		for (j = 0; j < 8; j++)
		{
			uint16_t val;
			cpu.get_mem(add, val);
			printf("%04X ", val);
			add++;
			if (add >= 0x10000)
			{
				add = 0;
				std::cout << std::endl;
				return;
			}
		}
		std::cout << std::endl;
	}
	return;
}

void enter(int argc, const std::vector<std::string>& args)
{
	if (argc < 1)
	{
		std::cout << "  ^ Error" << std::endl;
		return;
	}
	uint32_t add = static_cast<uint32_t>(stoul(args[0]));
	for (int i = 1; i < argc && add < 0x10000; i++)
		cpu.set_mem(add++, static_cast<uint16_t>(stoul(args[i])));
	return;
}

void run()
{
	running = true;
	std::thread timer_thread(timer);
	timer_thread.detach();
	std::thread worker_thread(run_worker);
	worker_thread.detach();
}

int main()
{
	
    return 0;
}
