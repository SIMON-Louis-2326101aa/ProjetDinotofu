# V3.48.00 — Locked branding and checked shortcuts   

- Confirms official assets in `assets/branding/` from the validated pack: game logo, site logo, banner, intro/menu cover, graphical launcher icon and terminal launcher icon.   
- Regenerates `.ico` variants for Windows shortcuts and PNG 512 variants for Linux/site usage.   
- Adds `scripts/validate_branding_assets.sh` to check all branding files, paths, GUI references and installer shortcut references.   
- Updates the release manifest example to the current version.   
- The GUI uses the official site logo in the header and keeps the official banner as a darkened readable background.   
- Images remain a visual supplement and never replace written gameplay information.   

# V3.47.00 — Official visual identity and launcher icons   

- Adds `assets/branding/` with the Dinotofu logo, site logo, banner, intro/menu cover, graphical launcher icon and terminal launcher icon.   
- Adds `.ico` variants for Windows shortcuts and 512 PNG variants for Linux/site shortcuts.   
- The graphical interface now displays the official logo in its header and uses the banner as an active visual background.   
- Windows shortcuts created by the installer use the graphical icon for the normal launcher and the terminal icon for the terminal launcher.   
- Linux `.desktop` shortcuts created by the installer also use the dedicated icons.   
- Reminder: images are a visual supplement for GUI/branding and never replace written game information.   
