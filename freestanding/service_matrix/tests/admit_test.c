/* Hosted negative admission tests only. */
#include <stdio.h>
#include "../raf_fs_admit.h"
static int faults;
static void one(unsigned abi,unsigned module,unsigned flags,unsigned expected) {
    unsigned r[8]={abi,module,flags,8u,0u,0u,0u,0u};
    unsigned out[8]={0}; raf_fs_admit(r,out);
    if(out[0]!=expected || out[1]!=module || out[2]!=flags || out[5]!=0u ||
       out[6]!=(expected==RAF_ADMIT_PURE_CONTRACT) || out[7]!=1u)faults++;
}
int main(void) {
    one(1,RAF_MOD_PREFIX_PROBE,0,RAF_ADMIT_PURE_CONTRACT);
    one(1,RAF_MOD_FLAG_GATE,0,RAF_ADMIT_PURE_CONTRACT);
    one(1,RAF_MOD_ANDROID_BRIDGE,0,RAF_ADMIT_NOT_IMPLEMENTED);
    one(1,RAF_MOD_PLATFORM_SYSCALL,0,RAF_ADMIT_NOT_IMPLEMENTED);
    one(1,RAF_MOD_PREFIX_PROBE,RAF_CAP_JNI,RAF_ADMIT_HOSTED_REQUIRED);
    one(1,RAF_MOD_FLAG_GATE,RAF_CAP_LIBC|RAF_CAP_HEAP|RAF_CAP_OS_LOG,RAF_ADMIT_HOSTED_REQUIRED);
    one(2,RAF_MOD_FLAG_GATE,0,RAF_ADMIT_BAD_ABI);
    unsigned out[8];raf_fs_admit(NULL,out);
    if(out[0]!=RAF_ADMIT_NULL || out[6]!=0)faults++;
    raf_fs_admit(NULL,NULL);
    puts(faults?"FAIL admission matrix":"PASS admission matrix (8 positive/negative fixtures)");
    return faults?1:0;
}
