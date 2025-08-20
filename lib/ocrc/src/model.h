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

/* Model configuration constants */

#ifndef MODEL_H
#define MODEL_H

#define INPUT_QTT 11
#define MAX_CLASSES 5

/*Activation function used in all layers except in the last one*/
#define ACTIVATION_FN(X) tanh(X)
#define DERIVATIVE_ACTIVATION_FN(Z) (1 - powf(tanh(Z), 2))

/*Learning rate and momentum*/
#define RATE 0.5f
#define MOMENTUM 0.3f

/*Last layer activation function*/
#define LAST_ACTIVATION_FN \
    do {\
        for (i=0; i < ptrl->n; i++)\
            ptrl->a[i] = ACTIVATION_FN(ptrl->z[i]);\
    } while (0);

#endif
