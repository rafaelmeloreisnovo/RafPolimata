#include "raf_ci_gate_v4.h"
#define C(x) if(!(x))return __LINE__
#define N(x) ((unsigned int)(sizeof(x)-1u))
static int go(void) {
    const unsigned char real[]="name: PR\non: [push]\npermissions: read-all\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - uses: actions/checkout@0123456789abcdef0123456789abcdef01234567\n";
    const unsigned char absent_jobs[]="name: PR\non: [push]\npermissions: read-all\n";
    const unsigned char empty_jobs[]="on: push\njobs:\n";
    const unsigned char no_event[]="jobs:\n  build:\n    runs-on: ubuntu-latest\n";
    const unsigned char empty_job[]="on: push\njobs:\n  build:\n";
    const unsigned char reuse[]="on: workflow_call\njobs:\n  build:\n    uses: owner/repo/.github/workflows/test.yml@0123456789abcdef0123456789abcdef01234567\n";
    const unsigned char risk[]="on: pull_request_target\npermissions: write-all\njobs:\n  run:\n    steps:\n      - uses: actions/checkout@v5\n";
    const unsigned char not_workflow[]="Ω00 | molho ≠ molho 🍅🗝️🪵💧";
    const unsigned char nul[]={'o','n',':',' ',0u,'p','u','s','h'};
    const unsigned char bad[]={0xffu};
    raf_ci_gate_v4 g=raf_ci_gate_v4_check(real,N(real));
    C(sizeof(g)==24u && g.state==RAF_CI_G4_REVIEW && g.gaps==0u && g.voids==127u);
    g=raf_ci_gate_v4_check(absent_jobs,N(absent_jobs));C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_JOBS));
    g=raf_ci_gate_v4_check(empty_jobs,N(empty_jobs));C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_JOB_DECL));
    g=raf_ci_gate_v4_check(no_event,N(no_event));C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_ON));
    g=raf_ci_gate_v4_check(empty_job,N(empty_job));C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_EMPTY_BODY));
    g=raf_ci_gate_v4_check(reuse,N(reuse));C(g.state==RAF_CI_G4_REVIEW);
    g=raf_ci_gate_v4_check(risk,N(risk));C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_V2) && g.urgency==0u);
    g=raf_ci_gate_v4_check(not_workflow,N(not_workflow));C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_ON));
    g=raf_ci_gate_v4_check(nul,sizeof(nul));C(g.state==RAF_CI_G4_INVALID && g.voids==127u);
    g=raf_ci_gate_v4_check(bad,sizeof(bad));C(g.state==RAF_CI_G4_INVALID);
    g=raf_ci_gate_v4_check(0u,0u);C(g.state==RAF_CI_G4_INVALID);
    g=raf_ci_gate_v4_check(real,RAF_CI_MAX_INPUT+1u);C(g.state==RAF_CI_G4_INVALID);
    /* Enterprise gate regressions: formerly incorrectly emitted REVIEW. */
    {
        const unsigned char blank_runner[]="on: push\njobs:\n  build:\n    runs-on: # absent\n    steps:\n      - run: echo ok\n";
        const unsigned char blank_on[]="on: # absent\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo ok\n";
        const unsigned char dup_on[]="on: push\non: pull_request\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo ok\n";
        const unsigned char no_steps[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n";
        const unsigned char empty_steps[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n";
        const unsigned char no_exec_step[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - name: only label\n";
        const unsigned char positive[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo ok\n";
        const unsigned char nested_event[]="on:\n  push:\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo ok\n";
        const unsigned char split_step[]="on: push\njobs:\n  build:\n    runs-on: ubuntu-latest\n    steps:\n      - name: checkout\n        run: echo ok\n";
        const unsigned char many_jobs_bad[]="on: push\njobs:\n  first:\n    runs-on: ubuntu-latest\n    steps:\n      - run: echo ok\n  second:\n    runs-on: ubuntu-latest\n";
        g=raf_ci_gate_v4_check(blank_runner,N(blank_runner)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_EMPTY_BODY));
        g=raf_ci_gate_v4_check(blank_on,N(blank_on)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_ON));
        g=raf_ci_gate_v4_check(dup_on,N(dup_on)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_UNSUPPORTED));
        g=raf_ci_gate_v4_check(no_steps,N(no_steps)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_EMPTY_BODY));
        g=raf_ci_gate_v4_check(empty_steps,N(empty_steps)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_EMPTY_BODY));
        g=raf_ci_gate_v4_check(no_exec_step,N(no_exec_step)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_EMPTY_BODY));
        g=raf_ci_gate_v4_check(positive,N(positive)); C(g.state==RAF_CI_G4_REVIEW && !g.gaps);
        g=raf_ci_gate_v4_check(nested_event,N(nested_event)); C(g.state==RAF_CI_G4_REVIEW && !g.gaps);
        g=raf_ci_gate_v4_check(split_step,N(split_step)); C(g.state==RAF_CI_G4_REVIEW && !g.gaps);
        g=raf_ci_gate_v4_check(many_jobs_bad,N(many_jobs_bad)); C(g.state==RAF_CI_G4_HOLD && (g.gaps&RAF_CI_G4_GAP_EMPTY_BODY));
    }
    return 0;
}
int main(void){return go();}
