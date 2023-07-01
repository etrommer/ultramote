+++
title = "List of blog posts"
sort_by = "weight"
+++
Ultramote is a long-distance camera shutter release system. Ultramote uses LoRa long-distance radio communication to achieve distances of up to several kilometers - well above the range of commercially-available 2.4GHz shutter release systems. Ultramote lets you put your camera in places where you _really_ don't want to be and allows you to capture shots from a safe distance.

# Hardware
## Interface
Ultramote has two 2.5mm jacks for input and output. Each input/output has two independent channels (tip and sleeve). Output connectors are optically isolated from the Ultramote's electronics, inputs are equipped with high-voltage protection circuitry. Any regular remote shutter or time lapse controller can be connected and is relayed to all Ultramotes listening on the same channel. For camera output, an adapter cable from a 2.5mm jack to the camera's remote port is required.

Ultramote can be connected to a PC via USB-C and controlled via a Serial Terminal.

Firmware updates can also be carried out over USB-C.

## Battery
A 400mAh LiPo battery allows for an idle time of several hours. The battery is charged over USB-C.

## Enclosure
_currently work in progress_  
The Ultramote's enclosure is 3D-printed in two parts and comes with hotshoe and GoPro-style attachement points as well as a 1/4" thread.


## Documentation
![Rendering Top](kibot_generated/docs/img/ultramote-top.png)
![Rendering Bottom](kibot_generated/docs/img/ultramote-bottom.png)

Documentation is built using [KiBot](https://github.com/INTI-CMNB/KiBot) and automatically updated from the repositorie's main branch::wireless
- [Schematic (PDF)](kibot_generated/docs/ultramote-schematic.pdf)
- [Board Layot (PDF)](kibot_generated/docs/ultramote-pcb-layers.pdf)
- [BoM (CSV)](kibot_generated/docs/bom/bom_ultramote.csv)
- [Interactive BoM](kibot_generated/docs/bom/ultramote-ibom.html) (generated using the [KiCAD IBoM Plugin](https://github.com/openscopeproject/InteractiveHtmlBom))
- [CAD File (STEP)](kibot_generated/ultramote.step)

# Firmware
The firmware currently supports these features:
- Pass on trigger events to other devices listening on the same channel
- Report battery voltage
- Command-line interface over USB/Serial. Over the interface, users can:
    - Put the device in bootloader mode to flash firmware over USB. This allows users to easily update their device or develop their own firmware.
    - Reprogram the device to a different channel. Useful when triggering more than one camera.
    - Select LED modes, set the state of the output, inspect device state, etc.

There is a long list of potential features that _might_ be added at a later point in time.

# License
Ultramote Hardware and Software are licensed under the [GNU General Public License v3.0](https://github.com/etrommer/ultramote/tree/main/LICENSE)
