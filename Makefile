
.PHONY: all clean
all: env

clean:
	rm -f env env.o env.s

env.s: env.c
	gcc -S -nostdinc -nostdlib -nodefaultlibs -nostartfiles -ffreestanding \
		-fno-stack-protector -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables \
		-Os -static -o $@ $<

env.o: env.s
	as -o $@ $<

env: env.o
	ld -z now -z norelro -s \
		-T script.ld \
		$< -o $@
	strip -o $@ -s \
		-R '.gnu.hash' -R '.gnu.version' \
		-R '.tm_clone_table' \
		-R '.eh_frame*' \
		-R '.note*' \
		-R '.comment*' \
		--strip-unneeded $@
	python3 section-header-stripper.py $@ $@
