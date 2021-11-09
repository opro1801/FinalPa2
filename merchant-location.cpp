#include "merchant-location.h"

#include "merchant.h"

MerchantLocation::MerchantLocation(const std::string& name, int vp_for_adjacent_link)
    : Location(name), vp_for_adjacent_link_(vp_for_adjacent_link) {}

std::vector<Merchant*> MerchantLocation::merchants() const  {
  std::vector<Merchant*> merchants;
  for (const MerchantMap::value_type& pair : merchants_) {
    merchants.insert(merchants.end(), pair.second.begin(), pair.second.end());
  }
  return merchants;
}

std::vector<Merchant*> MerchantLocation::merchants(IndustryType industry_type) const  {
  std::vector<Merchant*> merchants;
  for (const MerchantMap::value_type& pair : merchants_) {
    if (pair.first == industry_type || pair.first == IndustryType::kAny) {
      merchants.insert(merchants.end(), pair.second.begin(), pair.second.end());
    }
  }
  return merchants;
}

int MerchantLocation::vp_for_adjacent_link() const {
  return vp_for_adjacent_link_;
}
