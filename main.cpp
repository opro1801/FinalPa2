#include "config.h"

#include "draw-deck.h"
#include "game-map.h"
#include "market.h"

#include "game.h"
#include "imtui-io.h"

#include "CLI11.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
  using namespace config;

  CLI::App app{"Brass (COMP2012 Version)"};

  const int num_players = 2;

  std::string default_config_dir = "../config/default";
  std::string config_dir = "../config/default";
  std::string log_file = "brass.log";
  std::string error_log_file = "error.log";
  int seed = 2012;
  bool randomize = false;
  bool log_stderr = false;

  app.add_option("--default-config-dir", default_config_dir, "The default config directory. Defaults to \"../config/default\".")->check(CLI::ExistingDirectory);
  app.add_option("--config-dir", config_dir, "The config directory which contains files which you want to override the default ones.")->check(CLI::ExistingDirectory);
  app.add_option("--log-file", log_file, "The log file that will be written to. Defaults to \"brass.log\".");
  app.add_option("--log-stderr", log_stderr, "Log standard error to file. An alternative to error redirection, especially for Windows users. Defaults to false.");
  app.add_option("--error-log-file", error_log_file, "The error log file that will be written to. Defaults to \"error.log\".");
  app.add_option("--randomize", randomize,  "If true, this shuffles the card deck with the seed specified by --seed. Defaults to false.");
  app.add_option("--seed", seed,  "The seed for shuffling the card deck. Defaults to 2012.");

  CLI11_PARSE(app, argc, argv);

  if (log_stderr) {
    freopen(error_log_file.c_str(), "w", stderr);
  }

  Io* io = new ImtuiIo(log_file);

  Map* game_map;
  DrawDeck* card_deck;
  Market *coal_market, *iron_market;

  CreateMapAndDrawDeck(config_dir, default_config_dir, num_players, game_map, card_deck);
  CreateMarkets(config_dir, default_config_dir, iron_market, coal_market);

  if (randomize) {
    card_deck->set_seed(seed);
  }

  Game game(game_map, card_deck, coal_market, iron_market, *io);
  game.AddPlayer("Alice", CreatePlayerUnbuilt(default_config_dir, config_dir));
  game.AddPlayer("Bob", CreatePlayerUnbuilt(default_config_dir, config_dir));

  game.Run();

  delete io;
}
