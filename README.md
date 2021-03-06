# Eosos
Tower Defense Roguelike game (what) made with SDL2 for 7DRL 2018 ([itch.io link](https://shakajiub.itch.io/eosos)!). Status: refactoring and cleaning up the code for a post-jam release with more content. For a stable game version, download the source from the `Releases` tab. The current master branch makefile is not guaranteed to work for anyone at this time.

---

### Linux instructions (Debian/Ubuntu/Linux Mint)

#### Install build dependencies:
- `sudo apt-get install g++ libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev`

#### Build the game:
- `make all`

#### (Optional) Install runtime dependencies:
- `sudo apt-get install freepats`

The game uses midi files for music, so you will most probably need `freepats` to play them. The game will run fine without it, just without music. The executable `eosos` will be found under `./build/`. If anyone builds this on a non-Debian based distro (or even on Windows), I would love to get the list of dependencies added here.
