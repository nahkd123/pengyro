# PenGyro Firmware

## Supported boards

- [**Promicro nRF52840**](../hardware/promicro-nrf52840/README.md) (UF2 flashing
  method): `promicro_nrf52840/nrf52840/uf2`

## Building the firmware

### With `west`

`west` is highly recommended for building Zephyr firmwares. Make sure you have
installed the Zephyr SDK 1.0.1 and prepared Zephyr workspace before. Once that's
done, simply run the following build command:

```bash
west build -b <BOARD_ID>
```

Your firmware file will be located in `./build/zephyr`.

## Development environment

I personally use [IDE for Zephyr][zephyr-ide] for development, along with
symbolic link from this firmware directory to the Zephyr workspace.

[zephyr-ide]: https://open-vsx.org/extension/mylonics/zephyr-ide
