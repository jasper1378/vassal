# Remember:
# GNU make is a picky little bugger who doesn't like spaces in his file paths

BIN_NAME := vassal
VERSION_MAJOR := 0
VERSION_MINOR := 0
CXX := g++
COMPILE_FLAGS := -std=c++20 -Wall -Wextra -g
RELEASE_COMPILE_FLAGS := -O2 -DNDEBUG
DEBUG_COMPILE_FLAGS := -Og -DDEBUG
LINK_FLAGS :=
RELEASE_LINK_FLAGS :=
DEBUG_LINK_FLAGS :=
SOURCE_DIRS := ./source
SOURCE_FILE_EXT := .cpp
SUBMODULE_DIR := ./submodules
INCLUDE_DIRS := ./include
HEADER_FILE_EXT := .hpp
LIBRARIES :=
INSTALL_PATH := /usr/local

##########

SHELL := /bin/bash

.SUFFIXES:

export BUILD_DIR := ./build

INCLUDE_DIRS += $(wildcard $(SUBMODULE_DIR)/*/include)
LINK_FLAGS += $(addprefix -l, $(LIBRARIES))
SUBMODULE_OBJECTS := $(wildcard $(SUBMODULE_DIR)/*/build/*.a)
INCLUDE_FLAGS := $(addprefix -I, $(shell find $(INCLUDE_DIRS) -type d))

BIN_INSTALL_PATH := $(INSTALL_PATH)/bin

export CPPFLAGS := $(INCLUDE_FLAGS) -MMD -MP

release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RELEASE_COMPILE_FLAGS)
release: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(RELEASE_LINK_FLAGS)
debug: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DEBUG_COMPILE_FLAGS)
debug: export LDFLAGS := $(LDFLAGS) $(LINK_FLAGS) $(DEBUG_LINK_FLAGS)

SOURCES := $(shell find $(SOURCE_DIRS) -type f -name '*$(SOURCE_FILE_EXT)')
OBJECTS := $(SOURCES:%=$(BUILD_DIR)/%.o)
DEPENDENCIES := $(OBJECTS:.o=.d)

.PHONY: release
release:
	@$(MAKE) all --no-print-directory

.PHONY: debug
debug:
	@$(MAKE) all --no-print-directory

.PHONY: all
all: $(BUILD_DIR)/$(BIN_NAME)

$(BUILD_DIR)/$(BIN_NAME): $(OBJECTS)
	$(CXX) $(OBJECTS) $(SUBMODULE_OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%$(SOURCE_FILE_EXT).o: %$(SOURCE_FILE_EXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDENCIES)

.PHONY: install
install:
	@install -v -Dm755 $(BUILD_DIR)/$(BIN_NAME) -t $(BIN_INSTALL_PATH)/

.PHONY: uninstall
uninstall:
	@rm -v $(BIN_INSTALL_PATH)/$(BIN_NAME)

.PHONY: clean
clean:
	@rm -v -r $(BUILD_DIR)
