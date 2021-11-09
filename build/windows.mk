CXX = g++

include default.mk

LIB_DIR = ../lib/windows
EXTRA_CXXFLAGS = -DPDC_NCMOUSE -DPDC_RGB -DCSV_IO_NO_THREAD 

pa2: pa2.exe

pa2.exe: $(SRC_DIR)/main.cpp $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o $@ $(OBJ_FILES) $< \
	  -L$(LIB_DIR) -limgui-for-imtui -limtui -limtui-ncurses -lpdcurses

pa2_sample.exe: $(SRC_DIR)/main.cpp $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o $@ $(OBJ_FILES) $< \
	  -L$(LIB_DIR) -static-libgcc -static-libstdc++ \
	  -Wl,-Bstatic -limgui-for-imtui -limtui -limtui-ncurses -lpdcurses -lstdc++ -lpthread 

clean:
	powershell -noprofile rm -force *.o
	powershell -noprofile rm -force *.exe

distclean: clean
	powershell -noprofile rm -force .depend.mk

-include .depend.mk
