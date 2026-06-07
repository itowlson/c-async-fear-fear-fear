#include "bindings/http_trigger.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RT_STATE_INITING 0
#define RT_STATE_WAITING_FOR_WRITE 1
#define RT_STATE_WAITING_FOR_TIMER 2

struct respond_task {
    uint32_t state;
    uint32_t count;
    http_trigger_waitable_set_t set;
    // wasi_http_0_3_0_rc_2026_03_15_types_tuple2_stream_u8_future_result_option_own_trailers_error_code_t body;
    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_writer_t body_writer;
};


http_trigger_callback_code_t exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle(exports_wasi_http_0_3_0_rc_2026_03_15_handler_own_request_t request) {
    struct respond_task* task = (struct respond_task*)malloc(sizeof(struct respond_task));
    memset(task, 0, sizeof(struct respond_task));
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
    task->set = http_trigger_waitable_set_new();
    http_trigger_waitable_join(task->body_writer, task->set);
    task->state = RT_STATE_WAITING_FOR_WRITE;

    http_trigger_context_set_0(task);

    printf("waity time\n");
    fflush(stdout);

    return HTTP_TRIGGER_CALLBACK_CODE_WAIT(task->set);
}

void set_timer(struct respond_task* task) {
    wasi_clocks_0_3_0_rc_2026_03_15_monotonic_clock_duration_t how_long = 1 * 1000 * 1000 * 1000;  // 1 sec
    http_trigger_subtask_status_t delay_st = wasi_clocks_0_3_0_rc_2026_03_15_monotonic_clock_wait_for(how_long);
    http_trigger_subtask_t delay_task = HTTP_TRIGGER_SUBTASK_HANDLE(delay_st);
    task->set = http_trigger_waitable_set_new();
    http_trigger_waitable_join(delay_task, task->set);
    task->state = RT_STATE_WAITING_FOR_TIMER;
}

void end_response_task(struct respond_task* task) {
    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_drop_writable(task->body_writer);

    http_trigger_waitable_set_drop(task->set);
    task->set = 0;

    free(task);
}

http_trigger_callback_code_t exports_wasi_http_0_3_0_rc_2026_03_15_handler_handle_callback(http_trigger_event_t *event) {
    struct respond_task *task = (struct respond_task*) http_trigger_context_get_0();

    printf("in callback\n");
    fflush(stdout);

    if (task->count >= 5) {
        printf("done 5: exiting\n");
        fflush(stdout);

        end_response_task(task);
        return HTTP_TRIGGER_CALLBACK_CODE_EXIT;
    }

    if (task->state == RT_STATE_WAITING_FOR_WRITE) {
        printf("pending write completed, setting timer\n");
        fflush(stdout);

        set_timer(task);

        printf("waiting for timer...\n");
        fflush(stdout);

        return HTTP_TRIGGER_CALLBACK_CODE_WAIT(task->set);
    } else {
        // task->state == RT_STATE_WAITING_FOR_TIMER
        printf("timer expires, writing\n");
        fflush(stdout);

        http_trigger_string_t body_text;
        http_trigger_string_set(&body_text, "HELLO AGAIN!!!\n");
        http_trigger_waitable_status_t st = wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_write(task->body_writer, body_text.ptr, body_text.len);
        ++task->count;

        if (HTTP_TRIGGER_WAITABLE_STATE(st) == HTTP_TRIGGER_WAITABLE_COMPLETED) {
            // go back and do the timer thing immediately
            printf("insta-complete, setting timer\n");
            fflush(stdout);
            set_timer(task);
            return HTTP_TRIGGER_CALLBACK_CODE_WAIT(task->set);
        } else if (HTTP_TRIGGER_WAITABLE_STATE(st) == HTTP_TRIGGER_WAITABLE_CANCELLED) {
            // drop and exit
            end_response_task(task);
            return HTTP_TRIGGER_CALLBACK_CODE_EXIT;
        }

        // Pending
        printf("write pending, waiting for write\n");
        fflush(stdout);
        task->set = http_trigger_waitable_set_new();
        http_trigger_waitable_join(task->body_writer, task->set); // passing `st` results in instant completion so it's not that
        task->state = RT_STATE_WAITING_FOR_WRITE;

        // http_trigger_context_set_0(task);

        printf("waity time W\n");
        fflush(stdout);

        return HTTP_TRIGGER_CALLBACK_CODE_WAIT(task->set);
    }

    printf("woe\n");
    fflush(stdout);

    wasi_http_0_3_0_rc_2026_03_15_types_stream_u8_drop_writable(task->body_writer);

    http_trigger_waitable_set_drop(task->set);
    task->set = 0;

    free(task);

    return HTTP_TRIGGER_CALLBACK_CODE_EXIT;

}
