#ifndef BRASS_CONFIG_H_
#define BRASS_CONFIG_H_

#include "industry-location.h"
#include "merchant-location.h"
#include "industry.h"
#include "merchant.h"

#include "draw-deck.h"
#include "player-unbuilt.h"
#include "market.h"

#include "game-map.h"

#include <fstream>

namespace config {

inline std::string DetermineConfigFilePath(const std::string& file_name,
                                           const std::string& config_dir,
                                           const std::string& default_config_dir) {
  std::string path = config_dir + "/" + file_name;
  std::ifstream f(path);
  if (f.good()) {
    return path;
  }
  return default_config_dir + "/" + file_name;
}

// Creates the game map and card deck.
// This reads the configurations supplied by "industry-locations.csv",
// "merchant-locations.csv", "adjacencies.csv",
// "industry-dist.csv", "merchant-dist.csv", and "cards.csv"
// from the given `config_dir`. See the default configuration files for
// the required format.
void CreateMapAndDrawDeck(const std::string& config_dir, 
                          const std::string& default_config_dir, int num_players,
                          Map*& map, DrawDeck*& card_deck);

// Creates the industry tiles for a player.
// This requires "link-tile-dist.csv", "industry-tile-dist.csv"
PlayerUnbuilt* CreatePlayerUnbuilt(const std::string& config_dir,
                                   const std::string& default_config_dir);

// Creates the resource markets.
// This requires "markets.csv".
void CreateMarkets(const std::string& config_dir,
                   const std::string& default_config_dir,
                   Market*& iron_market, Market*& coal_market);

}

#endif  // BRASS_CONFIG_H_
