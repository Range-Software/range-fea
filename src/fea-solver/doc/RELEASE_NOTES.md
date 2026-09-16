## Version 1.3.0

### Improvements

- `--verify-jacobian` compares the matrix the fluid solver assembles against a
  finite difference of the residual it is solved against, once, and reports where
  the two disagree. It is a diagnostic for the formulation rather than something
  to run on a real model: each unknown is perturbed in turn and the whole system
  reassembled either side of it, so it belongs on a mesh of a few elements and
  nowhere else. The run is slow and says so
- The report gives, per block of the system, the ratio of the assembled entry to
  the measured one, grouped so that a block made of several terms can be told
  apart. A block whose ratio is one throughout is assembled correctly; a block
  whose ratio is some other number throughout carries that factor too many.
  `RSolverFluid` documents what the check has found so far
