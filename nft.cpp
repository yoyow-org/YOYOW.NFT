#include <nft.hpp>
#include <math.h>

//uid of system manager
#define  SYSTEM_MANAGER_UID  28182

//action
void non_fungible_token::init(const uint8_t& version)
{
    graphene_assert(get_trx_sender() == SYSTEM_MANAGER_UID, "invalid authority");
    auto config_itr = _config.find(0);
    graphene_assert(config_itr == _config.end(), "already initialized");
    _config.emplace(SYSTEM_MANAGER_UID, [&](auto& cfg) {
        cfg.key = 0;
        cfg.version = version;
        cfg.category_name_id = 1;
        cfg.next_item_id = 1;
    });
}

//action
void non_fungible_token::create(const uint64_t& issuer, const name& category, const name& tkname, const bool& burnable, const bool& transferable, const string& uri, const uint32_t& max_issue_days, const int64_t& max_supply)
{
    graphene_assert(get_trx_sender() == issuer, "invalid authority");

    int64_t max_issue_window = 0;
    if (max_issue_days == 0) 
    {
        graphene_assert(max_supply > 0, "max-supply must be positive");
    }
    else 
    {
        uint32_t max_issue_seconds = max_issue_days * 24 * 3600;
        max_issue_window = get_head_block_time() + max_issue_seconds;
        graphene_assert(max_supply >= 0, "max supply must be 0 or greater");
    }

    const auto& config = _config.get(0, "config table does not exist, must init first");
    auto category_name_id = config.category_name_id;

    auto existing_category = _category.find(category.value);

    if (existing_category == _category.end()) 
    {
        _category.emplace(issuer, [&](auto& cat) {
            cat.category = category;
        });
    }

    stats_index stats_table(_self, category.value);
    auto existing_token = stats_table.find(tkname.value);
    graphene_assert(existing_token == stats_table.end(), "Token with category and tkname exists");

    stats_table.emplace(issuer, [&](auto& stats) {
        stats.category_name_id = category_name_id;
        stats.issuer = issuer;
        stats.tkname = tkname;
        stats.burnable = burnable;
        stats.transferable = transferable;
        stats.current_supply = 0;
        stats.issued_supply = 0;
        stats.uri = uri;
        stats.max_supply = max_supply;
        stats.max_issue_window = max_issue_window;
    });

    _config.modify(config, 0, [&](auto& a) {
        a.category_name_id++;
    }); 
}

//action
void non_fungible_token::issue(const uint64_t& to, const name& category, const name& tkname, const int64_t& quantity, const string& relative_uri, const string& memo)
{
    char toname[32];
    graphene_assert(get_account_name_by_id(toname, 32, to) != -1, "to account does not exist");
    graphene_assert(memo.size() <= 256, "memo has more than 256 bytes");

    graphene_assert(quantity > 0, "quantity must be positive");

    graphene_assert(quantity <= 100, "can issue up to 100 at a time");

    stats_index stats_table(_self, category.value);
    const auto& token_stats = stats_table.get(tkname.value, "Token with category and tkname does not exist");

    graphene_assert(get_trx_sender() == token_stats.issuer, "invalid authority");

    if (token_stats.max_issue_window != 0) 
    {
        graphene_assert( get_head_block_time() <= token_stats.max_issue_window, "issue window has closed, cannot issue more");
    }

    if (token_stats.max_supply != 0) 
    {
        graphene_assert(quantity <= (token_stats.max_supply - token_stats.issued_supply), "Cannot issue more than max supply");
    }

    auto issued_supply = token_stats.issued_supply;
    for (uint64_t i = 1; i <= quantity; i++) 
    {
        _mint(to, token_stats.issuer, category, tkname,issued_supply, relative_uri);
        issued_supply += 1;
    }
    _add_balance(to, get_trx_sender(), category, tkname, token_stats.category_name_id, quantity);

    stats_table.modify(token_stats, 0, [&](auto& s) {
        s.current_supply += quantity;
        s.issued_supply += quantity;
    });
}


//action
void non_fungible_token::fixmaxsupply(const name& category, const name& tkname)
{
    graphene_assert(get_trx_sender() == SYSTEM_MANAGER_UID, "invalid authority");
    stats_index stats_table(_self, category.value);
    const auto& token_stats = stats_table.get(tkname.value, "Token with category and tkname does not exist");
    graphene_assert(token_stats.max_issue_window != 0, "can't freeze max supply unless time based minting");
    graphene_assert(token_stats.issued_supply != 0, "need to issue at least one token before freezing");
    stats_table.modify(token_stats, 0, [&](auto& s) {
        s.max_supply = s.issued_supply;
        s.max_issue_window = 0;
    });
}

//action
void non_fungible_token::burn(const uint64_t& owner,const vector<uint64_t>& item_ids)
{
    graphene_assert(get_trx_sender() == owner, "invalid authority");
    graphene_assert(item_ids.size() <= 20, "max batch size of 20");
    for (auto const& item_id : item_ids) 
    {
        const auto& token = _item.get(item_id, "token does not exist");
        graphene_assert(token.owner == owner, "must be token owner");

        stats_index stats_table(_self, token.category.value);
        const auto& token_stats = stats_table.get(token.tkname.value, "token stats not found");

        graphene_assert(token_stats.burnable, "Not burnable");

        stats_table.modify(token_stats, 0, [&](auto& s) {
            s.current_supply -= 1;
        });

        _sub_balance(owner, token_stats.category_name_id, 1);
        _item.erase(token);
    }
}

//action
void non_fungible_token::transfer(const uint64_t& from, const uint64_t& to, const vector<uint64_t>& item_ids, const string& memo)
{
    graphene_assert(item_ids.size() <= 20, "max batch size of 20");
    graphene_assert(from != to, "cannot transfer to self");
    graphene_assert(get_trx_sender() == from, "invalid authority");

    char toname[32];
    graphene_assert(get_account_name_by_id(toname, 32, to) != -1, "to account does not exist");

    graphene_assert(memo.size() <= 256, "memo has more than 256 bytes");

    _changeowner(from, to, item_ids, memo, true);
}


void non_fungible_token::_changeowner(const uint64_t& from, const uint64_t& to, const vector<uint64_t>& item_ids, const string& memo, const bool& istransfer)
{
    graphene_assert(item_ids.size() <= 20, "max batch size of 20");
    for (auto const& item_id : item_ids) 
    {
        const auto& token = _item.get(item_id, "token does not exist");

        stats_index stats_table(_self, token.category.value);
        const auto& token_stats = stats_table.get(token.tkname.value, "token stats not found");

        if (istransfer) {
            graphene_assert(token.owner == from, "must be token owner");
            graphene_assert(token_stats.transferable, "not transferable");
        }

        _item.modify(token, 0, [&](auto& t) {
            t.owner = to;
        });

        _sub_balance(from, token_stats.category_name_id, 1);
        _add_balance(to, get_trx_sender(), token.category, token.tkname, token_stats.category_name_id, 1);
    }
}

void non_fungible_token::_mint(const uint64_t& to, const uint64_t& issuer, const name& category, const name& tkname,const int64_t& issued_supply, const string& relative_uri)
{
    auto item_id = gen_next_item_id();

    _item.emplace(issuer, [&](auto& dg) {
        dg.id = item_id;
        dg.serial_number = issued_supply + 1;
        dg.owner = to;
        dg.category = category;
        dg.tkname = tkname;
        dg.relative_uri = relative_uri;
    });
}

uint64_t non_fungible_token::gen_next_item_id()
{
    const auto& config = _config.get(0, "non_fungible_token config table does not exist, setconfig first");
    auto next_item_id = config.next_item_id;

    _config.modify(config, 0, [&](auto& a) {
        a.next_item_id++;
    });

    return next_item_id;
}

void non_fungible_token::_add_balance(const uint64_t& owner, const uint64_t& ram_payer, const name& category, const name& tkname,const uint64_t& category_name_id, const int64_t& quantity)
{
    account_index to_account(_self, owner);
    auto acct = to_account.find(category_name_id);
    if (acct == to_account.end())
    {
        to_account.emplace(ram_payer, [&](auto& a) {
            a.category_name_id = category_name_id;
            a.category = category;
            a.tkname = tkname;
            a.amount = quantity;
        });
    }
    else
    {
        to_account.modify(acct, 0, [&](auto& a) {
            a.amount += quantity;
        });
    }
}

void non_fungible_token::_sub_balance(const uint64_t& owner, const uint64_t& category_name_id, const int64_t& quantity)
{
    account_index from_account(_self, owner);
    const auto& acct = from_account.get(category_name_id, "token does not exist in account");
    graphene_assert(acct.amount >= quantity, "quantity is more than account balance");

    if (acct.amount == quantity) 
    {
        from_account.erase(acct);
    }
    else 
    {
        from_account.modify(acct, 0, [&](auto& a) {
            a.amount -= quantity;
        });
    }
}

GRAPHENE_ABI(non_fungible_token, (init)(create)(issue)(burn)(transfer)(fixmaxsupply))

