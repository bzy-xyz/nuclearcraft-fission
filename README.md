# nuclearcraft-fission

A program to search for viable NuclearCraft fission reactors.

## Requirements

* A c++11 compatible C compiler with OpenMP support.

## Limitations

* Uses the stock cooling configuration. Your modpack may be different! Change
  values in `Reactor.cpp` to match your modpack.
* Supports only one type of active cooling at a time; will yield incorrect
  results if you attempt to mix them.

## Usage

`search [x y z] [fuelType] [coolerRestrictions] [strategy]`

`x y z` dimensions of reactor (default 5x5x5).

`fuelType` (2 - 53) is the index of the fuel type to optimise for (default
LEU235O).

`coolerRestrictions` (0 - 2):

* 0: any passive or cryotheum active
* 1: passive only (default)
* 2: passive enderium or active cryotheum only

`strategy` (0 - 2):

* 0: efficiency (effective output per cell) (default)
* 1: effective output
* 2: effective reactor cell count (heat adjusted)

## Strategy

* Uses a pseudo-simulated-annealing strategy.
* At every step, generates up to 400 "nearby" reactors by changing between 1
  and 5 cells at a time, and chooses probabilistically weighted by a power of
  the objective function value (power dependent on step count).
    * This step is parallelized with OpenMP support.
* Before iteration 500, imposes XYZ symmetry on the reactor to "kickstart" the
  search process.
* Does its best not to revisit reactor patterns seen in the past (but doesn't
  know about rotations or reflections).
* Runs for 20k steps.

## Output

Upon finishing or aborting early, produces a report:

* `N` number of reactor cells.
* `P` efficiency multiplier.
* `H` heat multiplier.
* `C` total cooling.
* `[fuel] totalOutput heatBalance effectiveOutput effectiveOutputPerCell`

followed by the reactor structure.
