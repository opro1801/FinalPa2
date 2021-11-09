infrastructure.o: ../infrastructure.cpp ../infrastructure.h ../player.h \
 ../util-common.h ../game-common.h
merchant.o: ../merchant.cpp ../merchant.h ../game-common.h
market.o: ../market.cpp ../market.h ../game-common.h ../game-observer.h \
 ../util-common.h
location.o: ../location.cpp ../location.h ../game-common.h \
 ../util-common.h ../game-observer.h ../link.h ../infrastructure.h \
 ../player.h
industry-location.o: ../industry-location.cpp ../industry-location.h \
 ../location.h ../game-common.h ../util-common.h ../game-observer.h \
 ../link.h ../infrastructure.h ../player.h ../industry.h
merchant-location.o: ../merchant-location.cpp ../merchant-location.h \
 ../merchant.h ../game-common.h ../location.h ../util-common.h \
 ../game-observer.h ../link.h ../infrastructure.h ../player.h
industry.o: ../industry.cpp ../industry.h ../location.h ../game-common.h \
 ../util-common.h ../game-observer.h ../link.h ../infrastructure.h \
 ../player.h
industry-todo.o: ../industry-todo.cpp ../industry.h ../location.h \
 ../game-common.h ../util-common.h ../game-observer.h ../link.h \
 ../infrastructure.h ../player.h
link.o: ../link.cpp ../link.h ../game-common.h ../location.h \
 ../util-common.h ../game-observer.h ../infrastructure.h ../player.h
link-todo.o: ../link-todo.cpp ../link.h ../game-common.h ../location.h \
 ../util-common.h ../game-observer.h ../infrastructure.h ../player.h
game-map.o: ../game-map.cpp ../game-map.h ../game-common.h ../link.h \
 ../location.h ../util-common.h ../game-observer.h ../infrastructure.h \
 ../player.h ../industry.h ../merchant.h ../merchant-location.h \
 ../industry-location.h
player-unbuilt.o: ../player-unbuilt.cpp ../player-unbuilt.h ../link.h \
 ../game-common.h ../location.h ../util-common.h ../game-observer.h \
 ../infrastructure.h ../player.h ../industry.h
player-network.o: ../player-network.cpp ../player-network.h ../location.h \
 ../game-common.h ../util-common.h ../game-observer.h ../link.h \
 ../infrastructure.h ../player.h ../industry.h
player-hand.o: ../player-hand.cpp ../player-hand.h ../card.h \
 ../location.h ../game-common.h ../util-common.h ../game-observer.h \
 ../link.h ../infrastructure.h ../player.h ../industry.h \
 ../player-network.h
player.o: ../player.cpp ../player.h ../util-common.h ../game-common.h \
 ../player-hand.h ../card.h ../location.h ../game-observer.h ../link.h \
 ../infrastructure.h ../industry.h ../player-network.h \
 ../player-unbuilt.h
card.o: ../card.cpp ../card.h ../location.h ../game-common.h \
 ../util-common.h ../game-observer.h ../link.h ../infrastructure.h \
 ../player.h ../industry.h ../player-network.h
draw-deck.o: ../draw-deck.cpp ../draw-deck.h ../card.h ../location.h \
 ../game-common.h ../util-common.h ../game-observer.h ../link.h \
 ../infrastructure.h ../player.h ../industry.h ../player-network.h
consumption-request.o: ../consumption-request.cpp \
 ../consumption-request.h ../game-common.h ../location.h ../util-common.h \
 ../game-observer.h ../link.h ../infrastructure.h ../player.h \
 ../industry.h ../game-map.h ../merchant.h ../merchant-location.h \
 ../industry-location.h ../market.h ../game-io.h ../player-network.h
player-action.o: ../player-action.cpp ../player-action.h ../game-io.h \
 ../game-common.h ../util-common.h ../player.h ../consumption-request.h \
 ../location.h ../game-observer.h ../link.h ../infrastructure.h \
 ../industry.h ../game-map.h ../merchant.h ../merchant-location.h \
 ../industry-location.h ../market.h ../player-network.h ../draw-deck.h \
 ../card.h ../player-hand.h ../player-unbuilt.h
game-observer.o: ../game-observer.cpp ../game-observer.h ../game-common.h \
 ../util-common.h
config.o: ../config.cpp ../config.h ../industry-location.h ../location.h \
 ../game-common.h ../util-common.h ../game-observer.h ../link.h \
 ../infrastructure.h ../player.h ../industry.h ../merchant-location.h \
 ../merchant.h ../draw-deck.h ../card.h ../player-network.h \
 ../player-unbuilt.h ../market.h ../game-map.h \
 ../include/fast-cpp-csv-parser/csv.h
util-common.o: ../util-common.cpp ../util-common.h ../game-common.h
imtui-io.o: ../imtui-io.cpp ../imtui-io.h ../game-io.h ../game-common.h \
 ../util-common.h ../include/imtui/imtui.h ../include/imgui/imgui.h \
 ../include/imgui/imconfig.h ../include/imtui/imtui-impl-text.h \
 ../include/imtui/imtui-impl-ncurses.h
game-io.o: ../game-io.cpp ../game-io.h ../game-common.h ../util-common.h
game.o: ../game.cpp ../game.h ../game-map.h ../game-common.h ../link.h \
 ../location.h ../util-common.h ../game-observer.h ../infrastructure.h \
 ../player.h ../industry.h ../merchant.h ../merchant-location.h \
 ../industry-location.h ../draw-deck.h ../card.h ../player-network.h \
 ../market.h ../consumption-request.h ../game-io.h ../player-action.h \
 ../player-hand.h ../player-unbuilt.h ../config.h
