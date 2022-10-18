/**
 * @file lcz_lwm2m_conn_mon_modem.c
 * @brief
 *
 * Copyright (c) 2021-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(lwm2m_conn_mon, CONFIG_LCZ_LWM2M_CONN_MON_LOG_LEVEL);

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <init.h>
#include <lcz_lwm2m.h>

#include "system_message_task.h"
#include "lcz_snprintk.h"

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define LTE_FDD_BEARER 6
#define NB_IOT_BEARER 7

static const attr_id_t ATTR_INIT_LIST[] = {
	ATTR_ID_lte_rsrp,
	ATTR_ID_lte_sinr,
	ATTR_ID_ipv4_addr,
#if defined(CONFIG_LCZ_LWM2M_CONN_MON_IPV6)
	ATTR_ID_ipv6_addr,
#endif
};

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static uint8_t network_bearers[] = { LTE_FDD_BEARER, NB_IOT_BEARER };

static bool conn_mon_initialized;

static struct smt_attr_changed_agent cm_attr_agent;

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/
static void initialize_resources(void);

static void cm_attr_callback(const attr_id_t *id_list, size_t list_count, void *context);

/**************************************************************************************************/
/* SYS INIT                                                                                       */
/**************************************************************************************************/
static int lcz_lwm2m_conn_mon_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	/* Register attribute changed callback so that LwM2M items can be updated. */
	cm_attr_agent.context = NULL;
	cm_attr_agent.callback = cm_attr_callback;
	smt_register_attr_changed_agent(&cm_attr_agent);

	return 0;
}

SYS_INIT(lcz_lwm2m_conn_mon_init, APPLICATION, CONFIG_LCZ_LWM2M_CONN_MON_SYS_INIT_PRIORITY);

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
static void initialize_resources(void)
{
	char *str;

	lwm2m_engine_create_res_inst("4/0/1/0");
	lwm2m_engine_set_res_data("4/0/1/0", &network_bearers[0], sizeof(network_bearers[0]),
				  LWM2M_RES_DATA_FLAG_RO);

	lwm2m_engine_create_res_inst("4/0/1/1");
	lwm2m_engine_set_res_data("4/0/1/1", &network_bearers[1], sizeof(network_bearers[1]),
				  LWM2M_RES_DATA_FLAG_RO);

	/* Resource data points to value in attribute table */

	lwm2m_engine_create_res_inst("4/0/4/0");
	str = (char *)attr_get_quasi_static(ATTR_ID_ipv4_addr);
	lwm2m_engine_set_res_data("4/0/4/0", str, attr_get_size(ATTR_ID_ipv4_addr),
				  LWM2M_RES_DATA_FLAG_RO);

#if defined(CONFIG_LCZ_LWM2M_CONN_MON_IPV6)
	lwm2m_engine_create_res_inst("4/0/4/1");
	str = (char *)attr_get_quasi_static(ATTR_ID_ipv6_addr);
	lwm2m_engine_set_res_data("4/0/4/1", str, attr_get_size(ATTR_ID_ipv6_addr),
				  LWM2M_RES_DATA_FLAG_RO);
#endif

	lwm2m_engine_create_res_inst("4/0/7/0");
	str = (char *)attr_get_quasi_static(ATTR_ID_lte_apn);
	lwm2m_engine_set_res_data("4/0/7/0", str, attr_get_size(ATTR_ID_lte_apn),
				  LWM2M_RES_DATA_FLAG_RO);

	/* Delete unused resources created by Zephyr */
	lwm2m_engine_delete_res_inst("4/0/3/0");
	lwm2m_engine_delete_res_inst("4/0/8/0");
	lwm2m_engine_delete_res_inst("4/0/9/0");
	lwm2m_engine_delete_res_inst("4/0/10/0");
#if defined(CONFIG_LCZ_LWM2M_CONNMON_OBJECT_VERSION_1_2)
	lwm2m_engine_delete_res_inst("4/0/12/0");
#endif

	conn_mon_initialized = true;
}

static void cm_attr_callback(const attr_id_t *id_list, size_t list_count, void *context)
{
	ARG_UNUSED(context);
	size_t i;
	bool update = false;

	if (!conn_mon_initialized) {
		initialize_resources();
		id_list = ATTR_INIT_LIST;
		list_count = ARRAY_SIZE(ATTR_INIT_LIST);
	}

	for (i = 0; i < list_count; i++) {
		switch (id_list[i]) {
		case ATTR_ID_lte_rsrp:
			lwm2m_engine_set_s8("4/0/2",
					    (int8_t)attr_get_signed32(ATTR_ID_lte_rsrp, 0));
			break;

		case ATTR_ID_lte_sinr:
			if (IS_ENABLED(CONFIG_LCZ_LWM2M_CONNMON_OBJECT_VERSION_1_2)) {
				lwm2m_engine_set_s32("4/0/11",
						     attr_get_signed32(ATTR_ID_lte_sinr, 0));
			}
			break;

		case ATTR_ID_ipv4_addr:
			update = true;
			break;

#if defined(CONFIG_LCZ_LWM2M_CONN_MON_IPV6)
		case ATTR_ID_ipv6_addr:
			update = true;
			break;
#endif

		default:
			/* Don't care about all values - attribute changed is a broadcast */
			break;
		}
	}

	/* Some values aren't broadcast when the modem updates their state because they
	 * are also configurable [writable] using same attribute ID.
	 */
	if (update) {
		switch (attr_get_uint32(ATTR_ID_lte_rat, 0)) {
		case LTE_RAT_CAT_M1:
			lwm2m_engine_set_u8("4/0/0", LTE_FDD_BEARER);
			break;
		case LTE_RAT_CAT_NB1:
			lwm2m_engine_set_u8("4/0/0", NB_IOT_BEARER);
			break;
		default:
			LOG_ERR("LTE bearer unknown");
			break;
		}
	}
}
