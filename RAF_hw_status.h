#ifndef RAF_HW_STATUS_H
#define RAF_HW_STATUS_H

/* Hardware operation results. TOKEN_VAZIO is epistemic, never numeric zero (CLOSURE_L16).
 * OK means the targeted operation completed; it is not a physical receipt.
 */
#define RAF_HW_OK                0
#define RAF_HW_ERR_NO_DEVICE (-1001)
#define RAF_HW_ERR_UNSUPPORTED (-1002)
#define RAF_HW_ERR_MAPPING (-1003)
#define RAF_HW_ERR_TIMEOUT (-1004)
#define RAF_HW_ERR_IO (-1005)
#define RAF_HW_ERR_INVALID (-1006)
#define RAF_HW_ERR_CM_REQUIRED (-1007)

/* Bounded polling is not a calibrated deadline. Validate elapsed time on device. */
#ifndef RAF_HW_POLL_LIMIT
#define RAF_HW_POLL_LIMIT 100000u
#endif
#endif
