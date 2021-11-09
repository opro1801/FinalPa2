#include "industry.h"

#include "location.h"
#include "player.h"

int Industry::counter = 0;

Industry::Industry(IndustryType industry_type, Location* location,
                   ResourceType resource_type)
    : industry_type_(industry_type), resource_type_(resource_type),
      location_(location), creation_id_(Industry::counter++) {}

Industry::~Industry() {}

bool Industry::Sell() {
  return false;
}

void Industry::ProduceResources() {
  return;
}

void Industry::RemoveResources() {
  return;
}

bool Industry::ProvideResources(int units) {
  static_cast<void>(units);
  return false;
}

Location* Industry::location() const {
  return location_;
}

MainIndustryType Industry::main_industry_type() const {
  switch (industry_type_) {
    case IndustryType::kCoalMine:
    case IndustryType::kIronWorks:
    case IndustryType::kBrewery:
      return MainIndustryType::kPrimaryIndustry;
    case IndustryType::kCottonMill:
    case IndustryType::kManufacturer:
    case IndustryType::kPottery:
      return MainIndustryType::kSecondaryIndustry;
    default:
      break;
  }
  return MainIndustryType::kUndefined;
}

void Industry::set_location(Location* location) {
  location_ = location;
}

void IndustryImpl::set_industry_observer(IndustryObserver* observer) {
  industry_observer_ = observer;
}

IndustryImpl::IndustryImpl(IndustryType industry_type, IndustryProperties properties,
                           ResourceType resource_type)
    : Industry(industry_type, nullptr, resource_type),
      properties_(properties) {}

IndustryImpl::~IndustryImpl() {}

bool IndustryImpl::IsBuilt() const {
  return location_ != nullptr;
}

bool IndustryImpl::Build(Industry* industry) {
  if (CanOverbuild(industry)) {
    location_ = industry->location();
    industry->BeOverbuilt(this);  // updated on Nov 1st
    ProduceResources();
    return true;
  }
  return false;
}

void IndustryImpl::BeOverbuilt(Industry* other) {
  static_cast<void>(other);
  Demolish();
}

void IndustryImpl::Demolish() {
  if (IsBuilt()) {
    RemoveResources();
    industry_observer_->UpdateIndustry(this);
    resource_observer_ = nullptr;
    industry_observer_ = nullptr;
    location_ = nullptr;
    demolished_ = true;
    player_->RemoveFromNetwork(this);
  }
}

Industry* IndustryImpl::impl() {
  return this;
}

std::string IndustryImpl::player_name() const {
  if (player_ != nullptr) {
    return player_->name();
  }
  return kEmptyPlayerName;
}

int IndustryImpl::vp() const {
  return (IsEligibleForVp())? properties_.vp : 0;
}

int IndustryImpl::vp_for_adjacent_link() const {
  return (IsEligibleForVp())? properties_.vp_for_adj_link : 0;
}

int IndustryImpl::available_units() const {
  return 0;
}

Player* IndustryImpl::player() const {
  return player_;
}

void IndustryImpl::set_player(Player* player) {
  player_ = player;
}

IndustryProperties IndustryImpl::properties() const {
  return properties_;
}

IndustryProxy::IndustryProxy(IndustryType industry_type, Location* location,
                             ResourceType resource_type)
    : Industry(industry_type, location, resource_type) {
  impl_observer_ = new IndustryObserver();
  impl_observer_->RegisterSubscriber(this);
}

IndustryProxy::~IndustryProxy() {
  delete impl_observer_;
}

bool IndustryProxy::IsBuilt() const {
  return (impl_ != nullptr && (impl_->IsBuilt()));
}

bool IndustryProxy::Build(Industry* industry) {
  static_cast<void>(industry);
  return false;
}

bool IndustryProxy::CanOverbuild(Industry* industry) const {
  static_cast<void>(industry);
  return false;
}

void IndustryProxy::BeOverbuilt(Industry* other) {
  if (impl_ != nullptr) {
    impl_->BeOverbuilt(other);
  }
  impl_ = static_cast<IndustryImpl*>(other);
  other->set_resource_observer(resource_observer_);
  other->set_industry_observer(impl_observer_);
  impl_->set_location(location_);
  if (industry_observer_) {
    industry_observer_->UpdateIndustry(this); // added on 1st Nov
  }
}

bool IndustryProxy::ProvideResources(int units) {
  if (impl_ != nullptr) {
    return impl_->ProvideResources(units);
  }
  return false;
}

void IndustryProxy::UpdateIndustryInfo(std::map<IndustryType, std::vector<IndustryInfo> > industry_info) {
  std::vector<IndustryInfo>& vec = industry_info[industry_type_];
  if (vec.size() > 0 && vec.at(0).demolished) {
    impl_ = nullptr;
    impl_observer_->RemoveIndustry(impl_);
  }
  if (industry_observer_)
    industry_observer_->UpdateIndustry(this);
}

void IndustryProxy::Demolish() {
  return;
}

bool IndustryProxy::IsEligibleForVp() const {
  return (impl_ == nullptr)? false : impl_->IsEligibleForVp();
}

Industry* IndustryProxy::impl() {
  return impl_;
}

int IndustryProxy::vp() const {
  return (impl_ == nullptr)? 0 : impl_->vp();
}

int IndustryProxy::vp_for_adjacent_link() const {
  return (impl_ == nullptr)? 0 : impl_->vp_for_adjacent_link();
}

int IndustryProxy::available_units() const {
  return (impl_ == nullptr)? 0 : impl_->available_units();
}

Player* IndustryProxy::player() const {
  return (impl_ == nullptr)? nullptr : impl_->player();
}

std::string IndustryProxy::player_name() const {
  if (impl_ != nullptr) {
    return impl_->player_name();
  }
  return kEmptyPlayerName;
}

IndustryProperties IndustryProxy::properties() const {
  if (impl_ != nullptr) {
    return impl_->properties();
  }
  return {};
}

void IndustryProxy::set_player(Player* player) {
  static_cast<void>(player);
}

void IndustryProxy::set_industry_observer(IndustryObserver* observer) {
  industry_observer_ = observer;
  industry_observer_->UpdateIndustry(this);
}

void IndustryProxy::set_resource_observer(ResourceObserver* resource_observer) {
  resource_observer_ = resource_observer;
}

void IndustryObserver::RegisterSubscriber(IndustrySubscriber* subscriber) {
  subscribers_.insert(subscriber);
  Notify();
}

IndustryObserver::IndustryObserver() {}
IndustryObserver::~IndustryObserver() {}

void IndustryObserver::UpdateIndustry(Industry* industry) {
  if (updating_ == true) {
    return;
  }
  updating_ = true;
  IndustryProperties properties = industry->properties();
  if (!Contains(industry_info_, industry)) {
    industry_info_.emplace(
      industry,
      IndustryInfo {
        ToString(industry->industry_type()),
        industry->location()->name(),
        industry->location()->color(),
        properties.tech_level,
        properties.vp,
        properties.vp_for_adj_link,
        properties.beer_cost
      }
    );
  } else {
    IndustryInfo& info = industry_info_.at(industry);
    info.tech_level = properties.tech_level;
    info.vp = properties.vp;
    info.vp_for_link = properties.vp_for_adj_link;
    info.beer_cost = properties.beer_cost;
    info.available_units = industry->available_units();
    info.eligible_for_vp = industry->IsEligibleForVp();
    info.demolished = industry->IsDemolished();
  }
  IndustryInfo& info = industry_info_.at(industry);
  info.owner = industry->player_name();
  info.connected_to_merchant = (industry->main_industry_type() == MainIndustryType::kPrimaryIndustry)?
    industry->location()->IsConnectedToMerchantLocation()
      : industry->location()->IsConnectedToMerchant(industry->industry_type());
  industry->location()->Update();
  updating_ = false;
  Notify();
}

void IndustryObserver::RemoveIndustry(Industry* industry) {
  IndustryInfoMap::iterator it = industry_info_.find(industry);
  if (it != industry_info_.end()) {
    industry_info_.erase(it);
  }
}

void IndustryObserver::Notify() {
  std::map<IndustryType, std::vector<IndustryInfo> > industry_info;
  for (IndustryInfoMap::value_type pair : industry_info_) {
    industry_info[pair.first->industry_type()].push_back(pair.second);
  }
  for (IndustrySubscriber* subscriber : subscribers_) {
    subscriber->UpdateIndustryInfo(industry_info);
  }
}

IndustrySubscriber::~IndustrySubscriber() {}
