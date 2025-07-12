#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
#    ifdef ENABLE_RGB_MATRIX_SOLID_REACTIVE_BASIC
RGB_MATRIX_EFFECT(SOLID_REACTIVE_BASIC)
#        ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static HSV SOLID_REACTIVE_BASIC_math(HSV hsv, uint16_t offset) {
    hsv.v = scale8(0 + offset, hsv.v);
    return hsv;
}

bool SOLID_REACTIVE_BASIC(effect_params_t* params) { return effect_runner_reactive(params, &SOLID_REACTIVE_BASIC_math); }

#        endif  // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#    endif      // ENABLE_RGB_MATRIX_SOLID_REACTIVE_BASIC
#endif          // RGB_MATRIX_KEYREACTIVE_ENABLED
