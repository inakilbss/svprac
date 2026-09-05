# SVprac - Takes the vanity out of Stellavanity

In-memory patch that streamlines SV for practice without overwriting legitimate progress.
Features include:
- Functional unlocks bypassed
  - System shop
  - Practice options
- Grinding factors removed
  - Crystal (stat stone) count ignored
- Practice limitations removed
  - Hidden bosses in full area 7
  - Extra hazards in locked rank EX
- Quality of life
  - Game runs in background
  - Stat level limit assumes level 50 and Slot x1 for a starting limit of 650

### Building from source

Required to build:
- MinGW G++ >= 14
- CMake >= 4.3 (maybe it works on lower if you change the req line)
- Something in between (IDE set it up for me so I can't help you)

You can let builds install automatically in the game's folder, this requires:
- POSIX commands
- Game install symlinked as `install-target`, run `ln -s "/home/lemmy/Games/STELLAVANITY DL版 ver1.23" install-target`

Alternatively, you can remove the last section of `CMakeLists.txt` and install them yourself

To run your builds, even if you don't have an IDE you can probably `&& run_from_target.sh`