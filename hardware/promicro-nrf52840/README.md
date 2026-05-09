# PenGyro on Promicro nRF52840

Promicro nRF52840 in reality is just a family of cheap Chinese clone of
[nice!nano][nice-nano] board. Pretty much everything in this document also
applies to nice!nano (in case you have one). This is also the first family of
board used during the development of PenGyro.

## Where to find?

- [nRFmicro wiki][nrfmicro-wiki] contains some useful information about
  nice!nano and the clones.
- [nice!nano][nice-nano] - It's more on expensive side, but this is what most
  Chinese clones are based on.
- Promicro nRF52840 boards that can be found on random Chinese marketplaces.

## Wiring

_TODO: Insert wiring diagram here_

- **Battery**: B+ and B- connects to positive and negative terminals of battery,
  respectively
  - **Do not connect in reverse!** I'm sure you don't want a small fireball on
    your desk while charging.
  - Ideally, you'd want to attach JST connectors on battery and the board for
    easy battery replacement, just in case the battery worn out over time.
- **BMI160 IMU**:
  - Connect **VCC**/**3.3V** on the board to **3.3V** on the IMU
  - Connect **GND** on the board to **GND** on the IMU
  - Connect **GPIO 0.11** on the board to **SCL** on the IMU
  - Connect **GPIO 1.00** on the board to **SDA** on the IMU
  - Connect **SD0**/**SA0** to **GND** (which set the IMU address to `0x68`)

## Flashing firmware

1. Obtain the firmware file

   There are no prebuilt binaries available for download at current moment, due
   to how long it is to prepare Zephyr environment on GitHub Actions. You will
   have to build your own firmware file for now. See
   [this page](../../firmware/README.md) for instructions on how to build the
   firmware. Replace `<BOARD_ID>` with `promicro_nrf52840/nrf52840/uf2`. The
   firmware file will be placed in `./build/zephyr/zephyr.uf2`.

1. Connect the board to your computer/phone

1. Put the board to firmware updating mode

   - If there is a RESET button, simply press the RESET button twice in quick
     succession.

   - If there is no RESET button, but there are GND and RESET/RST contacts
     exposed, you can use a wire or something conductive to short them. Make
     sure to do it twice as fast as possible.

   Once the board entered the firmware updating mode, the red LED (blue LED in
   some cases) will start pulsing and a new USB mass storage device will show up
   in your computer/phone.

1. Copy the firmware file to the board and wait for it to reconnect

[nice-nano]: https://nicekeyboards.com/nice-nano
[nrfmicro-wiki]: https://github.com/joric/nrfmicro/wiki/Alternatives
