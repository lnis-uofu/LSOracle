Encoders
========

knuth_encoder
-------------

The :cpp:class:`knuth_encoder` class is based on the exact synthesis
formulation developed by That implementation is itself based on earlier
work by Éen [#]_ and Knuth [2]_.

.. doxygenclass:: percy::knuth_encoder
    :members:

fence_encoder
-------------

The :cpp:class:`fence_encoder` is intended to be used by a synthesizer
that generates families of DAG topologies, also known as *fences*.

.. doxygenclass:: percy::fence_encoder
    :members:

dag_encoder
-----------

The :cpp:class:`fence_encoder` is intended to be used by a synthesizer
that generates DAGs. Encoding DAGs requires much fewer variables, since
their structure is completely fixed.

.. doxygenclass:: percy::dag_encoder
    :members:


.. [#] Niklas Éen, "Practical SAT – a tutorial on applied satisfiability solving," 2007, slides of invited talk at FMCAD.

.. [#] Donald Ervin Knuth, "The Art of Computer Programming, Volume 4,
    Fascicle 6: Satisfiability," 2015
