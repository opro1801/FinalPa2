#include "location.h"

#include "link.h"

Location::Location(const std::string& name, Color color)
    : name_(name), color_(color) {}

Location::~Location() {}

void Location::AddLink(Link* link) {
  adj_links_.insert(link);
}

bool Location::IsConnectedTo(const Location* other) const {
  return IsConnected([other](const Location* location){ return location == other; });
}

bool Location::IsConnectedToMerchantLocation() const {
  return IsConnected([](const Location* location){
    return location->location_type() == LocationType::kMerchantLocation;
  });
}

bool Location::IsConnectedToMerchant(IndustryType industry_type) const {
  return IsConnected([industry_type](const Location* location){
    return (location->location_type() == LocationType::kMerchantLocation)
             && (location->HasIndustryType(industry_type));
  });
}

std::vector<Location*> Location::adjacent_locations() const {
  std::vector<Location*> locations;
  for (Link* link : adj_links_) {
    locations.push_back(link->another_end(this));
  }
  return locations;
}

std::vector<Location*> Location::adjacent_connected_locations() const {
  std::vector<Location*> locations;
  for (Link* link : adj_links_) {
    if (link->IsBuilt()) {
      locations.push_back(link->another_end(this));
    }
  }
  return locations;
}

std::vector<Location*> Location::connected_locations() const {
  std::stack<Location*> stack;
  LocationPtrSet locations;

  stack.push(const_cast<Location*>(this));

  while (!stack.empty()) {
    Location* location = stack.top();
    stack.pop();
    locations.insert(location);
    for (Location* other : location->adjacent_connected_locations()) {
      if (locations.count(other) <= 0) {
        stack.push(other);
      }
    }
  }
  return {locations.begin(), locations.end()};
}

std::vector<Link*> Location::adjacent_links() const {
  return { adj_links_.begin(), adj_links_.end() };
}

void Location::Update() const {
  if (link_observer_ != nullptr) {
    for (Link* link : adj_links_) {
      link_observer_->UpdateLink(link);
    }
  }
}
