Convulational Neural Network Classifier
=======================================

In order to determine the best fit algorithm to optimize a partition, a Convolutional Neural Network (CNN) is used. In order to model a logic network's functionality as a 2D image, image representations of the karnaugh-map is used (KM-Image). An example of a KM-Image of a 6-input logic cone is shown below. 

.. image:: /../../../docs/source/lsoracle/images/kmimage.png
		:width: 600

The CNN model was based primarily on a solution to the MNIST handwriting problem because the input for both applications are simple grayscale images and there are very few classes (10 in MNIST and 2 in this application). The topology is shown below.

.. image:: /../../../docs/source/lsoracle/images/cnn_top.png
		:width: 600

To create the dataset for training, combinational networks from the EPFL and ISCAS85 benchmark suites were partitioned into smaller sub-circuits and the logic cones from these sub-circuits were optimized using both AIG and MIG methods. The best optimization method was determined which reduced the area of the cone the most. This metric was used as it resulted in a relatively balanced dataset. After training with this dataset, the overall accuracy achieved with this model was 79% with a similar accuracy for each of the classes meaning that the model is not biased to one method over the other.
