# Toolchain configuration
CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS  := 

# Build directories
TEST_BUILD_DIR := test_build
TEST_TARGET    := sfml_game_test
CONFIG_DIR     := config

# Homebrew configuration
HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo "/opt/homebrew")
BREW_LIBS       := spdlog fmt sfml@2 yaml-cpp  

# Dependency paths (updated with yaml-cpp paths)
SPDLOG_INCLUDE  := $(HOMEBREW_PREFIX)/opt/spdlog/include
FMT_INCLUDE     := $(HOMEBREW_PREFIX)/opt/fmt/include
SFML_INCLUDE    := $(HOMEBREW_PREFIX)/opt/sfml/include
CATCH2_INCLUDE  := $(HOMEBREW_PREFIX)/opt/catch2/include
YAML_INCLUDE    := $(HOMEBREW_PREFIX)/opt/yaml-cpp/include

SPDLOG_LIB      := $(HOMEBREW_PREFIX)/opt/spdlog/lib
FMT_LIB         := $(HOMEBREW_PREFIX)/opt/fmt/lib
SFML_LIB        := $(HOMEBREW_PREFIX)/opt/sfml/lib
YAML_LIB        := $(HOMEBREW_PREFIX)/opt/yaml-cpp/lib

# Include paths (fixed directory structure)
TEST_INCLUDES   := -Isrc \
                   -Iassets \
                   -Ilibs \
                   -Itest/test-src \
                   -Itest/test-src/game \
                   -Itest/test-src/game/globals \
                   -Itest/test-assets \
                   -Itest/test-logging \
                   -Itest/test-testing \
                   -I$(SPDLOG_INCLUDE) \
                   -I$(FMT_INCLUDE) \
                   -I$(SFML_INCLUDE) \
                   -I$(YAML_INCLUDE) \
                   -I$(HOMEBREW_PREFIX)/include/catch2 \
                   -DTESTING

# File collections
TEST_SRC  := $(shell find test -name '*.cpp')
TEST_OBJ  := $(TEST_SRC:test/%.cpp=$(TEST_BUILD_DIR)/%.o)

# Compiler flags
CXXFLAGS += $(TEST_INCLUDES)

# Linker configuration (updated with yaml-cpp paths)
LIB_DIRS  := -L$(SFML_LIB) -L$(SPDLOG_LIB) -L$(FMT_LIB) -L$(YAML_LIB)
TEST_LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio \
             -lspdlog -lfmt -lyaml-cpp

# Environment setup
export DYLD_FALLBACK_LIBRARY_PATH := $(SFML_LIB)

.PHONY: all clean install_deps test help

# Default target
all: test

# Build and run tests
test: $(TEST_TARGET) copy_test_config
	@./$(TEST_TARGET)

# Link test executable
$(TEST_TARGET): $(TEST_OBJ)
	@echo "Linking test executable..."
	@$(CXX) $(CXXFLAGS) $^ -o $@ $(LIB_DIRS) $(TEST_LIBS)

# Build test objects
$(TEST_BUILD_DIR)/%.o: test/%.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Copy test configuration
copy_test_config:
	@mkdir -p $(TEST_BUILD_DIR)/$(CONFIG_DIR)
	@if [ -f test/test-src/$(CONFIG_DIR)/config.yaml ]; then \
		cp test/test-src/$(CONFIG_DIR)/config.yaml $(TEST_BUILD_DIR)/$(CONFIG_DIR)/; \
	fi

# Dependency management
install_deps:
	@for lib in $(BREW_LIBS); do \
		if ! brew list $$lib &>/dev/null; then \
			echo "Installing $$lib..."; \
			brew install $$lib; \
		fi; \
	done
	@# Verify yaml-cpp symlinks
	@if [ ! -L $(HOMEBREW_PREFIX)/opt/yaml-cpp/lib/libyaml-cpp.dylib ]; then \
		echo "Creating missing yaml-cpp symlink..."; \
		ln -s $(HOMEBREW_PREFIX)/Cellar/yaml-cpp/0.8.0/lib/libyaml-cpp.0.8.0.dylib $(HOMEBREW_PREFIX)/opt/yaml-cpp/lib/libyaml-cpp.dylib; \
	fi

# Clean build artifacts
clean:
	@rm -rf $(TEST_BUILD_DIR) $(TEST_TARGET)

# Help message
help:
	@echo "Test-focused Makefile"
	@echo "Available targets:"
	@echo "  all         Build and run tests (default)"
	@echo "  test        Build and run tests"
	@echo "  clean       Remove test build artifacts"
	@echo "  install_deps Install required dependencies (including symlink fixes)"
	@echo "  help        Show this help message"