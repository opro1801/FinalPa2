#ifndef BRASS_CARD_H_
#define BRASS_CARD_H_

#include "location.h"
#include "industry.h"
#include "player-network.h"

#include <string>

enum class CardType {
  kLocationCard,
  kIndustryCard,
  kWildLocationCard,
  kWildIndustryCard,
};

//// TODO: Add virtual specifiers for the member functions of the Card class
//// to make it an abstract one.

//// Hint 1: You need not make every function virtual.
//// Hint 2: Cards are dynamically created. At the end of the game,
////         players and the draw deck will delete the cards discarded / unused.

// The Card class. It sets additional constraints for the "Build" action,
// e.g. the location where the holder can build an industry on.
// To the other actions, cards are just used to meet the discard requirement
// for performing an action but nothing more.
// @see `Action` (especially `BuildAction`), `DrawDeck`, `PlayerHand` classes
class Card {
 public:
  Card(const std::string& name);
  virtual ~Card();

  // Returns true if the card can be realized as a "Wild Card".
  virtual bool IsWildCard() const = 0;
  
  // Returns true if the card's specification allows us to overbuild
  // the industry `to_be_overbuilt`, i.e., assume we are going to build
  // a valid industry on it, does the card permit us to do so?
  virtual bool CanIndustryBeOverbuilt(const PlayerNetwork* network,
                              const Industry* to_be_overbuilt) const = 0;

  // Returns the card type.
  virtual CardType card_type() const = 0;

  // Returns the name of the card.
  // For wild cards, it is just "Wild Location" or "Wild Industry".
  // For non-wild cards, it is just the name of the location,
  // or the string representation of the industry type (check the util header file).
  std::string name() const;

 protected:
  std::string name_;
};

//// TODO ends


//// TODO: Define the subclasses of `Card` below.
//// They are not complete: it's more than adding the missing function declarations.

class LocationCard : public Card {
 public:
  LocationCard(Location* location);
  ~LocationCard() = default;

  virtual bool IsWildCard() const override;
  virtual bool CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* to_be_overbuilt) const;
  virtual CardType card_type() const;

 private:
  Location* const location_;
};

class WildLocationCard : public Card {
 public:
  WildLocationCard();
  ~WildLocationCard() = default;

  virtual bool IsWildCard() const override;
  virtual bool CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* to_be_overbuilt) const;
  virtual CardType card_type() const;
  
};

class IndustryCard : public Card{
 public:
  IndustryCard(IndustryType industry_type);
  ~IndustryCard() = default;

  virtual bool IsWildCard() const override;
  virtual bool CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* to_be_overbuilt) const;
  virtual CardType card_type() const;
  
 private:
  const IndustryType industry_type_;
};

class WildIndustryCard : public Card{
 public:
  WildIndustryCard();
  ~WildIndustryCard() = default;

  virtual bool IsWildCard() const override;
  virtual bool CanIndustryBeOverbuilt(const PlayerNetwork* network, const Industry* to_be_overbuilt) const;
  virtual CardType card_type() const;
  
};

//// TODO ends

#endif  // BRASS_CARD_H_
