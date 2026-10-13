include ../common/config.mk
BINS := hello burn

all: $(BINS)
hello: hello.c
	$(CC) $(CFLAGS) $< -o $@
burn: burn.c
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -f $(BINS)
.PHONY: all clean
