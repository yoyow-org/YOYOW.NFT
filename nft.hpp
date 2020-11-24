#pragma once

#include <graphenelib/contract.hpp>
#include <graphenelib/dispatcher.hpp>
#include <graphenelib/multi_index.hpp>
#include <graphenelib/print.hpp>
#include <graphenelib/contract_asset.hpp>

#include <algorithm> 

using namespace graphene;
using namespace graphenelib;
using namespace std;

class non_fungible_token : public contract
{
public:
    using contract::contract;

    non_fungible_token(uint64_t id)
        : contract(id),
        _config(_self, _self),
        _item(_self, _self),
        _category(_self, _self)
    {};

    //@abi action
    void init(const uint8_t& version);

    //@abi action
    void create(const uint64_t& issuer, const name& category, const name& tkname, const bool& burnable, const bool& transferable, const string& uri, const uint32_t& max_issue_days, const int64_t& max_supply);

    //@abi action
    void issue(const uint64_t& to, const name& category, const name& tkname, const int64_t& quantity, const string& relative_uri, const string& memo);

    //@abi action
    void fixmaxsupply(const name& category, const name& tkname);

    //@abi action
    void burn(const uint64_t& owner, const vector<uint64_t>& item_ids);

    //@abi action
    void transfer(const uint64_t& from, const uint64_t& to, const vector<uint64_t>& item_ids, const string& memo);


    //@abi table configs
    struct configs
    {
        uint64_t    key = 0;
        uint8_t 	version;
        uint64_t 	category_name_id;
        uint64_t 	next_item_id;

        uint64_t primary_key() const { return key; }
        
        GRAPHENE_SERIALIZE(configs, (key)(version)(category_name_id)(next_item_id))
    };
    typedef  multi_index <N(config), configs> config_index;

    //@abi table categoryinfo
    struct categoryinfo
    {
        name category;

        uint64_t primary_key() const { return category.value; }
        GRAPHENE_SERIALIZE(categoryinfo, (category))
    };

    typedef multi_index < N(category), categoryinfo > category_index;

    //@abi table itemstats
    struct itemstats
    {
        name           tkname;
        bool           burnable;
        bool           transferable;
        uint64_t       issuer;
        uint64_t       category_name_id;
        int64_t        max_supply;
        int64_t        max_issue_window;
        int64_t        current_supply;
        int64_t        issued_supply;
        string         uri;

        uint64_t primary_key() const { return tkname.value; }
        GRAPHENE_SERIALIZE(itemstats, (tkname)(burnable)(transferable)(issuer)(category_name_id)(max_supply)(max_issue_window)(current_supply)(issued_supply)(uri))
    };

    typedef multi_index < N(itemstats), itemstats > stats_index;

    //@abi table item
    struct item
    {
        uint64_t id;
        uint64_t serial_number;
        uint64_t owner;
        name category;
        name tkname;
        string relative_uri;

        uint64_t primary_key() const { return id; }
        uint64_t get_owner() const { return owner; }

        GRAPHENE_SERIALIZE(item, (id)(serial_number)(owner)(category)(tkname)(relative_uri))
    };

    typedef  multi_index < N(item), item,indexed_by< N(byowner), const_mem_fun< item, uint64_t, &item::get_owner> > >   item_index;

    //@abi table accounts
    struct accounts
    {
        uint64_t category_name_id;
        name category;
        name tkname;
        int64_t amount;

        uint64_t primary_key() const { return category_name_id; }

        GRAPHENE_SERIALIZE(accounts, (category_name_id)(category)(tkname)(amount))
    };

    typedef  multi_index < N(accounts), accounts >  account_index;

    config_index _config;
    item_index	 _item;
    category_index _category;

private:
    void _changeowner(const uint64_t& from, const uint64_t& to, const vector<uint64_t>& item_ids, const string& memo, const bool& istransfer);
    void _mint(const uint64_t& to, const uint64_t& issuer, const name& category, const name& tkname,const int64_t& issued_supply, const string& relative_uri);
    uint64_t gen_next_item_id();
    void _add_balance(const uint64_t& owner, const uint64_t& ram_payer, const name& category, const name& tkname,const uint64_t& category_name_id, const int64_t& quantity);
    void _sub_balance(const uint64_t& owner, const uint64_t& category_name_id, const int64_t& quantity);
};
