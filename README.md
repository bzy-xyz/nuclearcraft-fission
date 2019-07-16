# nuclearcraft-fission

A program to search for viable NuclearCraft fission reactors.

## Requirements

* A c++17 compatible C compiler with OpenMP support.

## Limitations

* The search method is currently pretty bad. I'm working on that.

## Usage

`search [x y z] [fuelType] [coolerRestrictions] [strategy]`

`x y z` dimensions of reactor (default 5x5x5).

`fuelType` (2 - ???) is the fuel to optimize for.

`coolerRestrictions` (0 - 2):

* 0: any
* 1: types expected to be available early

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
* Imposes symmetry in early iterations. Relaxes symmetry restrictions over time.
* Does its best not to revisit reactor patterns seen in the past (but doesn't
  know about rotations or reflections).
* Runs for 20k steps.

## Output

Upon finishing or aborting early, produces a report:

* `N` number of reactor cells.
* `Cl` number of clusters.
* `O` raw objective function value.
* `i` number of inactive cells.
* `[fuel] totalOutput effectiveOutput effectiveOutputPerCell`

followed by the reactor structure and per-cluster stats.
