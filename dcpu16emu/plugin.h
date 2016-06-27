#ifndef _H_PLUGIN
#define _H_PLUGIN

class plugin_loading_error :public std::runtime_error
{
public:
	plugin_loading_error() :std::runtime_error("Failed to load plugin") {};
};

int load_plugin(const std::wstring& plugin_full_path);
void unload_plugin();

#endif
