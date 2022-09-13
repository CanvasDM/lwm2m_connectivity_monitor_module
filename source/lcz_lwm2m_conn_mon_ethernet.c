/**
 * @file lcz_lwm2m_conn_mon_ethernet.c
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
#define ETHERNET_BEARER 41

/**************************************************************************************************/
/* Local Data Definitions                                                                         */
/**************************************************************************************************/
static uint8_t network_bearer = ETHERNET_BEARER;

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/
static void initialize_resources(void);

/**************************************************************************************************/
/* SYS INIT                                                                                       */
/**************************************************************************************************/
static int lcz_lwm2m_conn_mon_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	initialize_resources();

	return 0;
}

SYS_INIT(lcz_lwm2m_conn_mon_init, APPLICATION, CONFIG_LCZ_LWM2M_CONN_MON_SYS_INIT_PRIORITY);

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
static void initialize_resources(void)
{
	char *str;

	lwm2m_engine_set_u8("4/0/0", network_bearer);

	/* available bearers */
	lwm2m_engine_create_res_inst("4/0/1/0");
	lwm2m_engine_set_res_data("4/0/1/0", &network_bearer, sizeof(network_bearer),
				  LWM2M_RES_DATA_FLAG_RO);

	/* Resource data points to value in attribute table */

	lwm2m_engine_create_res_inst("4/0/4/0");
	str = (char *)attr_get_quasi_static(ATTR_ID_ipv4_addr);
	lwm2m_engine_set_res_data("4/0/4/0", str, attr_get_size(ATTR_ID_ipv4_addr),
				  LWM2M_RES_DATA_FLAG_RO);

	/* Gateway Address */
	lwm2m_engine_create_res_inst("4/0/5/0");
	str = (char *)attr_get_quasi_static(ATTR_ID_gw_ipv4_addr);
	lwm2m_engine_set_res_data("4/0/5/0", str, attr_get_size(ATTR_ID_gw_ipv4_addr),
				  LWM2M_RES_DATA_FLAG_RO);

	/* Delete unused resources created by Zephyr */
	lwm2m_engine_delete_res_inst("4/0/8/0");
	lwm2m_engine_delete_res_inst("4/0/9/0");
	lwm2m_engine_delete_res_inst("4/0/10/0");
}
