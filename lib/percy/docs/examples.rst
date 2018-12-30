Examples
============

In the following example, we show how `percy` can be used to synthesize an
optimum full adder. While simple, the example shows some common interactions
between the library's components.

.. literalinclude:: ../test/synthesizer.cpp
   :language: c++
   :lines: 53-87

In this example, we see how a synthesizer is instantiated based on a
specification. The synthesizer is of the `std_synthesizer` type, which is the
conventional synthesis engine. By default all engines use ABC's `bsat` solver
backend [1]_.  Suppose that this particular combination is not suitable for our
workflow. We can then easily switch to a new synthesizer and solving backend by
changing one line of code:

.. code-block:: c++

    auto synth = new_std_synth<3, Glucose::MultiSolvers*>();

In doing so we switch to a synthesis engine which synthesizes 3-input Boolean
chains, with the Knuth CNF encoder, and the parallel Glucose-Syrup SAT
solver backend. While we now use a completely different synthesis engine, its
interface remains the same.

.. [1] https://github.com/berkeley-abc/abc 
