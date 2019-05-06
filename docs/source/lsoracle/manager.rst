Partition Manager
=================

The partition manager is the class responsible for creating and managing partitions over a Boolean network. The manager is in charge of partitions classification and assignment to different optimizers. Finally, the manager synchronizes optimized partitions over the original Boolean network.

For partition generation, the manager receives as input the circuit Boolean network within the number of desired partitions by the user, and interfaces with the `KaHyPar tool <https://github.com/SebastianSchlag/kahypar>`_ for k-way partitioning. KaHyPar returns a vector *A*, where the index *i* corresponds to the node index, and the value *A[i]*  refers to which partition the *i th* node belongs. From this vector, the partition manager keeps track of partitions inputs, outputs, and nodes, as well as connections in between partitions. These kinds of information are used by the *partition_view* method for optimization purposes.  

As for the classification, the partition manager is responsible for generating the KM-Images for every logic cone. The generated images are then classified and assigned to different optimizers by `Frugally Deep <https://github.com/Dobiasd/frugally-deep>`_. Since the same partitions might have different cones assigned to different optimizers, the manager applies a heuristic to weight each cone. The optimizer with the highest sum of weights is used for the partition optimization. 

After optimization has taken place over a set of nodes (partition), the manager synchronizes the optimized partition with the original network. Therefore, proper connections are established with adjacent partitions, keeping the functional equivalence.

