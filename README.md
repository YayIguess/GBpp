# GameBoy++
## Limitations and Features

Currently supports 32 KiB roms and MBC1 roms.
Currently passes [dmg-acid2](https://github.com/mattcurrie/dmg-acid2?tab=readme-ov-file), [blargg's cpu_instrs test](https://github.com/retrio/gb-test-roms/tree/master/cpu_instrs), and the [jsmoo SM83 JSON tests](https://github.com/raddad772/jsmoo-json-tests/tree/main/tests/sm83)

Tested running Super Mario Land, Dr Mario and Tetris.
Does not currently support MBC3 (needed for games like Pokemon) or loading saves. Some MBC1 games have issues still.

## Building
`mkdir build && cd build`

`cmake ..`

`./GameBoy++ <bios> <rom>`

## Controls
WASD is mapped to the d-pad
K and L are mapped to A and B
O and P are mapped to select and start
