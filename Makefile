
# # Compiler and linker settings
# CXX := clang++

# # Compiler flags
# CXXFLAGS := -std=c++17 -Wall \
#             -I./src -I./src/game -I./src/game/globals -I./src/game/core -I./src/game/physics -I./src/game/camera -I./src/game/utils -I./src/game/scenes \
#             -I./assets/sprites -I./assets/fonts -I./assets/sound -I./assets/tiles \
#             -I./libs/logging

# # Detect Homebrew installation and set path
# HOMEBREW_PREFIX ?= $(shell brew --prefix 2>/dev/null || echo "/opt/homebrew")

# # If Homebrew is not found, install it
# ifeq (, $(shell which brew))
# $(error "Homebrew not found! Please install Homebrew manually: https://brew.sh/")
# endif

# # Homebrew paths, allow override via environment variables
# SPDLOG_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/spdlog/include
# FMT_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/fmt/include
# SFML_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/sfml@2/include
# SPDLOG_LIB ?= $(HOMEBREW_PREFIX)/opt/spdlog/lib
# FMT_LIB ?= $(HOMEBREW_PREFIX)/opt/fmt/lib
# SFML_LIB ?= $(HOMEBREW_PREFIX)/opt/sfml@2/lib
# YAML_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/yaml-cpp/include

# export DYLD_FALLBACK_LIBRARY_PATH=$(SFML_LIB)

# # Include paths for Homebrew libraries
# BREW_INCLUDE_FLAGS := -I$(SPDLOG_INCLUDE) -I$(FMT_INCLUDE) -I$(SFML_INCLUDE) -I$(YAML_INCLUDE)
# CXXFLAGS += $(BREW_INCLUDE_FLAGS)

# TEST_CXXFLAGS := -std=c++17 -Wall \
#                  -I./test/test-src \
#                  -I./test/test-src/game/core -I./test/test-src/game/camera \
#                  -I./test/test-src/game/globals -I./test/test-src/game/physics \
#                  -I./test/test-src/game/scenes -I./test/test-src/game/utils \
#                  -I./test/test-assets -I./test/test-assets/fonts \
#                  -I./test/test-assets/sound -I./test/test-assets/tiles \
#                  -I./test/test-assets/sprites \
#                  -I./test/test-logging \
#                  -I./test/test-network \
#                  -I$(SPDLOG_INCLUDE) -I$(FMT_INCLUDE) -I$(SFML_INCLUDE) -I$(YAML_INCLUDE) \
#                  -DTESTING

# # Library paths and linking
# LDFLAGS = -L$(SPDLOG_LIB) -L$(FMT_LIB) -L$(SFML_LIB) -L$(HOMEBREW_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lspdlog -lfmt -lyaml-cpp
# LDFLAGS += -L$(SFML_LIB)

# # Build directories
# BUILD_DIR := build
# TEST_BUILD_DIR := test_build

# OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# # Test source and object files
# TEST_SRC := test/test-src/testMain.cpp \
#             test/test-src/game/globals/globals.cpp \
#             test/test-src/game/core/game.cpp \
#             test/test-src/game/physics/physics.cpp \
#             test/test-src/game/camera/window.cpp \
#             test/test-src/game/utils/utils.cpp \
#             test/test-src/game/scenes/scenes.cpp \
#             test/test-assets/sprites/sprites.cpp \
#             test/test-assets/fonts/fonts.cpp \
#             test/test-assets/sound/sound.cpp \
#             test/test-assets/tiles/tiles.cpp \
#             test/test-logging/log.cpp \
#             test/test-network/network.cpp

# TEST_OBJ := $(TEST_SRC:%.cpp=$(TEST_BUILD_DIR)/%.o)

# # New target to copy YAML config file
# COPY_CONFIG:
# 	@mkdir -p $(TEST_BUILD_DIR)/config
# 	@test -f test/test-src/game/globals/config.yaml && cp test/test-src/game/globals/config.yaml $(TEST_BUILD_DIR)/config/ || echo "Warning: config.yaml not found."

# # Target executables
# TARGET := sfml_game
# TEST_TARGET := sfml_game_test

# .PHONY: all install_deps build clean test run

# # Default target (build the main application)
# all: $(TARGET)

# # Check and install required dependencies via Homebrew
# install_deps:
# 	@brew list spdlog >/dev/null 2>&1 || (echo "Installing spdlog..."; brew install spdlog)
# 	@brew list fmt >/dev/null 2>&1 || (echo "Installing fmt..."; brew install fmt)
# 	@brew list sfml@2 >/dev/null 2>&1 || brew install sfml@2 || echo "Make sure SFML 2 is installed."
# 	@brew list yaml-cpp >/dev/null 2>&1 || (echo "Installing yaml-cpp..."; brew install yaml-cpp) 

# # Test build target
# $(TEST_TARGET): $(TEST_OBJ)
# 	$(CXX) $(TEST_CXXFLAGS) -o $@ $(TEST_OBJ) $(LDFLAGS)

# # Rule to build test object files
# $(TEST_BUILD_DIR)/%.o: %.cpp
# 	@mkdir -p $(dir $@)
# 	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

# # Clean up all build artifacts
# clean:
# 	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) $(TARGET) $(TEST_TARGET)

# test: $(TEST_TARGET) COPY_CONFIG
# 	./$(TEST_TARGET) 

# Compiler and linker settings
CXX := clang++

# Compiler flags
CXXFLAGS := -std=c++17 -Wall \
            -I./src -I./src/game -I./src/game/globals -I./src/game/core -I./src/game/physics -I./src/game/camera -I./src/game/utils -I./src/game/scenes \
            -I./assets/sprites -I./assets/fonts -I./assets/sound -I./assets/tiles \
            -I./libs/logging \
            -DRUN_NETWORK=1

# Detect Homebrew installation and set path
HOMEBREW_PREFIX ?= $(shell brew --prefix 2>/dev/null || echo "/opt/homebrew")

# If Homebrew is not found, install it
ifeq (, $(shell which brew))
$(error "Homebrew not found! Please install Homebrew manually: https://brew.sh/")
endif

# Homebrew paths, allow override via environment variables
SPDLOG_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/spdlog/include
FMT_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/fmt/include
SFML_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/sfml@2/include
SPDLOG_LIB ?= $(HOMEBREW_PREFIX)/opt/spdlog/lib
FMT_LIB ?= $(HOMEBREW_PREFIX)/opt/fmt/lib
SFML_LIB ?= $(HOMEBREW_PREFIX)/opt/sfml@2/lib
YAML_INCLUDE ?= $(HOMEBREW_PREFIX)/opt/yaml-cpp/include

export DYLD_FALLBACK_LIBRARY_PATH=$(SFML_LIB)

# Include paths for Homebrew libraries
BREW_INCLUDE_FLAGS := -I$(SPDLOG_INCLUDE) -I$(FMT_INCLUDE) -I$(SFML_INCLUDE) -I$(YAML_INCLUDE)
CXXFLAGS += $(BREW_INCLUDE_FLAGS)

TEST_CXXFLAGS := -std=c++17 -Wall \
                 -I./test/test-src \
                 -I./test/test-src/game/core -I./test/test-src/game/camera \
                 -I./test/test-src/game/globals -I./test/test-src/game/physics \
                 -I./test/test-src/game/scenes -I./test/test-src/game/utils \
                 -I./test/test-assets -I./test/test-assets/fonts \
                 -I./test/test-assets/sound -I./test/test-assets/tiles \
                 -I./test/test-assets/sprites \
                 -I./test/test-logging \
                 -I./test/test-network \
                 -I$(SPDLOG_INCLUDE) -I$(FMT_INCLUDE) -I$(SFML_INCLUDE) -I$(YAML_INCLUDE) \
                 -DTESTING \
                 -DRUN_NETWORK=1

# Library paths and linking
LDFLAGS = -L$(SPDLOG_LIB) -L$(FMT_LIB) -L$(SFML_LIB) -L$(HOMEBREW_PREFIX)/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lspdlog -lfmt -lyaml-cpp
LDFLAGS += -L$(SFML_LIB)

# Build directories
BUILD_DIR := build
TEST_BUILD_DIR := test_build

OBJ := $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# Test source and object files
TEST_SRC := test/test-src/testMain.cpp \
            test/test-src/game/globals/globals.cpp \
            test/test-src/game/core/game.cpp \
            test/test-src/game/physics/physics.cpp \
            test/test-src/game/camera/window.cpp \
            test/test-src/game/utils/utils.cpp \
            test/test-src/game/scenes/scenes.cpp \
            test/test-assets/sprites/sprites.cpp \
            test/test-assets/fonts/fonts.cpp \
            test/test-assets/sound/sound.cpp \
            test/test-assets/tiles/tiles.cpp \
            test/test-logging/log.cpp \
            test/test-network/network.cpp

TEST_OBJ := $(TEST_SRC:%.cpp=$(TEST_BUILD_DIR)/%.o)

# New target to copy YAML config file
COPY_CONFIG:
	@mkdir -p $(TEST_BUILD_DIR)/config
	@test -f test/test-src/game/globals/config.yaml && cp test/test-src/game/globals/config.yaml $(TEST_BUILD_DIR)/config/ || echo "Warning: config.yaml not found."

# Target executables
TARGET := sfml_game
TEST_TARGET := sfml_game_test

.PHONY: all install_deps build clean test run

# Default target (build the main application)
all: $(TARGET)

# Check and install required dependencies via Homebrew
install_deps:
	@brew list spdlog >/dev/null 2>&1 || (echo "Installing spdlog..."; brew install spdlog)
	@brew list fmt >/dev/null 2>&1 || (echo "Installing fmt..."; brew install fmt)
	@brew list sfml@2 >/dev/null 2>&1 || brew install sfml@2 || echo "Make sure SFML 2 is installed."
	@brew list yaml-cpp >/dev/null 2>&1 || (echo "Installing yaml-cpp..."; brew install yaml-cpp) 

# Test build target
$(TEST_TARGET): $(TEST_OBJ)
	$(CXX) $(TEST_CXXFLAGS) -o $@ $(TEST_OBJ) $(LDFLAGS)

# Rule to build test object files
$(TEST_BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

# Clean up all build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TEST_BUILD_DIR) $(TARGET) $(TEST_TARGET)

test: $(TEST_TARGET) COPY_CONFIG
	./$(TEST_TARGET)