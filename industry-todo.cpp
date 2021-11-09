
#include "industry.h"

#include "player.h"

//// TODO: Complete primary and secondary industries

PrimaryIndustry::PrimaryIndustry(IndustryType industry_type, IndustryProperties properties,
                                ResourceType resource_type) : IndustryImpl(industry_type, properties, resource_type) {}

bool PrimaryIndustry::CanOverbuild(Industry* other) const {
  if(properties().tech_level > other->properties().tech_level && 
    industry_type() == other->industry_type() &&
    (player_name() == other->player_name() ||
    !other->IsBuilt() ||
    !resource_observer_->HasResource(resource_type()))) {
    return true;
  }
  return false;
}

bool PrimaryIndustry::IsEligibleForVp() const {
  return IsBuilt() && available_units() == 0;
}

void PrimaryIndustry::ProduceResources() {
  available_units_+=properties().output_units;
  // resource_observer_->Notify();
  resource_observer_->UpdateResourceCount(resource_type(), properties().output_units);
  industry_observer_->UpdateIndustry(this);
}

void PrimaryIndustry::RemoveResources() {
  int temp = -1*available_units();
  available_units_ = 0;
  resource_observer_->UpdateResourceCount(resource_type(), temp);
  industry_observer_->UpdateIndustry(this);
}

bool PrimaryIndustry::ProvideResources(int units) {
  if(available_units() >= units) {
    available_units_-=units;
    resource_observer_->UpdateResourceCount(resource_type(), -units);
    industry_observer_->UpdateIndustry(this);
    return true;
  }
  return false;
}

int PrimaryIndustry::available_units() const {
  return available_units_;
}

void PrimaryIndustry::set_resource_observer(ResourceObserver* rsrc_observer) {
  resource_observer_ = rsrc_observer;
}


SecondaryIndustry::SecondaryIndustry(IndustryType industry_type, IndustryProperties properties)
: IndustryImpl(industry_type, properties) {}

bool SecondaryIndustry::CanOverbuild(Industry* other) const {
  if(properties().tech_level > other->properties().tech_level && 
    industry_type() == other->industry_type() &&
    (!other->IsBuilt() || (player_name() == other->player_name()))) return true;
  return false;
}

bool SecondaryIndustry::IsEligibleForVp() const {
  return sold();
}

bool SecondaryIndustry::Sell() {
  if(sold()) return false;
  sold_ = true;
  player()->IncreaseExp(properties().exp_increase);
  return true;
}

bool SecondaryIndustry::sold() const {
  return sold_;
}

void SecondaryIndustry::set_resource_observer(ResourceObserver* rsrc_observer) {
 static_cast<void>(rsrc_observer); 
}

//// TODO ends
