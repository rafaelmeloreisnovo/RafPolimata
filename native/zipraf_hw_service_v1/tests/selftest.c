#include "../include/zipraf_hw_service.h"
#include "../fs/zipraf_cap_fs.h"

int main(void) {
    zhw_weights_q16 w = {16384,16384,16384,8192,4096,4096};
    zhw_norm_cost_q16 c = {10000,12000,9000,8000,7000,6000,0x3f};
    zfs_object_policy p = {1,2,ZFS_R_ALL,ZFS_R_READ|ZFS_R_VERIFY,0,ZFS_F_APPEND_ONLY};
    zfs_acl_entry acl[1] = {{3,ZFS_R_READ|ZFS_R_APPEND,0}};
    const zhw_algorithm_info *md5 = zhw_algorithm(ZHW_ALG_MD5);
    zhw_measurement m = {1,1,1,1,1,1,ZHW_CAP_C11|ZHW_CAP_SCALAR,1,1,0,0};

    if (zhw_registry_count() != 13u) return 1;
    if (!md5 || md5->default_status != ZHW_LEGACY_ONLY) return 2;
    if (!zhw_weights_valid(&w)) return 3;
    if (zhw_score_q16(&c,&w,0x3f) == ZHW_TOKEN_VAZIO_U32) return 4;
    c.complete_mask = 0x1f;
    if (zhw_score_q16(&c,&w,0x3f) != ZHW_TOKEN_VAZIO_U32) return 5;
    if (!zhw_backend_eligible(&m,ZHW_CAP_C11|ZHW_CAP_SCALAR,1,1)) return 6;
    if (zhw_backend_eligible(&m,ZHW_CAP_AVX2,1,1)) return 7;

    if (!zfs_check(&p,acl,1,0,9,0,3,99,ZFS_R_APPEND)) return 8;
    if (zfs_check(&p,acl,1,0,9,0,3,99,ZFS_R_WRITE)) return 9;

    /* Explicit DENY remains final even if a scoped capability would grant it. */
    {
        zfs_acl_entry deny_read = {3,0,ZFS_R_READ};
        zfs_capability cap_read = {3,9,0,ZFS_R_READ};
        if (zfs_check(&p,&deny_read,1,&cap_read,9,0,3,99,ZFS_R_READ)) return 10;
    }

    /* Capability is scoped to one object and one principal. */
    {
        zfs_object_policy q = {1,2,0,0,0,0};
        zfs_capability cap_append = {3,77,100,ZFS_R_APPEND};
        if (!zfs_check(&q,0,0,&cap_append,77,99,3,99,ZFS_R_APPEND)) return 11;
        if (zfs_check(&q,0,0,&cap_append,78,99,3,99,ZFS_R_APPEND)) return 12;
        if (zfs_check(&q,0,0,&cap_append,77,101,3,99,ZFS_R_APPEND)) return 13;
    }

    p.flags |= ZFS_F_IMMUTABLE;
    if (zfs_check(&p,acl,1,0,9,0,3,99,ZFS_R_APPEND)) return 14;
    return 0;
}
