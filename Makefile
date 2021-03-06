.PHONY: all clean

USERVER_DIR:=../userver
PROTOCOLS_DIR:=../protocols

COMMON_DIR:=../common
COMMON_SRCS:=ansi-utils.c

GEN_DIR:=./gen
GEN_A_SRCS:=arbiter-server.c
GEN_O_SRCS:=oiu-client.c
GEN_R_SRCS:=riu-client.c

ARBITER_DIR:=.
ARBITER_SRCS:=arbiter-func.c arbiter.c

LIBUT_DIR:=../libut

NODES_DIR:=../ics-model
NODES_SRCS:=ics-node.c

O_DIR:=../ansi-opool
O_SRCS:=object-pool.c

CFLAGS:=-std=c99 -fms-extensions -Wall -I$(ARBITER_DIR)/include -I$(COMMON_DIR)/include -I$(USERVER_DIR)/include 
CFLAGS+=-I../json-c/output/include/json-c
CFLAGS+=-I$(NODES_DIR)/include -I$(LIBUT_DIR)/include
CFLAGS+=-I$(GEN_DIR) -I$(O_DIR)/include
CFLAGS+=-I$(PROTOCOLS_DIR)/include
CFLAGS+=-D_GNU_SOURCE

LIBS:=../json-c/output/lib/libjson-c.a -lpthread

ARBITER_APP:=arbiter

A_PROTOCOL:=arbiter_proto.u
O_PROTOCOL:=oiu_proto.u
R_PROTOCOL:=riu_proto.u

all: gen-a gen-o gen-r $(ARBITER_APP)

gen-a: $(PROTOCOLS_DIR)/$(A_PROTOCOL)
	mkdir -p gen
	awk -f $(USERVER_DIR)/gen-tools/gen.awk $(PROTOCOLS_DIR)/$(A_PROTOCOL)
	touch gen-a

gen-o: $(PROTOCOLS_DIR)/$(O_PROTOCOL)
	mkdir -p gen
	awk -f $(USERVER_DIR)/gen-tools/gen.awk $(PROTOCOLS_DIR)/$(O_PROTOCOL)
	touch gen-o

gen-r: $(PROTOCOLS_DIR)/$(R_PROTOCOL)
	mkdir -p gen
	awk -f $(USERVER_DIR)/gen-tools/gen.awk $(PROTOCOLS_DIR)/$(R_PROTOCOL)
	touch gen-r

$(ARBITER_APP): $(COMMON_SRCS:.c=.o) $(GEN_A_SRCS:.c=.o) $(GEN_O_SRCS:.c=.o) $(GEN_R_SRCS:.c=.o) $(ARBITER_SRCS:.c=.o) $(NODES_SRCS:.c=.o) $(O_SRCS:.c=.o) 
	gcc -o $@ $^ $(LIBS)

$(COMMON_SRCS:.c=.o): %.o: $(COMMON_DIR)/src/%.c
	gcc -o $@ -c $< $(CFLAGS)

$(GEN_A_SRCS:.c=.o): %.o: $(GEN_DIR)/%.c
	gcc -o $@ -c $< $(CFLAGS)

$(GEN_O_SRCS:.c=.o): %.o: $(GEN_DIR)/%.c
	gcc -o $@ -c $< $(CFLAGS)

$(GEN_R_SRCS:.c=.o): %.o: $(GEN_DIR)/%.c
	gcc -o $@ -c $< $(CFLAGS)

$(ARBITER_SRCS:.c=.o): %.o: $(ARBITER_DIR)/src/%.c
	gcc -o $@ -c $< $(CFLAGS)

$(NODES_SRCS:.c=.o): %.o: $(NODES_DIR)/src/%.c
	gcc -o$@ -c $< $(CFLAGS)

$(O_SRCS:.c=.o): %.o: $(O_DIR)/src/%.c
	gcc -c -o $@ $< $(CFLAGS)

clean:
	rm -fr $(ARBITER_APP) *.o gen gen-a gen-o gen-r
