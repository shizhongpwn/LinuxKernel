#include <linux/fs.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

void kvm(uint8_t code[], size_t code_len) {
    int kvmfd = open("/dev/kvm", O_RDWR|O_CLOEXEC); // open kvm device
    if (kvmfd == -1) {
        errx(1, "failed open /dev/kvm");
    }
    int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0); // create vm, and kvm gives us a handle to this VM
    size_t mem_size = 0x40000000; // 1G memory
    void *mem = mmap(0, mem_size, PROT_READ|PROT_WRITE , MAP_SHARED|MAP_ANONYMOUS, -1, 0); // allocate memory
    int user_entry = 0;
    memcpy((void *)((size_t)mem + user_entry), code, code_len); // create asm code area in mem entry
    struct kvm_userspace_memory_region region = {
        .slot = 0,  // identifying each region of memory we had to  KVM
        .flags = 0, 
        .guest_phys_addr = 0, 
        .memory_size = mem_size,
        .userspace_addr = (size_t)mem
    };
    ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region); // todo 


    // create cpu and it will run in real mode defaultly
    // （20位分页内存，即1M地址空间，地址访问没有限制，不支持内存保护、多任务或代码优先级）
    // ，也即只执行16-bit汇编代码，若想运行32或64-bit，需设置页表。
    int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
    // set up memory for vcpu
    // each cpu has an associated struct kvm run data strcture, used to communicate 
    // information about the  CPU between the kernel and the user space 
    size_t vcpu_map_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
    struct kvm_run* run = (struct kvm_run*)mmap(0, vcpu_map_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0); // one run per vcpu

    // create regs and set it and vcpu has two sets of registers: standard registers and special registers
    struct kvm_regs regs; 
    ioctl(vcpufd, KVM_GET_REGS, &regs);
    regs.rip = user_entry; //  
    regs.rsp = 0x200000; // stack address

    regs.rflags = 0x2; // in x86 cpu, the 0x2 bit should always be set, or starting the VM will fail with this not set
    ioctl(vcpufd, KVM_SET_REGS, &regs);

    // spcial registers include segment registers and control registers
    struct kvm_sregs sregs;
    ioctl(vcpufd, KVM_GET_REGS, &sregs); // read 
    // cs register's default state (along with the initial instruction pointer) points to the reset vector at 16 bytes below the top of memory
    // but we want to change it to point to zero instead.
    // each segment register in sregs include a full segment descriptor and we zero the selector determine what address in memory the segment points to.
    
    setup_page_tables(mem, &sregs);
    setup_segment_registers(&sregs)
    // so we can execute 64 bit code
    ioctl(vcpufd, KVM_SET_REGS, &regs); // write

    //run vm and handler the reason
    //With our VM and VCPU created, our memory mapped and initialized, and our initial register states set,
    // we can use KVM_RUN running instructions with the VCPU, that will return successfully each time virtualization stops,
    // such as for us to emulate hardware. so we'll run it in a loop.
    // KVM_RUN runs the VM in the context of the current thread and don't return until emulation stop , so if you want
    // run a multi CPU ,the user space process must spawn multiple threads, and call KVM_RUN for different virtual cpus in 
    // different thread.
    while (1) {
        ioctl(vcpufd, KVM_RUN, NULL);
        switch (run->exit_reason) { // to handle the emulation exit, we can check run->exit_reason to see why we exited.
            case KVM_EXIT_HLT: // exit
                fputs("KVM_EXIT_HIT\n", stderr);
                return 0;
            case KVM_EXIT_IO: // let virtualized code output its result
                putchar(*(((char *)run) + run->io.data_offset));
                break;
                
            case KVM_EXIT_FAIL_ENTRY: // it indicates that the underlying handware virtualization mechanism can't start the VM 
            // because the initial condition don't match its requirements.
                errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
        run->fail_entry.hardware_entry_failure_reason);
            // it indicates that an error from linux kvm subsystem ranther than from the handware.
            case KVM_EXIT_INTERNAL_ERROR:
                errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x",
        run->internal.suberror);
            case KVM_EXIT_SHUTDOWN:
                errx(1, "KVM_EXIT_SHUTDOWN");
            default:
                errx(1, "Unhandled reason: %d", run->exit_reason);
        }
    }

}
void setup_page_tables(void *mem, struct kvm_sregs *sregs) { // now we use 2M as a page , so we don't have to set PDT
    uint64_t pml4_addr = 0x1000;
    uint64_t *pml4 = (void *)(mem + plm4_addr);
    uint64_t pdpt_adr = 0x2000;
    uint64_t *pdpt = (void *)(mem + pdpt_adr);
    uint64_t pd_addr = 0x3000;
    uint64_t *pd = (void *)(mem + pd_addr);
    pml4[0] = 3 | pd_addr; //  PDE64_PRESENT | PDE64_RW (it indicates the page can be mmaped and writen and readed) | pdpt_addr
    pdpt[0] = 3 | pd_addr; // PDE64_PRESENT | PDE64_RW | pd_addr
    pd[0] = 3 | 0x80; // PDE64_PRESENT | PDE64_RW | PDE64_PS (it indicates that page size is 2M)
    sregs->cr3 = pml4_addr;
    sregs->cr4 = 1 << 5; // CR4_PAE
    sregs->cr4 |= 0x600; //  CR4_OSFXSR | CR4_OSXMMEXCPT; /* enable SSE instructions */
    sregs->cr0 = 0x80050033; // CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG there are some import flags
    sregs->efer = 0x500; // EFER_LME | EFER_LMA
    // efer is extended feature enable register, to allow enabling SYSCALL/SYSRET instruction, and later
    // for entering and exiting long mode 
}
// now we set the segment registers
void setup_segment_registers(struct kvm_sregs *sregs) {
  struct kvm_segment seg = {
    .base = 0,
    .limit = 0xffffffff,
    .selector = 1 << 3,
    .present = 1,
    .type = 11, /* execute, read, accessed */
    .dpl = 0, /* privilege level 0 , this is descriptor privilege level, and in user space it will be set to three*/
    .db = 0,
    .s = 1,
    .l = 1,
    .g = 1, // 4 KByte granularity
  };
  sregs->cs = seg;
  seg.type = 3; /* read/write, accessed */
  seg.selector = 2 << 3;
  sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;
}
int main() {
  /*
  movabs rax, 0x0a33323144434241
  push 8
  pop rcx
  mov edx, 0x217
OUT:
  out dx, al
  shr rax, 8
  loop OUT
  hlt
  */
  uint8_t code[] = "H\xB8\x41\x42\x43\x44\x31\x32\x33\nj\bY\xBA\x17\x02\x00\x00\xEEH\xC1\xE8\b\xE2\xF9\xF4";
  kvm(code, sizeof(code));
}