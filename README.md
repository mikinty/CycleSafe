# CycleSafe

A safety-system integrated onto you and your bike to keep you safe on the road.

Consists of a smart jacket that is bluetooth integrated with a bike mount.

# Bike

![Bike Main Photo](./paper/images/bike_mount.png)

Bike view from car
![Bike On the Road](./paper/images/side_by.png)

Bike visibility
![Bike On the Road Night](./paper/images/visibility.png)

## Jacket

Chip inside the jacket
![Jacket chip](./paper/images/jacket_chip.png)

Turn Signals
![Jacket Turn Signal](./paper/images/turn_signals.png)

Buzzer in the jacket to notify the rider
![Jacket Buzzer](./paper/images/buzzer_motor_prox.png)

## Directory Structure

- `paper`: source code for the paper
- `src`: contains all the code for the project, including

  - `arduino`: modules for the arduino, for sensor and peripheral integration
  - `android`: phone app, uses Google Map data and customizes jacket display
  - `raspberry`: code for the Raspberry Pi controller, safety sensor integration and control center for all operations