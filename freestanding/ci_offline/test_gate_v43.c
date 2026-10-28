#include "raf_ci_gate_v43.h"
#include "raf_ci_gate_v4_wire.h"
#define LEN(x) ((unsigned int)(sizeof(x)-1u))
#define TEST(x) do { if (!(x)) return __LINE__; } while (0)
static int run(void){
    static const unsigned char one_bad[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - name: not executable\n      - run: echo yes\n";
    static const unsigned char duplicate[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n";
    static const unsigned char bad_first[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - name: nope\n      - uses: actions/checkout@0123456789abcdef0123456789abcdef01234567\n";
    static const unsigned char bad_null[]="on: null\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n";
    static const unsigned char bad_empty[]="on: []\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n";
    static const unsigned char both[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n        uses: actions/checkout@0123456789abcdef0123456789abcdef01234567\n";
    static const unsigned char valid[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - name: greeting\n        run: echo ok\n      - uses: actions/checkout@0123456789abcdef0123456789abcdef01234567\n";
    static const unsigned char reusable[]="on: workflow_call\njobs:\n  call:\n    uses: org/repo/.github/workflows/ci.yml@0123456789abcdef0123456789abcdef01234567\n";
    static const unsigned char existing_invalid[]="on: # no event\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n";
    static const unsigned char empty_runner[]="on: push\njobs:\n  build:\n    runs-on: []\n    steps:\n      - run: echo yes\n";
    static const unsigned char duplicate_steps[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo yes\n    steps:\n      - run: echo yes\n";
    unsigned int why=0u;
    raf_ci_gate_v4 g;
#define BAD(x,reason) do{g=raf_ci_gate_v43_check(x,LEN(x),&why);TEST(g.state==RAF_CI_G4_HOLD && (why&(reason)));}while(0)
    BAD(one_bad,RAF_CI_G43_STEP_MISSING);
    BAD(duplicate,RAF_CI_G43_DUPLICATE_JOB);
    BAD(bad_first,RAF_CI_G43_STEP_MISSING);
    BAD(bad_null,RAF_CI_G43_BAD_EVENT);
    BAD(bad_empty,RAF_CI_G43_BAD_EVENT);
    BAD(both,RAF_CI_G43_STEP_CONFLICT);
    BAD(empty_runner,RAF_CI_G43_BAD_RUNNER);
    BAD(duplicate_steps,RAF_CI_G43_DUP_STEPS);
#undef BAD
    g=raf_ci_gate_v43_check(valid,LEN(valid),&why);TEST(g.state==RAF_CI_G4_REVIEW && why==0u && g.gaps==0u);
    g=raf_ci_gate_v43_check(reusable,LEN(reusable),&why);TEST(g.state==RAF_CI_G4_REVIEW && why==0u);
    g=raf_ci_gate_v43_check(existing_invalid,LEN(existing_invalid),&why);TEST(g.state==RAF_CI_G4_HOLD);
    g=raf_ci_gate_v43_check((const unsigned char *)0,0u,&why);TEST(g.state==RAF_CI_G4_INVALID);
    TEST(sizeof(raf_ci_gate_v4)==24u && RAF_CI_G4_WIRE_BYTES==32u);
    return 0;
}
int main(void){return run();}
