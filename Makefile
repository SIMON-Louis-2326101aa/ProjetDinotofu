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
#   make test            Build and run lightweight project checks / Compiler et tester les invariants
#   make check           Clean, validate tree, branding and run tests / Contrôles complets avant commit
#   make rebuild         Clean then rebuild / Nettoyer puis recompiler
#   make strip           Strip debug symbols from binary / Retirer les symboles de debug
#   make launch          Build then launch / Compiler puis lancer
#   make help            Display help with all available commands / Afficher l'aide
#   make install-desktop Create a Linux desktop launcher / Créer un lanceur Linux
#   make desktop         Alias for install-desktop / Alias de install-desktop
#   make remove-desktop  Remove the Linux desktop launcher / Supprimer le lanceur Linux
#   make package-source  Create clean source ZIP / Créer ZIP source propre
#   make package-linux-release Build and package Linux release / Créer release Linux
#   make package-windows-release Build and package Windows release / Créer release Windows
#   make bump-patch      Increase patch version / Augmenter la version patch
#   make release-push    Bump patch, commit and push / Publier un patch
#   make release-check   Verify source tree before sharing / Vérifier le projet avant ZIP
#   make gui-preview     Build, launch the game and serve the GUI debug preview / Lancer aperçu IG
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
.DEFAULT_GOAL := all

# Compilation parallèle automatique sur tous les cœurs disponibles
ifeq ($(filter -j%,$(MAKEFLAGS)),)
  NPROCS ?= $(shell nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)
  MAKEFLAGS += -j$(NPROCS)
endif

CXX         ?= g++
TARGET_ARCH ?= native
OPT_LEVEL   ?= -O3
CXX_STD_FLAG ?= $(shell bash ./scripts/detect_cpp23_flag.sh "$(CXX)")
CXXFLAGS    := $(CXX_STD_FLAG) $(OPT_LEVEL) -march=$(TARGET_ARCH) -pipe -Wall -Wextra -Iinclude -MMD -MP -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS     ?=

# Qt6
QT_CXXFLAGS := $(shell pkg-config --cflags Qt6Widgets)
QT_LIBS     := $(shell pkg-config --libs Qt6Widgets)
QT_MOC := /usr/lib/qt6/libexec/moc

SRC_DIR  := src
OBJ_DIR  := build
BIN_DIR  := output

APP_NAME := Dinotofu
TARGET   := $(BIN_DIR)/$(APP_NAME)

MOC_NEWGAME_CPP := $(OBJ_DIR)/interface/qt/screens/moc_NewGameScreen.cpp
MOC_NEWGAME_OBJ := $(OBJ_DIR)/interface/qt/screens/moc_NewGameScreen.o


# =========================================================
# SOURCES / OBJECT FILES
# SOURCES / FICHIERS OBJETS
# =========================================================

SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp")

# Sources du jeu terminal
GAME_SRCS := $(filter-out $(SRC_DIR)/gui_main.cpp,$(SRCS))
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(GAME_SRCS))
DEPS := $(OBJS:.o=.d)

# Sources de l'interface Qt
GAME_OBJS_NO_MAIN := $(filter-out $(OBJ_DIR)/main.o,$(OBJS))
GUI_EXTRA_SRCS := $(SRC_DIR)/gui_main.cpp \
                  $(SRC_DIR)/interface/qt/MainWindow.cpp

GUI_EXTRA_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(GUI_EXTRA_SRCS))
GUI_OBJS := $(GAME_OBJS_NO_MAIN) $(GUI_EXTRA_OBJS) $(MOC_NEWGAME_OBJ)
GUI_DEPS := $(GUI_EXTRA_OBJS:.o=.d)

GUI_TARGET := $(BIN_DIR)/DinotofuGUI


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

$(OBJ_DIR)/interface/qt/%.o: $(SRC_DIR)/interface/qt/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/gui_main.o: $(SRC_DIR)/gui_main.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@

$(GUI_TARGET): $(GUI_OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(QT_LIBS)
	@chmod u+x $@
$(MOC_NEWGAME_CPP): $(SRC_DIR)/interface/qt/screens/NewGameScreen.hpp
	@mkdir -p $(dir $@)
	$(QT_MOC) $< -o $@

$(MOC_NEWGAME_OBJ): $(MOC_NEWGAME_CPP)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(QT_CXXFLAGS) -c $< -o $@		
gui: $(GUI_TARGET)
	@echo ""
	@echo "Interface Qt compilée avec succès."
	@echo "Exécutable : $(GUI_TARGET)"
	@echo "Pour lancer : ./$(GUI_TARGET)"
	@echo ""	

-include $(DEPS)


# =========================================================
# USEFUL COMMANDS
# COMMANDES UTILES
# =========================================================


test: all
	@bash ./scripts/test_project.sh

check: clean
	@echo "=== [1/3] Validation de l'arborescence et des conventions ==="
	@chmod +x ./scripts/validate_release_tree.sh
	@./scripts/validate_release_tree.sh
	@echo ""
	@echo "=== [2/3] Validation des assets de branding ==="
	@chmod +x ./scripts/validate_branding_assets.sh
	@./scripts/validate_branding_assets.sh
	@echo ""
	@echo "=== [3/3] Compilation et exécution de la suite de tests ==="
	@$(MAKE) test
	@echo ""
	@echo "========================================================="
	@echo " Tous les contrôles ont réussi avec succès !"
	@echo "========================================================="

help:
	@echo "========================================================="
	@echo " DINOTOFU - AIDE MAKEFILE"
	@echo "========================================================="
	@echo ""
	@echo " Compilation & Exécution locale :"
	@echo "   make                     Compiler le projet en parallèle (output/$(APP_NAME))"
	@echo "   make run [ARGS=...]      Compiler puis lancer le jeu dans le terminal"
	@echo "   make launch [ARGS=...]   Compiler, effacer l'écran puis lancer"
	@echo "   make clean               Supprimer les objets, binaires et dossiers de debug"
	@echo "   make rebuild             Nettoyer puis recompiler de zéro"
	@echo "   make strip               Retirer les symboles de débogage du binaire"
	@echo ""
	@echo " Tests & Qualité de code :"
	@echo "   make test                Exécuter la suite de tests du projet"
	@echo "   make check               Nettoyer, valider l'arborescence, branding et tests"
	@echo "   make release-check       Vérifier l'arborescence avant création d'archive"
	@echo ""
	@echo " Interface Graphique Expérimentale :"
	@echo "   make gui-preview         Lancer le jeu avec le serveur d'aperçu web IG"
	@echo ""
	@echo " Raccourcis Système (Linux Desktop) :"
	@echo "   make install-desktop     Créer le raccourci (.desktop) sur le bureau"
	@echo "   make remove-desktop      Supprimer le raccourci (.desktop)"
	@echo ""
	@echo " Distribution & Packaging :"
	@echo "   make package-linux-release   Compiler et créer l'archive 7z Linux"
	@echo "   make package-windows-release Compiler et créer l'archive 7z Windows"
	@echo "   make package-source          Créer l'archive 7z des sources propres"
	@echo "   make bump-patch / minor      Incrémenter la version (patch/minor)"
	@echo "   make release-push            Incrémenter le patch, commiter et pusher"
	@echo "   make help                Afficher ce message d'aide"
	@echo "========================================================="

run: all
	@echo "Lancement de $(APP_NAME)..."
	@./$(TARGET) $(ARGS)

launch: all
	@clear 2>/dev/null || true
	@./$(TARGET) $(ARGS)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR) gui_debug
	@echo "Nettoyage terminé."

rebuild: clean
	@$(MAKE) all

strip: $(TARGET)
	@strip --strip-all $(TARGET) 2>/dev/null || true
	@echo "Symboles de debug retirés de $(TARGET)."


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
	@DESK_DIR=$$(command -v xdg-user-dir >/dev/null 2>&1 && xdg-user-dir DESKTOP 2>/dev/null || true); \
	for d in "$$DESK_DIR" "$$HOME/Desktop" "$$HOME/Bureau" "$$HOME/Escritorio" "$$HOME/Schreibtisch"; do \
		if [ -n "$$d" ] && [ -d "$$d" ]; then \
			cp ~/.local/share/applications/$(APP_NAME).desktop "$$d/$(APP_NAME).desktop"; \
			chmod +x "$$d/$(APP_NAME).desktop"; \
		fi; \
	done
	@echo "Lanceur installé : ~/.local/share/applications/$(APP_NAME).desktop"

desktop: install-desktop

remove-desktop:
	@rm -f ~/.local/share/applications/$(APP_NAME).desktop
	@DESK_DIR=$$(command -v xdg-user-dir >/dev/null 2>&1 && xdg-user-dir DESKTOP 2>/dev/null || true); \
	for d in "$$DESK_DIR" "$$HOME/Desktop" "$$HOME/Bureau" "$$HOME/Escritorio" "$$HOME/Schreibtisch"; do \
		if [ -n "$$d" ] && [ -d "$$d" ]; then \
			rm -f "$$d/$(APP_NAME).desktop"; \
		fi; \
	done
	@echo "Lanceur supprimé."



# =========================================================
# RELEASE PACKAGING
# PACKAGING DE RELEASE
# =========================================================

package-source: clean
	@chmod +x ./scripts/package_source_clean.sh
	@./scripts/package_source_clean.sh

package-linux-release:
	@chmod +x ./scripts/package_linux_release.sh
	@./scripts/package_linux_release.sh

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

release-trigger:
	@chmod +x ./scripts/trigger_release.sh
	@./scripts/trigger_release.sh

release-check: clean
	@chmod +x ./scripts/validate_release_tree.sh
	@./scripts/validate_release_tree.sh

gui-preview: all
	@chmod +x ./tools/gui/run_gui_debug.sh
	@./tools/gui/run_gui_debug.sh

.PHONY: all test check run launch clean rebuild strip help install-desktop desktop remove-desktop package-source package-linux-release package-windows-release bump-patch bump-minor bump-major release-push release-trigger release-check gui-preview gui
