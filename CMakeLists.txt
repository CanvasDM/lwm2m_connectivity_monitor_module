#
# Copyright (c) 2022 Laird Connectivity LLC
#
# SPDX-License-Identifier: LicenseRef-LairdConnectivity-Clause
#
zephyr_sources_ifdef(CONFIG_LCZ_LWM2M_CONN_MON_MODEM
    source/lcz_lwm2m_conn_mon_modem.c)

zephyr_sources_ifdef(CONFIG_LCZ_LWM2M_CONN_MON_ETHERNET
    source/lcz_lwm2m_conn_mon_ethernet.c)

zephyr_sources_ifdef(CONFIG_LCZ_LWM2M_CONN_MON_WIFI
    source/lcz_lwm2m_conn_mon_wifi.c)

zephyr_sources_ifdef(CONFIG_LCZ_LWM2M_CONN_MON_BLUETOOTH
    source/lcz_lwm2m_conn_mon_bluetooth.c)
