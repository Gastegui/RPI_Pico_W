//
// Created by julen on 7/10/24.
//

#ifndef BUZZER_H
#define BUZZER_H

#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include "hardware/clocks.h"
#include <pico/time.h>

#define DO_3   131   // C3
#define RE_3   147   // D3
#define MI_3   165   // E3
#define FA_3   175   // F3
#define SOL_3  196   // G3
#define LA_3   220   // A3
#define LAS_3  233  // A#3
#define SI_3   247   // B3
// Note frequencies in Hz - 4th octave (standard)
#define DO_4   262  // C4
#define DOS_4  277  // C#4
#define RE_4   294  // D4
#define RES_4  311  // D#4
#define MI_4   330  // E4
#define FA_4   349  // F4
#define FAS_4  370  // F#4
#define SOL_4  392  // G4
#define SOLS_4 415  // G#4
#define LA_4   440  // A4
#define LAS_4  466  // A#4
#define SI_4   494  // B4

// 5th octave
#define DO_5   523  // C5
#define DOS_5  554  // C#5
#define RE_5   588  // D5
#define RES_5  622  // D#5
#define MI_5   659  // E5
#define FA_5   698  // F5
#define FAS_5  740  // F#5
#define SOL_5  784  // G5
#define SOLS_5 831  // G#5
#define LA_5   880  // A5
#define LAS_5  932  // A#5
#define SI_5   988  // B5

class Buzzer
{
    uint8_t pin;
    const uint slice_num;
    const uint channel;
    const float clock_freq = static_cast<float>(125000000);

public:
    explicit Buzzer(const uint8_t pin)
        : pin(pin), slice_num(pwm_gpio_to_slice_num(pin)), channel(pwm_gpio_to_channel(pin))
    {
        gpio_set_function(pin, GPIO_FUNC_PWM);
        pwm_set_enabled(slice_num, true);
    }

    // Volume range: 0 (silent) to 100 (max volume)
    void play_frequency(const uint frequency_hz, const uint duration_ms, const uint volume) const
    {
        // Calculate the clock divider and wrap value for the desired frequency
        const float clock = clock_get_hz(clk_sys);
        const float divider = clock / frequency_hz / 4096;

        // Calculate PWM level based on volume (0-100)
        const uint pwm_level = (volume * 2048) / 100;

        // PWM configuration
        pwm_set_clkdiv(slice_num, divider);
        pwm_set_wrap(slice_num, 4095);
        pwm_set_chan_level(slice_num, channel, pwm_level);

        sleep_ms(duration_ms);
    }

    void stop_sound() const
    {
        pwm_set_chan_level(slice_num, channel, 0);
    }


    // Imperial March (Star Wars) - Main theme
    void play_imperial_march(const uint volume) const
    {
        // First phrase
        play_frequency(RE_4, 400, volume);
        stop_sound(); // Stop the sound for a true pause
        sleep_ms(100);

        play_frequency(RE_4, 400, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(RE_4, 400, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(LAS_3, 300, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(FA_4, 150, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, 400, volume);
        stop_sound();
        sleep_ms(200);

        play_frequency(LAS_3, 300, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(FA_4, 150, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, 400, volume);
        stop_sound();
        sleep_ms(300);

        // Second phrase
        play_frequency(RE_5, 400, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(RE_4, 300, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(RE_4, 300, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(RE_5, 400, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(LAS_4, 300, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(FA_4, 150, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, 400, volume);
        stop_sound();
        sleep_ms(500);
    }

    void play_take_on_me(const uint volume) const
    {
        constexpr uint main_note_duration = 150; // Adjust this to change overall speed

        // Main riff pattern (x2)
        for (int i = 0; i < 2; i++)
        {
            // LA - SI - MI
            play_frequency(LA_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(MI_4, main_note_duration * 2, volume);
            stop_sound();
            sleep_ms(100);

            // LA - SI - MI - SI
            play_frequency(LA_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(MI_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(100);

            // LA - SI - DO5 - SI
            play_frequency(LA_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(DO_5, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(100);

            // LA - SOL4 - LA
            play_frequency(LA_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SOL_4, main_note_duration, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(LA_4, main_note_duration * 2, volume);
            stop_sound();
            sleep_ms(200);
        }

        // Final high part
        play_frequency(MI_5, main_note_duration * 2, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(RE_5, main_note_duration, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(FA_5, main_note_duration * 2, volume);
        stop_sound();
        sleep_ms(100);

        play_frequency(MI_5, main_note_duration * 1.5, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(MI_5, main_note_duration, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_5, main_note_duration, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(SI_4, main_note_duration * 2, volume);
        stop_sound();
        sleep_ms(300);
    }


    void play_mario_theme(const uint volume) const
    {
        // Main melody
        play_frequency(MI_4, 150, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(MI_4, 150, volume);
        stop_sound();
        sleep_ms(150);

        play_frequency(MI_4, 150, volume);
        stop_sound();
        sleep_ms(150);

        play_frequency(DO_4, 150, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(MI_4, 150, volume);
        stop_sound();
        sleep_ms(150);

        play_frequency(SOL_4, 400, volume);
        stop_sound();
        sleep_ms(400);

        play_frequency(SOL_3, 400, volume);
        stop_sound();
        sleep_ms(400);

        // Second part
        play_frequency(DO_4, 400, volume);
        stop_sound();
        sleep_ms(200);

        play_frequency(SOL_3, 200, volume);
        stop_sound();
        sleep_ms(200);

        play_frequency(MI_3, 400, volume);
        stop_sound();
        sleep_ms(200);

        play_frequency(LA_3, 400, volume);
        stop_sound();
        sleep_ms(200);

        play_frequency(SI_3, 400, volume);
        stop_sound();
        sleep_ms(200);

        play_frequency(LAS_3, 200, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_3, 400, volume);
        stop_sound();
        sleep_ms(400);
    }

    void play_tetris_theme(const uint volume) const
    {
        const uint base_duration = 150; // Adjust this to change tempo

        // Main melody
        for (int i = 0; i < 2; i++)
        {
            // Play main part twice
            // First measure
            play_frequency(MI_4, base_duration * 4, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_3, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(DO_4, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(RE_4, base_duration * 4, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(DO_4, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(SI_3, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            // Second measure
            play_frequency(LA_3, base_duration * 4, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(LA_3, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(DO_4, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(MI_4, base_duration * 4, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(RE_4, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);

            play_frequency(DO_4, base_duration * 2, volume);
            stop_sound();
            sleep_ms(50);
        }

        // Ending phrase
        play_frequency(SI_3, base_duration * 3, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(DO_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, base_duration * 4, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(MI_4, base_duration * 4, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(DO_4, base_duration * 4, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_3, base_duration * 4, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_3, base_duration * 4, volume);
        stop_sound();
        sleep_ms(400);
    }

    void play_megalovania(const uint volume)
    {
        const uint base_duration = 120; // Adjust for speed

        // Main riff (x2)
        for (int i = 0; i < 2; i++)
        {
            // First measure: D D D2 A
            play_frequency(RE_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(RE_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(RE_5, base_duration * 2, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(LA_4, base_duration * 1.5, volume);
            stop_sound();
            sleep_ms(100);

            // Second measure: G# G F D F G
            play_frequency(SOL_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(FA_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(RE_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(FA_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(SOL_4, base_duration, volume);
            stop_sound();
            sleep_ms(100);
        }

        // Second part
        for (int i = 0; i < 2; i++)
        {
            play_frequency(DO_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(DO_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(DO_5, base_duration * 2, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(LA_4, base_duration * 1.5, volume);
            stop_sound();
            sleep_ms(100);

            play_frequency(SOL_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(FA_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(RE_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(FA_4, base_duration, volume);
            stop_sound();
            sleep_ms(20);

            play_frequency(SOL_4, base_duration, volume);
            stop_sound();
            sleep_ms(100);
        }
    }

    void play_zelda_theme(const uint volume)
    {
        const uint base_duration = 150; // Adjust for speed

        // Main theme
        play_frequency(LA_3, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_3, base_duration, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_3, base_duration / 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_3, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(DO_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        // Second measure
        play_frequency(RE_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, base_duration, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, base_duration / 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(RE_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(FA_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        // Climax
        play_frequency(SOL_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(SI_4, base_duration * 1.5, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(DO_5, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(DO_5, base_duration, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(SI_4, base_duration * 2, volume);
        stop_sound();
        sleep_ms(50);

        play_frequency(LA_4, base_duration * 3, volume);
        stop_sound();
        sleep_ms(300);
    }
};

#endif //BUZZER_H
