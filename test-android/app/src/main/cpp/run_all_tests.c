#include <assert.h>

int preset_pinning_main();
int seq_event_queue_remove_main();
int seq_scale_main();
int jack_obtaining_synth_main();
int settings_unregister_callback_main();
int pointer_alignment_main();
int sfont_zone_main();
int utf8_open_main();
int seqbind_unregister_main();
int sf3_sfont_loading_main();
int sample_cache_main();
int synth_process_main();
int ct2hz_main();
int seq_evt_order_main();
int snprintf_main();
int seq_event_queue_sort_main();
int sfont_loading_main();
int preset_sample_loading_main();
int sfont_unloading_main();
int sample_rate_change_main();
int sample_validate_main();
int synth_chorus_reverb_main();
int bug_635_main();

int run_all_fluidsynth_tests() {
    int ret = 0; 
    //ret += preset_pinning_main();
    ret += seq_event_queue_remove_main();
    ret += seq_scale_main();
    ret += jack_obtaining_synth_main();
    ret += settings_unregister_callback_main();
    ret += pointer_alignment_main();
    ret += sfont_zone_main();
    //ret += utf8_open_main();
    ret += seqbind_unregister_main();
    //ret += sf3_sfont_loading_main();
    //ret += sample_cache_main();
    ret += synth_process_main();
    ret += ct2hz_main();
    ret += seq_evt_order_main();
    ret += snprintf_main();
    ret += seq_event_queue_sort_main();
    //ret += sfont_loading_main();
    //ret += preset_sample_loading_main();
    //ret += sfont_unloading_main();
    //ret += sample_rate_change_main();
    ret += sample_validate_main();
    ret += synth_chorus_reverb_main();
    //ret += bug_635_main();
    assert(ret == 0);
    return ret;
}