# Select problem

Select what should be solved for the model.

Click the **problem types** to solve, then click **OK**. Double-clicking a tile selects it and closes the dialog. A tile shows **Required** when another selected problem needs it, and a greyed tile is **Incompatible** with the current selection. Its tool tip gives the reason.

The dialog opens automatically for a model without a problem. **Skip for now** closes it, and the problem can be selected later from this action or from the **Problem** tab.

When several problems are selected, **Solve order** shows the order they run in, so that each problem receives the results it needs. When a selection includes a flow or radiation, **Maximum iterations** and **Convergence** can be set. Iterations left at their default change with the **Time-solver**: a steady-state flow takes up to 2000, a time step up to 20.

## Customize

**Customize** shows the **task flow**, a tree of **problem tasks** which defines their order and how many times they are repeated. Use **Up** and **Down** to reorder tasks, **>** and **<** to move a task into or out of a repeating group, and double-click **# of iterations** or **Convergence** to edit it. Selecting or clearing a problem type adds or removes its task while keeping the customized tree.

**Use automatic order** discards the customized task flow.
