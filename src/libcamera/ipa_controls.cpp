/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * Copyright (C) 2019, Google Inc.
 *
 * ipa_controls.h - IPA control handling
 */

/**
 * \file ipa_controls.h
 * \brief Type definitions for serialized controls
 *
 * This file defines binary formats to store ControlList and ControlInfoMap
 * instances in contiguous, self-contained memory areas called control packets.
 * It describes the layout of the packets through a set of C structures. These
 * formats shall be used when serializing ControlList and ControlInfoMap to
 * transfer them through the IPA C interface and IPA IPC transports.
 *
 * A control packet contains a list of entries, each of them describing a single
 * control range or control value. The packet starts with a fixed-size header
 * described by the ipa_controls_header structure, followed by an array of
 * fixed-size entries. Each entry is associated with data, stored either
 * directly in the entry, or in a data section after the entries array.
 *
 * The following diagram describes the layout of the ControlList packet.
 *
 * ~~~~
 *           +-------------------------+    .                      .
 *  Header / | ipa_controls_header     |    |                      |
 *         | |                         |    |                      |
 *         \ |                         |    |                      |
 *           +-------------------------+    |                      |
 *         / | ipa_control_value_entry |    | hdr.data_offset      |
 *         | | #0                      |    |                      |
 * Control | +-------------------------+    |                      |
 *   value | | ...                     |    |                      |
 * entries | +-------------------------+    |                      |
 *         | | ipa_control_value_entry |    |             hdr.size |
 *         \ | #hdr.entries - 1        |    |                      |
 *           +-------------------------+    |                      |
 *           | empty space (optional)  |    |                      |
 *           +-------------------------+ <--´  .                   |
 *         / | ...                     |       | entry[n].offset   |
 *    Data | | ...                     |       |                   |
 * section | | value data for entry #n | <-----´                   |
 *         \ | ...                     |                           |
 *           +-------------------------+                           |
 *           | empty space (optional)  |                           |
 *           +-------------------------+ <-------------------------´
 * ~~~~
 *
 * The packet header contains the size of the packet, the number of entries, and
 * the offset from the beginning of the packet to the data section. The packet
 * entries array immediately follows the header. The data section starts at the
 * offset ipa_controls_header::data_offset from the beginning of the packet, and
 * shall be aligned to a multiple of 8 bytes.
 *
 * Entries are described by the ipa_control_value_entry structure. They contain
 * the numerical ID of the control, its type, and the number of control values.
 *
 * The control values are stored in the data section in the platform's native
 * format. The ipa_control_value_entry::offset field stores the offset from the
 * beginning of the data section to the values.
 *
 * All control values in the data section shall be stored in the same order as
 * the respective control entries, shall be aligned to a multiple of 8 bytes,
 * and shall be contiguous in memory.
 *
 * Empty spaces may be present between the end of the entries array and the
 * data section, and after the data section. They shall be ignored when parsing
 * the packet.
 *
 * The following diagram describes the layout of the ControlInfoMap packet.
 *
 * ~~~~
 *           +-------------------------+    .                      .
 *  Header / | ipa_controls_header     |    |                      |
 *         | |                         |    |                      |
 *         \ |                         |    |                      |
 *           +-------------------------+    |                      |
 *         / | ipa_control_range_entry |    | hdr.data_offset      |
 *         | | #0                      |    |                      |
 * Control | +-------------------------+    |                      |
 *   range | | ...                     |    |                      |
 * entries | +-------------------------+    |                      |
 *         | | ipa_control_range_entry |    |             hdr.size |
 *         \ | #hdr.entries - 1        |    |                      |
 *           +-------------------------+    |                      |
 *           | empty space (optional)  |    |                      |
 *           +-------------------------+ <--´  .                   |
 *         / | ...                     |       | entry[n].offset   |
 *    Data | | ...                     |       |                   |
 * section | | range data for entry #n | <-----´                   |
 *         \ | ...                     |                           |
 *           +-------------------------+                           |
 *           | empty space (optional)  |                           |
 *           +-------------------------+ <-------------------------´
 * ~~~~
 *
 * The packet header is identical to the ControlList packet header.
 *
 * Entries are described by the ipa_control_range_entry structure. They contain
 * the numerical ID and type of the control. The control range data is stored
 * in the data section as described by the ipa_control_range_data structure.
 * The ipa_control_range_entry::offset field stores the offset from the
 * beginning of the data section to the range data.
 *
 * Range data in the data section shall be stored in the same order as the
 * entries array, shall be aligned to a multiple of 8 bytes, and shall be
 * contiguous in memory.
 *
 * As for the ControlList packet, empty spaces may be present between the end of
 * the entries array and the data section, and after the data section. They
 * shall be ignored when parsing the packet.
 */

/**
 * \def IPA_CONTROLS_FORMAT_VERSION
 * \brief The current control serialization format version
 */

/**
 * \struct ipa_controls_header
 * \brief Serialized control packet header
 * \var ipa_controls_header::version
 * Control packet format version number (shall be IPA_CONTROLS_FORMAT_VERSION)
 * \var ipa_controls_header::handle
 * For ControlInfoMap packets, this field contains a unique non-zero handle
 * generated when the ControlInfoMap is serialized. For ControlList packets,
 * this field contains the handle of the corresponding ControlInfoMap.
 * \var ipa_controls_header::entries
 * Number of entries in the packet
 * \var ipa_controls_header::size
 * The total packet size in bytes
 * \var ipa_controls_header::data_offset
 * Offset in bytes from the beginning of the packet of the data section start
 * \var ipa_controls_header::reserved
 * Reserved for future extensions
 */

/**
 * \struct ipa_control_value_entry
 * \brief Description of a serialized ControlValue entry
 * \var ipa_control_value_entry::id
 * The numerical ID of the control
 * \var ipa_control_value_entry::type
 * The type of the control (defined by enum ControlType)
 * \var ipa_control_value_entry::count
 * The number of control array entries for array controls (1 otherwise)
 * \var ipa_control_value_entry::offset
 * The offset in bytes from the beginning of the data section to the control
 * value data (shall be a multiple of 8 bytes).
 */

/**
 * \struct ipa_control_range_entry
 * \brief Description of a serialized ControlRange entry
 * \var ipa_control_range_entry::id
 * The numerical ID of the control
 * \var ipa_control_range_entry::type
 * The type of the control (defined by enum ControlType)
 * \var ipa_control_range_entry::offset
 * The offset in bytes from the beginning of the data section to the control
 * range data (shall be a multiple of 8 bytes)
 * \var ipa_control_range_entry::padding
 * Padding bytes (shall be set to 0)
 */

/**
 * \union ipa_control_value_data
 * \brief Serialized control value
 * \var ipa_control_value_data::b
 * Value for ControlTypeBool controls
 * \var ipa_control_value_data::i32
 * Value for ControlTypeInteger32 controls
 * \var ipa_control_value_data::i64
 * Value for ControlTypeInteger64 controls
 */

/**
 * \struct ipa_control_range_data
 * \brief Serialized control range
 * \var ipa_control_range_data::min
 * The control minimum value
 * \var ipa_control_range_data::max
 * The control maximum value
 */
