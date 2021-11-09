#include "consumption-request.h"
#include "industry-location.h"
#include "util-common.h"

#include "player.h"
#include "player-network.h"

#include <queue>
#include <vector>
#include <set>
#include <algorithm>

using CostLocationPair = std::pair<int, Location*>;
using LocationCostPair = std::pair<Location*, int> ;
using LocationPair = std::pair<Location*, Location*>;
using CostLocationMap = std::map<int, Location*>;
using LocationCostMap = std::map<Location*, int, ptr_less<Location> > ;
using LocationMap = std::map<Location*, Location*, ptr_less<Location> >;

ConsumptionRequirement::ConsumptionRequirement(ResourceType resource_type, int units)
    : resource_type_(resource_type), units_(units) {}

ConsumptionRequirement::~ConsumptionRequirement() {}

void ConsumptionRequirement::Consume() {
  for (std::vector<std::pair<Industry*, int> >::const_reference pair : chosen_units_) {
    if (pair.second > 0) {
      pair.first->ProvideResources(pair.second);
    }
  }
}

std::vector<Industry*> ConsumptionRequirement::GetConnectedIndustries(IndustryType industry_type,
                                                                      Location* source,
                                                                      std::vector<Merchant*>* connected_merchants,
                                                                      IndustryType merchant_industry_type) const {
  std::vector<Industry*> industries;
  std::vector<Location*> locations = source->connected_locations();
  bool get_connected_merchants = connected_merchants != nullptr;
  for (Location* location : locations) {
    IndustryLocation* industry_location = dynamic_cast<IndustryLocation*>(location);
    if (industry_location != nullptr) {
      std::vector<Industry*> industries_obtained = industry_location->industries_of_type(industry_type);
      industries.insert(industries.end(), industries_obtained.begin(), industries_obtained.end());
    } else if (get_connected_merchants) {
      MerchantLocation* loc = dynamic_cast<MerchantLocation*>(location);
      std::vector<Merchant*> merchants = loc->merchants(merchant_industry_type);
      connected_merchants->insert(connected_merchants->end(), merchants.begin(), merchants.end());
    }
  }
  std::vector<Industry*> industries_impl;
  for (Industry* industry : industries) {
    if (industry->impl() != nullptr)
      industries_impl.push_back(industry->impl());
  }
  return industries_impl;
}

std::vector<Industry*> ConsumptionRequirement::GetConnectedIndustries(IndustryType industry_type,
                                                                      std::vector<Location*> sources,
                                                                      std::vector<Adjacency*> extra_connections,
                                                                      std::vector<Merchant*>* connected_merchants,
                                                                      IndustryType merchant_industry_type) const {
  using LocationPtrSet = std::set<Location*, ptr_less<Location> >;
  
  std::vector<Industry*> industries;
  std::stack<Location*> stack;
  LocationPtrSet locations;

  bool get_connected_merchants = connected_merchants != nullptr;

  for (Location* source : sources) {
    stack.push(source);
  }

  while (!stack.empty()) {
    Location* location = stack.top();
    stack.pop();
    locations.insert(location);
    std::vector<Location*> adjs = location->adjacent_connected_locations();
    for (Adjacency* adj : extra_connections) {
      Location* other = adj->another_end(location);
      if (other != nullptr) {
        adjs.push_back(other);
      }
    }
    for (Location* other : adjs) {
      if (locations.count(other) <= 0) {
        stack.push(other);
      }
    }
  }
  for (Location* location : locations) {
    IndustryLocation* industry_location = dynamic_cast<IndustryLocation*>(location);
    if (industry_location != nullptr) {
      std::vector<Industry*> industries_obtained = industry_location->industries_of_type(industry_type);
      industries.insert(industries.end(), industries_obtained.begin(), industries_obtained.end());
    } else if (get_connected_merchants) {
      MerchantLocation* loc = dynamic_cast<MerchantLocation*>(location);
      std::vector<Merchant*> merchants = loc->merchants(merchant_industry_type);
      connected_merchants->insert(connected_merchants->end(), merchants.begin(), merchants.end());
    }
  }
  std::vector<Industry*> industries_impl;
  for (Industry* industry : industries) {
    if (industry->impl() != nullptr)
      industries_impl.push_back(industry->impl());
  }
  return industries_impl;
}

std::vector<Industry*> ConsumptionRequirement::GetConnectedIndustriesSortedByDistance(IndustryType industry_type,
                                                                                      std::vector<Location*> sources,
                                                                                      std::vector<Adjacency*> extra_connections) const {
  std::vector<Industry*> industries;
  std::vector<Location*> locations = GetConnectedLocationsSortedByDistance(sources, extra_connections);
  for (Location* location : locations) {
    IndustryLocation* industry_location = dynamic_cast<IndustryLocation*>(location);
    if (industry_location != nullptr) {
      std::vector<Industry*> industries_obtained = industry_location->industries_of_type(industry_type);
      industries.insert(industries.end(), industries_obtained.begin(), industries_obtained.end());
    }
  }
  std::vector<Industry*> industries_impl;
  for (Industry* industry : industries) {
    if (industry->impl() != nullptr)
      industries_impl.push_back(industry->impl());
  }
  return industries_impl;
}

std::vector<Location*> ConsumptionRequirement::GetConnectedLocationsSortedByDistance(std::vector<Location*> sources,
                                                                                     std::vector<Adjacency*> extra_connections) const {
  using LocationPtrSet = std::set<Location*, ptr_less<Location> >;
  
  std::priority_queue<CostLocationPair*> q;
  LocationPtrSet dq;  // locations already expanded 

  LocationCostMap dist;
  for (Location* source : sources) {
    dist[source] = 0;
  }

  while (!q.empty()) {
    CostLocationPair* pair = q.top();
    Location* location = pair->second;
    q.pop();
    if (dq.count(location) == 0) {
      std::vector<Location*> adjs = location->adjacent_connected_locations();
      for (Adjacency* adj : extra_connections) {
        Location* other = adj->another_end(location);
        if (other != nullptr) {
          adjs.push_back(other);
        }
      }
      for (Location* adj : adjs) {  // bfs
        if ((dist.count(adj) == 0) || (dist[location] + 1 < dist[adj])) {
          dist[adj] = dist[location] + 1;
          q.push(new CostLocationPair(-dist[adj], adj));
        }
      }
      dq.insert(location);
    }
    delete pair;
  }

  std::vector<LocationCostPair> vec = { dist.begin(), dist.end() };
  std::sort(vec.begin(), vec.end(),
            [](const LocationCostPair& a, const LocationCostPair& b)
            { return a.second < b.second; });
  std::vector<Location*> locations;
  for (LocationCostPair& location : vec) {
    locations.push_back(location.first);
  }
  return locations;
}

ResourceType ConsumptionRequirement::resource_type() const {
  return resource_type_;
}

bool ConsumptionRequirement::may_be_feasible() const {
  return may_be_feasible_;
}

int ConsumptionRequirement::industry_units() const {
  return industry_units_;
}

int ConsumptionRequirement::market_units() const {
  return 0;
}

int ConsumptionRequirement::market_price() const {
  return 0;
}

int ConsumptionRequirement::basic_price() const {
  return basic_price_;
}

const std::vector<std::pair<Industry*, int> >& ConsumptionRequirement::chosen_units() const {
  return chosen_units_;
}

void ConsumptionRequirement::set_chosen_units(std::vector<std::pair<Industry*, int> > chosen_units) {
  chosen_units_ = chosen_units;
}

CoalConsumptionRequirement::CoalConsumptionRequirement(Location* location,
                                                       int units, Market* market)
    : ConsumptionRequirement(ResourceType::kCoal, units), market_(market) {
  sources_.push_back(location);
}

CoalConsumptionRequirement::CoalConsumptionRequirement(Adjacency* adj, int units,
                                                       Market* market)
    : ConsumptionRequirement(ResourceType::kCoal, units), market_(market) {
  sources_.push_back(adj->ends()[0]);
  sources_.push_back(adj->ends()[1]);
}

CoalConsumptionRequirement::CoalConsumptionRequirement(Adjacency* adj, int units,
                                                       Market* market,
                                                       Adjacency* extra_adj)
    : CoalConsumptionRequirement(adj, units, market) {
  extra_connections_.push_back(extra_adj);
}

bool CoalConsumptionRequirement::ComputeRequirement(int& basic_price) {
  std::vector<Industry*> industries =
      GetConnectedIndustriesSortedByDistance(IndustryType::kCoalMine, sources_, extra_connections_);

  int units_in_short = units_;
  for (Industry* other : industries) {
    int available_units = other->available_units();
    if (available_units > 0) {
      int consumption_units = min(available_units, units_in_short);
      chosen_units_.emplace_back(other, consumption_units);
      units_in_short -= consumption_units;
    }
    if (units_in_short == 0) {
      break;
    }
  }

  basic_price_ = 0;
  industry_units_ = 0;
  market_units_ = 0;
  may_be_feasible_ = false;
  if (units_in_short == 0) {
    industry_units_ = units_;
    may_be_feasible_ = true;
  } else {
    for (Location* source : sources_) {
      if (source->IsConnectedToMerchantLocation()) {
        may_be_feasible_ = true;
        industry_units_ = units_ - units_in_short;
        market_units_ = units_in_short;
        basic_price_ = market_->GetTotalPrice(market_units_);
        break;
      }
    }
  }
  basic_price = basic_price_;
  return may_be_feasible_;
}

void CoalConsumptionRequirement::Consume() {
  if (may_be_feasible_) {
    ConsumptionRequirement::Consume();
    market_->Sell(market_units_);
  }
}

int CoalConsumptionRequirement::market_units() const {
  return market_units_;
}

int CoalConsumptionRequirement::market_price() const {
  return market_->GetTotalPrice(market_units_);
}

IronConsumptionRequirement::IronConsumptionRequirement(Map* map, int units, Market* market)
    : ConsumptionRequirement(ResourceType::kIron, units), map_(map), market_(market) {}

bool IronConsumptionRequirement::ComputeRequirement(int& basic_price) {
  std::vector<Industry*> industries = map_->industries_of_type(IndustryType::kIronWorks);

  int units_in_short = units_;
  for (Industry* other : industries) {
    int available_units = other->available_units();
    if (available_units > 0) {
      int consumption_units = min(available_units, units_in_short);
      chosen_units_.emplace_back(other, 0);
      units_in_short -= consumption_units;
    }
  }

  basic_price_ = 0;
  industry_units_ = 0;
  market_units_ = 0;
  may_be_feasible_ = false;
  if (units_in_short == 0) {
    industry_units_ = units_;
    may_be_feasible_ = true;
  } else {
    may_be_feasible_ = true;
    industry_units_ = units_ - units_in_short;
    market_units_ = units_in_short;
    basic_price_ = market_->GetTotalPrice(market_units_);
  }
  basic_price = basic_price_;
  return may_be_feasible_;
}

void IronConsumptionRequirement::Consume() {
  if (may_be_feasible_) {
    ConsumptionRequirement::Consume();
    market_->Sell(market_units_);
  }
}

int IronConsumptionRequirement::market_units() const {
  return market_units_;
}

int IronConsumptionRequirement::market_price() const {
  return market_->GetTotalPrice(market_units_);
}

BeerConsumptionRequirement::BeerConsumptionRequirement(Location* location, int units, PlayerNetwork* network)
    : ConsumptionRequirement(ResourceType::kBeer, units), network_(network) {
  sources_.push_back(location);
}

BeerConsumptionRequirement::BeerConsumptionRequirement(Location* location, int units, PlayerNetwork* network, IndustryType industry_type)
    : ConsumptionRequirement(ResourceType::kBeer, units), use_merchant_beer_(true), industry_type_(industry_type), network_(network) {
  sources_.push_back(location);
}

BeerConsumptionRequirement::BeerConsumptionRequirement(Adjacency* adj, int units, PlayerNetwork* network)
    : ConsumptionRequirement(ResourceType::kBeer, units), network_(network) {
  sources_.push_back(adj->ends()[0]);
  sources_.push_back(adj->ends()[1]);
}

BeerConsumptionRequirement::BeerConsumptionRequirement(Adjacency* adj, int units, PlayerNetwork* network,
                                                       Adjacency* extra_adj)
    : BeerConsumptionRequirement(adj, units, network) {
  extra_connections_.push_back(extra_adj);
}

bool BeerConsumptionRequirement::ComputeRequirement(int& basic_price) {
  basic_price = 0;

  IndustryPtrSet set;
  std::vector<Industry*> player_industries = network_->industries_of_type(IndustryType::kBrewery);
  set.insert(player_industries.begin(), player_industries.end());
  std::vector<Merchant*> merchants;
  std::vector<Industry*> connected_industries = GetConnectedIndustries(
    IndustryType::kBrewery, sources_, extra_connections_, (use_merchant_beer_)? &merchants : nullptr, industry_type_);
  set.insert(connected_industries.begin(), connected_industries.end());

  int units_in_short = units_;
  for (Industry* other : set) {
    int available_units = other->available_units();
    if (available_units > 0) {
      int consumption_units = min(available_units, units_in_short);
      chosen_units_.emplace_back(other, 0);
      units_in_short -= consumption_units;
    }
  }

  industry_units_ = 0;
  if (units_in_short == 0) {
    industry_units_ = units_;
    may_be_feasible_ = true;
  } else {
    may_be_feasible_ = false;
  }

  return may_be_feasible_;
}

void BeerConsumptionRequirement::Consume() {
  if (may_be_feasible_) {
    ConsumptionRequirement::Consume();
  }
}

ActionRequirement::ActionRequirement(Player* player, Map* map,
                                       Market* coal_market, Market* iron_market, int price)
    : player_(player), map_(map),
      coal_market_(coal_market), iron_market_(iron_market), basic_price_(price) {}

ActionRequirement::~ActionRequirement() {
  for (ConsumptionRequirement* req : reqs_) {
    delete req;
  }
}

bool ActionRequirement::ComputeRequirement(Player* player) {
  bool feasible = true;
  total_price_ = basic_price_;
  for (ConsumptionRequirement* req : reqs_) {
    bool may_be_feasible = false;
    int consumption_price = 0;
    may_be_feasible = req->ComputeRequirement(consumption_price);
    feasible = feasible && may_be_feasible;
    total_price_ += consumption_price;
  }
  if (feasible) {
    feasible = player->CanWithdraw(total_price_);
  }
  return feasible;
}

void ActionRequirement::AddCoalRequirement(Adjacency* adj, int units) {
  reqs_.push_back(new CoalConsumptionRequirement(adj, units, coal_market_));
}

void ActionRequirement::AddCoalRequirement(Location* loc, int units) {
  reqs_.push_back(new CoalConsumptionRequirement(loc, units, coal_market_));
}

void ActionRequirement::AddIronRequirement(int units) {
  reqs_.push_back(new IronConsumptionRequirement(map_, units, iron_market_));
}

void ActionRequirement::AddBeerRequirement(Location* loc, int units) {
  reqs_.push_back(new BeerConsumptionRequirement(loc, units, &player_->network()));
}

void ActionRequirement::AddBeerRequirement(Adjacency* loc, int units) {
  reqs_.push_back(new BeerConsumptionRequirement(loc, units, &player_->network()));
}

void ActionRequirement::Consume() {
  for (ConsumptionRequirement* req : reqs_) {
    req->Consume();
  }
}

bool ActionRequirement::ChooseIndustriesToConsume(Io& io) {
  std::map<ResourceType, ConsumptionInfo> consumption_info;
  for (ConsumptionRequirement* req : reqs_) {
    ResourceType resource_type = req->resource_type();
    consumption_info.emplace(
      resource_type,
      ConsumptionInfo {
        req->industry_units(),
        req->market_units(),
        req->market_price()
      }
    );
    std::vector<std::pair<IndustryInfo, int> >& info = consumption_info.at(resource_type).chosen_units;
    for (std::vector<std::pair<Industry*, int> >::const_reference pair : req->chosen_units()) {
      Industry* industry = pair.first;
      const IndustryProperties properties = industry->properties();
      info.emplace_back(
        IndustryInfo {
          ToString(industry->industry_type()),
          industry->location()->name(),
          industry->location()->color(),
          properties.tech_level,
          properties.vp,
          properties.vp_for_adj_link,
          0,
          industry->available_units(),
          false,
          industry->player_name(),
        },
        pair.second
      );
    }
  }
  if (!io.ChooseIndustriesToConsume(consumption_info)) {
    return false;
  }
  bool satisfied = true;
  for (ConsumptionRequirement* req : reqs_) {
    ResourceType resource_type = req->resource_type();
    std::vector<std::pair<Industry*, int> > chosen_units;
    const std::vector<std::pair<Industry*, int> >& orig_chosen_units = req->chosen_units();
    const std::vector<std::pair<IndustryInfo, int> >& new_chosen_units = consumption_info.at(resource_type).chosen_units;
    int consumption_units = 0;
    for (std::vector<std::pair<Industry*, int> >::size_type i = 0; i < orig_chosen_units.size(); i++) {
      chosen_units.emplace_back(orig_chosen_units[i].first, new_chosen_units[i].second);
      consumption_units += new_chosen_units[i].second;
    }
    req->set_chosen_units(chosen_units);
    satisfied = satisfied && (consumption_units == req->industry_units());
  }
  if (satisfied) {
    io.ofstream << "[ActionRequirement::ChooseIndustriesToConsume()] consumption report" << NEW_LINE;
    for (std::map<ResourceType, ConsumptionInfo>::const_reference pair : consumption_info) {
      io.ofstream << "== " << ToString(pair.first) << " consumption ==" << NEW_LINE;
      const ConsumptionInfo& info = pair.second;
      for (std::vector<std::pair<IndustryInfo, int> >::const_reference p : info.chosen_units) {
        const IndustryInfo& industry_info = p.first;
        const int& units = p.second;
        io.ofstream << ">> " << industry_info.industry_type << "@" << industry_info.location
          << " | owner = " << industry_info.owner
          << " | available units = " << industry_info.available_units
          << " | chosen units = " << units
          << NEW_LINE;
      }
      io.ofstream << "* market units: " << info.market_units << " for $" << info.market_price << std::endl;
    }
  }
  return satisfied;
}

ActionRequirementFactory::ActionRequirementFactory(Map* map,
                                                     Market* coal_market,
                                                     Market* iron_market)
    : map_(map), coal_market_(coal_market), iron_market_(iron_market) {}

ActionRequirement* ActionRequirementFactory::CreateActionRequirement(Player* player, int price) {
  return new ActionRequirement(player, map_, coal_market_, iron_market_, price);
}
