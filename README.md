# Overview
For a general overview as well as automatically updated documentation, please refer to the [Ultramote website](https://etrommer.github.io/ultramote/)


## Firmware debugging

The firmware debug configuration uses a Black Magic Probe directly as a GDB remote target. It does not require an OpenOCD or pyOCD server. Install `arm-none-eabi-gdb`, build the firmware, and start a direct session with:

```bash
cd firmware
make
arm-none-eabi-gdb build/firmware.elf -x blackmagic.gdb
```

The two Cortex-Debug profiles provide either a flash-and-launch session or an attach-only session.

## Firmware development container

The repository includes a VS Code Dev Container with the pinned STM32CubeMX 6.16.0
Linux package, the STM32CubeWL 1.3.0 firmware package, the GNU Arm Embedded toolchain,
GDB, OpenOCD, and ST-Link tools. Open the repository in VS Code and run **Dev
Containers: Reopen in Container**. The container opens `firmware` as the VS Code
workspace; the repository root is available at `..`.

Build the firmware inside the container:

```bash
make
```

Regenerate CubeMX-managed files from the IOC file without opening a GUI:

```bash
cubemx-generate firmware.ioc
make
```

The image verifies both downloaded archives with SHA-256 checksums. The ST-Link or
Black Magic Probe device must be passed through to the container separately when
debugging; the image does not assume a host device path.

The `pyocd` server type is an alternative only when the connected probe exposes a CMSIS-DAP interface. It is not the transport used by a Black Magic Probe's native GDB interface.

# Firmware
## Remove Lora-E5 readout protection
The Seeedstudio Lora-E5 comes with a preloaded firmware and readout protection enabled. This needs to be disabled before other firmware can be flashed. Run:
```bash
openocd -f interface/stlink.cfg -f target/stm32wlx.cfg
```
then connect to the OpenOCD session from another terminal, using: `nc localhost 4444`. In the telnet session, run:
```
init
reset halt
stm32wlx unlock 0
reset halt
exit
```

# Hardware
## Documentation
Schematics, BoM, Gerber files etc. are generated with the [KiCad command line
interface](https://docs.kicad.org/master/en/cli/cli.html). Change to the `pcb`
directory and run:

```bash
cd pcb
make
```

This generates all documentation in the local subfolder `generated`. `make
docs`, `make fab`, `make step`, `make render` and `make ibom` build single
groups, and `make OUT=<dir>` writes the assets somewhere else. `make` needs
KiCad 10 (`kicad-cli`) and `bsdtar`.

CI runs the same build and publishes the result together with the website
(`.github/workflows/website.yml`). The assets are build output: they are not
committed to the repository, so a local site preview needs

```bash
make -C pcb OUT=website/static/pcb_assets
```

Building the website itself needs Zola 0.22 (`zola --root website build`); 0.23
and later cannot build it, as they replaced the template engine with one that no
longer supports the macros used by the site templates and the `juice` theme.

The interactive BoM needs the
[InteractiveHtmlBom](https://github.com/openscopeproject/InteractiveHtmlBom)
CLI, which must see KiCad's Python bindings. `make` uses one from `PATH` if there
is one, otherwise the copy that ships with the plugin installed through KiCad's
Plugin and Content Manager, so a plugin installation needs no extra setup. Point
`IBOM=` at a CLI from elsewhere (CI installs one from PyPI):

```bash
python3 -m venv --system-site-packages ~/.local/venv/ibom
~/.local/venv/ibom/bin/pip install interactivehtmlbom
make IBOM=~/.local/venv/ibom/bin/generate_interactive_bom
```

If you think that an important piece of documentation is missing, please open an issue.
