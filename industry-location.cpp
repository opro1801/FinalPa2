#include "industry-location.h"

#include "industry.h"

IndustryLocation::IndustryLocation(const std::string& name, Color color)
    : Location(name, color) {}

void IndustryLocation::AddIndustry(Industry* industry) {
  industries_[industry->industry_type()].push_back(industry);
}

std::vector<Industry*> IndustryLocation::industries_of_type(IndustryType industry_type) const  {
  IndustryMap::const_iterator pair = industries_.find(industry_type);
  if (pair == industries_.end()) {
    return {};
  }
  return { pair->second.begin(), pair->second.end() };
}

std::vector<Industry*> IndustryLocation::industries() const  {
  std::vector<Industry*> industries;
  for (const IndustryMap::value_type& pair : industries_) {
    industries.insert(industries.end(), pair.second.begin(), pair.second.end());
  }
  return industries;
}

int IndustryLocation::vp_for_adjacent_link() const {
  int vp = 0;
  for (const IndustryMap::value_type& pair : industries_) {
    for (Industry* industry : pair.second) {
      vp += industry->vp_for_adjacent_link();
    }
  }
  return vp;
}

void IndustryLocation::Update() const {
  Location::Update();
  if (industry_observer_ != nullptr) {
    for (Industry* industry : industries()) {
      industry_observer_->UpdateIndustry(industry);
    }
  }
}
