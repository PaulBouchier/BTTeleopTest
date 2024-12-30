# BTTeleopTest
Robot code to interface with a RoboRemo teleop GUI

## Overview
RoboRemo is a cell-phone app that supports designing and using GUIs which connect
to a remote device like a robot and control it and show its status. RoboRemo supports
multiple comms channels, including Bluetooth, wifi, serial, etc for bidirectional
comms between the cell phone (which provides the user interface) and the device.

This module is intended as a "minimal setup", "always available" way to take control
of a robot. It runs on an ESP32 and listens on a Bluetooth serial connection, to which
the RoboRemo app on a previously-paired phone can connect and send commands to teleop the robot.

## Capabilities
The only setup required is to pair the phone with the robot bluetooth
serial connection offered by this module. This module is conceived of as
always listening for commands, and causing the robot to execute user teleop commands
in preference to any commands which may be issued by autonomy software. Thus it
can act as a deadman switch or a way to drive the robot somewhere, or a way to
manually inject perturbations or otherwise supercede autonomous operation.

The capabilities of this module are expressed in the RoboRemo interfaces, which
are found in the interface files in the RoboRemoInterfaces folder. The user
starts RoboRemo and selects either of the interfaces:

- Teleop.interface
- Autonomous Enable.interface

In the Teleop interface, the user connects to the robot and presses the Teleop
button, after which the phone's attitude causes RoboRemo to send speed and rotation
commands to the robot. Phone held in front of the user at 45 degrees sets requested
linear velocity to 0, tilting forward sends a forward linear velocity, and tilting
backward sends a backward linear velocity. Tilting left to right (roll) sends
rotation commands.

In the Autonomous Enable interface, if the "Auto Button Active" switch is on, the
robot ignores commands from autonomy software unless the "Autonomous Motion Enable" button
is pressed. This lets it act as a deadman, suppressing autonomy SW commands unless the user
is pressing the deadman button (Autonomous Motion Enable button).

## Robot Interface
The robot control SW should periodically get the Bluetooth state using the provided
method. It is returned four data items:

- TeleopActive: true when the user is pressing the "Teleop" button.
- vel: requested linear speed when "Teleop" button is pressed, zero otherwise
- rotSpeed: requested rotational speed when "Teleop" button is pressed, zero otherwise
- autonomousEnable: true when not actively requesting teleop, or when using the Autonomous
Enable interface with Autonomous Motion Enable button pressed

Robot control SW should ignore motion commands from autonomy SW if autonomousEnable is false
and leave the robot in a safe state (typically stopped). If teleopActive is true, it should
ignore motion commands from autonomy SW and instead act on requested speed and rotation 
from the bluetooth interface.

## Concurrency
This module listens for bluetooth commands whenever the ESP23 is running. This behavior is
enabled by it being its own task under the FreeRTOS OS that runs on ESP32s.

## Library and Testing
This module implements a class BTTeleop which spawns the bluetooth monitor task. The class
is provided as a library and is intended to be linked with the rest of the robot SW.
In addition, main() in the source directory instantiates the BTTeleop class and prints
its output for testing purposes.