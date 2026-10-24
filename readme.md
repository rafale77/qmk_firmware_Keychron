# Keychron QMK Firmware

This fork from the Keychron repo is not meant to contribute to the parent but I am happy to share my own personalizations.

A few differences on my fork:
1. Use of the latest Chibios updates
2. Only supports the two QMK keyboards I own: The Lemokey P3 HE, GMMK Numpad and the Aaru TKL which has gone stale for many years. (Yes it isn't a Keychron Keyboard)
3. All: Added ability to add a solid background color to the "wide" and "splash animations which can also be controlled in Via.
4. Numpad: Added control of both the knob and slider independently and sidelights control to match the GMMK software. Technically there is nothing the GMMK firmware does that cannot be done with my version except for BT support.

## Contributing

Contributions are welcome — whether it's a new keymap, a bug fix, or documentation improvement. See [`docs/contributing.md`](docs/contributing.md) for guidelines on submitting pull requests.

## License

This project is licensed under the [GNU General Public License v2.0](LICENSE).

This repository tracks the [upstream QMK firmware](https://github.com/qmk/qmk_firmware) with Keychron-specific board definitions and firmware additions.
