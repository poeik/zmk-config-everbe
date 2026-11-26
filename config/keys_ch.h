#pragma once

#include <behaviors.dtsi>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/bt.h>
#include <dt-bindings/zmk/outputs.h>

// see https://github.com/qmk/qmk_firmware/blob/master/quantum/keymap_extras/keymap_swiss_de.h
// and https://zmk.dev/docs/keymaps/list-of-keycodes

#define CH_QUOT MINUS       // '
#define CH_CIRC EQUAL       // ^ (dead)
#define CH_Z    Y           // Z
#define CH_Y    Z           // Y
#define CH_DIAE RBKT        // ¨ (dead)
#define CH_DLR  NUHS        // $
#define CH_MINS FSLH        // -
#define CH_LABK GRAVE       // <  
#define CH_RABK LS(CH_LABK) // >
#define CH_LPRN LS(N8)      // (
#define CH_RPRN LS(N9)      // )
#define CH_LBRC LA(N5)      // [
#define CH_RBRC LA(N6)      // ]
#define CH_LCBR LA(N8)      // {
#define CH_RCBR LA(N9)      // }

#define CH_SLSH LS(N7)      // /
#define CH_BSLS LS(RA(N7))  // backslash
#define CH_PIPE LA(N7)      // |
#define CH_HASH LA(N3)      // #
#define CH_EQ   LS(N0)      // =
#define CH_AMPS LS(N6)      // &

// macOS: screenshots
#define CH_SHOT_FULL LS(LG(N3)) // whole screen
#define CH_SHOT_AREA LS(LG(N4)) // selection

// Windows Swiss German (AltGr = RALT) where it differs from macOS.
// Everything not listed here is identical on both systems.
#define CHW_LBRC RA(LBKT)   // [  (AltGr+ü)
#define CHW_RBRC RA(RBKT)   // ]  (AltGr+¨)
#define CHW_LCBR RA(SQT)    // {  (AltGr+ä)
#define CHW_RCBR RA(NUHS)   // }  (AltGr+$)
#define CHW_BSLS RA(NUBS)   // backslash (AltGr+<)
#define CHW_PIPE RA(N7)     // |  (AltGr+7)
#define CHW_LABK NUBS       // <
#define CHW_RABK LS(NUBS)   // >
#define CHW_HASH RA(N3)     // #  (AltGr+3)
#define CHW_SHOT_FULL PSCRN      // whole screen
#define CHW_SHOT_AREA LS(LG(S))  // Snipping Tool selection
