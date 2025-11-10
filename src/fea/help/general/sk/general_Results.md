# Results

**Results** are produced by the **Solver** and consist of **Result variables**.

## Results variable

**Results variable** is set of values computed by the **Solver**. **Results variable** is defined **type** (e.g., **Temperature** or **Displacement** and a vector of values. Based on **type** **Results variable** can contain vector of scalar values or vector values.


For example, **Temperature** results variable will contain vector of temperatures for each node whereas **Displacement** will contain vector of displacement vectors for each node.


**Results variable** can be computed either on node or on element level. For example, **Displacement** is computed on node level, whereas **Stress** is computed on element level. Each element level variable once computed can be converted to node level. Converting element level variable to node level is one time operation and can not be reverted. To do so results have to be recalculated by the **Solver**.

