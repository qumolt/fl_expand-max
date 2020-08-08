
# About

External for Max/MSP written in C, Windows version. _flexpand~_ uses a nomalized signal of phasor [0,1] to play a buffer as if it were a musical bar with n beats duration. The audio file is divided by the amount of beats desired, and will play for n beats the delta range (in beats) from the starting beat position (starting from 0.0). For example, choosing division by 4, start beat in 1, delta 2, for 3 beats of duration, this means the audio file will be divided in 4 parts: 0123, and if feeded with a _phasor~_ objet, will play 121 121 121...
On the second outlet, the external will produce a ramp signal for each beat played for general syncing use.
It is also possible to set the range in samples used from the audio file with a message called "samplim", or restart its values to the first and last sample sending a bang in the first inlet.


### Inlets and Outlets

(from left to right)

**Inlets**

- (sig~) nomalized phasor [0,1]
- (float) n divisions (beats)
- (float) start (beats)
- (float) delta (beats)
- (float) total time to play (beats)

**Outlets**

- (sig~) output
- (bang) final flag

------------------------------------------------------

# Versions History

**v0.1**
- It is possible to change limits (in samples) of source