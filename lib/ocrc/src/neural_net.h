/*
    OCRC, a AI for optical character recognition written in C
    Copyright (C) 2023-2025 João E. R. Manica
    
    OCRC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    OCRC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NEURAL_NET_H
#define NEURAL_NET_H

/* 'neurons_per_layer': This vector defines the number of neurons at each layer. 
   (Feedforward sense: first to last)
 * 'num_layers': Indicates the total number of layers in the net
 * 'num_input': Represents the total number of inputs in the network
 * 'source': Specifies whether the net receives inputs from convolution
 * 'output': Specifies the index of the net that receives the output of the current net as input.
   (-1 indicates that the output is the last layer of the network)
 * */
typedef struct {
    unsigned *neurons_per_layer, num_layers, num_input;
    unsigned char source;
    short output;
} neural_net_create_network, neural_net_create_network_arr[], *neural_net_create_network_ptr;

typedef struct {
    struct net *arr;
    unsigned num_classes;
    unsigned char back_on, num_nets;
    float N;
    float *network_output;
} neural_net_bignet, *neural_net_bignet_ptr;

/* neural_net_hit:
 *     Checks if the highest probability predicted by the net is equal to the expected 'class'.
 *     Returns 1 if true and 0 if false. This needs to be float to enable 
     using with pointers that also use 'cross_entropy'.
 *     'predi' receives the class index predicted by the net.
 *     'predv' receives the corresponding probability.
 * */
float neural_net_hit(neural_net_bignet_ptr model, int classidx, int *predi, float *predv);

/* neural_net_cross_entropy:
 *     Calculates the Cross Entropy, measured in nats, of the net output.
 *     'class' is the index of the expected class.
 * */
float neural_net_cross_entropy(neural_net_bignet_ptr model, int classidx);

/* neural_net_init_net_topology:
 *     Uses 'nets' array to assemble the network.
 *     'n' is the number of nets in the network.
 *     If 'verbose' is true, it displays messages indicating the start and end.    
 * */
neural_net_bignet_ptr neural_net_init_net_topology(neural_net_create_network_arr nets, int n, int verbose);

/* neural_net_load_weights:
 *     Reads 'file_name' file and loads the network.
 *     If 'verbose', displays messages indicating the start and end.
 * */
neural_net_bignet_ptr neural_net_load_weights(char file_name[], int verbose);

/* neural_net_init_random_weights:
 *     Assigns random values to biases and weights of the network.
 *     Is assumed that net has already been loaded into memory using 'load_weights' or
 *     'init_net_topology' functions.
 * */
void neural_net_init_random_weights(neural_net_bignet_ptr model);

/* neural_net_save_weights:
 *     Writes in file_name the network.
 * */
void neural_net_save_weights(neural_net_bignet_ptr model, char file_name[]);

/* neural_net_run:
 *     Computes feedforward using 'flat_input' and, in the model, 'network_output' receives the output of the network.
 * */
void neural_net_run(neural_net_bignet_ptr model, float *flat_input);

/* neural_net_ini_backpr:
 *     Allocates the necessary memory for backpropagation.
 *     'n' is used to calculate the mean of the samples in one batch.
 *     If this is equal to 1, no mean CAN be calculated.
 * */
void neural_net_ini_backpr(neural_net_bignet_ptr model, int n);

/* neural_net_clear_backpr:
 *     Clears the values stored during a backpropagation iteration.        
 * */
void neural_net_clear_backpr(neural_net_bignet_ptr model);

/* neural_net_backpr:
 *     Performs the backpropagation using the sample 'flat_input'.
 *     Prior to using this function, 'run' function needs to be executed at
 *     least one time.
 *     'expected' is the expected class array.
 * */
void neural_net_backpr(neural_net_bignet_ptr model, float *flat_input, float *expected);

/* neural_net_apply_backpr:
 *     Modifies the weights and biases using the gradients of error obtained from 'backpr' and applies momentum.
 * */
void neural_net_apply_backpr(neural_net_bignet_ptr model);

/* neural_net_end_backpr:
 *     Deallocates memory used during the backpropagation.
 * */
void neural_net_end_backpr(neural_net_bignet_ptr model);
#endif
