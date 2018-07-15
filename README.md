# esp32-freqcount-example

## Introduction

This is an example application for the [esp32-freqcount] component.

It is written and tested for the [ESP-IDF](https://github.com/espressif/esp-idf) environment, using the xtensa-esp32-elf toolchain (gcc version 5.2.0).

Ensure that submodules are cloned:

    $ git clone --recursive https://github.com/DavidAntliff/esp32-freqcount-example.git

Build the application with:

    $ cd esp32-freqcount-example.git
    $ make menuconfig    # set your serial configuration and IO configuration
    $ make flash monitor

The program should monitor pulses on the configured input and display the measured frequency in the console.

## Dependencies

This application makes use of the following components (included as submodules):

 * components/[esp32-freqcount](https://github.com/DavidAntliff/esp32-freqcount)

## Hardware

TODO

## Features

This example provides:

 * ...

## Source Code

The source is available from [GitHub](https://www.github.com/DavidAntliff/esp32-freqcount-example).

## License

The code in this project is licensed under the MIT license - see LICENSE for details.

## Links

 * [Espressif IoT Development Framework for ESP32](https://github.com/espressif/esp-idf)

## Acknowledgements

Thank you to [Chris Morgan](https://github.com/chmorgan) for converting the original demo application into a reusable IDF component, which is used by this example.

