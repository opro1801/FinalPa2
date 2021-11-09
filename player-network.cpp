#include "player-network.h"

#include <algorithm>

PlayerNetwork::~PlayerNetwork() {
  for (LinkVectorMap::value_type pair : links_) {
    for (Link* link : pair.second) {
      delete link;
    }
  }
  for (IndustryVectorMap::value_type pair : industries_) {
    for (Industry* industry : pair.second) {
      delete industry;
    }
  }
  for (IndustryVector::value_type industry : removed_industries_) {
    delete industry;
  }
}

bool PlayerNetwork::HasEstablished() const {
  return (num_built_industries() > 0) || (num_built_links() > 0);
}

bool PlayerNetwork::HasBuiltIndustryOn(const Location* location) const {
  for (const IndustryVectorMap::value_type& pair : industries_) {
    if (!location->HasIndustryType(pair.first)) {
      continue;
    }
    for (Industry* industry : pair.second) {
      if (industry->location() == location) {
        return true;
      }
    }
  }
  return false;
}

bool PlayerNetwork::IsCovering(const Location* location) const {
  if (HasBuiltIndustryOn(location)) {
    return true;
  } else {
    for (const LinkVectorMap::value_type& pair : links_) {
      for (Link* link : pair.second) {
        if (link->IsAdjacentTo(location)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool PlayerNetwork::IsAdjacentTo(const Link* link) const {
  for (int i = 0; i < kNumEnds; i++) {
    Location* location = dynamic_cast<Location*>(link->adjacency()->ends()[i]);
    if (location != nullptr) {
      if (HasBuiltIndustryOn(location)) {
        return true;
      }
    }
  }
  return false;
}

void PlayerNetwork::AddToNetwork(Infrastructure* infrastructure) {
  Link* link = dynamic_cast<Link*>(infrastructure);
  if (link != nullptr) {
    links_[link->link_type()].push_back(link);
  } else {
    Industry* industry = dynamic_cast<Industry*>(infrastructure);
    industries_[industry->industry_type()].insert(industry);
  }
}

int PlayerNetwork::TotalColleteralValue() const {
  int total_worth = 0;
  for (const IndustryVectorMap::value_type& pair : industries_) {
    for (Industry* industry : pair.second) {
      total_worth += industry->properties().basic_price / 2;
    }
  }
  return total_worth;
}

int PlayerNetwork::ScoreVp() {
  int vp = 0;
  for (IndustryVectorMap::value_type& pair : industries_) {
    for (const Industry* industry : pair.second) {
      vp += industry->vp();
    }
  }
  for (LinkVectorMap::value_type& pair : links_) {
    for (const Link* link : pair.second) {
      vp += link->vp();
    }
  }
  return vp;
}

PlayerNetwork::IndustryVector PlayerNetwork::industries_of_type(IndustryType industry_type) const {
  IndustryVectorMap::const_iterator pair = industries_.find(industry_type);
  if (pair == industries_.end()) {
    return {};
  }
  return { pair->second.begin(), pair->second.end() };
}

PlayerNetwork::IndustryVector PlayerNetwork::industries_of_type(MainIndustryType industry_type) const {
  IndustryVector vec;
  for (IndustryVectorMap::const_reference pair : industries_) {
    if (ToMainIndustryType(pair.first) == industry_type) {
      vec.insert(vec.end(), pair.second.begin(), pair.second.end());
    }
  }
  return vec;
}

void PlayerNetwork::Remove(Industry* industry) {
  if (industries_.at(industry->industry_type()).erase(industry) == 1) {
    removed_industries_.push_back(industry);
  }
}

int PlayerNetwork::num_built_industries() const {
  int num_built = 0;
  for (const IndustryVectorMap::value_type& pair : industries_) {
    num_built += pair.second.size();
  }
  return num_built;
}

int PlayerNetwork::num_built_links() const {
  int num_built = 0;
  for (const LinkVectorMap::value_type& pair : links_) {
    num_built += pair.second.size();
  }
  return num_built;
}

PlayerNetwork::LocationPtrSet PlayerNetwork::network(PlayerNetwork::LinkPtrSet* adj_links) const {
  LocationPtrSet locs;
  for (IndustryVectorMap::const_reference industries : industries_) {
    for (Industry* industry : industries.second) {
      locs.insert(industry->location());
    }
  }
  for (LinkVectorMap::const_reference links : links_) {
    for (Link* link : links.second) {
      Location * const * ends = link->adjacency()->ends();
      for (int i = 0; i < kNumEnds; i++) {
        locs.insert(ends[i]);
      }
    }
  }
  if (adj_links != nullptr) {
    for (Location* loc : locs) {
      std::vector<Link*> loc_adj_links = loc->adjacent_links();
      adj_links->insert(loc_adj_links.begin(), loc_adj_links.end());
    }
  }
  return locs;
}
