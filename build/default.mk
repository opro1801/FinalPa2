CXXFLAGS = -std=c++11 -I../include -g1 -O0 -Wall -Wextra -pedantic -Wno-switch

OBJ_FILES = infrastructure.o merchant.o market.o \
  location.o industry-location.o merchant-location.o \
  industry.o industry-todo.o \
  link.o link-todo.o \
  game-map.o \
  player-unbuilt.o player-network.o player-hand.o player.o \
  card.o draw-deck.o \
  consumption-request.o player-action.o \
  game-observer.o config.o util-common.o \
  imtui-io.o game-io.o game.o

SRC_DIR = ..
SRC_FILES = $(addprefix $(SRC_DIR)/, $(OBJ_FILES:.o=.cpp))

.PHONY: clean distclean

all: pa2

%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/%.h
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c -o $@ $<

industry-todo.o: $(SRC_DIR)/industry-todo.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c -o $@ $<

link-todo.o: $(SRC_DIR)/link-todo.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c -o $@ $<

depend:
	$(CXX) $(CXXFLAGS) -MM $(SRC_FILES) -c >.depend.mk
