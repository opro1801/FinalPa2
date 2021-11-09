#ifndef BRASS_PLAYER_HAND_H_
#define BRASS_PLAYER_HAND_H_

#include "card.h"

#include <vector>
#include <set>

const int kMaxNumCardsInHand = 8;

// The PlayerHand class. It holds the cards the player draws.
class PlayerHand {
 public:
  PlayerHand();
  ~PlayerHand();

  // Returns true if there is a wild card in the player's hand.
  bool HasWildCard() const;

  // Discards a card at the given `index` on hand. The discarded card will be sent to
  // the player's discard pile. This also shifts all cards on hand to the left to fill up
  // the unused space which was originally occupied by the discarded card.
  void Discard(int index);
  void Discard(std::vector<int>& index);

  // This adds the provided `card` to the player's hand.
  bool DrawCard(Card* card);

  // Returns true if the player has recently discarded wild cards.
  bool HasDiscardedWildCard() const;

  // Gives up the ownership of the top discarded wild card, and returns it.
  Card* PopDiscardedWildCard();

  // Access a card in hand at the given index.
  Card* at(int index) const;

  // Returns the number of cards in hand.
  int num_cards() const;

  // Returns the player's hand of cards.
  std::vector<Card*> cards() const;

 private:
  std::vector<Card*> cards_;
  std::vector<Card*> discard_pile_;
  std::vector<Card*> wild_card_discard_pile_;
};

inline Card* PlayerHand::at(int index) const {
  return cards_.at(index);
}

inline bool PlayerHand::HasWildCard() const {
  for (const Card* card : cards_) {
    if (card->IsWildCard()) {
      return true;
    }
  }
return false;
}

inline int PlayerHand::num_cards() const {
  return cards_.size();
}

inline std::vector<Card*> PlayerHand::cards() const {
  return cards_;
}

inline bool PlayerHand::HasDiscardedWildCard() const {
  return !wild_card_discard_pile_.empty();
}

inline Card* PlayerHand::PopDiscardedWildCard() {
  if (!wild_card_discard_pile_.empty()) {
    Card* card = wild_card_discard_pile_.back();
    wild_card_discard_pile_.pop_back();
    return card;
  }
  return nullptr;
}

#endif  // BRASS_PLAYER_HAND_H_
