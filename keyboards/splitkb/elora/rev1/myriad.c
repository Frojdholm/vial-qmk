/* Copyright 2023 splitkb.com <support@splitkb.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#include "myriad.h"

#include "pointing_device.h"
#include "i2c_master.h"
#include "analog.h"

#include <stdint.h>

typedef struct __attribute__((__packed__)) {
    char magic_numbers[3];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint32_t checksum;
    uint16_t payload_length;
} myriad_header_t;

typedef struct __attribute__((__packed__)) {
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t revision;
} identity_record_t;

static bool myriad_reader(uint8_t *data, uint16_t length) {
    i2c_init();

    const uint8_t eeprom_address = 0x50; // 1010 000 - NOT shifted for R/W bit
    const uint16_t i2c_timeout = 100; // in milliseconds

    uint8_t num_pages = (length / 256) + 1;
    uint8_t last_page_size = length % 256;

    for (int i = 0; i < num_pages; i++) {
        uint8_t reg = 0; // We always start on a page boundary, so this is always zero 
        uint16_t read_length;
        if (i == num_pages - 1) {
            read_length = last_page_size;
        } else {
            read_length = 256;
        }
        i2c_status_t s = i2c_readReg((eeprom_address + i) << 1, reg, &(data[i*256]), read_length, i2c_timeout);
        if (s != I2C_STATUS_SUCCESS) { return false; }
    }
    return true;
}

static bool verify_header(myriad_header_t *header) {
    char magic_numbers[] = {'M', 'Y', 'R'};
    uint8_t version_major = 1;
    uint16_t version_minor = 0;

    for (int i = 0; i < sizeof(magic_numbers); i++) {
        // Check that the header starts with 'MYR', indicating that this is indeed a Myriad card.
        if (header->magic_numbers[i] != magic_numbers[i]) {
            return false;
        }
    }

    if (header->version_major != version_major || header->version_minor > version_minor) {
        // We obviously don't support cards with a different major version, because that indicates a breaking change.
        // We also don't support cards with HIGHER minor version,
        // as we are not guaranteed to be able to properly use all its features.
        return false;
    }

    if (header->payload_length > (2048 - sizeof(myriad_header_t))) {
        // The EEPROM chips are *at most* 16kb / 2kB large,
        // and some of that is taken up by the header.
        // We obviously can't have a payload which exceeds the EEPROM's size.
        return false;
    }

    return true;
}

// Sourced from https://en.wikipedia.org/wiki/Adler-32#Example_implementation
static bool verify_checksum(uint8_t *data, uint16_t length, uint32_t checksum) {
    // Skip the header
    data += sizeof(myriad_header_t);
    length -= sizeof(myriad_header_t);

    const uint32_t MOD_ADLER = 65521;

    uint32_t a = 1, b = 0;
    size_t index;
    
    // Process each byte of the data in order
    for (index = 0; index < length; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    uint32_t calculated = ((b << 16) | a);
    
    return calculated == checksum;
}

// Locates a specific entry by type
// Returns the offset of the PAYLOAD.
static int16_t locate_entry(uint8_t entry_type, uint8_t entry_data_length, uint8_t *data, uint16_t minimum, uint16_t maximum) {
    if (minimum < sizeof(myriad_header_t)) {
        // Records must start *after* the header.
        // We silently allow this so the caller can just specify `0` as minimum for the first entry.
        minimum = sizeof(myriad_header_t);
    }

    uint16_t offset = minimum;
    while (offset < maximum) {
        if (data[offset] == entry_type) {
            // Type matches!
            if (data[offset+1] == entry_data_length) {
                // We found what we are looking for, so return payload reference.
                return offset+2;
            } else {
                // The entry is the wrong length?
                return -2;
            }
        } else {
            // No type match, so skip this one
            // We skip the type byte, the length byte, and any potential data (with length stored in the length byte)
            offset += 2 + data[offset+1];
        }
    }

    // We hit the maximum and didn't find what we are looking for
    return -1;
}

static bool read_card_identity(uint8_t *data, uint16_t length, identity_record_t *record) {
    const uint8_t identity_type = 0x01;
    const uint8_t entry_data_length = sizeof(identity_record_t);
    int16_t result = locate_entry(identity_type, entry_data_length, data, 0, length);
    if (result < 0) { return false; }

    for (int i = 0; i < sizeof(identity_record_t); i++) {
        ((uint8_t*)record)[i] = data[result + i];
    }
    return true;
}

static myriad_card_t _detect_myriad(void) {
    setPinInput(MYRIAD_PRESENT);
    wait_ms(100);
    // The pin has an external pull-up, and a Myriad card shorts it to ground.
    #ifndef MYRIAD_OVERRIDE_PRESENCE
    if (readPin(MYRIAD_PRESENT)) {
        return NONE;
    }
    #endif

    // Attempt to read header
    myriad_header_t header;
    if (!myriad_reader((uint8_t*)&header, sizeof(header))) { return INVALID; }
    if (!verify_header(&header)) { return INVALID; }

    // Now that we have determined that the header is valid
    // and we know the payload length, read the entire thing
    uint8_t data[2048]; // Guaranteed to be large enough.
    uint16_t data_size = sizeof(header)+header.payload_length;
    if (!myriad_reader(data, data_size)) { return INVALID; }
    if (!verify_checksum(data, data_size, header.checksum)) { return INVALID; }

    identity_record_t identity;
    if (!read_card_identity(data, data_size, &identity)) { return INVALID; }

    if (identity.vendor_id == 0x0001 && identity.product_id == 0x0001) {
        return SKB_JOYSTICK;
    } else if (identity.vendor_id == 0x0001 && identity.product_id == 0x0002) {
        return SKB_JOYSTICK;
    } else if (identity.vendor_id == 0x0001 && identity.product_id == 0x0003) {
        return SKB_SWITCHES;
    }

    return UNKNOWN;
}

// Determine card presence & identity
// Does NOT initialize the card for use!
myriad_card_t detect_myriad(void) {
    static myriad_card_t card = UNINITIALIZED;

    if (card == UNINITIALIZED) {
        card = _detect_myriad();
    }

    return card;
}

static void myr_switches_init(void) {
    setPinInputHigh(MYRIAD_GPIO1); // S4
    setPinInputHigh(MYRIAD_GPIO2); // S2
    setPinInputHigh(MYRIAD_GPIO3); // S1
    setPinInputHigh(MYRIAD_GPIO4); // S3
}

static void myr_encoder_init(void) {
    setPinInputHigh(MYRIAD_GPIO1); // Press
    setPinInputHigh(MYRIAD_GPIO2); // A
    setPinInputHigh(MYRIAD_GPIO3); // B
}

static uint16_t myr_joystick_timer;
static void myr_joystick_init(void) {
    setPinInputHigh(MYRIAD_GPIO1); // Press

    myr_joystick_timer = timer_read();
}

// Make sure any card present is ready for use
static myriad_card_t myriad_card_init(void) {
    static bool initialized = false;

    myriad_card_t card = detect_myriad();
    if (initialized) {
        return card;
    }
    initialized = true;

    switch (card) {
        case SKB_SWITCHES:
            myr_switches_init();
            break;
        case SKB_ENCODER:
            myr_encoder_init();
            break;
        case SKB_JOYSTICK:
            myr_joystick_init();
            break;
        default:
            break;
    }
    return card;
}

bool myriad_hook_matrix(matrix_row_t current_matrix[]) {
    myriad_card_t card = myriad_card_init();
    uint8_t word = 0;

    if (card == SKB_SWITCHES) {
        word |= ((!readPin(MYRIAD_GPIO3)) & 1) << 0;
        word |= ((!readPin(MYRIAD_GPIO2)) & 1) << 1;
        word |= ((!readPin(MYRIAD_GPIO4)) & 1) << 2;
        word |= ((!readPin(MYRIAD_GPIO1)) & 1) << 3;
    } else if (card == SKB_ENCODER) {
        word |= ((!readPin(MYRIAD_GPIO1)) & 1) << 4;
    } else if (card == SKB_JOYSTICK) {
        word |= ((!readPin(MYRIAD_GPIO1)) & 1) << 4;
    } else {
        return false;
    }

    // 5 bytes of on-board keys, so we are the 6th
    bool matrix_has_changed = current_matrix[5] ^ word;
    current_matrix[5] = word;

    return matrix_has_changed;
}

void myriad_hook_encoder(uint8_t count, bool pads[]) {
    if (myriad_card_init() != SKB_ENCODER) { return; }
    // 3 onboard encoders, so we are number 4
    // pads goes in pairs, which means we are index 6 & 7
    pads[6] = !readPin(MYRIAD_GPIO2);
    pads[7] = !readPin(MYRIAD_GPIO3);
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {
    if (myriad_card_init() != SKB_JOYSTICK) { return mouse_report; }

    if (timer_elapsed(myr_joystick_timer) < 10) {
        wait_ms(2);
        return mouse_report;
    }

    myr_joystick_timer = timer_read();

    // `analogReadPin` returns 0..1023
    int16_t y = (analogReadPin(MYRIAD_ADC1) - 512) * -1; // Note: axis is flipped
    int16_t x = analogReadPin(MYRIAD_ADC2) - 512;
    // Values are now -512..512

    // Create a dead zone in the middle where the mouse doesn't move
    const int16_t dead_zone = 10;
    if ((y < 0 && y > -1*dead_zone) || (y > 0 && y < dead_zone)) {
        y = 0;
    }
    if ((x < 0 && x > -1*dead_zone) || (x > 0 && x < dead_zone)) {
        x = 0;
    }

    x = x / 16;
    y = y / 16;

    // Clamp final value to make sure we don't under/overflow
    if (y < -127) { y = -127; }
    if (y > 127) { y = 127; }
    if (x < -127) { x = -127; }
    if (x > 127) { x = 127; }

    mouse_report.x = x;
    mouse_report.y = y;

    return mouse_report;
}

void pointing_device_driver_init(void) {
    setPinInput(MYRIAD_ADC1); // Y
    setPinInput(MYRIAD_ADC2); // X
}

void myriad_task(void) {
    myriad_card_t card = myriad_card_init();

    switch (card) {
        case SKB_SWITCHES:
            // Handled via hook
            break;
        case SKB_ENCODER:
            // Handled via hook
            break;
        case SKB_JOYSTICK:
            // Handled via pointing_device_driver_get_report
            break;
        default:
            break;
    }
}
