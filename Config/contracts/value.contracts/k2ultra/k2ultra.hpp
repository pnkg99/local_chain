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

struct msg_response
{
    uint64_t id;
    time_point time;
};

class [[inery::contract("k2ultra")]] k2ultra : public contract
{
    struct [[inery::table("requests")]] adresses
    {
        uint64_t scope;

        uint64_t primary_key() const { return scope; }
    };
    typedef inery::multi_index<"adresses"_n, adresses> adresses_t;

    struct [[inery::table("requests")]] requests
    {
        uint64_t scope;
        name contact;
        string alias;
        string key;
        bool cntct;
        bool favorite;
        bool blocked;
        bool muted;
        bool pending;

        uint64_t primary_key() const { return scope; }
        uint64_t by_contact() const { return contact.value; }
    };
    typedef inery::multi_index<"requests"_n, requests, indexed_by<"contact"_n, const_mem_fun<requests, uint64_t, &requests::by_contact>>> requests_t;

    struct [[inery::table("messages")]] messages
    {
        uint64_t id;
        time_point time;
        name sender;
        uint64_t reply_to;
        uint64_t seen;
        string data;
        uint64_t primary_key() const { return id; }
        uint64_t by_sender() const { return sender.value; }
        uint64_t by_seen() const { return seen; }
    };
    typedef inery::multi_index<
        "messages"_n,
        messages,
        indexed_by<
            "sender"_n,
            const_mem_fun<messages, uint64_t, &messages::by_sender>>,
        indexed_by<
            "seen"_n,
            const_mem_fun<messages, uint64_t, &messages::by_seen>>>
        messages_t;

    struct [[inery::table("profiles")]] profiles
    {
        name owner;
        string official_name;
        string password;
        uint64_t twordhash;
        string rsa_pub;
        vector<name> requests;
        vector<name> contacts;
        vector<name> favorites;
        vector<name> blocked;
        vector<name> pending;

        uint64_t primary_key() const { return owner.value; }
        uint64_t by_twords() const { return twordhash; }
    };
    typedef inery::multi_index<"profiles"_n, profiles,
                               indexed_by<"twordhash"_n, const_mem_fun<profiles, uint64_t, &profiles::by_twords>>>
        profiles_t;

    // typedef inery::multi_index<"${table.name}"_n, ${table.name}, indexed_by<"${indxs[1].name}"_n, const_mem_fun<${table.name}, ${indxs[1].function_type}, &${table.name}::by_${indxs[1].name}>> \n> ${table.name}_t;

public:
    using contract::contract;
    k2ultra(name reciever, name code, datastream<const char *> ds);
    ~k2ultra();

    ACTION reguser(name owner, string password, string twordhash, string rsa_pub);
    ACTION login(name owner);
    ACTION getuserbyhash(string s);
    ACTION deleteuser(name owner);
    ACTION deletereqs(name owner);
    ACTION deletereqs2(name owner, name contact);

    ACTION changersa(name owner, string rsa_pub);

    ACTION sendrequest(name from, name to, string key);
    ACTION resprequest(name owner, name from, string key, bool accept);
    ACTION rmcontact(name owner, name contact);
    ACTION blockcontact(name owner, name contact, bool status);
    ACTION favcontact(name owner, name contact, bool status);
    ACTION mutecontact(name owner, name contact, bool status);

    ACTION sendmsg(uint64_t scope, name sender, string data, uint64_t reply_to);
    void validate_accounts(name owner, name from);

    ACTION deletemesgs(uint64_t scope);
    ACTION rraddr();

    ACTION changealias(name owner, name contact, string alias);
    ACTION changepsw(name owner, string password);
    ACTION setoffname(name owner, string official_name);
    ACTION getunseen(uint64_t scope, name sender);

    ACTION seenmsg(uint64_t scope, name receiver);
    ACTION addadr();

    ACTION rrprofiles();
    ACTION pump(uint64_t max);
    ACTION forcerequest(uint64_t scope, name cnt1, name cnt2, string key1, string key2);
    ACTION pushmsg(name scope, name sender, string data, uint64_t reply_to);

    uint64_t string_to_uint64(string s)
    {
        checksum256 hash = sha256(s.c_str(), s.size());
        auto bytes = hash.extract_as_byte_array();
        uint64_t value;
        std::memcpy(&value, bytes.data(), sizeof(value));
        return value;
    };

private:
    profiles_t _profiles;
};
