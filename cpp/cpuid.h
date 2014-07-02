void __cpuid(int CPUInfo[4], int InfoType)
{
 __asm 
  {
     mov    esi, CPUInfo
     mov    eax, InfoType
     xor    ecx, ecx  
     cpuid  
     mov    dword ptr [esi +  0], eax
     mov    dword ptr [esi +  4], ebx  
     mov    dword ptr [esi +  8], ecx  
     mov    dword ptr [esi + 12], edx  
  }
}

void __cpuidex(int CPUInfo[4], int InfoType, int ECXValue)
{
 __asm 
  {
     mov    esi, CPUInfo
     mov    eax, InfoType
     mov    ecx, ECXValue
     cpuid  
     mov    dword ptr [esi +  0], eax
     mov    dword ptr [esi +  4], ebx  
     mov    dword ptr [esi +  8], ecx  
     mov    dword ptr [esi + 12], edx  
  }
}



#ifndef CPUID_H
#define CPUID_H

#ifdef _WIN32
#include <limits.h>
typedef unsigned __int32  uint32_t;

#else
#include <stdint.h>
#endif

class CPUID {
  uint32_t regs[4];

public:
  void load(unsigned i) {
#ifdef _WIN32
    __cpuid((int *)regs, (int)i);

#else
    asm volatile
      ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
       : "a" (i), "c" (0));
    // ECX is set to zero for CPUID function 4
#endif
  }

  const uint32_t &EAX() const {return regs[0];}
  const uint32_t &EBX() const {return regs[1];}
  const uint32_t &ECX() const {return regs[2];}
  const uint32_t &EDX() const {return regs[3];}
};

#endif // CPUID_H


// tests bit 23 of ECX for popcnt instruction support
printf("MMX - %s\n", cpuid.EDX() & (1 << 23) ? "yes" : "no");
printf("SSE - %s\n", cpuid.EDX() & (1 << 25) ? "yes" : "no"); 
printf("SSE2 - %s\n", cpuid.EDX() & (1 << 26) ? "yes" : "no"); 
printf("SSE3 - %s\n", cpuid.ECX() & (1 << 0) ? "yes" : "no"); 
printf("SSSE3 - %s\n", cpuid.ECX() & (1 << 9) ? "yes" : "no"); 
printf("SSE4.1 - %s\n", cpuid.ECX() & (1 << 19) ? "yes" : "no"); 
printf("SSE4.2 - %s\n", cpuid.ECX() & (1 << 20) ? "yes" : "no"); 
printf("AES - %s\n", cpuid.ECX() & (1 << 25) ? "yes" : "no"); 
printf("AVX - %s\n", cpuid.ECX() & (1 << 28) ? "yes" : "no"); 
printf("HT - %s\n", cpuid.EDX() & (1 << 28) ? "yes" : "no"); 
printf("IA64 (emulating x86) - %s\n", cpuid.EDX() & (1 << 30) ? "yes" : "no"); 
printf("Hypervisor? - %s\n", cpuid.ECX() & (1 << 31) ? "yes" : "no"); 
printf("popcnt - %s\n", cpuid.ECX() & (1 << 23) ? "yes" : "no");

/*

Hypervisor CPUID Interface Proposal
-----------------------------------

Intel & AMD have reserved cpuid levels 0x40000000 - 0x400000FF for
software use.  Hypervisors can use these levels to provide an interface
to pass information from the hypervisor to the guest running inside a
virtual machine.

This proposal defines a standard framework for the way in which the
Linux and hypervisor communities incrementally define this CPUID space.

(This proposal may be adopted by other guest OSes.  However, that is not
a requirement because a hypervisor can expose a different CPUID
interface depending on the guest OS type that is specified by the VM
configuration.)

Hypervisor Present Bit:
        Bit 31 of ECX of CPUID leaf 0x1.

        This bit has been reserved by Intel & AMD for use by
        hypervisors, and indicates the presence of a hypervisor.

        Virtual CPU's (hypervisors) set this bit to 1 and physical CPU's
        (all existing and future cpu's) set this bit to zero.  This bit
    can be probed by the guest software to detect whether they are
    running inside a virtual machine.

Hypervisor CPUID Information Leaf:
        Leaf 0x40000000.

        This leaf returns the CPUID leaf range supported by the
        hypervisor and the hypervisor vendor signature.

        # EAX: The maximum input value for CPUID supported by the hypervisor.
        # EBX, ECX, EDX: Hypervisor vendor ID signature.

Hypervisor Specific Leaves:
        Leaf range 0x40000001 - 0x4000000F.

        These cpuid leaves are reserved as hypervisor specific leaves.
        The semantics of these 15 leaves depend on the signature read
        from the "Hypervisor Information Leaf".

Generic Leaves:
        Leaf range 0x40000010 - 0x4000000FF.

        The semantics of these leaves are consistent across all
        hypervisors.  This allows the guest kernel to probe and
        interpret these leaves without checking for a hypervisor
        signature.

        A hypervisor can indicate that a leaf or a leaf's field is
        unsupported by returning zero when that leaf or field is probed.

        To avoid the situation where multiple hypervisors attempt to define the
        semantics for the same leaf during development, we can partition
        the generic leaf space to allow each hypervisor to define a part
        of the generic space.

        For instance:
          VMware could define 0x4000001X
          Xen could define 0x4000002X
          KVM could define 0x4000003X
      and so on...

        Note that hypervisors can implement any leaves that have been
        defined in the generic leaf space whenever common features can
        be found.  For example, VMware hypervisors can implement leafs
        that have been defined in the KVM area 0x4000003X and vice
        versa.

        The kernel can detect the support for a generic field inside 
        leaf 0x400000XY using the following algorithm:

        1.  Get EAX from Leaf 0x400000000, Hypervisor CPUID information.
            EAX returns the maximum input value for the hypervisor CPUID
            space.

            If EAX < 0x400000XY, then the field is not available.

        2.  Else, extract the field from the target Leaf 0x400000XY 
                    by doing cpuid(0x400000XY).

            If (field == 0), this feature is unsupported/unimplemented
                    by the hypervisor.  The kernel should handle this case 
                    gracefully so that a hypervisor is never required to 
                    support or implement any particular generic leaf.

--------------------------------------------------------------------------------

Definition of the Generic CPUID space.
        Leaf 0x40000010, Timing Information.

        VMware has defined the first generic leaf to provide timing
        information.  This leaf returns the current TSC frequency and
        current Bus frequency in kHz.

        # EAX: (Virtual) TSC frequency in kHz.
        # EBX: (Virtual) Bus (local apic timer) frequency in kHz.
        # ECX, EDX: RESERVED (Per above, reserved fields are set to zero).

--------------------------------------------------------------------------------

Written By,
    Alok N Kataria <akataria@vmware.com>
    Dan Hecht <dhecht@vmware.com>
Inputs from,
    Jun Nakajima <jun.nakajima@intel.com>

*/

/*

According to Microsoft4, a flag bit in the ECX register (bit #31, "Hypervisor present"), after executing CPUID with the EAX register set to 0x000000001, will be set to 1 in a (Microsoft) virtual machine and set to 0 on real hardware. This is indeed the official Hypervisor detection mechanism. It's also the official detection mechanism for VMWare6.

But here Microsoft and VMWare are incorrectly relying upon an accident of hardware implementation. Both Intel's2 and AMD's3 CPUID specifications state that bit #31 of the ECX register is reserved. Intel's specification even explicitly states that one should not count on the value of the bit. That includes not counting on the fact of it being zero on real hardware. As such, Microsoft's "official" detection mechanism is bogus.

The proper official detection mechanism is to follow the bit #31 check by executing CPUID with the EAX register set to 0x400000005. The output EAX register should be a number between 0x40000001 and 0x400000FF (as all protocol-compliant virtual machines are required to implement at least functions 0x40000000 and 0x40000001).

Even this is slightly bogus, because the real hardware specifications don't explicitly guarantee that on real hardware EAX will be 0x40000000. Formally, function 0x40000000 is in the "standard function" range of the CPUID instruction but outside of the subset of that range returned by function 0x00000000 and hence the behaviour of which is, as AMD's specification puts it, "undefined and should not be relied upon". But at least it is more than just one bit for the real hardware to accidentally happen to yield the same values as virtual machines do.

There is most definitely no guarantee that CPUID function 0x40000000 will be a no-operation on real hardware. Indeed, on the contrary, it is far more likely that it will not be. A hardware designer can legitimately note that all software that adheres to its part of the protocol will always set zeroes in some bits of the EAX register; and so can, whilst still remaining conformant with the specification, arrange not to decode those bits. So it is legitimately possible for function 0x40000000 to decode to the same as function 0x00000000, which is not a no-operation. It's equally possible, with yet another hardware design choice, that function 0x40000000 will pull some meaningless bit patterns out of a ROM, that may well accidentally happen to satisfy the virtual machine presence check. The behaviour on real hardware of function 0x40000000 really is, simply, "undefined and should not be relied upon".

*/

/*

Detecting when software is running in a VMware virtual machine relies on two mechanisms:

Testing the CPUID hypervisor present bit
Testing the virtual BIOS DMI information and the hypervisor port
Testing the CPUID hypervisor present bit

Intel and AMD CPUs have reserved bit 31 of ECX of CPUID leaf 0x1 as the hypervisor present bit. This bit allows hypervisors to indicate their presence to the guest operating system. Hypervisors set this bit and physical CPUs (all existing and future CPUs) set this bit to zero. Guest operating systems can test bit 31 to detect if they are running inside a virtual machine.
 
Intel and AMD have also reserved CPUID leaves 0x40000000 - 0x400000FF for software use. Hypervisors can use these leaves to provide an interface to pass information from the hypervisor to the guest operating system running inside a virtual machine. The hypervisor bit indicates the presence of a hypervisor and that it is safe to test these additional software leaves. VMware defines the 0x40000000 leaf as the hypervisor CPUID information leaf. Code running on a VMware hypervisor can test the CPUID information leaf for the hypervisor signature. VMware stores the string "VMwareVMware" in EBX, ECX, EDX of CPUID leaf 0x40000000.
Sample code

int cpuid_check()
{
        unsigned int eax, ebx, ecx, edx;
        char hyper_vendor_id[13];

        cpuid(0x1, &eax, &ebx, &ecx, &edx);
        if  (bit 31 of ecx is set) {
                cpuid(0x40000000, &eax, &ebx, &ecx, &edx);
                memcpy(hyper_vendor_id + 0, &ebx, 4);
                memcpy(hyper_vendor_id + 4, &ecx, 4);
                memcpy(hyper_vendor_id + 8, &edx, 4);
                hyper_vendor_id[12] = '\0';
                if (!strcmp(hyper_vendor_id, "VMwareVMware"))
                        return 1;               // Success - running under VMware
        }
        return 0;
}
 

Testing the virtual BIOS DMI information and the hypervisor port

Apart from the CPUID-based method for VMware virtual machine detection, VMware also provides a fallback mechanism for the following reasons:

This CPUID-based technique will not work for guest code running at CPL3 when VT/AMD-V is not available or not enabled.
The hypervisor present bit and hypervisor information leaf are only defined for products based on VMware hardware version 7.
Virtual BIOS DMI information

The VMware virtual BIOS has many VMware-specific identifiers which programs can use to detect hypervisors. For the DMI string check, use the BIOS serial number and check for either string "VMware-" or "VMW" (for Mac OS X guests running on Fusion).
Sample code

int dmi_check(void)
{
        char string[10];
        GET_BIOS_SERIAL(string);

        if (!memcmp(string, "VMware-", 7) || !memcmp(string, "VMW", 3))
                return 1;                       // DMI contains VMware specific string.
        else
                return 0;
}
Performing just the DMI check is insufficient because the BIOS' serial number might, by chance, contain the string "VMware-" or "VMW". You should also test the hypervisor port.

Hypervisor port

VMware implements an I/O port that programs can query to detect if software is running in a VMware hypervisor. This hypervisor port behaves differently depending on magic values in certain registers and modifies some registers as a side effect. VMware hypervisor is detected by performing an IN operation to port 0x5658 (the VMware hypervisor port).

Doing a IN on port 0x5658 with

eax = 0x564D5868 (VMware hypervisor magic value)
ebx = 0xFFFFFFFF (UINT_MAX)
ecx = 10 (Getversion command identifier)
edx = 0x5658 (hypervisor port number)
On VMware, this operation modifies the value of register ebx to 0x564D5868 (the VMware hypervisor magic value).

Sample code

#define VMWARE_HYPERVISOR_MAGIC 0x564D5868
#define VMWARE_HYPERVISOR_PORT  0x5658

#define VMWARE_PORT_CMD_GETVERSION      10

#define VMWARE_PORT(cmd, eax, ebx, ecx, edx)                            \
        __asm__("inl (%%dx)" :                                          \
                        "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx) :    \
                        "0"(VMWARE_HYPERVISOR_MAGIC),                   \
                        "1"(VMWARE_PORT_CMD_##cmd),                     \
                        "2"(VMWARE_HYPERVISOR_PORT), "3"(UINT_MAX) :    \
                        "memory");

int hypervisor_port_check(void)
{
        uint32_t eax, ebx, ecx, edx;
        VMWARE_PORT(GETVERSION, eax, ebx, ecx, edx);
        if (ebx == VMWARE_HYPERVISOR_MAGIC)
                return 1;               // Success - running under VMware
        else
                return 0;
}
 
Although this port lies outside the x86 ISA space, a physical system might have a device that uses the same port number as VMware's hypervisor port. Accessing the physical device port on such systems might have an undefined effect on that device. To prevent this, test the virtual BIOS information before querying the hypervisor port.
Recommended code

int Detect_VMware(void)
{
        if (cpuid_check())
                return 1;               // Success running under VMware.
        else if (dmi_check() && hypervisor_port_check())
                return 1;
        return 0;
}

*/

