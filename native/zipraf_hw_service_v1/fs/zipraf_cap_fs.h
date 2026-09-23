#ifndef ZIPRAF_CAP_FS_V1_H
#define ZIPRAF_CAP_FS_V1_H

/* Mixed permission model: POSIX-like base mode + ordered ACL + scoped capability.
 * DENY wins over ALLOW. Capabilities can narrow or delegate rights but never
 * override immutable policy flags. This is a policy core, not an OS filesystem.
 */

typedef unsigned int zfs_u32;
typedef unsigned long long zfs_u64;

#define ZFS_R_READ    (1u << 0)
#define ZFS_R_WRITE   (1u << 1)
#define ZFS_R_EXEC    (1u << 2)
#define ZFS_R_APPEND  (1u << 3)
#define ZFS_R_VERIFY  (1u << 4)
#define ZFS_R_SIGN    (1u << 5)
#define ZFS_R_ADMIN   (1u << 6)
#define ZFS_R_ALL     ((1u << 7) - 1u)

#define ZFS_F_IMMUTABLE   (1u << 0)
#define ZFS_F_APPEND_ONLY (1u << 1)
#define ZFS_F_AUDIT       (1u << 2)
#define ZFS_F_NOEXEC      (1u << 3)

typedef struct zfs_acl_entry {
    zfs_u64 principal;
    zfs_u32 allow;
    zfs_u32 deny;
} zfs_acl_entry;

typedef struct zfs_capability {
    zfs_u64 principal;
    zfs_u64 object_scope;
    zfs_u64 not_after_epoch;
    zfs_u32 rights;
} zfs_capability;

typedef struct zfs_object_policy {
    zfs_u64 owner;
    zfs_u64 group;
    zfs_u32 owner_rights;
    zfs_u32 group_rights;
    zfs_u32 other_rights;
    zfs_u32 flags;
} zfs_object_policy;

static inline zfs_u32 zfs_base_rights(const zfs_object_policy *p, zfs_u64 principal, zfs_u64 group) {
    if (!p) return 0;
    if (principal == p->owner) return p->owner_rights & ZFS_R_ALL;
    if (group == p->group) return p->group_rights & ZFS_R_ALL;
    return p->other_rights & ZFS_R_ALL;
}

static inline int zfs_check(const zfs_object_policy *p,
                            const zfs_acl_entry *acl, zfs_u32 acl_count,
                            const zfs_capability *cap,
                            zfs_u64 object_id, zfs_u64 now_epoch,
                            zfs_u64 principal, zfs_u64 group,
                            zfs_u32 requested) {
    zfs_u32 rights, deny = 0, i;
    if (!p || (requested & ~ZFS_R_ALL)) return 0;
    rights = zfs_base_rights(p, principal, group);
    for (i = 0; i < acl_count; ++i) {
        if (acl[i].principal == principal) {
            rights |= acl[i].allow;
            deny |= acl[i].deny;
        }
    }
    if (cap && cap->principal == principal && cap->object_scope == object_id &&
        (cap->not_after_epoch == 0 || now_epoch <= cap->not_after_epoch)) {
        rights |= (cap->rights & ZFS_R_ALL);
    }
    /* Explicit ACL DENY is final even when a scoped capability is present. */
    rights &= ~deny;
    if ((p->flags & ZFS_F_IMMUTABLE) && (requested & (ZFS_R_WRITE|ZFS_R_APPEND|ZFS_R_ADMIN))) return 0;
    if ((p->flags & ZFS_F_APPEND_ONLY) && (requested & ZFS_R_WRITE)) return 0;
    if ((p->flags & ZFS_F_NOEXEC) && (requested & ZFS_R_EXEC)) return 0;
    return (rights & requested) == requested;
}

#endif
