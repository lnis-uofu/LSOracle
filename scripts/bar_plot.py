import matplotlib.pyplot as plt; plt.rcdefaults()
import numpy as np
import matplotlib.pyplot as plt
 
plt.ylim(0.9, 1.1)
objects = ('ABC\nresyn2', 'Original\nAIG Recipe', 'Last\nAIG Recipe', 'New\nAIG Recipe', 'DSD\nRecipe')
y_pos = np.arange(len(objects))
# performance = [1,0.675134395,0.982868221,0.849285918]
performance = [1,1.091930835,1.047811748,1.044621126,1.039145244]
# plt.title('Area Comparison')
plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
plt.xticks(y_pos, objects)
plt.title('AIG Optimization Recipe Product Comparison')
 
plt.savefig('aigscript3_product.png')
plt.show()

plt.ylim(0.9, 1.2)
objects = ('ABC\nresyn2', 'Original\nAIG Recipe', 'Last\nAIG Recipe', 'New\nAIG Recipe', 'DSD\nRecipe')
y_pos = np.arange(len(objects))
# performance = [1,0.675134395,0.982868221,0.849285918]
performance = [1,1.170810811,1.166486486,1.043243243,1.060540541]
# plt.title('Area Comparison')
plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
plt.xticks(y_pos, objects)
plt.title('AIG Optimization Recipe Level Comparison')
 
plt.savefig('aigscript3_level.png')
plt.show()

plt.ylim(0.9, 1.05)
objects = ('ABC\nresyn2', 'Original\nAIG Recipe', 'Last\nAIG Recipe', 'New\nAIG Recipe', 'DSD\nRecipe')
y_pos = np.arange(len(objects))
# performance = [1,0.675134395,0.982868221,0.849285918]
performance = [1,1.029186305,1.019146532,1.034942092,1.033676169]
# plt.title('Area Comparison')
plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
plt.xticks(y_pos, objects)
plt.title('AIG Optimization Recipe Node# Comparison')
 
plt.savefig('aigscript3_node.png')
plt.show()

# plt.ylim(0.85, 1.03)
# objects = ('ABC\nresyn2', 'Original\nMIG Recipe', 'Updated\nMIG Recipe')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,1.01638415,0.986986616]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('MIG Optimization Recipe Node # Comparison')
 
# plt.savefig('migscript_node_no.png')
# plt.show()

# plt.ylim(0.4, 1.05)
# objects = ('ABC\nresyn2', 'Original\nMIG Recipe', 'Updated\nMIG Recipe')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.500521376,0.607924922]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('MIG Optimization Recipe Level Comparison')
 
# plt.savefig('migscript_level.png')
# plt.show()

# plt.ylim(0.4, 1.05)
# objects = ('ABC\nresyn2', 'Original\nMIG Recipe', 'Updated\nMIG Recipe')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.509213618,0.562141035]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('MIG Optimization Recipe Product Comparison')
 
# plt.savefig('migscript_product.png')
# plt.show()

# plt.ylim(0.9, 1.1)
# objects = ('ABC\nresyn2', 'Original\nAIG Recipe', 'Last Month\'s\nAIG Recipe', 'Updated\nAIG Recipe')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,1.091930835,1.047811748,1.044621126]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('AIG Optimization Recipe Product Comparison')
 
# plt.savefig('aigscript_product.png')
# plt.show()

# plt.ylim(0.9, 1.05)
# objects = ('ABC\nresyn2', 'Original\nAIG Recipe', 'Last Month\'s\nAIG Recipe', 'Updated\nAIG Recipe')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,1.029186305,1.019146532,1.034942092]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('AIG Optimization Recipe Node # Comparison')
 
# plt.savefig('aigscript_node_no.png')
# plt.show()

# plt.ylim(0.9, 1.2)
# objects = ('ABC\nresyn2', 'Original\nAIG Recipe', 'Last Month\'s\nAIG Recipe', 'Updated\nAIG Recipe')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,1.170810811,1.166486486,1.043243243]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color='r',edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('AIG Optimization Recipe Level Comparison')
 
# plt.savefig('aigscript_level.png')
# plt.show()

# plt.ylim(0.5, 1.3)
# objects = ('Orig', 'ABC', 'LSO', 'MIG\nPrep', 'AIG\nPrep', 'LSO\nPrep', 'LSO\nAIG', 'LSO\nMIG', 'Merg', 'Merg\nPrep')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.726050509,0.747929877,1.24534881,0.881295488,0.834347921,0.582841855,1.004219099,0.955398823,0.842801132]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['r', 'b', 'g', 'y', (0.1, 0.2, 0.5), 'c', (0.5, 0.5, 0.5), 'k', 'w', 'm'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('LSO BLIF Reader PDP')
 
# plt.savefig('lso_blif_yosys_pdp_results.png')
# plt.show()

# plt.ylim(0.3, 1.7)
# objects = ('Orig', 'ABC', 'LSO', 'MIG\nPrep', 'AIG\nPrep', 'LSO\nPrep', 'LSO\nAIG', 'LSO\nMIG', 'Merg', 'Merg\nPrep')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.542342399,0.608212208,1.649422564,0.815052262,0.804910312,0.353575089,1.07251049,0.902548387,0.742389221]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['r', 'b', 'g', 'y', (0.1, 0.2, 0.5), 'c', (0.5, 0.5, 0.5), 'k', 'w', 'm'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('LSO BLIF Reader EDP')
 
# plt.savefig('lso_blif_yosys_edp_results.png')
# plt.show()

# plt.ylim(0.45, 1.55)
# objects = ('Orig', 'ABC', 'LSO', 'MIG\nPrep', 'AIG\nPrep', 'LSO\nPrep', 'LSO\nAIG', 'LSO\nMIG', 'Merg', 'Merg\nPrep')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.767778889,0.790334434,0.770319658,0.848343368,0.849869492,0.583329969,1.483973024,1.346928927,0.89408312]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['r', 'b', 'g', 'y', (0.1, 0.2, 0.5), 'c', (0.5, 0.5, 0.5), 'k', 'w', 'm'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('ADP')
 
# plt.savefig('new_yosys_adp_results.png')
# plt.show()

# plt.ylim(0.45, 1.44)
# objects = ('Orig', 'ABC', 'LSO', 'MIG\nPrep', 'AIG\nPrep', 'LSO\nPrep', 'LSO\nAIG', 'LSO\nMIG', 'Merg', 'Merg\nPrep')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.754124194,0.761272586,0.769703898,0.842197839,0.7874296,0.593514585,1.421598052,1.295306322,0.882611153]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['r', 'b', 'g', 'y', (0.1, 0.2, 0.5), 'c', (0.5, 0.5, 0.5), 'k', 'w', 'm'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('PDP')
 
# plt.savefig('new_yosys_pdp_results.png')
# plt.show()

# plt.ylim(0.2, 2.3)
# objects = ('Orig', 'ABC', 'LSO', 'MIG\nPrep', 'AIG\nPrep', 'LSO\nPrep', 'LSO\nAIG', 'LSO\nMIG', 'Merg', 'Merg\nPrep')
# y_pos = np.arange(len(objects))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.54557812,0.568961873,0.553858001,0.694727602,0.672105273,0.335969153,2.211626707,1.813727696,0.759888981]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['r', 'b', 'g', 'y', (0.1, 0.2, 0.5), 'c', (0.5, 0.5, 0.5), 'k', 'w', 'm'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects)
# plt.title('EDP')
 
# plt.savefig('new_yosys_edp_results.png')
# plt.show()

# plt.ylim(0.9, 1.13)
# objects = ('ABC\nBLIF Parse', 'LSO\nBLIF Parser')
# objects_2 = ('','','')
# y_pos = np.arange(len(objects_2))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1, 1.114455661, 1.061457011]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['r'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects_2)
# # plt.xticks(y_pos, objects)
# # plt.title("YOSYS Cell Number Comparison")
 
# # plt.savefig('aig_recipe_comparison.png')
# plt.show()

# objects = ('ABC\nBLIF Parse', 'LSO\nBLIF Parser')
# objects_2 = ('','','')
# y_pos = np.arange(len(objects_2))
# # performance = [1,0.675134395,0.982868221,0.849285918]
# performance = [1,0.597813011,0.581828425]
# # plt.title('Area Comparison')
# plt.bar(y_pos, performance, color=['b'], edgecolor='black', align='center', alpha=0.5)
# plt.xticks(y_pos, objects_2)
# # plt.xticks(y_pos, objects)
# # plt.title("YOSYS Cell Number Comparison")
 
# plt.savefig('updated_blif_parse_comparison.png')
plt.show()