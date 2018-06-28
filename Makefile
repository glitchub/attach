CFLAGS+=-Wall -Werror -std=gnu99

attach: attach.c

.PHONY: clean
clean:; rm -f attach
