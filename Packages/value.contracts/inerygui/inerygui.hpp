#include <inery/inery.hpp>
#include <inery/time.hpp>
#include <inery/system.hpp>
#include <inery/print.hpp>
#include <inery/singleton.hpp>
#include <inery/crypto.hpp>

#include <string>
#include <vector>
#include <algorithm>

using namespace inery;
using namespace std;

using inery::current_time_point;
using inery::indexed_by;
using inery::time_point;
using inery::time_point_sec;

struct tmplt
{
    uint64_t id;
    string category_name;
    std::string template_name;
    std::string template_desc;
    inery::name code;
    uint64_t rate_score;
    time_point date_created;
};

struct plugin
{
    uint64_t id;
    bool required;
    string plugin_name;
    string plugin_desc;
};

struct user_plugin
{
    uint64_t id;
    bool active;
    time_point last_time_used;
};
struct history_object
{
    uint64_t block_num;
    string transaction_id;
    time_point time_executed;
};

struct [[inery::table("global"), inery::contract("inerygui")]] global_info
{
    global_info() {}
    string version = "V-1.0.0";
    vector<tmplt> templates;
    vector<plugin> plugins;

    INELIB_SERIALIZE(global_info, (version)(templates)(plugins))
};

struct [[inery::table("users"), inery::contract("inerygui")]] user
{
    uint64_t id;
    string guiname;
    checksum256 password;
    vector<checksum256> twords;
    vector<name> accounts;
    vector<string> servers;
    vector<uint64_t> templates;
    vector<user_plugin> plugins;
    string version;

    uint64_t primary_key() const { return id; }

    INELIB_SERIALIZE(user, (id)(guiname)(password)(twords)(accounts)(servers)(templates)(plugins)(version))
};

struct [[inery::table("history"), inery::contract("inerygui")]] history
{
    name composition;
    string guiname;
    vector<history_object> createhistory;
    vector<history_object> updatehistory;
    vector<history_object> deletehistory;

    uint64_t primary_key() const { return composition.value; }

    INELIB_SERIALIZE(history, (composition)(guiname)(createhistory)(updatehistory)(deletehistory))
};

struct [[inery::table("permissions"), inery::contract("inerygui")]] permissions
{
    name composition;
    name perm_name;

    uint64_t primary_key() const { return composition.value; }

    INELIB_SERIALIZE(permissions, (composition)(perm_name))
};

typedef inery::singleton<"global"_n, global_info> global_sing;

typedef inery::multi_index<"user"_n, user> user_t;

typedef inery::multi_index<"history"_n, history> history_t;

typedef inery::multi_index<"permissions"_n, permissions> permissions_t;

class [[inery::contract("inerygui")]] inerygui : public contract
{
public:
    using contract::contract;
    inerygui(name s, name code, datastream<const char *> ds);
    ~inerygui();

    // Global actions
    ACTION addtemplateg(uint64_t id, string category_name, string name, string desc, inery::name code, uint64_t rate_score);
    ACTION addpluging(uint64_t id, string plugin_name, string plugin_desc, bool required);
    ACTION upversiong(string version);
    ACTION rmpluging();

    // User actions
    ACTION rmusers();
    ACTION waversion();
    // registration and login
    ACTION adduser(string guiname, checksum256 password, string version);
    ACTION addtwords(string guiname, vector<checksum256> twords);
    ACTION recoverpsw(string guiname, vector<checksum256> twords, checksum256 newpassword);
    ACTION upversionu(string guiname);

    // accounts and servers
    ACTION addaccount(string guiname, name account);
    ACTION rmaccount(string guiname, name account);
    ACTION addserver(string guiname, string server);
    ACTION rmserver(string guiname, string server);

    // templates and plugins
    ACTION addtemplateu(string guiname, uint64_t template_id);
    ACTION rmtemplateu(string guiname, uint64_t template_id);

    ACTION addpluginu(string guiname, uint64_t plugin_id);
    ACTION rmpluginu(string guiname, uint64_t plugin_id);
    ACTION turnonplugin(string guiname, uint64_t plugin_id);
    ACTION turnofplugin(string guiname, uint64_t plugin_id);

    // history

    ACTION addhistory(name owner, name composition, history_object object, string type);

    // permission

    ACTION linkperm(name owner, name composition, name perm_name);

    // Helper actions
    ACTION sti(string s);
    uint64_t string_to_uint64(string s)
    {
        checksum256 hash = sha256(s.c_str(), s.size());
        ;
        auto bytes = hash.extract_as_byte_array();
        uint64_t value;
        std::memcpy(&value, bytes.data(), sizeof(value));
        print(hash);
        return value;
    };

    ACTION checkoff(name account_name);

private:
    global_sing _global;
    global_info _gstate;

    user_t _user;
};
