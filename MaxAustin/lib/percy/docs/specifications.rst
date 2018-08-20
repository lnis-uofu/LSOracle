Specifications
==============

In `percy` we use specification objects to tell synthesizers both *what* and
*how* to synthesize. Specifications are implemented by the
:cpp:class:`synth_spec` class, which can be found under ``<percy/spec.hpp>``.
We can specify the functions to synthesize, but we also control some optional
parameters such as conflict limits, and various symmetry breaking flags.
Symmetry breaking is used to rule out certain "symmetric" solutions. Often
we're not interested a solution if it is symmetric to one we have already
found. [#]_ Ruling out such solutions makes the search space smaller and thus
speeds up synthesis.  Note that the symmetry breaking flags should be
interpreted as *hints* and not as law: they may not be implemented in all
synthesizers. In fact, depending on the implementation, not all flags may apply
to some synthesizers' synthesis process.

Note that :cpp:class:`synth_spec` takes a template parameter. This parameter
specifies the type of functions to synthesize (i.e. static or dynamic `kitty`
truth tables).

.. doxygenclass:: percy::synth_spec
    :members:
    :private-members:
    :undoc-members:

.. [#] An example would be a solution in which we simply swap two inputs.
