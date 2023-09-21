# OCRC, a AI for optical character recognition written in C
# Copyright (C) 2023-2023 João Edison Roso Manica
#
# OCRC is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OCRC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

OPTIMIZATION = -O3
CFLAGS = -Wall -Wno-implicit -lm -lpng -lcblas

all: neural_img.o 

neural_img.o: neural_img.c neural_img.h
	gcc -c neural_img.c -Wall -Wno-implicit -Wno-multistatement-macros $(OPTIMIZATION)
