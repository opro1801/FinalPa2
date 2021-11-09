#ifndef BRASS_GAME_H_
#define BRASS_GAME_H_

#include "game-map.h"
#include "draw-deck.h"
#include "market.h"
#include "player.h"
#include "consumption-request.h"
#include "game-observer.h"
#include "game-io.h"

#include <vector>

const int kMinNumPlayers = 2;
const int kMaxNumPlayers = 4;

// The Game class. It handles the game logistics at the most abstract level:
// it manages the change of rounds, player turns, and player's balance update.
class Game : ResourceSubscriber, IndustrySubscriber, LinkSubscriber, PlayerSubscriber {
 public:
  Game(Map* map, DrawDeck* card_deck,
       Market* coal_market, Market* iron_market, Io& io);
  ~Game();

  // The main game loop.
  void Run();

  // Adds a new player.
  void AddPlayer(const std::string& name, PlayerUnbuilt* unbuilt);

  // Plays one round of the game.
  ProceedFlag PlayRound(int max_num_actions, GameStat& game_stat, bool& warn_exhausted);

  // Asks the `player` to perform an action.
  bool PerformAction(Player* player, ProceedFlag& flag);

  // This collects the wild cards discarded by the player.
  void CollectDiscardedWildCards(Player* player);

  // Refills the hand for the `player`, fully if possible, after the player finished
  // all actions. The player will have as many as 8 cards, unless the game deck
  // is exhausted.
  int RefillHand(Player* player);

  // Returns true if all cards have been played.
  bool AreCardsAllPlayed() const;

  // Resets the player's spending, and pay or collect from the player according to
  // his income level.
  void UpdateBalance();

  // This liquidates the player's industries to cover the shortfall.
  int Liquidate(PlayerNetwork& network, int shortfall);

  // Orders the players according to the amount they spent in the last round,
  // prioritizing the ones who spent the least.
  void ShufflePlayersForNextRound();

  void LogResourceInfo() const;
  void LogPlayerInfo() const;

  // Writes the game result.
  void WriteResult();

  // Updates the information displayed by the UI.
  virtual void UpdateResourceInfo(std::map<ResourceType, ResourceInfo> resource_info) override;
  virtual void UpdateIndustryInfo(std::map<IndustryType, std::vector<IndustryInfo> > industry_info) override;
  virtual void UpdateLinkInfo(std::vector<LinkInfo> link_info) override;
  virtual void UpdatePlayerInfo(std::map<Player*, PlayerInfo> player_info) override;

 private:
  std::vector<Player*> players_;
  Map* map_;
  DrawDeck* card_deck_;
  Market* coal_market_;
  Market* iron_market_;
  Io& io_;

  // A factory for generating ActionRequirements.
  ActionRequirementFactory* action_req_fty_ = nullptr;

  // Observers which will update the information to be used for the UI.
  ResourceObserver* resource_osvr_ = nullptr;
  IndustryObserver* industry_osvr_ = nullptr;
  LinkObserver* link_osvr_ = nullptr;
  PlayerObserver* player_osvr_ = nullptr;

  // Information used by the UI.
  std::map<ResourceType, ResourceInfo> resource_info_;
  std::map<IndustryType, std::vector<IndustryInfo> > industry_info_;
  std::vector<LinkInfo> link_info_;
  std::map<Player*, PlayerInfo> player_info_;
  const PlayerInfo* curr_player_info_ = nullptr;
  Player* curr_player_ = nullptr;
};

#endif  // BRASS_GAME_H_
