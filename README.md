# DIY-MIDI-to-CV
This is a DIY and open source MIDI to CV interface. It lets you use midi keyboards, loopers or computers to control all kinds of CV based synthesizers.


# Known Issues!
### 1. DAC data lines are not connected
Solution: the connector in the middle of the board has the data lines so I ran wires from there to the arduino pins
### 2. pin numbers were mostly all wrong lmao
tldr: a button group and one dac use the same pin
Solution: remove input side of diode 3 and connect to pin 9 of arduino


# Code
very much a work in progress.

Libraries needed:
Arduino MIDI Library by Francois Best
https://github.com/FortySevenEffects/arduino_midi_library

MCP DAC by Rob Tillaart
https://github.com/RobTillaart/MCP_DAC

# PCB

PCB Files are in easyeda format at:

https://oshwlab.com/tucker1/digital_copy

I've also included Gerber files here

# Schematic

[Click to fullscreen](https://raw.githubusercontent.com/TuckerMacor/DIY-MIDI-to-CV/main/Schematic/DIY%20MIDI%20to%20CV%20Schematic.svg)

![](/Schematic/DIY%20MIDI%20to%20CV%20Schematic.svg)
