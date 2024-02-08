# MHS-WCSP



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

1. original encoding:
`./mhs_wcsp -f instance.wcsp`

2. symbolic encoding on clusters from tree-decomposition:
`./mhs_wcsp -f instance.wcsp -p instance.wcsp.td.l2r -ac`

3. numeric encoding on clusters from tree-decomposition:
`./mhs_wcsp -f instance.wcsp -p instance.wcsp.td.l2r`
