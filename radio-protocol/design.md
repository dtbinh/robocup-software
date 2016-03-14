
# Radio Protocol Design

This document has a brief description of the design and implementation of the radio protocol for the 2015 robots.

The 2011 robots used a Texas Instruments (TI) CC1201 RF Transceiver chip to handle radio communication.  This was a component on the main control board that talked to the microcontroller via SPI and connected to an external antenna board.

todo: link to packet info, old documents, etc.


The new radio system is based on the previous one and makes some improvements and changes.  Most notably:
* it's $X faster in baud rate, and so can in theory provide $X faster over-the-air communication
* We're using the cc1201 from TI, which is the updated version from the previous cc1101


# Hardware

The radio hardware is all built onto a single chip that contains an MBED, the cc1201 chip, a small antenna (TODO: frequency), and several passive components (resistors, capacitors).  It connects to the computer via USB, which the soccer program connects to in order to communicate with the robots.

![file](build/party-dot.png)


# Protocol Purpose

The radio protocol outlines the specifics of how the field computer will communicate with each of the robots on the field via the base station.  The radio "base station" is a device built out of an mbed and a cc1201 radio board.


## Packets

Purpose:
* send
