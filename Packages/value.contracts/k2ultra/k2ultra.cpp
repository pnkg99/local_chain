#include "k2ultra.hpp"
#include <chrono>
#include <ctime>
using inery::name;
using namespace inery;
// Constructor & Destructors
k2ultra::k2ultra(name s, name code, datastream<const char *> ds) : contract(s, code, ds), _profiles(get_self(), get_self().value) {}
k2ultra::~k2ultra() {}
// Account actions
// Registration

// std::string time_point_to_string(const std::chrono::time_point<std::chrono::system_clock> &tp)
// {
//     auto t = std::chrono::system_clock::to_time_t(tp);
//     std::string ts = std::ctime(&t);
//     ts.erase(ts.length() - 1);
//     return ts;
// }

void k2ultra::validate_accounts(name owner, name from)
{
    check(_profiles.find(owner.value) != _profiles.end(), "Owner account is not registered");
    check(_profiles.find(from.value) != _profiles.end(), "From account is not registered");
}
void k2ultra::reguser(name owner, string password, string twordhash, string rsa_pub)
{
    require_auth(owner);
    auto itr = _profiles.find(owner.value);
    check(itr == _profiles.end(), "Account with that name is already registered!");
    uint64_t stivalue = string_to_uint64(twordhash);
    _profiles.emplace(get_self(), [&](auto &row)
                      { row.owner = owner;
                        row.password = password;
                        row.twordhash = stivalue;
                        row.rsa_pub = rsa_pub; });
};
void k2ultra::login(name owner)
{
    require_auth(owner);
    auto itr = _profiles.find(owner.value);
    check(itr != _profiles.end(), "Account is not registered");
};
void k2ultra::getuserbyhash(string s)
{
    require_auth(get_self());
    uint64_t stivalue = string_to_uint64(s);
    auto idx = _profiles.get_index<"twordhash"_n>();
    auto itr = idx.find(stivalue);
    check(itr != idx.end(), "User not found");

    auto found_user = _profiles.get(itr->owner.value);
    print(found_user.owner);
}
void k2ultra::deleteuser(name owner)
{
    require_auth(get_self());
    auto itr = _profiles.find(owner.value);
    check(itr != _profiles.end(), "User doesn't exist");
    _profiles.erase(itr);
    requests_t _requests(_self, owner.value);
    auto req_it = _requests.begin();
    while (req_it != _requests.end())
    {
        req_it = _requests.erase(req_it);
    }
}

void k2ultra::deletereqs(name owner)
{
    requests_t _requests(_self, owner.value);
    auto req_it = _requests.begin();
    while (req_it != _requests.end())
    {
        auto contact_name = req_it->contact;
        auto scope = req_it->scope;
        requests_t _requests2(_self, contact_name.value);
        auto req_it2 = _requests2.find(scope);
        req_it = _requests.erase(req_it);
        if (req_it2 != _requests2.end())
        {
            _requests2.erase(req_it2);
        };
    }
}
void k2ultra::deletereqs2(name owner, name contact)
{
    requests_t _requests(_self, contact.value);
    auto req_idx = _requests.get_index<name("contact")>();
    auto req_it = req_idx.find(owner.value);
    check(req_it != req_idx.end(), "Contact does not have owner as request");
    req_idx.erase(req_it);
}
void k2ultra::changersa(name owner, string rsa_pub)
{
    require_auth(owner);
    auto itr = _profiles.find(owner.value);
    check(itr != _profiles.end(), "Account with that name is already registered!");
    _profiles.modify(itr, get_self(), [&](auto &row)
                     { row.rsa_pub = rsa_pub; });
}

void k2ultra::sendrequest(name from, name to, string key)
{
    require_auth(from);
    validate_accounts(from, to);

    requests_t _requests(_self, to.value);
    requests_t _requests2(_self, from.value);
    adresses_t adrs(_self, _self.value);

    uint64_t scope = adrs.available_primary_key();

    auto contact_idx1 = _requests.get_index<name("contact")>();
    auto req_it = contact_idx1.find(from.value);
    check(req_it == contact_idx1.end(), "Request already sent to this user");

    auto contact_idx2 = _requests2.get_index<name("contact")>();
    auto req_it2 = contact_idx2.find(to.value);
    check(req_it2 == contact_idx2.end(), "You have pending request from this user");

    _requests.emplace(_self, [&](auto &row)
                      { 
                        row.scope = scope; 
                        row.contact = from;
                        row.alias = "";
                        row.key = key; });
    _requests2.emplace(_self, [&](auto &row)
                       { 
                        row.scope = scope; 
                        row.contact = to;
                        row.alias = "";
                        row.pending = true; });
    adrs.emplace(_self, [&](auto &row)
                 { row.scope = scope; });
};
void k2ultra::resprequest(name owner, name from, string key, bool accept)
{
    require_auth(owner);

    auto owner_itr = _profiles.find(owner.value);
    check(owner_itr != _profiles.end(), "Owner account is not reqistered");
    auto from_itr = _profiles.find(from.value);
    check(from_itr != _profiles.end(), "From account is not reqistered");

    requests_t owners_requests(_self, owner.value);
    auto req_idx = owners_requests.get_index<name("contact")>();
    auto req_it = req_idx.find(from.value);
    requests_t responders_requests(_self, from.value);
    if (req_it == req_idx.end())
    {
        check(false, "Request not found for this combination of owner and from");
    }
    uint64_t scope = req_it->scope;
    if (accept)
    {

        auto new_req_it = responders_requests.find(scope);
        check(new_req_it != responders_requests.end(), "Request dont exists");
        check(!new_req_it->cntct, "You are already contacts");

        responders_requests.modify(new_req_it, _self, [&](auto &row)
                                   {
                                    row.key = key; 
                                    row.pending = false;
                                    row.cntct = true; });
        auto owner_itr = owners_requests.find(scope);

        owners_requests.modify(owner_itr, _self, [&](auto &row)
                               {
                                    row.pending = false;
                                    row.cntct = true ; });
    }
    else
    {
        auto n_itr = owners_requests.find(scope);
        owners_requests.erase(n_itr);
        auto m_itr = responders_requests.find(scope);
        responders_requests.erase(m_itr);
    }
};

void k2ultra::rmcontact(name owner, name contact)
{
    require_auth(owner);
    requests_t req1(_self, owner.value);
    requests_t req2(_self, contact.value);
    auto req_idx = req1.get_index<name("contact")>();
    auto name_itr = req_idx.find(contact.value);
    if (name_itr == req_idx.end())
    {
        check(false, "Request not found for this combination of owner and from");
    }
    uint64_t scope = name_itr->scope;
    auto n_itr = req1.find(scope);
    auto m_itr = req2.find(scope);
    req1.erase(n_itr);
    req2.erase(m_itr);
};
void k2ultra::sendmsg(uint64_t scope, name sender, string data, uint64_t reply_to)
{
    require_auth(sender);
    messages_t _messages(_self, scope);
    time_point ct = current_time_point();
    uint64_t new_id = _messages.available_primary_key() == 0 ? 1 : _messages.available_primary_key();
    _messages.emplace(get_self(), [&](auto &row)
                      {
        row.id = new_id;
        row.time = ct;
        row.sender = sender;
        row.data = data;
        row.seen = 0;
        if (reply_to != 0)
        {
            row.reply_to = reply_to;
        } });
    // std::string time_str = std::to_string(ct.sec_since_epoch());
    auto time_str = ct.sec_since_epoch();
    std::string object_str = "{\"time\": \"" + std::to_string(time_str) + "\", \"id\":" + std::to_string(new_id) + "}";
    print(object_str);
}
void k2ultra::pushmsg(name scope, name sender, string data, uint64_t reply_to)
{
    messages_t _messages(_self, scope.value);
    time_point ct = current_time_point();
    uint64_t new_id = _messages.available_primary_key() == 0 ? 1 : _messages.available_primary_key();
    _messages.emplace(get_self(), [&](auto &row)
                      {
        row.id = new_id;
        row.time = ct;
        row.sender = sender;
        row.data = data;
        row.seen = 1;
        if (reply_to != 0)
        {
            row.reply_to = reply_to;
        } });
}
void k2ultra::getunseen(uint64_t scope, name sender)
{
    messages_t _messages(_self, scope);

    auto idx = _messages.get_index<"seen"_n>();
    auto itr = idx.upper_bound(0); // Find the upper bound for value 0

    int cnt = 0;
    int max_iterations = 1000;
    int iterations = 0;

    // Move the iterator to the last element with seen == 0
    if (itr != idx.begin())
    {
        --itr;
    }
    else
    {
        print(cnt);
        return;
    }

    while (itr != idx.begin() && itr->seen == 0 && iterations < max_iterations && cnt < 100)
    {
        if (itr->sender == sender)
        {
            cnt++;
        }
        iterations++;
        --itr;
    }

    // Check the first element if it matches the conditions
    if (itr->seen == 0 && itr->sender == sender && iterations < max_iterations)
    {
        cnt++;
    }

    print(cnt);
};

void k2ultra::blockcontact(name owner, name contact, bool status)
{
    validate_accounts(owner, contact);
    require_auth(owner);
    requests_t req1(_self, owner.value);
    auto req_idx = req1.get_index<name("contact")>();
    auto name_itr = req_idx.find(contact.value);
    if (name_itr == req_idx.end())
    {
        check(false, "Contact not found");
    }
    uint64_t scope = name_itr->scope;
    auto n_itr = req1.find(scope);
    if (status)
    {
        req1.modify(n_itr, _self, [&](auto &row)
                    { row.blocked = true; row.cntct = false; row.favorite = false; });
    }
    else
    {
        req1.modify(n_itr, _self, [&](auto &row)
                    { row.blocked = false ;row.cntct = true; });
    }
}
void k2ultra::favcontact(name owner, name contact, bool status)
{
    validate_accounts(owner, contact);
    require_auth(owner);
    requests_t req1(_self, owner.value);
    auto req_idx = req1.get_index<name("contact")>();
    auto name_itr = req_idx.find(contact.value);
    if (name_itr == req_idx.end())
    {
        check(false, "Contact not found");
    }
    uint64_t scope = name_itr->scope;
    auto n_itr = req1.find(scope);
    if (status)
    {
        req1.modify(n_itr, _self, [&](auto &row)
                    { row.favorite = true; });
    }
    else
    {
        req1.modify(n_itr, _self, [&](auto &row)
                    { row.favorite = false; });
    }
};
void k2ultra::seenmsg(uint64_t scope, name receiver)
{
    // require_auth(receiver);
    messages_t _messages(_self, scope);

    auto seen_idx = _messages.get_index<"seen"_n>();
    // Find the lower bound for value 0
    auto itr = seen_idx.lower_bound(0);

    if (itr == seen_idx.end() || itr->seen == 1)
    {
        print(0);
        return;
    }

    int max_iterations = 1000;
    int iterations = 0;

    while (itr != seen_idx.end() && iterations < max_iterations)
    {
        auto itr_modify = _messages.find(itr->id);
        if (itr_modify != _messages.end())
        {
            // Increment the iterator before modifying the table
            auto next_itr = itr;
            ++next_itr;

            _messages.modify(itr_modify, _self, [&](auto &row)
                             { row.seen = 1; });

            itr = next_itr;
        }
        else
        {
            ++itr;
        }

        iterations++;
    }
    if (iterations == max_iterations)
    {
        print(1);
    }
}
void k2ultra::mutecontact(name owner, name contact, bool status)
{
    validate_accounts(owner, contact);
    require_auth(owner);
    requests_t req1(_self, owner.value);
    auto req_idx = req1.get_index<name("contact")>();
    auto name_itr = req_idx.find(contact.value);
    if (name_itr == req_idx.end())
    {
        check(false, "Contact not found");
    }
    uint64_t scope = name_itr->scope;
    auto n_itr = req1.find(scope);
    if (status)
    {
        req1.modify(n_itr, _self, [&](auto &row)
                    { row.muted = true; });
    }
    else
    {
        req1.modify(n_itr, _self, [&](auto &row)
                    { row.muted = false; });
    }
};

void k2ultra::deletemesgs(uint64_t scope)
{
    messages_t _messages(_self, scope);
    auto msg_itr = _messages.begin();
    while (msg_itr != _messages.end())
    {
        msg_itr = _messages.erase(msg_itr);
    }
};
void k2ultra::rraddr()
{
    adresses_t adrs(_self, _self.value);
    auto addr_itr = adrs.begin();
    while (addr_itr != adrs.end())
    {
        addr_itr = adrs.erase(addr_itr);
    }
};

void k2ultra::rrprofiles()
{

    auto itr = _profiles.begin();
    while (itr != _profiles.end())
    {
        itr = _profiles.erase(itr);
    }
};

// OFFICIAL NAME
void k2ultra::setoffname(name owner, string official_name)
{
    require_auth(owner);

    auto itr = _profiles.find(owner.value);
    check(itr != _profiles.end(), "Owner account is not registered");

    _profiles.modify(itr, owner, [&](auto &row)
                     { row.official_name = official_name; });
};
// PASSWORD CHANGE
void k2ultra::changepsw(name owner, string password)
{
    require_auth(owner);

    auto itr = _profiles.find(owner.value);
    check(itr != _profiles.end(), "Owner account is not registered");

    _profiles.modify(itr, owner, [&](auto &row)
                     { row.password = password; });
};
// ALIAS NAME
void k2ultra::changealias(name owner, name contact, string alias)
{
    require_auth(owner);

    requests_t _requests(_self, owner.value);

    auto req_idx = _requests.get_index<name("contact")>();
    auto req_it = req_idx.find(contact.value);
    if (req_it == req_idx.end())
    {
        check(false, "Owner and Contact are not connected");
    }
    uint64_t scope = req_it->scope;
    auto new_itr = _requests.find(scope);

    _requests.modify(new_itr, _self, [&](auto &row)
                     { row.alias = alias; });
};

void k2ultra::addadr()
{
    adresses_t adrs(_self, _self.value);

    adrs.emplace(_self, [&](auto &row)
                 { row.scope = adrs.available_primary_key(); });
}

void k2ultra::pump(uint64_t max)
{
    adresses_t adrs(_self, _self.value);

    for (uint64_t i = 0; i < max; i++)
    {
        adrs.emplace(_self, [&](auto &row)
                     { row.scope = i; });
    }
};
void k2ultra::forcerequest(uint64_t scope, name cnt1, name cnt2, string key1, string key2)
{
    requests_t _requests(_self, cnt1.value);
    requests_t _requests2(_self, cnt2.value);

    _requests.emplace(_self, [&](auto &row)
                      { 
                        row.scope = scope; 
                        row.contact = cnt2;
                        row.alias = "";
                        row.key = key2;
                        row.cntct = true; });
    _requests2.emplace(_self, [&](auto &row)
                       { 
                        row.scope = scope; 
                        row.contact = cnt1;
                        row.alias = "";
                        row.key = key1;
                        row.cntct = true; });
};
