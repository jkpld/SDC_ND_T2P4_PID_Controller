# PID Controller Project
Self-Driving Car Engineer Nanodegree Program

**The goal of this project is to use a PID controller to drive a car around a track using the distance to the center of the lane as input.**


## P, I, and D component effects

* The proportional (P) component provides the restoring force that brings the car back to the center of the lane. Since the P component is directly proportional to the displacement from the lane center, the car ends up oscillating around the lane center. The strength of the oscillation is determined by the proportional coefficient &tau;<sub>P</sub>.
* The derivative (D) component provides a damping forces that reduces (or removes) the oscillations introduced by the proportional term. The strength of the damping is determined by the derivative coefficient &tau;<sub>D</sub>.
* The integral (I) component allows for the car to follow the lane center more quickly/smoothly when the lane center moves, which is the case during a turn. This term additionally results in some overshoot after a turn, which must be compensated for by the proportional term. The strength of this term is set with &tau;<sub>I</sub>.

## Parameter tuning
The parameters were first manually tuned until the car could drive around the track; these values were &tau;<sub>P</sub> = 0.07, &tau;<sub>I</sub> = 0.007, &tau;<sub>D</sub> = 1. The twiddle algorithm was then used to optimize the parameters over several hours; each twiddle run was 1500 steps (or about 2 laps at the set speed of 50). The final PID parameters were 

* &tau;<sub>P</sub> = 0.0585
* &tau;<sub>I</sub> = 0.008
* &tau;<sub>D</sub> = 1.1

With these values the car can drive around the track with a speed of 50 without leaving the track, but with tires going over the lanes sometimes. The simulator version is the most recent available (v1.45).
