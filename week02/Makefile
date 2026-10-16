include ../common/config.mk
BINS := matmul_seq 

all: $(BINS)
matmul_seq: matmul_seq.c
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -f $(BINS)
.PHONY: all clean
