#ifndef BRASS_LOCATABLE_INDUSTRY_LOCATION_H_
#define BRASS_LOCATABLE_INDUSTRY_LOCATION_H_

#include "location.h"
#include "industry.h"

class IndustryLocation : public Location {
  using IndustryMap = std::map<IndustryType, std::vector<Industry*> >;

 public:
  IndustryLocation(const std::string& name, Color color);

  virtual bool HasIndustryType(IndustryType industry_type) const override;

  void AddIndustry(Industry* industry);
  std::vector<Industry*> industries_of_type(IndustryType industry_type) const;

  virtual void Update() const override;

  std::vector<Industry*> industries() const;
  virtual int vp_for_adjacent_link() const override;

  virtual LocationType location_type() const override;

 private:
  IndustryMap industries_;

  IndustryObserver* industry_observer_ = nullptr;
};

inline bool IndustryLocation::HasIndustryType(IndustryType industry_type) const {
  return Contains(industries_, industry_type);
}

inline LocationType IndustryLocation::location_type() const {
  return LocationType::kIndustryLocation;
}

#endif  // BRASS_LOCATABLE_INDUSTRY_LOCATION_H_
