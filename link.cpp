#include "link.h"

#include "location.h"

Adjacency::Adjacency(Location* location_1, Location* location_2) {
  if (location_1->name() < location_2->name()) {
    locations_[0] = location_1;
    locations_[1] = location_2;
  } else {
    locations_[1] = location_1;
    locations_[0] = location_2;
  }
}

Location* Adjacency::another_end(const Location* location) const {
  if (location == locations_[0]) {
    return locations_[1];
  }
  if (location == locations_[1]) {
    return locations_[0];
  }
  return nullptr;
}

Location* const * Adjacency::ends() const {
  return locations_;
}

std::string Adjacency::name() const {
  return locations_[0]->name() + " <-> " + locations_[1]->name();
}

int Adjacency::vp_for_link() const {
  int vp = 0;
  for (int i = 0; i < kNumEnds; i++) {
    vp += locations_[i]->vp_for_adjacent_link();
  }
  return vp;
}

bool Adjacency::operator==(const Adjacency& other) {
  return (locations_[0] == other.locations_[0])
            && (locations_[1] == other.locations_[1]);
}

Link::Link(LinkType link_type, Adjacency* adj) : link_type_(link_type), adj_(adj) {}

bool Link::IsAdjacentTo(const Location* location) const {
  return adj_->IsAdjacentTo(location);
}

bool Link::IsConnectedTo(const Location* location) const {
  return IsAdjacentTo(location) && IsBuilt();
}

void Link::BeOverbuilt(Link* link) {
  static_cast<void>(link);
  adj_ = nullptr;
}

Location* Link::another_end(const Location* location) const {
  return adj_->another_end(location);
}

LinkType Link::link_type() const {
  return link_type_;
}

void Link::set_adjacency(Adjacency* adj) {
  adj_ = adj;
}

LinkObserver::LinkObserver() {}
LinkObserver::~LinkObserver() {}

void LinkObserver::UpdateLink(Link* link) {
  if (updating_ == true) {
    return;
  }
  updating_ = true;
  Location* const * ends = link->adjacency()->ends();
  if (!Contains(link_info_, link)) {
    link_info_.emplace(
      link,
      LinkInfo {
        ToString(link->link_type()),
        ends[0]->name(),
        ends[1]->name(),
        ends[0]->location_type(),
        ends[1]->location_type(),
        ends[0]->color(),
        ends[1]->color(),
      }
    );
  }
  LinkInfo& info = link_info_.at(link);
  info.set_owner(link->player_name());
  for (int i = 0; i < 2; i++) {
    if (ends[i]->IsConnectedToMerchantLocation()) {
      connected_to_merchant_[ends[i]] = true;
      if (i > 0 && link->IsBuilt()) {
        continue;
      }
      for (const Location* loc : ends[i]->connected_locations()) {
        connected_to_merchant_[loc] = true;
      }
    }
  }
  info.provisional_vp = link->vp();
  for (int i = 0; i < kNumEnds; i++) {
    Location* loc = link->adjacency()->ends()[i];
    loc->Update();
  }
  updating_ = false;
  Notify();
}

void LinkObserver::Notify() {
  std::vector<LinkInfo> link_info;
  for (std::map<Link*, LinkInfo>::value_type pair : link_info_) {
    Location* const * ends = pair.first->adjacency()->ends();
    pair.second.location_1_connected_to_merchant = connected_to_merchant_[ends[0]];
    pair.second.location_2_connected_to_merchant = connected_to_merchant_[ends[1]];
    link_info.push_back(pair.second);
  }
  for (LinkSubscriber* subscriber : subscribers_) {
    subscriber->UpdateLinkInfo(link_info);
  }
}

LinkSubscriber::~LinkSubscriber() {}

void LinkObserver::RegisterSubscriber(LinkSubscriber* subscriber) {
  subscribers_.insert(subscriber);
  Notify();
}
