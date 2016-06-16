#include "stdafx.h"
#include "cpu.h"
#include "asm.h"
#include "utils.h"
#include "main.h"

const std::string empty_string;

dcpu16 cpu;
dcpu16_asm assembler;

volatile bool running = false;
std::atomic_int cycles_available(0);
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

	const size_t cycles_per_sec = 100000;
	const intmax_t lcm = _lcm<cycles_per_sec, std::chrono::high_resolution_clock::period::den>::value;
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
			add++;
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

	uint16_t end = add + 0x40, val;
	unsigned int tmp;
	std::string ins;
	for (; add < end; )
	{
		tmp = add;
		printf("%04X:", tmp);
		for (; assembler.size() < 3; add++)
		{
			cpu.get_mem(add, val);
			if (val == 0)
				break;
			assembler.write(&val, 1);
		}
		ins.clear();
		assembler.read(ins);
		if (assembler.gcount() != 0)
		{
			std::cout << ins << std::endl;
		}
		else
		{
			std::cout << std::endl;
			break;
		}
		if (val == 0)
			break;
	}
	assembler.clear();
}

void enter(std::vector<std::string>::iterator begin, const std::vector<std::string>::iterator& end)
{
	uint32_t add = static_cast<uint32_t>(std::stoul(*begin++, 0, 0));
	if (begin == end)
		return;
	for (; begin != end && add < 0x10000; begin++)
		cpu.set_mem(add++, static_cast<uint16_t>(std::stoul(*begin, 0, 0)));
	return;
}

void load(const std::string& path)
{
	std::ifstream fin(path, std::ios::in | std::ios::binary);
	uint16_t val;
	for (uint32_t add = 0; add < 0x10000; add++)
	{
		fin.read(reinterpret_cast<char*>(&val), sizeof(val));
		cpu.set_mem(add, val);
	}
	fin.close();
}

void save(const std::string& path)
{
	std::ofstream fout(path, std::ios::out | std::ios::binary);
	uint16_t val;
	for (uint32_t add = 0; add < 0x10000; add++)
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
	assembler.write(ins, 3);
	assembler.read(ins_str);
	assembler.clear();
	registers();
	std::cout << "Next:" << ins << std::endl;
}

int main()
{
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

			for (; itr2 != itr_end; itr2++)
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
				proceed();
				getchar();
				running = false;
				break;
			case 't':
				trace();
				break;
			default:
				std::cout << "\t^ Error" << std::endl;
		}
	}
	return 0;
}
