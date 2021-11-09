#include "merchant.h"

int Merchant::counter = 0;

Merchant::Merchant(IndustryType industry_type, int vp_bonus)
    : industry_type_(industry_type), vp_bonus_(vp_bonus),
      creation_id_(Merchant::counter++) {}

bool Merchant::ProvideMerchantBeer(int& vp_bonus) {
  vp_bonus = 0;
  if (beer_available_ > 0) {
    --beer_available_;
    vp_bonus = vp_bonus_;
  }
  return true;
}
