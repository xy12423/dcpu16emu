#include "stdafx.h"
#include "cpu.h"
#include "plugin.h"

extern dcpu16 cpu;

typedef std::unique_ptr<wxDynamicLibrary> lib_ptr;
std::unordered_set<lib_ptr> plugins;

typedef uint16_t(*fGetHWCount)();
typedef void(*fGetInfo)(uint16_t n, char*);
typedef void(*fSetHandle)(int32_t(uint16_t, uint16_t), int32_t(uint16_t, uint16_t*), int32_t(uint16_t, uint16_t), int32_t(uint16_t, uint16_t*), int32_t(uint16_t));
typedef int32_t(*fInit)();

enum ExportFunc {
	GetHWCount,
	GetInfo,
	SetHandle,
	Init,

	ExportFuncCount
};
std::wstring ExportFuncName[ExportFuncCount] = {
	wxT("GetHWCount"),
	wxT("GetInfo"),
	wxT("SetHandle"),
	wxT("Init"),
};

int32_t setMem(uint16_t add, uint16_t val)
{
	cpu.set_mem(add, val);
	return 0;
}

int32_t getMem(uint16_t add, uint16_t *val)
{
	cpu.get_mem(add, *val);
	return 0;
}

int32_t setReg(uint16_t _reg, uint16_t val)
{
	cpu.set_reg(_reg, val);
	return 0;
}

int32_t getReg(uint16_t _reg, uint16_t *val)
{
	cpu.get_reg(_reg, *val);
	return 0;
}

int32_t addItr(uint16_t _itr)
{
	return cpu.interrupt(_itr);
}

int load_plugin(const std::wstring& plugin_full_path)
{
	try
	{
		lib_ptr plugin = std::make_unique<wxDynamicLibrary>(plugin_full_path, wxDL_QUIET);
		if (!plugin->IsLoaded())
			throw(plugin_loading_error());	//Plugin not loaded

		//Load basic symbols
		void *ExportFuncPtr[ExportFuncCount];
		for (int i = 0; i < ExportFuncCount; i++)
		{
			ExportFuncPtr[i] = plugin->GetSymbol(ExportFuncName[i]);
			if (ExportFuncPtr[i] == nullptr)
				throw(plugin_loading_error());	//Symbol not found
		}
		fGetHWCount getHWCount = reinterpret_cast<fGetHWCount>(ExportFuncPtr[GetHWCount]);
		fGetInfo getInfo = reinterpret_cast<fGetInfo>(ExportFuncPtr[GetInfo]);
		fSetHandle setHandle = reinterpret_cast<fSetHandle>(ExportFuncPtr[SetHandle]);
		fInit init = reinterpret_cast<fInit>(ExportFuncPtr[Init]);
		char buffer[sizeof(uint16_t) * 5 + sizeof(dcpu16::hardware::fHWInt*)];

		uint16_t hwCount = getHWCount();
		for (uint16_t i = 0; i < hwCount; i++)
		{
			getInfo(i, buffer);
			cpu.add_hardware(dcpu16::hardware(
				*reinterpret_cast<uint16_t*>(buffer),
				*reinterpret_cast<uint16_t*>(buffer + sizeof(uint16_t)),
				*reinterpret_cast<uint16_t*>(buffer + sizeof(uint16_t) * 2),
				*reinterpret_cast<uint16_t*>(buffer + sizeof(uint16_t) * 3),
				*reinterpret_cast<uint16_t*>(buffer + sizeof(uint16_t) * 4),
				*reinterpret_cast<dcpu16::hardware::fHWInt*>(buffer + sizeof(uint16_t) * 5)
				));
		}
		setHandle(setMem, getMem, setReg, getReg, addItr);

		const lib_ptr &plugin_ref = *(plugins.emplace(std::move(plugin)).first);

		if (init != nullptr)
		{
			try
			{
				init();
			}
			catch (...) {}
		}

		std::cout << "Plugin loaded:" << wxConvLocal.cWC2MB(plugin_full_path.c_str()) << std::endl;
	}
	catch (plugin_loading_error &) { return 1; }
	catch (std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}

void unload_plugin()
{
	plugins.clear();
}
