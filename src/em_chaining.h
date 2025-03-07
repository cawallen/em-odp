/*
 *   Copyright (c) 2020, Nokia Solutions and Networks
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /**
  * @file
  * EM event chaining support
  */

#ifndef EM_CHAINING_H_
#define EM_CHAINING_H_

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC visibility push(default)

/**
 * Send an event to out of EM.
 * This function is declared as a weak symbol in default implementation
 * in em_chaining.c, indicating that the user can override it during
 * linking with another implementation if event chaining is used.
 */
em_status_t event_send_device(em_event_t event, em_queue_t queue);

/**
 * Send multiple events to out of EM.
 * This function is declared as a weak symbol in default implementation
 * in em_chaining.c, indicating that the user can override it during
 * linking with another implementation if event chaining is used.
 */
int event_send_device_multi(const em_event_t events[], int num, em_queue_t queue);

#pragma GCC visibility pop

/**
 * Initialize event chaining during start-up.
 */
em_status_t chaining_init(event_chaining_t *event_chaining);

/**
 * Terminate event chaining during shut-down.
 */
em_status_t chaining_term(const event_chaining_t *event_chaining);

/**
 * Send an event to out of EM (e.g. to another device) via event-chaining and a
 * user-provided function 'event_send_device()'.
 * @see event_send_device()
 */
static inline em_status_t
send_chaining(em_event_t event, em_queue_t chaining_queue)
{
	const unsigned int num_outq = em_shm->event_chaining.num_output_queues;
	const em_sched_context_type_t sched_ctx_type =
		em_locm.current.sched_context_type;

	if (num_outq == 0 || sched_ctx_type != EM_SCHED_CONTEXT_TYPE_ORDERED)
		return event_send_device(event, chaining_queue);

	/* always use the same output queue for each chaining queue */
	const internal_queue_t iq = {.queue = chaining_queue};
	em_queue_t output_queue;
	queue_elem_t *output_q_elem;
	uint32_t idx;

	idx = ((uint32_t)iq.device_id + (uint32_t)iq.queue_id) % num_outq;
	output_queue = em_shm->event_chaining.output_queues[idx];
	output_q_elem = queue_elem_get(output_queue);

	RETURN_ERROR_IF(EM_CHECK_LEVEL >= 3 && !output_q_elem,
			EM_ERR_BAD_ID, EM_ESCOPE_EVENT_SEND_DEVICE,
			"Invalid output queue:%" PRI_QUEUE "", output_queue);

	return send_output(event, output_q_elem);
}

/**
 * Send 'num' events out of EM (e.g. to another device) via event-chaining and a
 * user-provided function 'event_send_device_multi()'.
 * @see event_send_device_multi()
 */
static inline int
send_chaining_multi(const em_event_t events[], const int num,
		    em_queue_t chaining_queue)
{
	const unsigned int num_outq = em_shm->event_chaining.num_output_queues;
	const em_sched_context_type_t sched_ctx_type =
		em_locm.current.sched_context_type;

	if (num_outq == 0 || sched_ctx_type != EM_SCHED_CONTEXT_TYPE_ORDERED)
		return event_send_device_multi(events, num, chaining_queue);

	/* always use the same output queue for each chaining queue */
	const internal_queue_t iq = {.queue = chaining_queue};
	em_queue_t output_queue;
	queue_elem_t *output_q_elem;
	uint32_t idx;

	idx = ((uint32_t)iq.device_id + (uint32_t)iq.queue_id) % num_outq;
	output_queue = em_shm->event_chaining.output_queues[idx];
	output_q_elem = queue_elem_get(output_queue);

	if (unlikely(EM_CHECK_LEVEL >= 3 && !output_q_elem)) {
		INTERNAL_ERROR(EM_ERR_BAD_ID, EM_ESCOPE_EVENT_SEND_DEVICE_MULTI,
			       "Invalid output queue:%" PRI_QUEUE "", output_queue);
		return 0;
	}

	return send_output_multi(events, num, output_q_elem);
}

/**
 * Send an event tagged with an event group out of EM (e.g. to another device)
 * via event-chaining and a user-provided function 'event_send_device()'.
 * @see event_send_device()
 */
static inline em_status_t
send_chaining_egrp(em_event_t event, event_hdr_t *const ev_hdr,
		   em_queue_t chaining_queue,
		   const event_group_elem_t *egrp_elem)
{
	if (!egrp_elem)
		return send_chaining(event, chaining_queue);

	em_event_group_t save_egrp;
	event_group_elem_t *save_egrp_elem;
	int32_t save_egrp_gen;

	/* Send to another DEVICE with an event group */
	save_current_evgrp(&save_egrp, &save_egrp_elem, &save_egrp_gen);
	/*
	 * "Simulate" a dispatch round from evgrp perspective,
	 * send-device() instead of EO-receive()
	 */
	event_group_set_local(ev_hdr->egrp, ev_hdr->egrp_gen, 1);

	em_status_t stat = send_chaining(event, chaining_queue);

	event_group_count_decrement(1);
	restore_current_evgrp(save_egrp, save_egrp_elem, save_egrp_gen);

	return stat;
}

/**
 * Send 'num' events tagged with an event group out of EM (e.g. to another device)
 * via event-chaining and a user-provided function 'event_send_device_multi()'.
 * @see event_send_device_multi()
 */
static inline int
send_chaining_egrp_multi(const em_event_t events[], event_hdr_t *const ev_hdrs[],
			 const int num, em_queue_t chaining_queue,
			 const event_group_elem_t *egrp_elem)
{
	if (!egrp_elem)
		return send_chaining_multi(events, num, chaining_queue);

	em_event_group_t save_egrp;
	event_group_elem_t *save_egrp_elem;
	int32_t save_egrp_gen;

	/* Send to another DEVICE with an event group */
	save_current_evgrp(&save_egrp, &save_egrp_elem, &save_egrp_gen);
	/*
	 * "Simulate" dispatch rounds from evgrp perspective,
	 * send-device() instead of EO-receive().
	 * Decrement evgrp-count by 'num' instead of by '1'.
	 * Note: event_group_set_local() called only once for
	 * all events.
	 */
	event_group_set_local(ev_hdrs[0]->egrp, ev_hdrs[0]->egrp_gen, num);

	int num_sent = send_chaining_multi(events, num, chaining_queue);

	event_group_count_decrement(num);
	restore_current_evgrp(save_egrp, save_egrp_elem, save_egrp_gen);

	return num_sent;
}

#ifdef __cplusplus
}
#endif

#endif /* EM_CHAINING_H_ */
