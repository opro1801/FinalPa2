#ifndef BRASS_LOCATABLE_MERCHANT_LOCATION_H_
#define BRASS_LOCATABLE_MERCHANT_LOCATION_H_

#include "merchant.h"
#include "location.h"

class MerchantLocation : public Location {
  using MerchantMap = std::map<IndustryType, std::vector<Merchant*> >;

 public:
  MerchantLocation(const std::string& name, int vp_for_adjacent_link);

  virtual bool HasIndustryType(IndustryType industry_type) const override;

  void AddMerchant(Merchant* merchant);
  virtual bool IsConnectedToMerchantLocation() const override;
  virtual bool IsConnectedToMerchant(IndustryType industry_type) const override;

  std::vector<Merchant*> merchants() const;
  std::vector<Merchant*> merchants(IndustryType industry_type) const;
  virtual int vp_for_adjacent_link() const override;

  virtual LocationType location_type() const override;

 private:
  MerchantMap merchants_;

  int vp_for_adjacent_link_;
};

inline bool MerchantLocation::HasIndustryType(IndustryType industry_type) const {
  return Contains(merchants_, industry_type) || Contains(merchants_, IndustryType::kAny);
}

inline bool MerchantLocation::IsConnectedToMerchantLocation() const {
  return true;
}

inline bool MerchantLocation::IsConnectedToMerchant(IndustryType industry_type) const {
  return HasIndustryType(industry_type);
}

inline LocationType MerchantLocation::location_type() const {
  return LocationType::kMerchantLocation;
}

inline void MerchantLocation::AddMerchant(Merchant* merchant) {
  merchants_[merchant->industry_type()].push_back(merchant);
}
#endif  // BRASS_LOCATABLE_MERCHANT_LOCATION_H_
