/* Name: main.c
 * Author: <insert your name here>
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include "m_general.h"
#include "maevarm-rf.h"
#include "m_usb.h"

int main(void)
{
	// unsigned int value;

	/* insert your hardware initialization here */
	m_usb_init();

	while (!m_usb_isconnected());

	for(;;) {
		// if (m_usb_rx_available()) {
		// 	value = m_usb_rx_char();
		// 	m_usb_tx_uint(value);
		// }
		m_wait(500);
		m_green(TOGGLE);
	}

	return 0;   /* never reached */
}
