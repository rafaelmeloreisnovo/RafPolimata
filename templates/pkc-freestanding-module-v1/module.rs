#![no_std]

/*
 * PKC RUST MODULE TEMPLATE V1
 * Governance: CLOSURE_L2
 *
 * External crates: none.
 * Final strict runtime parity: TOKEN_VAZIO_TARGET_GATE.
 * This template mirrors the caller-buffer contract; it does not assert that
 * every Rust target emits a zero-runtime final artifact without verification.
 */

#[repr(C)]
pub struct PkcModuleStatusV1 {
    pub warnings: u32,
    pub produced: u32,
}

#[no_mangle]
pub unsafe extern "C" fn pkc_module_step_v1(
    input: *const u8,
    input_len: u32,
    output: *mut u8,
    output_cap: u32,
    status: *mut PkcModuleStatusV1,
) -> i32 {
    if input.is_null() || output.is_null() || status.is_null() {
        return -1;
    }
    if input_len > output_cap {
        return -2;
    }

    (*status).warnings = 0;
    (*status).produced = 0;

    let mut i = 0u32;
    while i < input_len {
        *output.add(i as usize) = *input.add(i as usize);
        i += 1;
    }
    (*status).produced = input_len;
    0
}
