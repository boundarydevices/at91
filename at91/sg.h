#ifndef __SG_H__
#define __SG_H__ "$Id$"

/*
 * sg.h
 *
 * This header file declares the sg_t structure for use
 * in referring to a set of zero or more memory segments.
 *
 * Note that the data and length field are generally initialized
 * by the origin or recipient of a data set and that the
 * offset field is normally used by code which processes or
 * walks the data set.
 *
 * Scatter-gather lists are terminated by a zero-length buffer.
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */

struct sg_t {
	unsigned char *data ;
        unsigned       length ;
        unsigned       offset ;
};

#define SG_END(sg)  (0==((sg)->length))
#define SG_DONE(sg) ((sg)->offset == (sg)->length)

/*
 * Initialize a scatter-gather structure to point to something.
 * Don't use the address of the something since sizeof() is used
 * below...
 */
#define SG_INIT(sg,obj) { (sg)->data = (unsigned char *)(&obj); \
                          (sg)->length = sizeof(obj); \
                          (sg)->offset = 0 ; }

#define SG_LEFT(sg) ((sg)->length-(sg)->offset)

/*
 * calculate and return total length of scatter-gather list
 */
unsigned sg_len(struct sg_t const *sg);

/*
 * calculate and return total left in scatter-gather list
 */
unsigned sg_left(struct sg_t const *sg);

/*
 * clear offset field in a scatter-gather list
 */
void sg_zero_offs(struct sg_t *sg);

#endif

