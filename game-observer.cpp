#include "game-observer.h"

Observer::~Observer() {}

ResourceObserver::ResourceObserver() {}

ResourceObserver::~ResourceObserver() {}

void ResourceObserver::Notify() {
  notifying_ = true;
  for (ResourceSubscriber* subscriber : subscribers_) {
    subscriber->UpdateResourceInfo(resource_info_);
  }
  notifying_ = false;
}

ResourceSubscriber::~ResourceSubscriber() {}

bool ResourceObserver::HasResource(ResourceType resource_type) const {
  if (Contains(resource_info_, resource_type)) {
    return false;
  }
  const ResourceInfo& info = resource_info_.at(resource_type);
  return info.industry_supply + info.market_supply > 0;
}

void ResourceObserver::UpdateResourceCount(ResourceType resource_type, int units) {
  if (!Contains(resource_info_, resource_type)) {
    resource_info_.emplace(resource_type, resource_type);
  }
  resource_info_.at(resource_type).industry_supply += units;
  Notify();
}

void ResourceObserver::UpdateMarketResourceCount(ResourceType resource_type, int units,
                                                 std::array<int, kNumPricesToShow> selling_price,
                                                 std::array<int, kNumPricesToShow> buying_price, bool full) {
  if (!Contains(resource_info_, resource_type)) {
    resource_info_.emplace(resource_type, resource_type);
  }
  ResourceInfo& info = resource_info_.at(resource_type);
  info.market_supply += units;
  info.selling_price = selling_price;
  info.buying_price = buying_price;
  info.market_full = full;

  Notify();
}

void ResourceObserver::RegisterSubscriber(ResourceSubscriber* subscriber) {
  subscribers_.push_back(subscriber);
  Notify();
}
