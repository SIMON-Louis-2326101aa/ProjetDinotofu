# =========================================================
# PROJET DINOTOFU - MAKEFILE LINUX
# =========================================================
#
# COMMANDES DISPONIBLES :
#
# Compiler le projet :
#   make
#
# Lancer le jeu :
#   make run
#
# Nettoyer les fichiers générés :
#   make clean
#
# Recompiler depuis zéro :
#   make rebuild
#
# Créer un lanceur cliquable Linux :
#   make install-desktop
#
# Supprimer le lanceur cliquable Linux :
#   make remove-desktop
#
# Lancer directement l'exécutable après compilation :
#   ./output/ProjetDinotofu
#
# =========================================================


# =========================================================
# CONFIGURATION
# =========================================================

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS  :=

SRC_DIR  := src
OBJ_DIR  := build
BIN_DIR  := output

APP_NAME := ProjetDinotofu
TARGET   := $(BIN_DIR)/$(APP_NAME)


# =========================================================
# SOURCES / OBJETS
# =========================================================

SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)


# =========================================================
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