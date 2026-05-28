#include "bindings/http_trigger.h"

http_trigger_callback_code_t exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle(exports_wasi_http_0_3_0_rc_2026_03_15_handler_own_request_t request) {
    return HTTP_TRIGGER_CALLBACK_CODE_EXIT;
}

http_trigger_callback_code_t exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle_callback(http_trigger_event_t *event) {
    return HTTP_TRIGGER_CALLBACK_CODE_EXIT;
}
