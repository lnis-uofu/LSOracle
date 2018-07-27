Introduction
============

The `percy` library provides a collection of SAT based exact synthesis engines.
That includes engines based on conventional methods, as well as
state-of-the-art engines which can take advantage of DAG topology information
The aim of `percy` is to provide a flexible common interface that makes it easy
to construct a parameterizable synthesis engine suitable for different domains.
It is a header-only library, meaning that it can be used simply by including
the percy/include folder in your project.  Internally, `percy` uses the `kitty`
library [#]_ to represent the truth tables of the functions to be synthesized.

Synthesis using `percy` concerns five main components:

1. *Specifications* -- Specification objects contain the information essential
   to the synthesis process such as which functions to synthesize, I/O
   information, and possibly optional parameters such as conflict limits for
   time-bound synthesis, or topology information.  
2. *Encoders* -- Encoders are objects which convert specifications to CNF
   formulae. There are various ways to create such encodings, and by
   separating their implementations it becomes simple to experiment with
   different encodings in various settings.
3. *Solvers* -- Once an encoding has been created, we use a SAT solver to find
   a solution. Currently supported are ABC's `bsat` solver [#]_, the
   Glucose and Glucose-Syrup solvers, [#]_ and the CryptoMinisat solver. [#]_
   Adding a new solver to `percy` is as simple as declaring a handful of
   interface functions. [#]_
4. *Synthesizers* -- Synthesizers perform the task of composing encoders and
   solvers. Different synthesizers correspond to different synthesis flows. For
   example, some synthesizers may support synthesis flows that use topological
   constraints, or allow for parallel synthesis flows. To perform synthesis
   using `percy`, one creates a synthesizer object. Synthesizers are
   parameterizable: we can change their encoder and solver backends. This
   happens at compile time, so there is no runtime overhead.
5. *Chains* -- Boolean chains are the result of exact synthesis.  A Boolean
   chain is a compact multi-level logic representation that can be used to
   represent multi-output Boolean functions. 

A typical workflow will have some source for generating specifications, which
are then given to a synthesizer that converts the specifications into optimum
Boolean chains. Internally, the synthesizer will compose its underlying encoder
and SAT solver in its specific synthesis flow. For example, a resynthesis
algorithm might generate cuts in a logic network which serve as specifications.
They are then fed to a synthesizer, and if the resulting optimum Boolean chains
leads to an improvement, are replaced in the logic network. In optimizing this
workflow, `percy` makes it easy to swap out one synthesis flow for another, to
change CNF encodings, or to switch to a different SAT solver.

.. [#] https://github.com/msoeken/kitty 
.. [#] https://github.com/berkeley-abc/abc 
.. [#] http://www.labri.fr/perso/lsimon/glucose/ 
.. [#] https://github.com/msoos/cryptominisat 
.. [#] Unfortunately some solvers may not compile on your favorite OS...
