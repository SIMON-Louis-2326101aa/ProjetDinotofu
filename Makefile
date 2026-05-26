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
#   make launch          Build then launch / Compiler puis lancer
#   make install-desktop Create a Linux desktop launcher / Créer un lanceur Linux
#   make desktop         Alias for install-desktop / Alias de install-desktop
#   make remove-desktop  Remove the Linux desktop launcher / Supprimer le lanceur Linux
#   make package-source  Create source ZIP without executable / Créer ZIP source sans exécutable
#   make package-linux-release Build and package Linux release / Créer release Linux
#   make package-linux-installer Build Linux installer pack / Créer pack installer Linux
#   make package-windows-release Build and package Windows release / Créer release Windows
#   make bump-patch      Increase patch version / Augmenter la version patch
#   make release-push    Bump patch, commit and push / Publier un patch
#   make release-check   Verify source tree before sharing / Vérifier le projet avant ZIP
#
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

launch: all
	@clear 2>/dev/null || true
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
	@echo "Exec=bash -lc 'cd $(CURDIR) && ./output/Dinotofu'" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Terminal=true" >> ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Categories=Game;" >> ~/.local/share/applications/$(APP_NAME).desktop
	@chmod +x ~/.local/share/applications/$(APP_NAME).desktop
	@if [ -d "$$HOME/Desktop" ]; then cp ~/.local/share/applications/$(APP_NAME).desktop "$$HOME/Desktop/$(APP_NAME).desktop"; chmod +x "$$HOME/Desktop/$(APP_NAME).desktop"; fi
	@if [ -d "$$HOME/Bureau" ]; then cp ~/.local/share/applications/$(APP_NAME).desktop "$$HOME/Bureau/$(APP_NAME).desktop"; chmod +x "$$HOME/Bureau/$(APP_NAME).desktop"; fi
	@echo "Lanceur installé : ~/.local/share/applications/$(APP_NAME).desktop"

desktop: install-desktop

remove-desktop:
	@rm -f ~/.local/share/applications/$(APP_NAME).desktop
	@echo "Lanceur supprimé."



# =========================================================
# RELEASE PACKAGING
# PACKAGING DE RELEASE
# =========================================================

package-source: clean
	@chmod +x ./scripts/package_source_no_exe.sh
	@./scripts/package_source_no_exe.sh

package-linux-release:
	@chmod +x ./scripts/package_linux_release.sh
	@./scripts/package_linux_release.sh

package-linux-installer:
	@chmod +x ./scripts/package_linux_installer.sh
	@./scripts/package_linux_installer.sh

package-windows-release:
	@chmod +x ./scripts/package_windows_release.sh
	@./scripts/package_windows_release.sh

bump-patch:
	@python3 ./scripts/bump_version.py patch

bump-minor:
	@python3 ./scripts/bump_version.py minor

bump-major:
	@python3 ./scripts/bump_version.py major

release-push:
	@chmod +x ./scripts/release_push.sh
	@./scripts/release_push.sh patch

release-check: clean
	@chmod +x ./scripts/validate_release_tree.sh
	@./scripts/validate_release_tree.sh

.PHONY: all run launch clean rebuild install-desktop desktop remove-desktop package-source package-linux-release package-linux-installer package-windows-release bump-patch bump-minor bump-major release-push release-check
