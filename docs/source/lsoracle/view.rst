Partition View
==============

Partition view works in a similar way to the `window_view <https://mockturtle.readthedocs.io/en/latest/views.html#window-view-network-view-on-a-window>`_. It receives a Boolean network, and two sets of nodes: one bounding the partition inputs and the other defining the partitions outputs. For each output, the method recursively adds nodes in the partition scopes until it reaches the partitions inputs. Note that partitions are non-overlapping, and one node cannot belong to more than one partition. It is ensured by the *partition_manager* class. Conversely to the *window_view*, the *partition_view* is mutable and may be passed straight to any optimization method. 
