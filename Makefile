
env: env.c script.ld
	musl-gcc -Os -static \
		-fno-stack-protector -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables \
		-fuse-ld=lld -nostartfiles -T script.ld -Wl,-z -Wl,now -Wl,-z -Wl,norelro -Wl,-s \
		-o $@ $<
	strip --strip-all --strip-unneeded --strip-section-headers $@

clean:
	rm -f env
