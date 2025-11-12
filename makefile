# === CONFIGURATION ===
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS :=
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := output
APP_NAME := ProjetDinotofu

# Si Windows, ajoute .exe
ifeq ($(OS),Windows_NT)
    EXE := .exe
else
    EXE :=
endif

TARGET := $(BIN_DIR)/$(APP_NAME)$(EXE)

# === RÉCUPÉRATION DES SOURCES (récursif) ===
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# === RÈGLES ===
all: $(TARGET)
	@echo "Build terminé : $(TARGET)"

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

run: all
	@echo "Exécution..."
	$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Nettoyage complet effectué."

.PHONY: all run clean
