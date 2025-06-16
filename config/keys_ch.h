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
