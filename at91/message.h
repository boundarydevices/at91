#ifndef __MESSAGE_H__
#define __MESSAGE_H__ "$Id$"

/*
 * Module message.h
 *
 * This header file declares the appHeader_t structure,
 * which is prepended to all application USB messages
 * except for the Samba clone.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

struct appHeader_t {
	unsigned char  bd ;
	unsigned char  appId ;
	unsigned short length ;
};

#define BDAPP_TAG '\xbd'

#endif
