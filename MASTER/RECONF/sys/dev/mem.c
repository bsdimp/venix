/*		Version 86/0.0		Nov 6, 1982
*			Edited:	11/6/82
*
*****************************************************************
*								*
*	    (C)Copyright by VenturCom, Inc. 1982,1983		*
*								*
*	All rights reserved:	  VENTURCOM INC. 1982,1983	*
*								*
*	This source listing is supplied in accordance with	*
*	the Software Agreement you have with VenturCom and	*
*	the Western Electric Company.				*
*								*
*****************************************************************
*   Memory special file
*	minor device 0 is physical memory
*	minor device 1 is kernel memory
*	minor device 2 is EOF/RATHOLE
*/
#include <sys/param.h>
#include <sys/user.h>
#include <sys/conf.h>

mmread(dev){
	register char *cp;
	register int c;

	switch( minor(dev) ){
	case 0:
		cp = (char *)u.u_ds;
		do {
			u.u_ds = u.u_offset[0] << 12;
			if( (c = fubyte(u.u_offset[1])) == -1 )
				u.u_error = EFAULT;
			u.u_ds = (unsigned int) cp;
		} while( u.u_error==0 && passc(c)>=0 );
		break;
	case 1:
		cp = (char *)u.u_offset[1];
		while( u.u_error==0 && passc(*cp++)>=0 );
	}
}

mmwrite(dev){
	register char *cp;
	register int c;
	int laddr, haddr;

	switch( minor(dev) ){
	case 0:
		cp = (char *)u.u_ds;
		while( u.u_error==0 ){
			haddr = u.u_offset[0] << 12;
			laddr = u.u_offset[1];
			if( (c = cpass()) != -1 ){
				u.u_ds = haddr;
				if( suword(laddr,c) == -1 )
					u.u_error = EFAULT;
				u.u_ds = (unsigned int)cp;
			} else
				break;
		}
		break;
	case 1:
		cp = (char *)u.u_offset[1];
		while( (c = cpass())!=-1 && u.u_error==0 )
			*cp++ = c;
		break;
	case 2:
		dpadd(u.u_offset, u.u_count);
		u.u_count = 0;
	}
}
