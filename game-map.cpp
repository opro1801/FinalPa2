#include "game-map.h"

#include "industry-location.h"

Map::~Map() {
  for (IndustryLocation* location : industry_locations_) {
    delete location;
  }
  for (MerchantLocation* location : merchant_locations_) {
    delete location;
  }
  for (Adjacency* adj : adjacencies_) {
    delete adj;
  }
  for (Industry* industry : industries_) {
    delete industry;
  }
  for (Merchant* merchant : merchants_) {
    delete merchant;
  }
  for (Link* link : links_) {
    delete link;
  }
}

void Map::Add(IndustryLocation* location) {
  if (location == nullptr) {
    return;
  }
  industry_locations_.insert(location);
}

void Map::Add(MerchantLocation* location) {
  if (location == nullptr) {
    return;
  }
  merchant_locations_.insert(location);
}

void Map::Add(IndustryLocation* location, Industry* industry) {
  if (location == nullptr || industry == nullptr) {
    return;
  }
  Add(location);
  industry->set_location(location);
  industries_.insert(industry);
  location->AddIndustry(industry);
  industries_dict_[industry->industry_type()].insert(industry);
}

void Map::Add(MerchantLocation* location, Merchant* merchant) {
  if (location == nullptr || merchant == nullptr) {
    return;
  }
  Add(location);
  merchants_.insert(merchant);
  location->AddMerchant(merchant);
}

void Map::Add(Adjacency* adjacency) {
  if (adjacency == nullptr) {
    return;
  }
  adjacencies_.insert(adjacency);
}

void Map::Add(Adjacency* adjacency, Link* link) {
  if (adjacency == nullptr || link == nullptr) {
    return;
  }
  Add(adjacency);
  link->set_adjacency(adjacency);
  links_.insert(link);
  for (int i = 0; i < kNumEnds; i++) {
    adjacency->ends()[i]->AddLink(link);
  }
}

std::vector<Industry*> Map::industries() const {
  return { industries_.begin(), industries_.end() };
}

std::vector<Link*> Map::links() const {
  return { links_.begin(), links_.end() };
}

std::vector<Industry*> Map::industries_of_type(IndustryType industry_type) const {
  std::vector<Industry*> industries;
  for (IndustryLocation* location : industry_locations_) {
    std::vector<Industry*> industries_obtained = location->industries_of_type(industry_type);
    industries.insert(industries.end(),
                      industries_obtained.begin(), industries_obtained.end());             
  }
  return industries;
}

void Map::RegisterObserverForLinks(LinkObserver* observer) {
  for (Link* link : links_) {
    link->set_observer(observer);
  }
}

void Map::RegisterObserverForIndustries(IndustryObserver* observer, ResourceObserver* robserver) {
  for (Industry* industry : industries_) {
    industry->set_industry_observer(observer);
    industry->set_resource_observer(robserver);
  }
}
