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

#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include <math.h>

#include "neural_net.h"
#include "model.h"

typedef struct {
    float **w, *b, *z, *a;
    float **err_w, *err_b, *aux_b;
    float **change_w, *change_b; 
    unsigned n, prev_n;
} layer;

typedef struct net {
    layer *arr;
    float *input_first;
    struct net **in_nets;
    unsigned char num_layers, output_original, num_in_nets;
    short out_id;
} net;

void neural_net_run(model, flat_input)
neural_net_bignet_ptr model;
float *flat_input;
{
    net *ptrn;
    layer *ptrl;
    int i;

    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++) {
        ptrl = ptrn->arr;
        if (ptrn->input_first)
            cblas_sgemv(CblasRowMajor, CblasNoTrans, ptrl->n,
                    ptrl->prev_n, 1, ptrl->w[0],
                    ptrl->prev_n, ptrn->input_first, 1, 0,
                    ptrl->z, 1);
        else {
            cblas_sgemv(CblasRowMajor, CblasNoTrans, ptrl->n, ptrl->prev_n, 1, ptrl->w[0], ptrl->prev_n, flat_input, 1, 0, ptrl->z, 1);
            flat_input += ptrl->prev_n;
        }
        for (i=0; i < ptrl->n; i++) {
            ptrl->z[i] += ptrl->b[i];
            ptrl->a[i] = ACTIVATION_FN(ptrl->z[i]);
        }
        for (ptrl+=1; ptrl < ptrn->arr + ptrn->num_layers; ptrl++) {
            cblas_sgemv(CblasRowMajor, CblasNoTrans, ptrl->n, ptrl->prev_n, 1, ptrl->w[0], ptrl->prev_n, (ptrl-1)->a, 1, 0, ptrl->z, 1);
            for (i=0; i < ptrl->n; i++)
                ptrl->z[i] += ptrl->b[i];
            if (ptrl->a == model->network_output) {
                LAST_ACTIVATION_FN;
            } else
                for (i=0; i < ptrl->n; i++)
                    ptrl->a[i] = ACTIVATION_FN(ptrl->z[i]);
        }
    }
}

void neural_net_ini_backpr(model, n)
neural_net_bignet_ptr model;
{
    net *ptrn;
    layer *ptrl;
    int i;

    model->N = n;
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++)
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++) {
            ptrl->err_w = malloc(sizeof(float*) * ptrl->n);
            ptrl->err_w[0] = malloc(sizeof(float) * ptrl->n * ptrl->prev_n);
            ptrl->err_b = malloc(sizeof(float) * ptrl->n);
            if (model->N > 1)
                ptrl->aux_b = malloc(sizeof(float) * ptrl->n);
            ptrl->change_w = malloc(sizeof(float*) * ptrl->n);
            ptrl->change_w[0] = calloc(ptrl->n * ptrl->prev_n, sizeof(float));
            ptrl->change_b = calloc(ptrl->n, sizeof(float));
            for (i=1; i < ptrl->n; i++) {
                ptrl->err_w[i] = *(ptrl->err_w) + i * ptrl->prev_n;
                ptrl->change_w[i] = *(ptrl->change_w) + i * ptrl->prev_n;
            }
        }
    model->back_on = 1;
}

void neural_net_end_backpr(model)
neural_net_bignet_ptr model;
{
    net *ptrn;
    layer *ptrl;

    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++)
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++) {
            free(ptrl->err_w[0]); free(ptrl->err_w); free(ptrl->err_b);
            free(ptrl->change_w[0]); free(ptrl->change_w); free(ptrl->change_b);
            if (model->N > 1)
                free(ptrl->aux_b);
        }
    model->back_on = 0;
}

void neural_net_clear_backpr(model)
neural_net_bignet_ptr model;
{
    net *ptrn;
    layer *ptrl;
    int i, j;
    
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++)
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++)
            for (i=0; i < ptrl->n; i++) {
                for (j=0; j < ptrl->prev_n; j++)
                    ptrl->err_w[i][j] = 0;
                ptrl->err_b[i] = 0;
            }
}

/* Here are used the derivative of cost function with respect to a*/
/* and multiply by the derivative of activation function */
void neural_net_backpr(model, flat_input, expected)
neural_net_bignet_ptr model;
float *flat_input, *expected;
{
    net *ptrn, *ptrn_prev;
    layer *ptrl;
    float *ptrberr;
    int i, next_col;
    
    neural_net_run(model, flat_input);
    /*begins in the last inputs, because is being traversed backwards*/    
    flat_input += INPUT_QTT;
    next_col = 0;    
    for (ptrn = model->num_nets - 1 + model->arr; ptrn >= model->arr; ptrn--) {
        ptrl = ptrn->num_layers - 1 + ptrn->arr;
        ptrberr = model->N > 1? ptrl->aux_b : ptrl->err_b;
        /*delta*/
        if (ptrn->out_id == -1)
            for (i=0; i < ptrl->n; i++) {
                /* ptrberr[i] = expected[i]? ptrl->a[i] - 1 : ptrl->a[i]; */
                ptrberr[i] = ptrl->a[i] - expected[i];
            }
        else
            for (i=0; i < ptrl->n; i++)
                ptrberr[i] *= DERIVATIVE_ACTIVATION_FN(ptrl->z[i]);
        if (model->N > 1)
            for (i=0; i < ptrl->n; i++)
                ptrl->err_b[i] += ptrl->aux_b[i] / model->N;
        /*partial derivative of cost for weight*/
        while (ptrl > ptrn->arr) {
            cblas_sger(CblasRowMajor, ptrl->n, ptrl->prev_n, 1/model->N, ptrberr, 1, (ptrl-1)->a, 1, &ptrl->err_w[0][0], ptrl->prev_n);
            cblas_sgemv(CblasRowMajor, CblasTrans, ptrl->n, ptrl->prev_n, 1, &ptrl->w[0][0], ptrl->prev_n, ptrberr, 1, 0,
                       model->N > 1? (ptrl-1)->aux_b : (ptrl-1)->err_b, 1);
            ptrl--;
            ptrberr = model->N > 1? ptrl->aux_b : ptrl->err_b;
            for (i=0; i < ptrl->n; i++) {
                ptrberr[i] *= DERIVATIVE_ACTIVATION_FN(ptrl->z[i]);
                if (model->N > 1)
                    ptrl->err_b[i] += ptrl->aux_b[i] / model->N;
            }
        }
        cblas_sger(CblasRowMajor, ptrl->n, ptrl->prev_n, 1/model->N, ptrberr, 1, ptrn->input_first? ptrn->input_first : (flat_input -= ptrn->arr->prev_n), 1, &ptrl->err_w[0][0], ptrl->prev_n);
        if (ptrn->input_first) {
            /*this calculates the error for upper networks, stores it in err_b or aux_b for the last layer of upper network*/
            for (ptrn_prev = ptrn->in_nets[0]; ptrn_prev < ptrn->in_nets[0] + ptrn->num_in_nets; ptrn_prev++) {
                cblas_sgemv(CblasRowMajor, CblasTrans, ptrl->n, ptrn_prev->arr[ptrn_prev->num_layers-1].n,
                        1, &ptrl->w[0][next_col], ptrl->prev_n, 
                        model->N > 1? ptrl->aux_b : ptrl->err_b, 
                        1, 
                        0,
                        model->N > 1? ptrn_prev->arr[ptrn_prev->num_layers-1].aux_b : ptrn_prev->arr[ptrn_prev->num_layers-1].err_b,
                               1);
                next_col += ptrn_prev->arr[ptrn_prev->num_layers-1].n;
            }
        }
    }
}

#define NEW_CHANGE(ERR, CHA) ERR * RATE + MOMENTUM * CHA

void neural_net_apply_backpr(model)
neural_net_bignet_ptr model;
{
    net *ptrn;
    layer *ptrl;
    float change;
    int i, j;
        
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++)
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++)
            for (i=0; i < ptrl->n; i++) {
                for (j=0; j < ptrl->prev_n; j++) {
                    change = NEW_CHANGE(ptrl->err_w[i][j], ptrl->change_w[i][j]);
                    ptrl->w[i][j] -= change;
                    ptrl->change_w[i][j] = change;
                }
                change = NEW_CHANGE(ptrl->err_b[i], ptrl->change_b[i]);
                ptrl->b[i] -= change;
                ptrl->change_b[i] = change;
            }
}

#define FLOAT_RANDOM_WEIGHT ((float)rand() / RAND_MAX - 0.5)
#define FLOAT_RANDOM_BIAS (FLOAT_RANDOM_WEIGHT * 2)

neural_net_bignet_ptr neural_net_init_net_topology(nets, n, verbose)
neural_net_create_network_arr nets;
{
    net *ptrn;
    neural_net_create_network_ptr ptrc;
    int i, j, k;
    long unsigned *amount;
    neural_net_bignet_ptr model;
    
    model = malloc(sizeof(neural_net_bignet));
    model->num_nets = n;
    model->back_on = 0;
    if (verbose)
        puts("Allocating memory to the neural network...");
    model->arr = malloc(sizeof(net) * model->num_nets);
    amount = calloc(model->num_nets, sizeof(long unsigned));
    /* for each network */
    for (ptrn=model->arr, ptrc=nets; ptrn < model->arr + model->num_nets; ptrn++, ptrc++) {
        ptrn->num_layers = ptrc->num_layers;
        ptrn->arr = malloc(sizeof(layer) * ptrc->num_layers);
        ptrn->out_id = ptrc->output;
        ptrn->input_first = NULL;
        ptrn->in_nets = malloc(sizeof(net*) * (model->num_nets - 1));
        ptrn->num_in_nets = 0;
        for (i=0; i < ptrn->num_layers; i++) {
            ptrn->arr[i].n = ptrc->neurons_per_layer[i];
            ptrn->arr[i].prev_n = i? ptrc->neurons_per_layer[i-1] : ptrc->num_input;
            ptrn->arr[i].w = malloc(sizeof(float*) * ptrn->arr[i].n);
            ptrn->arr[i].w[0] = malloc(sizeof(float) * ptrn->arr[i].n * ptrn->arr[i].prev_n);
            for (k=1; k < ptrn->arr[i].n; k++)
                ptrn->arr[i].w[k] = ptrn->arr[i].w[0] + k * ptrn->arr[i].prev_n;
            ptrn->arr[i].b = malloc(sizeof(float) * ptrn->arr[i].n);
            ptrn->arr[i].z = malloc(sizeof(float) * ptrn->arr[i].n);
            /*allocates a unique output for the layer,
             *except for the last layers of input networks */
            if (i < ptrn->num_layers-1 || ptrc->output == -1)
                ptrn->arr[i].a = malloc(sizeof(float) * ptrn->arr[i].n);
        }    
        if ((ptrn->output_original = ptrn->out_id == -1)) {
            model->network_output = ptrn->arr[ptrn->num_layers-1].a;
            model->num_classes = ptrn->arr[ptrn->num_layers-1].n;
        }
    }
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets - 1; ptrn++) {
        model->arr[ptrn->out_id].in_nets[model->arr[ptrn->out_id].num_in_nets++] = ptrn;
        amount[ptrn->out_id] += ptrn->arr[ptrn->num_layers-1].n;
    }
    for (ptrn=model->arr, i=0; ptrn < model->arr + model->num_nets; ptrn++, i++)
        if (amount[i]) {
            /*the first net that dumps into this gets the allocate block of the total*/
            ptrn->in_nets[0]->arr[ptrn->in_nets[0]->num_layers-1].a = malloc(amount[i] * sizeof(float));
            ptrn->in_nets[0]->output_original = 1;
            /*then this network receives as input the block of that last layer*/
            ptrn->input_first = ptrn->in_nets[0]->arr[ptrn->in_nets[0]->num_layers-1].a;
            /*other networks put from the number of neurons of the previous net*/
            for (j=1; j < ptrn->num_in_nets; j++)
                ptrn->in_nets[j]->arr[ptrn->in_nets[j]->num_layers-1].a = ptrn->in_nets[j-1]->arr[ptrn->in_nets[j-1]->num_layers-1].a + 
                                              ptrn->in_nets[j-1]->arr[ptrn->in_nets[j-1]->num_layers-1].n;
        }
    free(amount);
    if (verbose)
        puts("Done.");
    return model;
}

void neural_net_init_random_weights(model)
neural_net_bignet_ptr model;
{
    net *ptrn;
    layer *ptrl;
    int i, j;
    
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++) {
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++)
            for (i=0; i < ptrl->n; i++) {
                for (j=0; j < ptrl->prev_n; j++)
                    ptrl->w[i][j] = FLOAT_RANDOM_WEIGHT;
                ptrl->b[i] = FLOAT_RANDOM_BIAS;
            }
    }
}

neural_net_bignet_ptr neural_net_load_weights(file_name, verbose)
char file_name[];
{
    FILE *fp;
    int i, j;
    neural_net_create_network_ptr nets, ptrc;
    net *ptrn;
    layer *ptrl;
    neural_net_bignet_ptr model;
    
    if (verbose)
        printf("Loading weights and biases to the network from: %s...\n", file_name);
    if (!(fp = fopen(file_name, "rb"))) {
        fprintf(stderr, "[load_weights] File %s could not be opened for reading\n", file_name);
        return NULL;
    }
    if (fscanf(fp, "%x\n", &i) != 1) {
        fputs("[load_weights] Unable to read the number of networks\n", stderr);
        return NULL;
    }
    nets = malloc(sizeof(neural_net_create_network) * i);
    for (ptrc=nets; ptrc < nets + i; ptrc++) {
        if (fscanf(fp, "%x %hhx %hd %x", &ptrc->num_layers, &ptrc->source, &ptrc->output, &ptrc->num_input) != 4) {
            fputs("[load_weights] Unable to read network info\n", stderr);
            return NULL;
        }
        ptrc->neurons_per_layer = malloc(sizeof(unsigned) * ptrc->num_layers);
        for (j=0; j < ptrc->num_layers; j++)
            if (fscanf(fp, " %x", &ptrc->neurons_per_layer[j]) != 1) {
                fprintf(stderr, "[load_weights] Unable to read number of neurons in layer %d\n", j+1);
                return NULL;
            }
        fgetc(fp);
    }
    model = neural_net_init_net_topology(nets, i, verbose);
    for (ptrc=nets; ptrc < nets + i; ptrc++)
        free(ptrc->neurons_per_layer);
    free(nets);
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++)
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++) {
            fread(ptrl->w[0], sizeof(float), ptrl->n * ptrl->prev_n, fp);
            fread(ptrl->b, sizeof(float), ptrl->n, fp);
            /*
            for (i=0; i < ptrl->n; i++) {
                for (j=0; j < ptrl->prev_n; j++)
                    if (fscanf(fp, "%a\n", &ptrl->w[i][j]) != 1) {
                        fprintf(stderr, "[load_weights] Unable to read the %d weight of neuron %d\n", j+1, i+1);
                        return NULL;
                    }
                if (fscanf(fp, "%a\n", &ptrl->b[i]) != 1) {
                    fprintf(stderr, "[load_weights] Unable to read the bias of neuron %d\n", i+1);
                    return NULL;
                }
            }
            */
        }
    fclose(fp);
    if (verbose)
        puts("Done.");
    return model;
}

void neural_net_save_weights(model, file_name)
neural_net_bignet_ptr model;
char file_name[];
{
    FILE *fp;
    net *ptrn;
    layer *ptrl;
    int i;
    
    puts("Saving network weights and biases to: weights...");
    if (!(fp = fopen(file_name, "wb"))) {
        fputs("[save_weights] Could not create network file. Discarding changes made.\n", stderr);
        return;
    }
    fprintf(fp, "%x\n", model->num_nets);
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++) {
        fprintf(fp, "%x %hhx %hd %x", ptrn->num_layers, ptrn->num_in_nets? 0 : 1, ptrn->out_id, ptrn->arr->prev_n);
        for (ptrl=ptrn->arr; ptrl < ptrn->arr + ptrn->num_layers; ptrl++)
            fprintf(fp, " %x", ptrl->n);
        fputc('\n', fp);
    }
    if (model->back_on)
        neural_net_end_backpr(model);
    for (ptrn=model->arr; ptrn < model->arr + model->num_nets; ptrn++) {
        for (i=0, ptrl=ptrn->arr; i < ptrn->num_layers; ptrl++, i++) {
            fwrite(ptrl->w[0], sizeof(float), ptrl->n * ptrl->prev_n, fp);
            fwrite(ptrl->b, sizeof(float), ptrl->n, fp);
            /*
            for (j=0; j < ptrl->n; j++) {
                for (k=0; k < ptrl->prev_n; k++)
                    fprintf(fp, "%a\n", ptrl->w[j][k]);
                fprintf(fp, "%a\n", ptrl->b[j]);
            }
            */
            free(ptrl->w[0]);
            free(ptrl->w); free(ptrl->b); free(ptrl->z);
            if (ptrn->output_original || i < ptrn->num_layers-1)
                free(ptrl->a);
        }
        free(ptrn->arr);
    }
    fclose(fp);
    puts("Done.");
}

float neural_net_hit(model, class, predi, predv)
neural_net_bignet_ptr model;
int *predi;
float *predv;
{
    int i, bigi;
    float big;

    bigi = 0;
    big = model->network_output[0];
    for (i=1; i < model->num_classes; i++)
        if (big < model->network_output[i]) {
            big = model->network_output[i];
            bigi = i;
        }
    if (predi)
        *predi = bigi; 
    if (predv)
        *predv = big; 
    return class == bigi;
}

float neural_net_cross_entropy(model, class)
neural_net_bignet_ptr model;
{
    int i;
    float c;
    
    for (c=i=0; i < model->num_classes; i++)
        c += (i == class) * log(model->network_output[i]);
    return -c;
}
