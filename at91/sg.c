/*
 * Module sg.c
 *
 * This module defines a handful of scatter-gather
 * utility routines.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */


#include "sg.h"

unsigned sg_len(struct sg_t const *sg){
	unsigned len = 0 ;
	while( sg && !SG_END(sg) ){
		len += sg->length ;
		sg++ ;
	}
	return len ;
}

unsigned sg_left(struct sg_t const *sg){
	unsigned len = 0 ;
	while( sg && !SG_END(sg) ){
		len += SG_LEFT(sg);
		sg++ ;
	}
	return len ;
}

void sg_zero_offs(struct sg_t *sg){
	while( sg && !SG_END(sg) ){
		sg->offset = 0 ;
		sg++ ;
	}
}

