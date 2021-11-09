#ifndef BRASS_DRAW_DECK_H_
#define BRASS_DRAW_DECK_H_

#include "card.h"

#include <vector>
#include <random>

// This holds the playing cards yet to be drawn.
class DrawDeck {
 public:
  DrawDeck();
  DrawDeck(int seed);
  ~DrawDeck();

  bool IsExhausted() const;

  Card* Back();
  Card* Pop();
  Card* PopWildIndustryCard();
  Card* PopWildLocationCard();
  void Put(Card* card);
  
  bool Shuffle();
  void Reverse();

  int draw_deck_size() const;
  int wild_industry_cards_size() const;
  int wild_location_cards_size() const;

  void set_seed(int seed);

 private:
  std::vector<Card*> draw_deck_;
  std::vector<Card*> wild_industry_cards_;
  std::vector<Card*> wild_location_cards_;

  std::mt19937* g_;  // a psuedo-random number generator
};

inline int DrawDeck::draw_deck_size() const {
  return draw_deck_.size();
}

inline int DrawDeck::wild_industry_cards_size() const {
  return wild_industry_cards_.size();
}

inline int DrawDeck::wild_location_cards_size() const {
  return wild_location_cards_.size();
}

#endif  // BRASS_DRAW_DECK_H_
