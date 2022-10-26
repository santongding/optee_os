global-incdirs-y += .
srcs-y += main.c
srcs-y += hf-dlog-serial.c
srcs-y += hvc_call.c
srcs-$(CFG_WITH_USER_TA) += vendor_props.c
