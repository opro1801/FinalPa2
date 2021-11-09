#ifndef BRASS_LOCATABLE_MERCHANT_H_
#define BRASS_LOCATABLE_MERCHANT_H_

#include "game-common.h"

const int kMaxNumMerchantBeer = 2;

class Merchant {
 public:
  Merchant(IndustryType industry_type, int vp_bonus);
  
  bool HasMerchantBeer() const;

  // Buys (intangible resources) from the industry. This returns true only if
  // the provided industry has not been sold before.
  bool ProvideMerchantBeer(int& vp_bonus);

  IndustryType industry_type() const;

  bool operator<(const Merchant& other) const;

 protected:
  IndustryType industry_type_;

  int vp_bonus_;
  int beer_available_ = kMaxNumMerchantBeer;

 private:
  static int counter;
  const int creation_id_;
};

inline bool Merchant::HasMerchantBeer() const {
  return beer_available_ > 0;
}

inline IndustryType Merchant::industry_type() const {
  return industry_type_;
}

inline bool Merchant::operator<(const Merchant& other) const {
  if (industry_type_ < other.industry_type_) {
    return true;
  }
  if (industry_type_ > other.industry_type_) {
    return false;
  }
  return creation_id_ < other.creation_id_;
}

#endif  // BRASS_LOCATABLE_MERCHANT_H_
