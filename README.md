# MHS-WCSP

This is the source code for the "Theoretical and Empirical Analysis of Cost-Function Merging for Implicit Hitting Set WCSP Solving", AAAI'24. 

It is only intended to be a proof of concept. Therefore, there is full room for improvement either on the structure and on the efficiency of the implementation.

## How to compile:

Our implementation uses CaDiCal as SAT solver and CPLEX as IP solver. These two solvers have to be installed before compiling our code. Edit the Makefile and adjust paths referring to them.

## How to execute:

See `mhs_wcsp -h` for a full description on options.

To reproduce the results in our AAAI'24 paper, use the following command lines:

### Experiments on case study problems:

1. original encoding:
`./mhs_wcsp -g num_vars type -t 4`

2. symbolic encoding:
`./mhs_wcsp -g num_vars type -ac -t 4`

3. numerical encoding:
`./mhs_wcsp -g num_vars type -p all -t 4`

### Experiments on wcsp instances:

1. original encoding: `./mhs_wcsp -f instance.wcsp` (Note: you may obtain slightly different results from the ones published due to some code polishing)

3. symbolic encoding on clusters from tree-decomposition:
`./mhs_wcsp -f instance.wcsp -p instance.wcsp.td.l2r -ac`

4. numeric encoding on clusters from tree-decomposition:
`./mhs_wcsp -f instance.wcsp -p instance.wcsp.td.l2r`
