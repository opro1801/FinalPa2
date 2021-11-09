#ifndef BRASS_MARKET_H_
#define BRASS_MARKET_H_

#include "game-common.h"
#include "game-observer.h"
#include "util-common.h"

// The Market class. It provides resources at a price.
class Market {
 public:
  Market(ResourceType resource_type, int initial_quantity,
         int max_quantity, int max_marginal_price);

  // Returns true if the market must sell resources at the maximum price.
  // This happens when the quantity remaining is 0. Note that the figure
  // is meant for formulating the pricing formula only.
  // Even if that is 0, the market may still sell resources to any player.
  bool Shortage() const;

  // Returns the current marginal price given the current number of free spaces remaining.
  int GetMarginalPrice() const;

  // Returns the marginal price given a specific number of free spaces remaining.
  int GetMarginalPrice(int num_used_spaces) const;

  // Returns the total price given the `quantity` requested.
  int GetTotalPrice(int quantity) const;

  // Buys the given quantity and returns the total offer price.
  int Buy(int quantity, int* quantity_bought = nullptr);
  
  // Sells the required `quantity`.
  void Sell(int quantity);

  // Registers the given `observer`.
  void RegisterObserver(ResourceObserver* observer);

  // Reports an update of the resource count to the resource observer.
  void UpdateResourceCount(int change) const;

  // Returns the number of slots availble for buying resource units from players.
  int num_free_slots() const;

 private:
  const ResourceType resource_type_;

  const int max_marginal_price_;
  const int max_quantity_;
  int num_used_spaces_;  // the market supply is infinite,
                         // this is just for fomulating the pricing formula

  ResourceObserver* rsrc_observer_;
};

inline void Market::RegisterObserver(ResourceObserver* observer) {
  rsrc_observer_ = observer;
  UpdateResourceCount(num_used_spaces_);
}

inline bool Market::Shortage() const {
  return num_used_spaces_ <= 0;
}

inline int Market::num_free_slots() const {
  return max_quantity_ - num_used_spaces_;
}

inline int Market::GetMarginalPrice() const {
  return GetMarginalPrice(num_used_spaces_);
}

inline int Market::GetMarginalPrice(int num_used_spaces) const {
  return max(1, max_marginal_price_ - (num_used_spaces + 1) / 2);
}

#endif  // BRASS_MARKET_H_
