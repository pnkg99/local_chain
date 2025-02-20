#include "inerygui.hpp"

using inery::name;
using namespace inery;

inerygui::inerygui(name s, name code, datastream<const char *> ds)
    : contract(s, code, ds),
      _global(get_self(), get_self().value),
      _user(get_self(), get_self().value)

{
    _gstate = _global.exists() ? _global.get() : global_info{};
}
inerygui::~inerygui()
{
    _global.set(_gstate, get_self());
}

void inerygui::sti(string s)
{
    checksum256 hash = sha256(s.c_str(), s.size());
    ;
    auto bytes = hash.extract_as_byte_array();
    uint64_t value;
    std::memcpy(&value, bytes.data(), sizeof(value));
    print(value);
};

// GLOBAL ACTIONS

void inerygui::addtemplateg(uint64_t id, string category_name, string name, string desc, inery::name code, uint64_t rate_score)
{
    require_auth(get_self());
    time_point time = current_time_point();
    tmplt t{id, category_name, name, desc, code, rate_score, time};
    _gstate.templates.push_back(t);
};

void inerygui::addpluging(uint64_t id, string plugin_name, string plugin_desc, bool required)
{
    require_auth(get_self());
    plugin p{id, required, plugin_name, plugin_desc};
    _gstate.plugins.push_back(p);
};

void inerygui::upversiong(string version)
{
    require_auth(get_self());
    _gstate.version = version;
};

void inerygui::waversion()
{
    require_auth(get_self());
    auto itr = _user.begin();
    while (itr != _user.end())
    {
        _user.modify(itr, get_self(), [&](auto &row)
                     { row.version = _gstate.version; });
        itr++;
    }
};

// USER ACTIONS

void inerygui::adduser(string guiname, checksum256 password, string version)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr == _user.end(), "User with that id already exists!");
    _user.emplace(get_self(), [&](auto &row)
                  {
		row.id = id;
		row.guiname = guiname;
		row.password = password;
		row.version = version; });
};
void inerygui::rmusers()
{
    require_auth(get_self());
    auto itr = _user.begin();
    while (itr != _user.end())
    {
        itr = _user.erase(itr);
    }
    print("All users deleted.");
};

void inerygui::addtwords(string guiname, vector<checksum256> twords)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		check(itr->twords.empty(), "User already set 12 words" );
		for(auto word: twords)
			row.twords.push_back(word); });
};

void inerygui::upversionu(string guiname)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    check(itr->version != _gstate.version, "User version is up to date");
    _user.modify(itr, get_self(), [&](auto &row)
                 { row.version = _gstate.version; });
};

void inerygui::recoverpsw(string guiname, vector<checksum256> twords, checksum256 newpassword)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI account with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		check(twords == row.twords, "Wrong!");
		row.password = newpassword; });
};

void inerygui::addaccount(string guiname, name account)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		auto acc_it = std::find(row.accounts.begin(), row.accounts.end(), account);
		check(acc_it == row.accounts.end(), "Account already exist");
		row.accounts.push_back(account); });
};
void inerygui::rmaccount(string guiname, name account)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		auto acc_it = std::find(row.accounts.begin(), row.accounts.end(), account);
		check(acc_it != row.accounts.end(), "Account does not exist");
		row.accounts.erase(std::remove(row.accounts.begin(), row.accounts.end(), account), row.accounts.end()); });
};

void inerygui::addserver(string guiname, string server)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 { row.servers.push_back(server); });
};
void inerygui::rmserver(string guiname, string server)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		auto acc_it = std::find(row.servers.begin(), row.servers.end(), server);
		check(acc_it != row.servers.end(), "Account does not exist");
		row.servers.erase(std::remove(row.servers.begin(), row.servers.end(), server), row.servers.end()); });
};

void inerygui::addtemplateu(string guiname, uint64_t template_id)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		auto it = std::find(row.templates.begin(), row.templates.end(), template_id);
		check( it == row.templates.end(), "User already has template");
		row.templates.push_back(template_id); });
};

void inerygui::addpluginu(string guiname, uint64_t plugin_id)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		auto pluginComparator = [plugin_id](const auto& plugin) {
			return plugin.id == plugin_id;
		};
		// Find the iterator for the plugin
		auto itr = std::find_if(row.plugins.begin(), row.plugins.end(), pluginComparator);
		check( itr == row.plugins.end(), "User already has plugin");
		bool active = false;
		time_point time = current_time_point();
		user_plugin plgo{plugin_id, active, time};
		row.plugins.push_back(plgo); });
};
void inerygui::rmtemplateu(string guiname, uint64_t template_id)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		auto tmp_it = std::find(row.templates.begin(), row.templates.end(), template_id);
		check(tmp_it != row.templates.end(), "User does not have that template");
		row.templates.erase(std::remove(row.templates.begin(), row.templates.end(), template_id), row.templates.end()); });
};
void inerygui::rmpluginu(string guiname, uint64_t plugin_id)
{
    require_auth(get_self());
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");

    vector<user_plugin> temp;
    _user.modify(itr, get_self(), [&](auto &row)
                 {
		for( user_plugin plg : row.plugins){
			if(plg.id != plugin_id){
				temp.push_back(plg);
			}
		row.plugins = temp;
		}; });
};

void inerygui::rmpluging()
{
    require_auth(get_self());
    vector<plugin> a;
    _gstate.plugins = a;
};

void inerygui::turnonplugin(string guiname, uint64_t plugin_id)
{
    require_auth(get_self());
    // Da li Gui account postoji?
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    // Da li ima taj plugin i da li je vec ukljucen?
    auto it = std::find_if(itr->plugins.begin(), itr->plugins.end(), [plugin_id](const auto &element)
                           { return element.id == plugin_id; });
    check(it != itr->plugins.end(), "User does not have plugin!");
    check(!it->active, "Plugin is already activated");
    // Iskljuci mu plugin

    _user.modify(itr, get_self(), [&](auto &row)
                 {
		for(int i=0;i<row.plugins.size();i++){
			if(row.plugins[i].id == plugin_id){
				row.plugins[i].active = true;
			}
		} });
};

void inerygui::turnofplugin(string guiname, uint64_t plugin_id)
{
    require_auth(get_self());
    // Da li Gui account postoji?
    uint64_t id = string_to_uint64(guiname);
    auto itr = _user.find(id);
    check(itr != _user.end(), "GUI with that name don't exists!");
    // Da li ima taj plugin i da li je vec ukljucen?
    auto it = std::find_if(itr->plugins.begin(), itr->plugins.end(), [plugin_id](const auto &element)
                           { return element.id == plugin_id; });
    check(it != itr->plugins.end(), "User does not have plugin!");
    check(it->active, "Plugin is not active");
    // Iskljuci mu plugin

    _user.modify(itr, get_self(), [&](auto &row)
                 {
		for(int i=0;i<row.plugins.size();i++){
			if(row.plugins[i].id == plugin_id){
				row.plugins[i].active = false;
			}
		} });
};

void inerygui::addhistory(name owner, name composition, history_object object, string type)
{
    history_t histories(_self, owner.value);
    auto itr = histories.find(composition.value);

    if (itr == histories.end())
    {
        // If the composition is not found, create a new entry
        histories.emplace(owner, [&](auto &row)
                          {
            row.composition = composition;
            if (type == "cr") {
                row.createhistory.push_back(object);
            } else if (type == "up") {
                row.updatehistory.push_back(object);
            } else if (type == "dl") {
                row.deletehistory.push_back(object);
            } });
    }
    else
    {
        // If the composition is found, modify the existing entry
        histories.modify(itr, owner, [&](auto &row)
                         {
            if (type == "cr") {
                row.createhistory.push_back(object);
            } else if (type == "up") {
                row.updatehistory.push_back(object);
            } else if (type == "dl") {
                row.deletehistory.push_back(object);
            } });
    }
}

void inerygui::linkperm(name owner, name composition, name perm_name)
{
    permissions_t permissions(_self, owner.value);
    auto itr = permissions.find(composition.value);
    if (itr == permissions.end())
    {
        // If the composition is not found, create a new entry
        permissions.emplace(owner, [&](auto &row)
                            {
            row.composition = composition;
			row.perm_name = perm_name; });
    }
    else
    {
        permissions.modify(itr, owner, [&](auto &row)
                           { row.perm_name = perm_name; });
    }
}

void inerygui::checkoff(name account_name)
{
    require_auth(account_name);
}