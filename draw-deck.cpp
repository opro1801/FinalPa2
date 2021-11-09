#include "draw-deck.h"

#include <algorithm>

DrawDeck::DrawDeck() : g_(nullptr) {}

DrawDeck::DrawDeck(int seed) : g_(new std::mt19937(seed)) {}

DrawDeck::~DrawDeck() {
  for (Card* card : draw_deck_) {
    delete card;
  }
  for (Card* card : wild_industry_cards_) {
    delete card;
  }
  for (Card* card : wild_location_cards_) {
    delete card;
  }
  delete g_;
}

bool DrawDeck::IsExhausted() const {
  return draw_deck_.size() <= 0;
}

Card* DrawDeck::Back() {
  return draw_deck_.back();
}

Card* DrawDeck::Pop() {
  Card* next = draw_deck_.back();
  draw_deck_.pop_back();
  return next;
}

Card* DrawDeck::PopWildIndustryCard() {
  Card* next = wild_industry_cards_.back();
  wild_industry_cards_.pop_back();
  return next;
}

Card* DrawDeck::PopWildLocationCard() {
  Card* next = wild_location_cards_.back();
  wild_location_cards_.pop_back();
  return next;
}

void DrawDeck::Put(Card* card) {
  switch (card->card_type()) {
    case CardType::kWildIndustryCard:
      wild_industry_cards_.push_back(card);
      break;
    case CardType::kWildLocationCard:
      wild_location_cards_.push_back(card);
      break;
    default:
      draw_deck_.push_back(card);
  }
}

bool DrawDeck::Shuffle() {
  if (g_ != nullptr) {
    std::shuffle(draw_deck_.begin(), draw_deck_.end(), *g_);
    return true;
  }
  return false;
}

void DrawDeck::Reverse() {
  std::reverse(draw_deck_.begin(), draw_deck_.end());
}

void DrawDeck::set_seed(int seed) {
  delete g_;
  g_ = new std::mt19937(seed);
}
