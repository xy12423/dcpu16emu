#include "stdafx.h"
#include "cpu.h"
#include "asm.h"
#include "plugin.h"
#include "utils.h"
#include "main.h"

const std::string empty_string;

dcpu16 cpu;
dcpu16_asm assembler;

volatile bool running = false;
std::atomic_int cycles_available(0);
std::shared_ptr<std::promise<void>> cycle_promise;
std::shared_ptr<std::promise<void>> stop_promise;

void run_worker()
{
	while (running)
	{
		if (cycles_available <= 0)
		{
			std::shared_ptr<std::promise<void>> _cycle_promise = std::make_shared<std::promise<void>>();
			cycle_promise = _cycle_promise;
			_cycle_promise->get_future().get();
		}
		int cycle = cpu.step();
		if (cycle < 0)
		{
			std::cout << "\t^ Error" << std::endl;
			running = false;
			return;
		}
		cycles_available -= cycle;
	}
}

void run_timer()
{
	auto time = std::chrono::high_resolution_clock::now();

	constexpr size_t cycles_per_sec = 100000;
	constexpr intmax_t lcm = _lcm<cycles_per_sec, std::chrono::high_resolution_clock::period::den>::value;
	constexpr size_t cycles_per_tick = std::chrono::high_resolution_clock::period::num * (lcm / std::chrono::high_resolution_clock::period::den);
	constexpr std::chrono::high_resolution_clock::duration interval(lcm / cycles_per_sec);

	while (running)
	{
		cycles_available += cycles_per_tick;
		if (cycle_promise)
		{
			cycle_promise->set_value();
			cycle_promise.reset();
		}
		time += interval;
		std::this_thread::sleep_until(time);
	}
	stop_promise->set_value();
}

void dump(const std::string& arg)
{
	static uint32_t add = 0;
	if (!arg.empty())
		add = static_cast<uint32_t>(std::stoul(arg, 0, 0));
	int i, j;
	for (i = 0; i < 8; i++)
	{
		printf("%04X:", add);
		for (j = 0; j < 8; j++)
		{
			uint16_t val;
			cpu.get_mem(add, val);
			printf("%04X ", val);
			add += 1;
			if (add >= 0x10000)
			{
				add = 0;
				i = 7;
				break;
			}
		}
		std::cout << std::endl;
	}
	return;
}

void registers()
{
	uint16_t reg[12];
	for (int i = 0; i < 12; i++)
		cpu.get_reg(i, reg[i]);
	printf("A=%04X\tB=%04X\tC=%04X\tX=%04X\tY=%04X\tZ=%04X\tI=%04X\tJ=%04X\t", reg[0], reg[1], reg[2], reg[3], reg[4], reg[5], reg[6], reg[7]);
	std::cout << std::endl;
	printf("PC=%04X\tSP=%04X\tEX=%04X\tIA=%04X\t", reg[8], reg[9], reg[10], reg[11]);
	std::cout << std::endl;
}

void unasm(int32_t _add = -1)
{
	static uint16_t add = 0;
	if (_add != -1)
		add = static_cast<uint16_t>(_add);

	uint16_t end = add + 0x40, val, display_add = add;
	unsigned int tmp;
	std::string ins;

	assembler.clear_buf();
	assembler.clear();
	for (; add < end; )
	{
		tmp = display_add;
		for (; assembler.rdbuf().size() < 3; add += 1)
		{
			cpu.get_mem(add, val);
			assembler.write(&val, 1);
		}
		ins.clear();
		assembler.read(ins);
		display_add += static_cast<uint16_t>(assembler.gcount());

		if (assembler.fail())
		{
			break;
		}
		else
		{
			printf("%04X:", tmp);
			std::cout << ins << std::endl;
		}
	}
}

void assemble(int32_t _add = -1)
{
	static uint16_t add = 0;
	std::string ins;
	if (_add != -1)
		add = _add;
	uint16_t buf[3];
	while (true)
	{
		printf("%04X:", add);
		std::getline(std::cin, ins);
		if (ins.empty())
			break;

		assembler.clear_buf();
		assembler.clear();
		assembler.write(ins);
		if (assembler.fail())
		{
			std::cout << "\t^ Error" << std::endl;
		}
		else
		{
			assembler.read(buf, assembler.rdbuf().size());
			for (size_t i = 0; i < assembler.gcount(); i++, add++)
				cpu.set_mem(add, buf[i]);
		}
	}
	return;
}

void enter(const std::vector<std::string>::iterator& _begin, const std::vector<std::string>::iterator& end)
{
	std::vector<std::string>::iterator begin = _begin;
	uint32_t add = static_cast<uint32_t>(std::stoul(*begin++, 0, 0));
	if (begin == end)
		return;
	for (; begin != end && add < 0x10000; ++begin)
		cpu.set_mem(add++, static_cast<uint16_t>(std::stoul(*begin, 0, 0)));
	return;
}

void load(const std::string& path)
{
	std::ifstream fin(path, std::ios::in | std::ios::binary);
	if (!fin || !fin.is_open())
	{
		std::cout << "\t^ Error" << std::endl;
		return;
	}
	uint16_t val;
	for (uint32_t add = 0; add < 0x10000; add += 1)
	{
		fin.read(reinterpret_cast<char*>(&val), sizeof(val));
		cpu.set_mem(add, val);
	}
	fin.close();
}

void save(const std::string& path)
{
	std::ofstream fout(path, std::ios::out | std::ios::binary);
	if (!fout || !fout.is_open())
	{
		std::cout << "\t^ Error" << std::endl;
		return;
	}
	uint16_t val;
	for (uint32_t add = 0; add < 0x10000; add += 1)
	{
		cpu.get_mem(add, val);
		fout.write(reinterpret_cast<char*>(&val), sizeof(val));
	}
	fout.close();
}

void proceed()
{
	running = true;
	std::thread timer_thread(run_timer);
	timer_thread.detach();
	std::thread worker_thread(run_worker);
	worker_thread.detach();
}

void trace()
{
	if (cpu.step() < 1)
	{
		std::cout << "\t^ Error" << std::endl;
		return;
	}
	std::string ins_str;
	uint16_t ins[3], pc;
	cpu.get_reg(dcpu16::REG_PC, pc);
	for (int i = 0; i < 3; i++)
		cpu.get_mem(pc + i, ins[i]);
	assembler.clear_buf();
	assembler.clear();
	assembler.write(ins, 3);
	assembler.read(ins_str);
	registers();
	std::cout << "Next:" << ins_str << std::endl;
}

void load_plugins(const char* file)
{
	std::wifstream fin(file);
	if (!fin.is_open())
		return;
	std::wstring line;
	std::getline(fin, line);
	while (!fin.eof())
	{
		load_plugin(line);
		std::getline(fin, line);
	}
}

void init()
{
	cpu.reset();
}

int main()
{
	load_plugins("plugins.txt");
	std::string cmd;
	std::vector<std::string> argv;
	bool exiting = false;
	while (!exiting)
	{
		while (true)
		{
			std::cout << '-';
			std::getline(std::cin, cmd);
			trim(cmd);
			if (cmd.length() >= 1)
				break;
		}
		argv.clear();
		try
		{
			cmd.push_back(' ');
			int state = 0;
			std::string::iterator itr = cmd.begin(), itr2 = cmd.begin(), itr_end = cmd.end();

			for (; itr2 != itr_end; ++itr2)
			{
				if (state == 0 && *itr2 != ' ')
				{
					if (*itr2 == '"')
					{
						state = 2;
						itr = itr2 + 1;
					}
					else
					{
						state = 1;
						itr = itr2;
					}
				}
				else if (state == 1 && *itr2 == ' ')
				{
					state = 0;
					argv.push_back(std::string(itr, itr2));
				}
				else if (state == 2 && *itr2 == '"')
				{
					state = 3;
					argv.push_back(std::string(itr, itr2));
				}
				else if (state == 3)
				{
					if (*itr2 != ' ')
						break;
					state = 0;
				}
			}
			if (state != 0)
				throw(std::runtime_error("\t^ Error"));
		}
		catch (std::exception &ex) { std::cout << ex.what() << std::endl; continue; }

		if (argv.empty() || argv[0].size() != 1)
		{
			std::cout << "\t^ Error" << std::endl;
			continue;
		}
		switch (tolower(argv[0].front()))
		{
			case 'q':
				exiting = true;
				break;
			case 'd':
				if (argv.size() < 2)
					dump(empty_string);
				else
					dump(argv[1]);
				break;
			case 'r':
				registers();
				break;
			case 'u':
				if (argv.size() < 2)
				{
					unasm();
				}
				else
				{
					try
					{
						unasm(std::stoi(argv[1], 0, 0));
					}
					catch (std::out_of_range &) { std::cout << "\t^ Error" << std::endl; }
					catch (std::invalid_argument &) { std::cout << "\t^ Error" << std::endl; }
				}
				break;
			case 'a':
				if (argv.size() < 2)
				{
					assemble();
				}
				else
				{
					try
					{
						assemble(std::stoi(argv[1], 0, 0));
					}
					catch (std::out_of_range &) { std::cout << "\t^ Error" << std::endl; }
					catch (std::invalid_argument &) { std::cout << "\t^ Error" << std::endl; }
				}
				break;
			case 'e':
				if (argv.size() < 2)
					std::cout << "\t^ Error" << std::endl;
				else
					enter(argv.begin() + 1, argv.end());
				break;
			case 'l':
				if (argv.size() < 2)
					std::cout << "\t^ Error" << std::endl;
				else
					load(argv[1]);
				break;
			case 's':
				if (argv.size() < 2)
					std::cout << "\t^ Error" << std::endl;
				else
					save(argv[1]);
				break;
			case 'p':
				if (argv.size() > 1)
				{
					try
					{
						cpu.set_reg(dcpu16::REG_PC, std::stoi(argv[1], 0, 0));
					}
					catch (std::out_of_range &) { std::cout << "\t^ Error" << std::endl; break; }
					catch (std::invalid_argument &) { std::cout << "\t^ Error" << std::endl; break; }
				}
				stop_promise = std::make_shared<std::promise<void>>();
				proceed();
				getchar();
				running = false;
				stop_promise->get_future().get();
				stop_promise.reset();
				if (cycle_promise)
				{
					cycle_promise->set_value();
					cycle_promise.reset();
				}
				break;
			case 't':
				if (argv.size() > 1)
				{
					try
					{
						cpu.set_reg(dcpu16::REG_PC, std::stoi(argv[1], 0, 0));
					}
					catch (std::out_of_range &) { std::cout << "\t^ Error" << std::endl; break; }
					catch (std::invalid_argument &) { std::cout << "\t^ Error" << std::endl; break; }
				}
				trace();
				break;
			case 'i':
				init();
				break;
			default:
				std::cout << "\t^ Error" << std::endl;
		}
	}
	unload_plugin();
	return 0;
}
