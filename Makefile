SRC_DIR := ./src
OBJ_DIR_GUI := ./src/obj/gui
OBJ_DIR_TUI := ./src/obj/tui
dummy_build_gui := $(shell mkdir -p $(OBJ_DIR_GUI))
dummy_build_tui := $(shell mkdir -p $(OBJ_DIR_TUI))
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES_GUI := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR_GUI)/%.o,$(SRC_FILES))
OBJ_FILES_TUI := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR_TUI)/%.o,$(SRC_FILES))
CPPFLAGS := -O3 -std=c++14 -nostdlib -fno-exceptions
CXXFLAGS += -MMD

OS := $(shell uname)
CPU := $(shell uname -m)
ifeq ($(CPU),x86_64)
	ABI := AMD64
else
	ifeq ($(CPU),riscv64)
		ABI := RISCV64
	else
		CPU := arm64
		ABI := ARM64
	endif
endif

all:		hostenv tui gui
gui:		hostenv obj/$(CPU)/$(ABI)/$(OS)/main_gui
tui:		hostenv obj/$(CPU)/$(ABI)/$(OS)/main_tui
install:	clean hostenv tui gui inst

hostenv:
	@echo $(CPU) > cpu
	@echo $(OS) > os
	@echo $(ABI) > abi
	mkdir -p obj/$(CPU)/$(ABI)/$(OS)	

snapshot:
	rm -f snapshot.zip
	zip -9q snapshot.zip \
		obj/vp64/VP64/sys/boot_image \
		`find obj -name "main_gui.exe"` \
		`find obj -name "main_tui.exe"`

inst:
	@./stop.sh
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 005-008 -l 002-008 -l 007-008 -l 006-008 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 004-007 -l 001-007 -l 006-007 -l 007-008 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 003-006 -l 000-006 -l 006-008 -l 006-007 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 002-005 -l 005-008 -l 004-005 -l 003-005 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 001-004 -l 004-007 -l 003-004 -l 004-005 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 000-003 -l 003-006 -l 003-005 -l 003-004 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 002-008 -l 002-005 -l 001-002 -l 000-002 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 001-007 -l 001-004 -l 000-001 -l 001-002 &
	@./obj/$(CPU)/$(ABI)/$(OS)/main_tui -e obj\vp64\VP64\sys\boot_image -l 000-006 -l 000-003 -l 000-002 -l 000-001 -run apps/tui/install.lisp

obj/$(CPU)/$(ABI)/$(OS)/main_gui:	$(OBJ_FILES_GUI)
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		-F/Library/Frameworks \
		$(SRC_DIR)/libusb/macos/$(CPU)/libusb-1.0.a \
		-framework CoreFoundation \
		-framework IOKit \
		-framework SDL2
endif
ifeq ($(OS),Linux)
	c++ -o $@ $^ \
		-pthread \
		-L/usr/local/lib -lusb-1.0 \
		$(shell sdl2-config --libs)
endif

obj/$(CPU)/$(ABI)/$(OS)/main_tui:	$(OBJ_FILES_TUI)
ifeq ($(OS),Darwin)
	c++ -o $@ $^ \
		$(SRC_DIR)/libusb/macos/$(CPU)/libusb-1.0.a \
		-framework CoreFoundation \
		-framework IOKit
endif
ifeq ($(OS),Linux)
	c++ -o $@ $^ \
		-pthread \
		-L/usr/local/lib -lusb-1.0
endif

$(OBJ_DIR_GUI)/%.o: $(SRC_DIR)/%.cpp
ifeq ($(OS),Darwin)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c -D_GUI=GUI \
		-I/Library/Frameworks/SDL2.framework/Headers/ \
		-I$(SRC_DIR)/libusb/ \
		-o $@ $<
endif
ifeq ($(OS),Linux)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c -D_GUI=GUI \
		-I/usr/include/SDL2/ \
		-I/usr/local/include/libusb-1.0/ \
		-o $@ $<
endif

$(OBJ_DIR_TUI)/%.o: $(SRC_DIR)/%.cpp
ifeq ($(OS),Darwin)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c \
		-I$(SRC_DIR)/libusb/ \
		-o $@ $<
endif
ifeq ($(OS),Linux)
	c++ $(CPPFLAGS) $(CXXFLAGS) -c \
		-I/usr/local/include/libusb-1.0/ \
		-o $@ $<
endif

clean:
	rm -rf $(OBJ_DIR_GUI)/*
	rm -rf $(OBJ_DIR_TUI)/*
	unzip -nq snapshot.zip

 -include $(OBJ_FILES_GUI:.o=.d)
 -include $(OBJ_FILES_TUI:.o=.d)
