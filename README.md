# Eosos
Tower Defense Roguelike game (what) made with SDL2 for 7DRL 2018.

---

### Linux instructions (Debian/Ubuntu/Linux Mint)

#### Install dependencies:
- `sudo apt-get install g++ libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev`

#### Install Lua (http://www.lua.org/download.html) however you wish. This should be enough:
- `sudo apt-get install libreadline-dev`
- `make linux`
- `sudo make install`

#### Build the game:
- `make all`

The executable will be found under `"./build/"`. If anyone builds this on a non-Debian based distro, I would love to get the list of dependencies added here, feel free to make a pull request adding to this readme.
