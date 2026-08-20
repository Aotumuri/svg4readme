CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11

.PHONY: all waves fire forest clean

all: waves_svg fire_svg forest_svg

waves_svg: waves_svg.c config.c config.h
	$(CC) $(CFLAGS) waves_svg.c config.c -lm -o $@

fire_svg: fire_svg.c config.c config.h
	$(CC) $(CFLAGS) fire_svg.c config.c -lm -o $@

forest_svg: forest_svg.c config.c config.h
	$(CC) $(CFLAGS) forest_svg.c config.c -o $@

waves: waves_svg
	./waves_svg > waves.svg

fire: fire_svg
	./fire_svg > fire.svg

forest: forest_svg
	./forest_svg > forest.svg

clean:
	rm -f waves_svg fire_svg forest_svg waves.svg fire.svg forest.svg
