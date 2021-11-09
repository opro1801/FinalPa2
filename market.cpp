#include "market.h"

Market::Market(ResourceType resource_type, int initial_quantity,
               int max_quantity, int max_marginal_price)
    : resource_type_(resource_type),
      max_marginal_price_(max_marginal_price),
      max_quantity_(max_quantity),
      num_used_spaces_(min(initial_quantity, max_quantity)) {}

int Market::GetTotalPrice(int quantity) const {
  if (quantity == 0) {
    return 0;
  }
  int total_amount = 0;
  for (int i = 0; i < quantity; i++) {
    total_amount += GetMarginalPrice(num_used_spaces_ - i);
  }
  return total_amount;
}

int Market::Buy(int quantity, int* quantity_bought) {
  int payment = 0;
  quantity = min(quantity, max_quantity_ - num_used_spaces_);
  if (quantity > 0) {
    payment = 0;
    for (int i = 1; i <= quantity; i++) {
      payment += GetMarginalPrice(num_used_spaces_ + i);
    }
    UpdateResourceCount(quantity);
  }
  if (quantity_bought != nullptr) {
    *quantity_bought = quantity;
  }
  return payment;
}

void Market::Sell(int quantity) {
  int prev_num_used_spaces = num_used_spaces_;
  num_used_spaces_ = max(0, num_used_spaces_ - quantity);
  UpdateResourceCount(-(prev_num_used_spaces - num_used_spaces_));
}

void Market::UpdateResourceCount(int change) const {
  if (rsrc_observer_ == nullptr) {
    return;
  }
  std::array<int, kNumPricesToShow> selling_price, buying_price;
  for (int i = 0; i < 5; i++) {
    selling_price[i] = GetMarginalPrice(num_used_spaces_ - i);
    buying_price[i] = (num_used_spaces_ + i >= max_quantity_)? 0 : GetMarginalPrice(num_used_spaces_ + i + 1);
  }
  rsrc_observer_->UpdateMarketResourceCount(resource_type_, change,
                                      selling_price, buying_price,
                                      num_free_slots() == 0);
}
