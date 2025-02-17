/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(lte_lc);

NRF_MODEM_LIB_ON_INIT(lte_lc_init_hook, on_modem_init, NULL);
NRF_MODEM_LIB_ON_SHUTDOWN(lte_lc_shutdown_hook, on_shutdown, NULL);

static void on_modem_init(int err, void *ctx)
{
	if (err) {
		return;
	}

#if defined(CONFIG_BOARD_THINGY91_NRF9160_NS)
	/* Configuring MAGPIO, so that the correct antenna
	 * matching network is used for each LTE band and GPS.
	 */
	err = nrf_modem_at_printf("AT%%XMAGPIO=1,1,1,7,1,746,803,2,698,748,"
				  "2,1710,2200,3,824,894,4,880,960,5,791,849,"
				  "7,1565,1586");
	if (err) {
		LOG_ERR("Failed to configure MAGPIO, err %d", err);
		return;
	}
#endif

#if defined(CONFIG_LTE_EDRX_REQ)
	/* Request configured eDRX settings to save power */
	err = lte_lc_edrx_req(true);
	if (err) {
		LOG_ERR("Failed to configure EDRX, err %d", err);
		return;
	}
#endif

#if defined(CONFIG_NRF_MODEM_LIB_TRACE_ENABLED)
	err = nrf_modem_at_printf("AT%%XMODEMTRACE=1,2");
	if (err) {
		LOG_ERR("Failed to enable modem trace, err %d", err);
		return;
	}
#endif

#if defined(CONFIG_LTE_LOCK_BANDS)
	/* Set LTE band lock (volatile setting).
	 * Has to be done every time before activating the modem.
	 */
	err = nrf_modem_at_printf("AT%%XBANDLOCK=2,\""CONFIG_LTE_LOCK_BAND_MASK "\"");
	if (err) {
		LOG_ERR("Failed to lock LTE bands, err %d", err);
		return;
	}
#endif

#if defined(CONFIG_LTE_LOCK_PLMN)
	/* Manually select Operator (volatile setting).
	 * Has to be done every time before activating the modem.
	 */
	err = nrf_modem_at_printf("AT+COPS=1,2,\"" CONFIG_LTE_LOCK_PLMN_STRING "\"");
	if (err) {
		LOG_ERR("Failed to lock PLMN, err %d", err);
		return;
	}
#elif defined(CONFIG_LTE_UNLOCK_PLMN)
	/* Automatically select Operator (volatile setting).
	 */
	err = nrf_modem_at_printf("AT+COPS=0");
	if (err) {
		LOG_ERR("Failed to unlock PLMN, err %d", err);
		return;
	}
#endif
}

static void on_shutdown(void *ctx)
{
	(void)lte_lc_deinit();
}
