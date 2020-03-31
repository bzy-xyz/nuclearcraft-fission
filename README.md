# nuclearcraft-fission

A program to search for viable NuclearCraft fission reactors.

## Requirements

* A c++2a compatible C compiler with OpenMP support.

## Limitations

* Uses the stock cooling configuration. Your modpack may be different! Change
  values in `Reactor.cpp` to match your modpack.
  (The Enigmatica 2 Expert values are included; you can switch at compile-time
  by changing `Reactor.cpp:18` to `#define RULESET_E2E`.)
* Supports only one type of active cooling at a time; will yield incorrect
  results if you attempt to mix them.

## Usage

`search [x y z] [fuelType] [coolerRestrictions] [strategy] [load_file]`

`x y z` dimensions of reactor (default 5x5x5).

`fuelType` (2 - 53) is the index of the fuel type to optimise for (default
LEU235O). LEU235O is 9, HECf251O is 53. Guideposts in `Reactor.h`.

`coolerRestrictions` (0 - ?):

* does absolutely nothing right now, just set this to 0

`strategy` (0 - 2):

* 0: efficiency (effective output per cell) (default)
* 1: effective output
* 2: effective reactor cell count (heat adjusted)

`load_file` (path to a Hellrage-compatible JSON file):

* will ignore the passed in reactor dimensions and load the target reactor as 
  an initial state

Will produce a Hellrage-compatible JSON as output to `out.json` upon finishing
or Ctrl-C.

## Strategy

* Uses a pseudo-simulated-annealing strategy.
* At every step, does the following:
  * Generates the set of "sensible" reactor differences. That is, for each cell,
    figures out which coolers could be active there and whether a moderator
    makes sense.
  * Draws with replacement 1 - 2 of these, applies them to the current reactor,
    and then scores the results. Does this 100 times.
  * Also generates 50 reactors with new reactor / moderator / air cells at 
    1 - 4 random places and scores those as well.
  * Before iteration 500, imposes XYZ symmetry on the reactor to "kickstart" the
    search process.
  * Picks a random reactor among the accumulated set, weighted by score. If it's
    the best one seen so far, set it aside. Nevertheless, keep evolving from the
    new reactor.
* With a 1/250 chance per step, resets to the best reactor.
* Does the above N/2 times in parallel (where N is the number of logical cores
  you have).
* Runs for 20k steps.

## Output

Upon finishing or aborting early, produces a report:

* `N` number of reactor cells.
* `P` efficiency multiplier.
* `H` heat multiplier.
* `C` total cooling.
* `[fuel] totalOutput heatBalance effectiveOutput effectiveOutputPerCell`

followed by the reactor structure.
