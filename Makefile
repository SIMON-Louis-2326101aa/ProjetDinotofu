# =========================================================
# DINOTOFU - LINUX MAKEFILE
# DINOTOFU - MAKEFILE LINUX
# =========================================================
#
# Available commands:
# Commandes disponibles :
#
#   make                 Build the project / Compiler le projet
#   make run             Build and run / Compiler et lancer
#   make clean           Remove generated files / Supprimer les fichiers générés
#   make rebuild         Clean then rebuild / Nettoyer puis recompiler
#   make install-desktop Create a Linux desktop launcher / Créer un lanceur Linux
#   make remove-desktop  Remove the Linux desktop launcher / Supprimer le lanceur Linux
#
# Direct launch after build:
# Lancement direct après compilation :
#
#   ./output/Dinotofu
#
# =========================================================


# =========================================================
# CONFIGURATION
# CONFIGURATION
# =========================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS  :=

SRC_DIR  := src
OBJ_DIR  := build
BIN_DIR  := output

APP_NAME := Dinotofu
TARGET   := $(BIN_DIR)/$(APP_NAME)


# =========================================================
# SOURCES / OBJECT FILES
# SOURCES / FICHIERS OBJETS
# =========================================================

SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)


# =========================================================
# MAIN RULES
# RÈGLES PRINCIPALES
# =========================================================

all: $(TARGET)
	@echo ""
	@echo "Build terminé avec succès."
	@echo "Exécutable : $(TARGET)"
	@echo "Pour lancer : make run"
	@echo ""

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@chmod u+x $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)


# =========================================================
# USEFUL COMMANDS
# COMMANDES UTILES
# =========================================================

run: all
	@echo "Lancement de $(APP_NAME)..."
	@./$(TARGET)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Nettoyage terminé."

rebuild: clean all


# =========================================================
# CLICKABLE LINUX LAUNCHER
# LANCEUR CLIQUABLE LINUX
# =========================================================

install-desktop: all
	@mkdir -p ~/.local/share/applications
	@echo "[Desktop Entry]" > ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Type=Application" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Name=$(APP_NAME)" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Comment=Jeu RPG terminal Dinotofu" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Exec=gnome-terminal -- bash -c 'cd $(CURDIR) && ./$(TARGET); echo; echo Appuyez sur Entrée pour fermer...; read'" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Terminal=false" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Categories=Game;" >> ~/.local/share/applications/$(APP_NAME).desktop
	@chmod +x ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Lanceur installé : ~/.local/share/applications/$(APP_NAME).desktop"

remove-desktop:
	@rm -f ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Lanceur supprimé."


.PHONY: all run clean rebuild install-desktop remove-desktop
