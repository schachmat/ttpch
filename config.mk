# Customize below to fit your system

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc -lmnl

# flags
CFLAGS = -std=c99 -pedantic -Wall -O0 -g ${INCS}
#CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS = -g ${LIBS}
#LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
