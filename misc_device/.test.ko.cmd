cmd_/home/sh/kernel/misc_device/test.ko := ld -r -m elf_x86_64  -z max-page-size=0x200000  --build-id  -T ./scripts/module-common.lds -o /home/sh/kernel/misc_device/test.ko /home/sh/kernel/misc_device/test.o /home/sh/kernel/misc_device/test.mod.o;  true
