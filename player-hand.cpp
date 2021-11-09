#include "player-hand.h"

#include <algorithm>

PlayerHand::PlayerHand() {}

PlayerHand::~PlayerHand() {
  for (Card* card : cards_) {
    delete card;
  }
  for (Card* card : discard_pile_) {
    delete card;
  }
}

void PlayerHand::Discard(int index) {
  Card* card = at(index);
  if (card->IsWildCard()) {
    wild_card_discard_pile_.push_back(card);
  } else {
    discard_pile_.push_back(card);
  }
  cards_.erase(cards_.begin() + index);
}

void PlayerHand::Discard(std::vector<int>& indices) {
  std::sort(indices.begin(), indices.end());
  std::vector<int>::reverse_iterator rit = indices.rbegin();
  for (; rit!= indices.rend(); ++rit) {
    Discard(*rit);
  }
}

bool PlayerHand::DrawCard(Card* card) {
  if (cards_.size() < kMaxNumCardsInHand) {
    cards_.push_back(card);
    return true;
  }
  return false;
}
