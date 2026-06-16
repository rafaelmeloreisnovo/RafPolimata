/* dirbrowse.h — directory listing via getdents64. No libc. No malloc.
 * Static entry table. Mirrors tbrowse navigation semantics. */
#pragma once
#include "sys_host.h"
#include "str_util.h"

#define DB_MAX   512
#define DB_NLEN   64

typedef struct {
    char name[DB_NLEN];
    u8   type;          /* DT_REG, DT_DIR, DT_LNK */
} DirEntry;

static DirEntry _db_ent[DB_MAX];
static int      _db_count = 0;
static int      _db_sel   = 0;
static char     _db_cwd[512];

/* simple insertion sort: dirs first, then files alphabetically */
static inline void _db_sort(void) {
    for(int i=1;i<_db_count;i++){
        DirEntry t=_db_ent[i]; int j=i-1;
        /* dirs before files */
        while(j>=0){
            int a_dir=(_db_ent[j].type==DT_DIR);
            int b_dir=(t.type==DT_DIR);
            if(b_dir>a_dir) break; /* t is dir, [j] is file → move t left */
            if(b_dir==a_dir){
                /* same type: alphabetical */
                const char *a=_db_ent[j].name, *b=t.name;
                int gt=0; while(*a&&*b&&*a==*b){a++;b++;}
                gt=(*a>*b);
                if(!gt) break;
            }
            _db_ent[j+1]=_db_ent[j]; j--;
        }
        _db_ent[j+1]=t;
    }
}

static inline void db_scan(const char *path) {
    _db_count=0; _db_sel=0;
    _copy(_db_cwd, path, sizeof _db_cwd);

    i32 fd = os_open(path, O_RDONLY|O_DIRECTORY, 0);
    if(fd<0) return;

    static char _de_buf[8192];
    i64 n;
    while((n=os_getdents64(fd,_de_buf,sizeof _de_buf))>0){
        i64 off=0;
        while(off<n){
            struct dirent64 *de=(struct dirent64*)(_de_buf+off);
            off+=(i64)de->d_reclen;
            const char *nm=de->d_name;
            /* skip . but keep .. */
            if(nm[0]=='.'&&nm[1]==0) continue;
            if(_db_count>=DB_MAX) break;
            _copy(_db_ent[_db_count].name, nm, DB_NLEN);
            _db_ent[_db_count].type = de->d_type;
            _db_count++;
        }
    }
    os_close(fd);
    _db_sort();
}

static inline void db_enter(void) {
    if(_db_count==0) return;
    DirEntry *e=&_db_ent[_db_sel];
    if(e->type!=DT_DIR) return;
    char newpath[512];
    if(e->name[0]=='.'&&e->name[1]=='.'&&e->name[2]==0){
        /* go up: strip last component */
        _copy(newpath,_db_cwd,sizeof newpath);
        int i=(int)_len(newpath)-1;
        while(i>0&&newpath[i]!='/') i--;
        if(i==0) newpath[1]=0; else newpath[i]=0;
    } else {
        _path_join(newpath,sizeof newpath,_db_cwd,e->name);
    }
    db_scan(newpath);
}

static inline void db_selected_path(char *out, int cap) {
    if(_db_count==0){_copy(out,"",cap);return;}
    _path_join(out,(sz)cap,_db_cwd,_db_ent[_db_sel].name);
}

static inline void db_move(int delta) {
    _db_sel+=delta;
    if(_db_sel<0)           _db_sel=0;
    if(_db_sel>=_db_count)  _db_sel=_db_count-1;
}
