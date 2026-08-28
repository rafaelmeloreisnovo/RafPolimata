\ Forth freestanding-oriented reference. Assumes cells >= 32 bits.
\ Source-only in V1: compiler/target evidence remains TOKEN_VAZIO.

HEX
FFFFFFFF CONSTANT RHF-U32-MASK
9E3779B9 CONSTANT RHF-PHI32
DECIMAL

: RHF-U32 ( x -- u ) RHF-U32-MASK AND ;

: RHF-ROL32 ( x n -- y )
  >R DUP R@ LSHIFT SWAP 32 R> - RSHIFT OR RHF-U32 ;

VARIABLE RHF-A
VARIABLE RHF-B
VARIABLE RHF-C

: RHF-MIX32 ( a b c -- u )
  RHF-C ! RHF-B ! RHF-A !
  RHF-A @ RHF-B @ + RHF-PHI32 + RHF-U32 RHF-A !
  RHF-C @ RHF-A @ XOR 16 RHF-ROL32 RHF-C !
  RHF-B @ RHF-C @ + RHF-U32 RHF-B !
  RHF-A @ RHF-B @ XOR 12 RHF-ROL32 RHF-A !
  RHF-A @ RHF-C @ XOR RHF-B @ 7 RHF-ROL32 XOR RHF-U32 ;

: RHF-LANES32 ( vector-bits -- lanes )
  DUP 512 = IF DROP 16 EXIT THEN
  DUP 256 = IF DROP 8 EXIT THEN
  DUP 128 = IF DROP 4 EXIT THEN
  DROP 1 ;
