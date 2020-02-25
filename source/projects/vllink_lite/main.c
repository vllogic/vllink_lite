/***************************************************************************
 *   Copyright (C) 2018 - 2020 by Chen Le <talpachen@gmail.com>            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

/*============================ INCLUDES ======================================*/

#include "vsf.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#if VSF_USE_USB_DEVICE == ENABLED
//extern void usrapp_usbd_init(void);
//extern void usrapp_usbd_cdcuart_get_stream(vsf_stream_t **stream_tx, vsf_stream_t **stream_rx);
//extern void usrapp_usbd_cdcshell_get_stream(vsf_stream_t **stream_tx, vsf_stream_t **stream_rx);
#endif

/*============================ IMPLEMENTATION ================================*/

int main(void)
{
#if VSF_USE_USB_DEVICE == ENABLED
    //usrapp_usbd_init();
#endif



    return 0;
}

/* EOF */
