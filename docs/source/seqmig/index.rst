Sequential MIG Support
======================

Majority-Inverter Graph (MIG) is a recently introduced Boolean network that uses the majority-of-three as its base logic primitive. Besides tha majority-of-three operand, nodes can also represent Primary Inputs (PIs), Primary Outputs (POs) and the edges may be inverted. Current results show that MIGs enable efficient logic manipulation, improving results in area, delay and power when compared to state-of-art academic tools and commercial flows.

While current results are encouraging, MIGs just have been explored in combinational circuits, limiting its usage. Thus, the `Mockturtle <https://mockturtle.readthedocs.io/en/latest/>`_ library was extended to support sequential MIGs, i.e., nodes representing registers are also supported. To do so, each register is decomposed in two nodes: the register input is represented by a pseudo PO, while the register output is seen as a pseudo PI.
By seeing registers as pseudo PIs and pseudo POs, depth and area-orientend optimization algorithms for combinational circuits are efficiently extended.

