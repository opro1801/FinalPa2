#ifndef BRASS_PLAYER_H_
#define BRASS_PLAYER_H_

#include "util-common.h"

#include <map>
#include <set>
#include <vector>
#include <stack>

const int kInitVp = 0;
const int kInitIncomeLevel = 0;
const int kInitExp = 0;
const int kInitBalance = 17;
const int kMinVp = -10;
const int kMinIncomeLevel = -10;
const int kMaxIncomeLevel = 30;

// Some forward declarations to defer the inclusions of the relevant headers
// until we do it in the source file. This prevents an inclusion hell
// because some other classes references the Player class.
// Another way to achieve this is to create an extra interface for a player.
class PlayerHand;
class PlayerUnbuilt;
class PlayerNetwork;
class Card;
class PlayerObserver;
class PlayerSubscriber;
class Industry;

// The Player class. A player is composed of a hand of cards,
// a set of unbuilt infrastructures, and a set of built infrastructures.
class Player {
  friend class Action;
  friend class BuildAction;
  friend class PlayerUnbuilt;

 public:
  Player(const std::string& name,
         PlayerHand* hand,
         PlayerUnbuilt* unbuilt, PlayerNetwork* network);
  ~Player();

  // Permanently increases the player's VP. 
  void IncreaseVp(int points);

  // Permanently decreases the player's VP.
  void DecreaseVp(int points);

  // This decreases the player's income level by `num_levels`.
  // The EXP is set to be the highest within the resulting income level.
  void DecreaseIncomeLevels(int num_levels);

  // This increases the player's EXP by `points`,
  // and advances the income level accordingly.
  void IncreaseExp(int points);
  
  // Deposit the given `amount` if possible. Returns true if successful.
  bool Deposit(int amount);

  // Withdraw the given `amount` if possible. Returns true if successful.
  bool Withdraw(int amount);

  // Returns true if the amount to withdraw is valid:
  // at least zero and at most the amount the player currently holds.
  bool CanWithdraw(int amount);

  // Resets the player's spending as 0.
  void ResetSpending();

  // Returns true if the player is holding a wild card.
  bool HasWildCardInHand() const;

  // Discard a card at the `index` position.
  void DiscardCard(int index);

  // Discard a list of card at the `indices` positions.
  void DiscardCard(std::vector<int>& indices);

  // Refills the player's hand of cards.
  bool DrawCard(Card* card);

  // Compute the player's final VP.
  void ScoreVp();

  // Removes the given industry from the player's network.
  void RemoveFromNetwork(Industry* industry);

  // Lets the `observer` observe this player.
  void set_observer(PlayerObserver* observer);

  int num_cards_in_hand() const;
  int num_built_industries() const;

  std::string name() const;
  int vp() const;
  int income_level() const;
  int exp() const;
  int balance() const;
  int spending() const;
  int provisional_vp() const;

  PlayerHand& hand() const;
  PlayerNetwork& network() const;
  PlayerUnbuilt& unbuilt() const;

 private:
  std::string name_;

  int vp_ = kInitVp;
  int income_level_ = kInitIncomeLevel;
  int exp_ = kInitExp;
  int balance_ = kInitBalance;
  int spending_ = 0;

  void Notify();

  PlayerObserver* observer_ = nullptr;

  PlayerHand* hand_;
  PlayerUnbuilt* unbuilt_;
  PlayerNetwork* network_;
};

class PlayerSubscriber {
  friend PlayerObserver;
 
 public:
  virtual ~PlayerSubscriber();

 protected:
  virtual void UpdatePlayerInfo(std::map<Player*, PlayerInfo> player_info) = 0;
};

class PlayerObserver {
 public:
  PlayerObserver();
  ~PlayerObserver();

  void UpdatePlayer(Player* player);
  void RegisterSubscriber(PlayerSubscriber* subscriber);
  void Notify();

 private:
  std::map<Player*, PlayerInfo> player_info_;
  std::vector<PlayerSubscriber*> subscribers_;
};

inline void Player::IncreaseVp(int points) {
  vp_ = vp_ + max(0, points);
  Notify();
}

inline void Player::DecreaseVp(int points) {
  vp_ = max(kMinVp, vp_ - points);
  Notify();
}

inline bool Player::Deposit(int amount) {
  if (amount >= 0) {
    balance_ += amount;
    Notify();
    return true;
  }
  return false;
}

inline bool Player::Withdraw(int amount) {
  bool can_withdraw = CanWithdraw(amount);
  if (can_withdraw) {
    balance_ -= amount;
    spending_ += amount;
    Notify();
  }
  return can_withdraw;
}

inline bool Player::CanWithdraw(int amount) {
  return (amount >= 0) && (amount <= balance_);
}

inline void Player::ResetSpending() {
  spending_ = 0;
}

inline std::string Player::name() const {
  return name_;
}

inline int Player::vp() const {
  return vp_;
}

inline int Player::income_level() const {
  return income_level_;
}

inline int Player::exp() const {
  return exp_;
}

inline int Player::balance() const {
  return balance_;
}

inline int Player::spending() const {
  return spending_;
}

inline void Player::set_observer(PlayerObserver* observer) {
  observer_ = observer;
  Notify();
}

#endif  // BRASS_PLAYER_H_
