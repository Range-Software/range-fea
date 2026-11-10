# Problem

**Problem** is a general term for one or more physical phenomena applied on a geometry and is defined by **Problem type**, **Problem specific setup**, **Boundary conditions**, **Initial conditions**, **Environment conditions** and **Material settings**.

A problem is defined for each model separately. Its parts are set up in the tabs on the right side of the main window: **Problem**, **Boundary conditions**, **Initial conditions**, **Environment conditions** and **Material**.

## Selecting a problem

**Menu:** _Problem -> Select problem_

The **Select problem** dialog is used to choose what should be solved (see [Problem type](general_Problem_type.md)). It is also shown automatically when a model without any problem is loaded. Until a problem is selected, the **Problem** tab shows only a **Select problem** button.

## Problem specific setup

Once a problem is selected, the **Problem** tab contains the setup which is specific to the selected **problem types**.

![Problem tab](image-Problem_tab.png)

- **Time-solver** - check to solve the problem as **transient** (time-dependent). When checked, the approximation method, start time, time-step size, number of time-steps and output frequency can be set; the end time is calculated from them (_End time = Start time + Time-step size * Number of time-steps_). If unchecked, the problem is solved as **steady-state**. Time-solver is available only for problem types which support it.
- Additional sections depend on the selected **problem types** (e.g., **Transient stress analysis setup**, radiation, acoustic or modal analysis setup).

Other problem related actions can be found in the **Problem** menu: _Setup Problem(s) matrix solver_, _Define monitoring points_ and _Reset setup_.
