/*
 * Copyright (c) 2005 Sandia Corporation. Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Governement
 * retains certain rights in this software.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.  
 * 
 *     * Neither the name of Sandia Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/*****************************************************************************
*
* excre - ex_create
*
* entry conditions - 
*   input parameters:
*       char*   path                    file name including path
*       int     cmode                   access mode r/w
*       int*    comp_ws                 compute word size
*       int*    io_ws                   storage word size
*
* exit conditions - 
*       return value                    exodus file id
*
* revision history - 
*
*  $Id$
*
*****************************************************************************/
#include "exodusII.h"
#include "exodusII_int.h"

/*
 * creates a new EXODUS II file and returns an id that can subsequently 
 * be used to refer to the file 
 */

int ex_create (const char *path,
               int   cmode,
               int  *comp_ws,
               int  *io_ws)
{
   int exoid, time_dim, dims[1];
   int lio_ws;
   int filesiz;
   float vers;
   char errmsg[MAX_ERR_LENGTH];
   char *mode_name;
   int mode = 0;
#if defined(NC_NETCDF4)
   static int netcdf4_mode = -1;
   char *option;
#endif /* NC_NETCDF4 */
   
   exerrval = 0; /* clear error code */

#if defined(NC_NETCDF4)
   if (cmode & EX_NETCDF4) {
     mode |= NC_NETCDF4;
   } else {
     if (netcdf4_mode == -1) {
       option = getenv("EXODUS_NETCDF4");
       if (option != NULL) {
	 fprintf(stderr, "EXODUSII: Using netcdf version 4 selected via EXODUS_NETCDF4 environment variable\n");
	 netcdf4_mode = NC_NETCDF4;
       } else {
	 netcdf4_mode = 0;
       }
     }
     mode |= netcdf4_mode;
   }
#endif

   /*
    * See if "large file" mode was specified in a ex_create cmode. If
    * so, then pass the NC_64BIT_OFFSET flag down to netcdf.
    * If netcdf4 mode specified, don't use NC_64BIT_OFFSET mode.
    */
   if ( (cmode & EX_LARGE_MODEL) && (cmode & EX_NORMAL_MODEL)) {
     exerrval = EX_BADPARAM;
     sprintf(errmsg,
             "Warning: conflicting mode specification for file %s, mode %d. Using normal",
             path, cmode);
     ex_err("ex_create",errmsg,exerrval);
   }
   if ((cmode & EX_NORMAL_MODEL) != 0)
     filesiz = 0;
#if defined(NC_NETCDF4)
   else if ((mode & NC_NETCDF4) != 0)
     filesiz = 1;
#endif
   else 
     filesiz = (int)(((cmode & EX_LARGE_MODEL) != 0) || (ex_large_model(-1) == 1));

   if (
#if defined(NC_NETCDF4)
       !(mode & NC_NETCDF4) &&
#endif
       filesiz == 1) {
     mode |= NC_64BIT_OFFSET;
   }

   if (cmode & EX_SHARE) {
     mode |= NC_SHARE;
   }

/*
 * set error handling mode to no messages, non-fatal errors
 */
  ex_opts(exoptval);    /* call required to set ncopts first time through */

   if (cmode & EX_CLOBBER) {
     mode |= NC_CLOBBER;
     mode_name = "CLOBBER";
   } else {
     mode |= NC_NOCLOBBER;
     mode_name = "NOCLOBBER";
   }

   if ((exoid = nccreate (path, mode)) == -1) {
     exerrval = ncerr;
     sprintf(errmsg,
             "Error: file create failed for %s, mode: %s",
             path, mode_name);
     ex_err("ex_create",errmsg,exerrval);
     return (EX_FATAL);
   }

/* turn off automatic filling of netCDF variables
 */

  if (ncsetfill (exoid, NC_NOFILL) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to set nofill mode in file id %d",
            exoid);
    ex_err("ex_create", errmsg, exerrval);
    return (EX_FATAL);
  }

/* initialize floating point size conversion.  since creating new file, 
 * i/o wordsize attribute from file is zero.
 */

   if (ex_conv_ini( exoid, comp_ws, io_ws, 0 ) != EX_NOERR) {
     exerrval = EX_FATAL;
     sprintf(errmsg,
           "Error: failed to init conversion routines in file id %d",
            exoid);
     ex_err("ex_create", errmsg, exerrval);
     return (EX_FATAL);
   }

/* put the EXODUS version number, and i/o floating point word size as
 * netcdf global attributes
 */

/* store Exodus API version # as an attribute */
  vers = EX_API_VERS;
  if (ncattput (exoid, NC_GLOBAL, ATT_API_VERSION, NC_FLOAT, 1, &vers) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
         "Error: failed to store Exodus II API version attribute in file id %d",
            exoid);
    ex_err("ex_create",errmsg, exerrval);
    return (EX_FATAL);
  }

/* store Exodus file version # as an attribute */
  vers = EX_VERS;
  if (ncattput (exoid, NC_GLOBAL, ATT_VERSION, NC_FLOAT, 1, &vers) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
        "Error: failed to store Exodus II file version attribute in file id %d",
            exoid);
    ex_err("ex_create",errmsg, exerrval);
    return (EX_FATAL);
  }

/* store Exodus file float word size  as an attribute */
  lio_ws = (int)(*io_ws);
  if (ncattput (exoid, NC_GLOBAL, ATT_FLT_WORDSIZE, NC_INT, 1, &lio_ws) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
 "Error: failed to store Exodus II file float word size attribute in file id %d",
           exoid);
    ex_err("ex_create",errmsg, exerrval);
    return (EX_FATAL);
  }

  /* store Exodus file size (1=large, 0=normal) as an attribute */
  if (ncattput (exoid, NC_GLOBAL, ATT_FILESIZE, NC_INT, 1, &filesiz) == -1) {
    exerrval = ncerr;
    sprintf(errmsg,
	    "Error: failed to store Exodus II file size attribute in file id %d",
	    exoid);
    ex_err("ex_create",errmsg, exerrval);
    return (EX_FATAL);
  }
  
  /* define some dimensions and variables
   */
  
  /* create string length dimension */
  if (ncdimdef (exoid, DIM_STR, (MAX_STR_LENGTH+1)) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to define string length in file id %d",exoid);
    ex_err("ex_create",errmsg,exerrval);
    return (EX_FATAL);
  }


  /* create line length dimension */
  if (ncdimdef (exoid, DIM_LIN, (MAX_LINE_LENGTH+1)) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to define line length in file id %d",exoid);
    ex_err("ex_create",errmsg,exerrval);
    return (EX_FATAL);
  }

  /* create number "4" dimension; must be of type long */
  if (ncdimdef (exoid, DIM_N4, 4L) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to define number \"4\" dimension in file id %d",exoid);
    ex_err("ex_create",errmsg,exerrval);
    return (EX_FATAL);
  }


  if ((time_dim = ncdimdef (exoid, DIM_TIME, NC_UNLIMITED)) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to define time dimension in file id %d", exoid);
    ex_err("ex_create",errmsg,exerrval);
    return (EX_FATAL);
  }

  dims[0] = time_dim;
  if ((ncvardef (exoid, VAR_WHOLE_TIME, nc_flt_code(exoid), 1, dims)) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to define whole time step variable in file id %d",
            exoid);
    ex_err("ex_create",errmsg,exerrval);
    return (EX_FATAL);
  }

  if (ncendef (exoid) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to complete definition for file id %d", exoid);
    ex_err("ex_create",errmsg,exerrval);
    return (EX_FATAL);
  }

  return (exoid);
}
