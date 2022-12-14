/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019, Linaro Limited
 */
#ifndef SCMI_MSG_COMMON_H
#define SCMI_MSG_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <types_ext.h>

#define SCMI_VERSION			0x30000
#define SCMI_IMPL_VERSION		0

/*
 * Secure copy of input payload: we expect small messages, at most the legacy
 * SMT messages that are 128 bytes (Linux kernel) including SMT header.
 */
#define SCMI_SEC_PAYLOAD_SIZE		92

/*
 * Copy name identifier in target buffer following the SCMI specification
 * that state name identifier shall be a null terminated string.
 */
#define COPY_NAME_IDENTIFIER(_dst_array, _name)				\
	do {								\
		assert(strlen(_name) < sizeof(_dst_array));		\
		strncpy((_dst_array), (_name), sizeof(_dst_array));	\
	} while (0)

/* Common command identifiers shared by all procotols */
enum scmi_common_message_id {
	SCMI_PROTOCOL_VERSION = 0x000,
	SCMI_PROTOCOL_ATTRIBUTES = 0x001,
	SCMI_PROTOCOL_MESSAGE_ATTRIBUTES = 0x002
};

/* Common platform-to-agent (p2a) PROTOCOL_VERSION structure */
struct scmi_protocol_version_p2a {
	int32_t status;
	uint32_t version;
};

/* Generic platform-to-agent (p2a) PROTOCOL_ATTRIBUTES structure */
struct scmi_protocol_attributes_p2a {
	int32_t status;
	uint32_t attributes;
};

/* Generic agent-to-platform (a2p) PROTOCOL_MESSAGE_ATTRIBUTES structure */
struct scmi_protocol_message_attributes_a2p {
	uint32_t message_id;
};

/* Generic platform-to-agent (p2a) PROTOCOL_MESSAGE_ATTRIBUTES structure */
struct scmi_protocol_message_attributes_p2a {
	int32_t status;
	uint32_t attributes;
};

/*
 * struct scmi_msg - SCMI message context
 *
 * @channel_id: SCMI channel ID, safely set from secure world
 * @protocol_id: SCMI protocol ID for the related message, set by caller agent
 * @message_id: SCMI message ID for the related message, set by caller agent
 * @in: Address of the incoming message payload copied in secure memory
 * @in_size: Byte length of the incoming message payload, set by caller agent
 * @out: Address of of the output message payload message in non-secure memory
 * @out_size: Byte length of the provisionned output buffer
 * @out_size_out: Byte length of the output message payload
 */
struct scmi_msg {
	unsigned int channel_id;
	unsigned int protocol_id;
	unsigned int message_id;
	char *in;
	size_t in_size;
	char *out;
	size_t out_size;
	size_t out_size_out;
};

/*
 * Type scmi_msg_handler_t is used by procotol drivers to safely find
 * the handler function for the incoming message ID.
 */
typedef void (*scmi_msg_handler_t)(struct scmi_msg *msg);

/*
 * Process Read, process and write response for input SCMI message
 *
 * @msg: SCMI message context
 */
void scmi_process_message(struct scmi_msg *msg);

/*
 * Write SCMI response payload to output message shared memory
 *
 * @msg: SCMI message context
 * @payload: Output message payload
 * @size: Byte size of output message payload
 */
void scmi_write_response(struct scmi_msg *msg, void *payload, size_t size);

/*
 * Write status only SCMI response payload to output message shared memory
 *
 * @msg: SCMI message context
 * @status: SCMI status value returned to caller
 */
void scmi_status_response(struct scmi_msg *msg, int32_t status);

/*
 * Claim access to channel
 * @channel: SCMI channel reference
 * Return true upon success or false if the channel is already busy
 */
bool scmi_msg_claim_channel(struct scmi_msg_channel *channel);

/*
 * Release access channel
 * @channel: SCMI channel reference
 */
void scmi_msg_release_channel(struct scmi_msg_channel *channel);

/*
 * Entry for processing a channel using SMT shared memory protocol
 * @channel_id: SCMI channel identifier provided by client
 * @payload_buf: Secure buffer where to copy input message
 */
void scmi_entry_smt(unsigned int channel_id, uint32_t *payload_buf);

/*
 * Entry for processing a channel using SMT shared memory protocol
 *
 * @channel_id: SCMI channel identifier provided by client
 * @in_buf: Shared buffer storing input SCMI message
 * @in_size: Byte size of @in_buf, including MSG header and message payload
 * @out_buf: Shared buffer storing input SCMI message
 * @out_size: [in] @out_buf max byte size
 *            [out] @out_buf output byte size (MSG header and message payload)
 * @sec_buf: Secure buffer where to copy input message
 */
TEE_Result scmi_entry_msg(unsigned int channel_id, void *in_buf, size_t in_size,
			  void *out_buf, size_t *out_size, uint32_t *sec_buf);
#endif /* SCMI_MSG_COMMON_H */
