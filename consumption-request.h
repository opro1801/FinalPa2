#ifndef BRASS_GAME_CONSUMPTION_REQUEST_H_
#define BRASS_GAME_CONSUMPTION_REQUEST_H_

#include "game-common.h"
#include "location.h"
#include "industry.h"
#include "game-map.h"
#include "market.h"
#include "game-io.h"
#include "player-network.h"

#include <vector>

class ConsumptionRequirement {
 public:
  ConsumptionRequirement(ResourceType resource_type, int units);
  virtual ~ConsumptionRequirement();

  virtual bool ComputeRequirement(int& basic_price) = 0;

  virtual void Consume() = 0;

  ResourceType resource_type() const;

  bool may_be_feasible() const;
  int industry_units() const;
  virtual int market_units() const;
  virtual int market_price() const;
  int basic_price() const;

  const std::vector<std::pair<Industry*, int> >& chosen_units() const;

  void set_chosen_units(std::vector<std::pair<Industry*, int> > chosen_units);

 protected:
  using IndustryPtrSet = std::set<Industry*, ptr_less<Industry> >;

  std::vector<Industry*> GetConnectedIndustries(IndustryType industry_type,
                                                Location* source,
                                                std::vector<Merchant*>* connected_merchants = nullptr,
                                                IndustryType mechant_industry_type = IndustryType::kUndefined) const;
  std::vector<Industry*> GetConnectedIndustries(IndustryType industry_type,
                                                std::vector<Location*> sources,
                                                std::vector<Adjacency*> extra_connections,
                                                std::vector<Merchant*>* connected_merchants = nullptr,
                                                IndustryType merchant_industry_type = IndustryType::kUndefined) const;
  std::vector<Industry*> GetConnectedIndustriesSortedByDistance(IndustryType industry_type,
                                                                std::vector<Location*> sources,
                                                                std::vector<Adjacency*> extra_connections) const;

  std::vector<Location*> GetConnectedLocationsSortedByDistance(std::vector<Location*> sources,
                                                               std::vector<Adjacency*> extra_connections) const;

  ResourceType resource_type_;
  int units_ = 0;

  int industry_units_ = 0;
  int basic_price_ = 0;
  bool may_be_feasible_ = false;

  std::vector<std::pair<Industry*, int> > chosen_units_;
};

class CoalConsumptionRequirement : public ConsumptionRequirement {
 public:
  CoalConsumptionRequirement(Location* source, int units, Market* market);
  CoalConsumptionRequirement(Adjacency* adj, int units, Market* market);
  CoalConsumptionRequirement(Adjacency* adj, int units, Market* market, Adjacency* extra_adj);

  virtual bool ComputeRequirement(int& basic_price) override;
  virtual void Consume() override;

  virtual int market_units() const override;
  virtual int market_price() const override;

 private:
  std::vector<Location*> sources_;
  std::vector<Adjacency*> extra_connections_;

  Market* market_;
  int market_units_;
};

class IronConsumptionRequirement : public ConsumptionRequirement {
 public:
  IronConsumptionRequirement(Map* map, int units, Market* market);

  virtual bool ComputeRequirement(int& basic_price) override;
  virtual void Consume() override;

  virtual int market_units() const override;
  virtual int market_price() const override;

 private:
  Map* map_;

  Market* market_;
  int market_units_;
};

class BeerConsumptionRequirement : public ConsumptionRequirement {
 public:
  BeerConsumptionRequirement(Location* source, int units, PlayerNetwork* network);
  BeerConsumptionRequirement(Location* source, int units, PlayerNetwork* network, IndustryType industry_type);
  BeerConsumptionRequirement(Adjacency* adj, int units, PlayerNetwork* network);
  BeerConsumptionRequirement(Adjacency* adj, int units, PlayerNetwork* network, Adjacency* extra_adj);

  virtual bool ComputeRequirement(int& basic_price) override;
  virtual void Consume() override;

 private:
  bool use_merchant_beer_ = false;
  IndustryType industry_type_ = IndustryType::kUndefined;

  std::vector<Location*> sources_;
  std::vector<Adjacency*> extra_connections_;

  PlayerNetwork* network_;
};

class ActionRequirementFactory;

// This represents an "atomic" consumption request.
// Consumption of resources is needed for performing certain actions.
// Each resource type comes with a different consumption rule.
// We define the consumption of a resource type as a "requirement".
// A request is just a collection of such requirments,
// of which a player is obliged to meet.
// If one or more requirment may not be satisfied,
// the entire request is considered not feasible and the player may not
// perform the associated action.
class ActionRequirement {
  friend ActionRequirementFactory;

 public:
  ~ActionRequirement();

  bool ComputeRequirement(Player* player);

  void AddCoalRequirement(Adjacency* adj, int units);
  void AddCoalRequirement(Location* source, int units);
  void AddIronRequirement(int units);
  void AddBeerRequirement(Location* source, int units);
  void AddBeerRequirement(Adjacency* adj, int units);

  void Consume();
  bool ChooseIndustriesToConsume(Io& io);

  int total_price() const;

 private:
  ActionRequirement(Player* player,
                     Map* map,
                     Market* coal_market,
                     Market* iron_market,
                     int price);

  std::vector<ConsumptionRequirement*> reqs_;

  Player* player_;
  Map* map_;
  Market* coal_market_;
  Market* iron_market_;
  int basic_price_;
  int total_price_ = 0;
};

class ActionRequirementFactory {
 public:
  ActionRequirementFactory(Map* map, Market* coal_market, Market* iron_market);

  ActionRequirement* CreateActionRequirement(Player* player, int price = 0);

 private:
  Map* map_;
  Market* coal_market_;
  Market* iron_market_;
};

inline int ActionRequirement::total_price() const {
  return total_price_;
}

#endif  // BRASS_GAME_CONSUMPTION_REQUEST_H_
