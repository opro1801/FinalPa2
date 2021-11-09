CXX = g++

include default.mk

LIB_DIR = ../lib/macos
EXTRA_CXXFLAGS =

pa2: $(SRC_DIR)/main.cpp $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o $@ $(OBJ_FILES) $< \
	  -L$(LIB_DIR) -limgui-for-imtui -limtui -limtui-ncurses -lncurses -lpthread

run: pa2
	LD_LIBRARY_PATH=$(LIB_DIR):$LD_LIBRARY_PATH \
		./pa2 2>error.log

clean:
	rm -f *.o pa2

distclean: clean
	rm -f .depend.mk

-include .depend.mk
