/*
Copyright (C) 2014 - 2018 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   bios_agent.h
    \brief  Thin mlm_client.h wrapper to incorporate ymsg_t transport message
    \author Karol Hrdina <KarolHrdina@Eaton.com>
*/

#ifndef INCLUDE_BIOS_AGENT_H__
#define INCLUDE_BIOS_AGENT_H__

#include <czmq.h>
#include <malamute.h>
#include <stdbool.h>

#include "ymsg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _bios_agent_t bios_agent_t;

/*!
 \brief Create a new bios_agent.

 Connect to malamute on specified endpoint, with specified timeout in msecs (zero means wait forever). Name of the agent can have form: user/password in which case client logins to the broker via PLAIN authentication.
 \param[in] endpoint    Server endpoint to which to connect
 \param[in] name        Name of the agent. If it is in form user/password, agent connects to the broker via PLAIN authentication.
 \note Please note that you have to call bios_agent_destroy() when you are done working with bios_agent to free up allocated resources.
 \return Newly allocated bios agent on successfull connection, NULL otherwise.
*/
BIOS_EXPORT bios_agent_t*
    bios_agent_new (const char* endpoint, const char* name);

/*!
 \brief Destroy bios_agent, free up resources and nullify the pointer.
 \param[in] self_p  bios agent to be destroyed and nullified
*/
BIOS_EXPORT void
    bios_agent_destroy (bios_agent_t **self_p);

/*!
 \brief Send STREAM SEND message to malamute.

 Takes ownership of message and destroys it when done sending it.
 \param[in] self    Bios agent
 \param[in] subject Message subject
 \param[in] msg_p   Message to be sent. Gets destroyed, nullified upon send.
 \note Message is destroyed and nullified when sent.
 \note Setting bios agent as producer/consumer of STREAM pattern is done by directly manipulating the 'client' member of bios agent using mlm_client_set_producer(), mlm_client_set_consumer().
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_send (bios_agent_t *self, const char *subject, ymsg_t **msg_p);

/*!
 \brief Send ROZP SEND message using pattern MAILBOX SEND to malamute

 Takes ownership of message and destroys it when done sending it.
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.
 \note Message is destroyed and nullified even on failure.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_sendto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p);

/*!
 \brief Send ROZP REPLY message using patter MAILBOX SEND to malamute.

 Message sequence number of supplied 'send' message (which must be of type ROZP SEND) is copied to 'rep' field of 'reply_p' message (which must be of type ROZP REPLY). Message sequence number of 'self' bios agent is assigned to field 'seq' of 'reply_p' message. Depending on presence/value of "repeat" key in field 'aux' of message 'send_p' field 'request' is copied to 'reply_p' message. Takes ownership of message 'reply_p' and destroys it when done sending it.
 \param[in] self     Bios agent
 \param[in] address  Name of target bios agent
 \param[in] subject  Message subject
 \param[in] reply_p  ROZP REPLY message to be sent. Gets destroyed and nullified upon send.
 \param[in] send     ROZP SEND message from
 \note Message 'reply_p' is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_replyto (bios_agent_t *self, const char *address, const char *subject, ymsg_t **reply_p, ymsg_t *send);

/*!
 \brief Send ROZP SEND message using patter SERVICE SEND to malamute

 Takes ownership of message and destroys it when done sending it.
 \param[in] self    Bios agent
 \param[in] address Name of target bios agent
 \param[in] subject Message subject
 \param[in] send_p  ROZP SEND message to be sent, upon which it gets destroyed and nullified.
 \note Message is destroyed and nullified when sent.
 \return 0 on success, -2 on bad input (bad arguments), -1 on fail for any other reason
*/
BIOS_EXPORT int
    bios_agent_sendfor (bios_agent_t *self, const char *address, const char *subject, ymsg_t **send_p);

/*!
 \brief Receive message from malamute.

 Takes ownership of the message received.
 \param[in] self    Bios agent
 \note Caller is responsible to destroy the received message when done
 \return Received ROZP message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_agent_recv (bios_agent_t *self);

/*!
 \brief Receive message from malamute without blocking forever

 Takes ownership of the message received.
 \param[in] self    Bios agent
 \param[in] timeout Don't wait if nothing comes within X ms
 \note Caller is responsible to destroy the received message when done
 \return Received ROZP message on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_agent_recv_wait (bios_agent_t *self, int timeout);

/*!
 \brief Prepare to publish to a specified stream.

 After this, all messages are sent to this stream exclusively.

 \param[in] self Bios agent
 \param[in] name of the stream

 \return -2 on bad input, -1 if interrupted, >=0 on success
 */
BIOS_EXPORT int
    bios_agent_set_producer (bios_agent_t *self, const char *stream);

/*!
 \brief Prepare to consume messages from a specified stream with
 topic that matches the pattern.

 The pattern is a regular expression
 using the CZMQ zrex syntax. The most useful elements are: ^ and $ to match the
 start and end, . to match any character, \s and \S to match whitespace and
 non-whitespace, \d and \D to match a digit and non-digit, \a and \A to match
 alphabetic and non-alphabetic, \w and \W to match alphanumeric and
 non-alphanumeric, + for one or more repetitions, * for zero or more repetitions,
 and ( ) to create groups.

 \param[in] self     Bios agent
 \param[in] stream   Name of the stream
 \param[in] pattern  regular expression (CZMQ syntax)

 \return 0 if subscription was successful, -2 on bad input, -1 on fail for any other reason.
*/
BIOS_EXPORT int
    bios_agent_set_consumer (bios_agent_t *self, const char *stream, const char *pattern);

/*
 \brief Return last received command.
 \param[in] self bios agent
 \return NULL on failure, one of the following values on success:
        "STREAM DELIVER"
        "MAILBOX DELIVER"
        "SERVICE DELIVER"
*/
BIOS_EXPORT const char *
    bios_agent_command (bios_agent_t *self);

/*
 \brief Return last received status
 \param[in] self bios agent
 \return -1 on failure, last received status on success
*/
BIOS_EXPORT int
    bios_agent_status (bios_agent_t *self);

/*
 \brief Return last received reason
 \param[in] self bios agent
 \return last received reason on success, NULL on failure
*/
BIOS_EXPORT const char *
    bios_agent_reason (bios_agent_t *self);

/*
 \brief Return last received address
 \param[in] self bios agent
 \return last received address on success, NULL on failure
*/
BIOS_EXPORT const char *
    bios_agent_address (bios_agent_t *self);

/*
 \brief Return last received sender
 \param[in] self bios agent
 \return last received sender on success, NULL on failure
*/
BIOS_EXPORT const char *
    bios_agent_sender (bios_agent_t *self);

/*
 \brief Return last received subject
 \param[in] self bios agent
 \return last received subject on success, NULL on failure
*/
BIOS_EXPORT const char *
    bios_agent_subject (bios_agent_t *self);

/*
 \brief Return last received content
 \param[in] self bios agent
 \return last received content on success, NULL on failure
*/
BIOS_EXPORT ymsg_t *
    bios_agent_content (bios_agent_t *self);

BIOS_EXPORT zactor_t *
    bios_agent_actor (bios_agent_t *self);

BIOS_EXPORT zsock_t *
    bios_agent_msgpipe (bios_agent_t *self);

//TODO: move out of bios_agent - this is our specific protocol
//! Get name of the ##BIOS## main stream
BIOS_EXPORT const char *
    bios_get_stream_main ();

//! Get name of the ##BIOS## stream for measurements
BIOS_EXPORT const char *
    bios_get_stream_measurements ();

//! Get name of the ##BIOS## stream for assets
BIOS_EXPORT const char *
    bios_get_stream_assets ();

//! Get array of ##BIOS## streams
BIOS_EXPORT const char **
    bios_get_streams (uint8_t *count);

//! Get value of client member 'seq' which will be used on next send
BIOS_EXPORT uint16_t
    bios_agent_seq (bios_agent_t *self);

//! Returns true if status value of ROZP message is OK, false otherwise
BIOS_EXPORT bool
    ymsg_is_ok (ymsg_t *self);

//! Set status value of ROZP message
BIOS_EXPORT void
    ymsg_set_status (ymsg_t *self, bool status);

//! Returns true if repeat value of ROZP message is YES, false otherwise
BIOS_EXPORT bool
    ymsg_is_repeat (ymsg_t *self);

//! Set repeat value of ROZP message
BIOS_EXPORT void
    ymsg_set_repeat (ymsg_t *self, bool repeat);

//! Get content type value of ROZP message
BIOS_EXPORT const char *
    ymsg_content_type (ymsg_t *self);
//! Set content type value of ROZP message
BIOS_EXPORT void
    ymsg_set_content_type (ymsg_t *self, const char *content_type);

BIOS_EXPORT mlm_client_t *
    bios_agent_client (bios_agent_t *self);

// ACE: there is a need to send new metrics from core
BIOS_EXPORT int
    bios_agent_send_proto_metric (bios_agent_t *self, const char *subject, zmsg_t **msg_p);

// TODO (miska): please change for utils_ymsg.h functions
BIOS_EXPORT const char * ymsg_get_string(ymsg_t* msg, const char *key);
BIOS_EXPORT int32_t ymsg_get_int32(ymsg_t* msg, const char *key);
BIOS_EXPORT int64_t ymsg_get_int64(ymsg_t* msg, const char *key);
BIOS_EXPORT void ymsg_set_string(ymsg_t* msg, const char *key, const char *value);
BIOS_EXPORT void ymsg_set_int32(ymsg_t* msg, const char *key, int32_t value);
BIOS_EXPORT void ymsg_set_int64(ymsg_t* msg, const char *key, int64_t value);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_BIOS_AGENT_H__

