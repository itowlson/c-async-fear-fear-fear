#include "bindings/http_trigger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RT_STATE_INITING 0
#define RT_STATE_WAITING_FOR_WRITE 1
#define RT_STATE_WROTE 2

struct respond_task {
    uint32_t state;
    http_trigger_waitable_set_t set;
    // wasi_http_0_3_0_rc_2026_03_15_types_tuple2_stream_u8_future_result_option_own_trailers_error_code_t body;
    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_writer_t body_writer;
};


http_trigger_callback_code_t exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle(exports_wasi_http_0_3_0_rc_2026_03_15_handler_own_request_t request) {
    struct respond_task* task = (struct respond_task*)malloc(sizeof(struct respond_task));
    memset(task, 0, sizeof(struct respond_task));
    task->set = http_trigger_waitable_set_new();
    task->state = RT_STATE_INITING;

    printf("in handler\n");
    fflush(stdout);

    wasi_http_0_3_0_rc_2026_03_15_types_own_headers_t headers = wasi_http_0_3_0_rc_2026_03_15_types_constructor_fields();
    wasi_http_0_3_0_rc_2026_03_15_types_borrow_fields_t bheaders = wasi_http_0_3_0_rc_2026_03_15_types_borrow_fields(headers);
    http_trigger_string_t name;
    http_trigger_string_set(&name, "Content-Type");
    http_trigger_string_t value_text;
    http_trigger_string_set(&value_text, "text/plain");
    wasi_http_0_3_0_rc_2026_03_15_types_field_value_t value = {
        .ptr = value_text.ptr,
        .len = value_text.len
    };
    wasi_http_0_3_0_rc_2026_03_15_types_list_field_value_t values = {
        .ptr = &value,
        .len = 1,
    };
    wasi_http_0_3_0_rc_2026_03_15_types_header_error_t hset_err;
    if (!wasi_http_0_3_0_rc_2026_03_15_types_method_fields_set(bheaders, &name, &values, &hset_err)) {
        return HTTP_TRIGGER_CALLBACK_CODE_EXIT;
    }

    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_writer_t body_writer;
    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_t maybe_contents = wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_new(&body_writer);
    task->body_writer = body_writer;

    wasi_http_0_3_0_rc_2026_03_15_types_future_result_option_own_trailers_error_code_writer_t trailers_writer;
    wasi_http_0_3_0_rc_2026_03_15_types_future_result_option_own_trailers_error_code_t trailers = wasi_http_0_3_0_rc_2026_03_15_types_future_result_option_own_trailers_error_code_new(&trailers_writer);

    wasi_http_0_3_0_rc_2026_03_15_types_tuple2_own_response_future_result_void_error_code_t response_and_error_fut;
    wasi_http_0_3_0_rc_2026_03_15_types_static_response_new(headers, &maybe_contents, trailers, &response_and_error_fut);
    wasi_http_0_3_0_rc_2026_03_15_types_own_response_t response = response_and_error_fut.f0;
    wasi_http_0_3_0_rc_2026_03_15_types_future_result_void_error_code_t error_fut = response_and_error_fut.f1;

    // wasi_http_0_3_0_rc_2026_03_15_types_tuple2_stream_u8_future_result_option_own_trailers_error_code_t ogbod;
    // wasi_http_0_3_0_rc_2026_03_15_types_static_response_consume_body(response, error_fut, &ogbod);
    // task->body = ogbod;

    exports_wasi_http_0_3_0_rc_2026_03_15_handler_result_own_response_error_code_t resp2 = {
        .is_err = false,
        .val = response,
    };
    exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle_return(resp2);

    http_trigger_string_t body_text;
    http_trigger_string_set(&body_text, "HELLO WORLD!!!\n");
    http_trigger_waitable_status_t st = wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_write(task->body_writer, body_text.ptr, body_text.len);
    http_trigger_waitable_join(task->body_writer, task->set);
    task->state = RT_STATE_WAITING_FOR_WRITE;

    http_trigger_context_set_0(task);

    printf("waity time\n");
    fflush(stdout);

    return HTTP_TRIGGER_CALLBACK_CODE_WAIT(task->set);
}

http_trigger_callback_code_t exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle_callback(http_trigger_event_t *event) {
    struct respond_task *task = (struct respond_task*) http_trigger_context_get_0();

    // if (task->state == RT_STATE_INITING) {
    //     http_trigger_string_t body_text;
    //     http_trigger_string_set(&body_text, "HELLO WORLD!!!\n");
    //     http_trigger_waitable_status_t st = wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_write(task->body_writer, body_text.ptr, body_text.len);
    //     http_trigger_waitable_join(task->body_writer, task->set);
    //     task->state = RT_STATE_WAITING_FOR_WRITE;
    //     return HTTP_TRIGGER_CALLBACK_CODE_WAIT(task->set);
    // }
    printf("in callback\n");
    fflush(stdout);

    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_drop_writable(task->body_writer);

    http_trigger_waitable_set_drop(task->set);
    task->set = 0;

    free(task);

    return HTTP_TRIGGER_CALLBACK_CODE_EXIT;
}
