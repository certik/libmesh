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
* expenm - ex_put_elem_num_map
*
* entry conditions - 
*   input parameters:
*       int     exoid                   exodus file id
*       int*    elem_map                element numbering map array
*
* exit conditions - 
*
* revision history - 
*
*
*****************************************************************************/

#include "exodusII.h"
#include "exodusII_int.h"
#include <stdlib.h> /* for free() */

/*!
 * writes out the element numbering map to the database; this allows
 * element numbers to be non-contiguous
 */

int ex_put_elem_num_map (int  exoid,
                         const int *elem_map)
{
  int numelemdim, dims[1], mapid, iresult;
  long num_elem, start[1], count[1]; 
  char errmsg[MAX_ERR_LENGTH];

  exerrval = 0; /* clear error code */

  /* inquire id's of previously defined dimensions  */

  /* determine number of elements. Return if zero... */
  if ((numelemdim = ncdimid (exoid, DIM_NUM_ELEM)) == -1)
    {
      return (EX_NOERR);
    }

  if (ncdiminq (exoid, numelemdim, (char *) 0, &num_elem) == -1)
    {
      exerrval = ncerr;
      sprintf(errmsg,
              "Error: failed to get number of elements in file id %d",
              exoid);
      ex_err("ex_put_elem_num_map",errmsg,exerrval);
      return (EX_FATAL);
    }


  /* put netcdf file into define mode  */

  if ((mapid = ncvarid (exoid, VAR_ELEM_NUM_MAP)) == -1) {
    if (ncredef (exoid) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
                "Error: failed to put file id %d into define mode",
                exoid);
        ex_err("ex_put_elem_num_map",errmsg,exerrval);
        return (EX_FATAL);
      }


    /* create a variable array in which to store the element numbering map  */

    dims[0] = numelemdim;

    if ((mapid = ncvardef (exoid, VAR_ELEM_NUM_MAP, NC_INT, 1, dims)) == -1)
      {
        if (ncerr == NC_ENAMEINUSE)
          {
            exerrval = ncerr;
            sprintf(errmsg,
                    "Error: element numbering map already exists in file id %d",
                    exoid);
            ex_err("ex_put_elem_num_map",errmsg,exerrval);
          }
        else
          {
            exerrval = ncerr;
            sprintf(errmsg,
                    "Error: failed to create element numbering map in file id %d",
                    exoid);
            ex_err("ex_put_elem_num_map",errmsg,exerrval);
          }
        goto error_ret;         /* exit define mode and return */
      }


    /* leave define mode  */

    if (ncendef (exoid) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
                "Error: failed to complete definition in file id %d",
                exoid);
        ex_err("ex_put_elem_num_map",errmsg,exerrval);
        return (EX_FATAL);
      }
  }

  /* write out the element numbering map  */
  start[0] = 0;
  count[0] = num_elem;

  iresult = ncvarput (exoid, mapid, start, count, elem_map);

  if (iresult == -1)
    {
      exerrval = ncerr;
      sprintf(errmsg,
              "Error: failed to store element numbering map in file id %d",
              exoid);
      ex_err("ex_put_elem_num_map",errmsg,exerrval);
      return (EX_FATAL);
    }


  return (EX_NOERR);

  /* Fatal error: exit definition mode and return */
 error_ret:
  if (ncendef (exoid) == -1)     /* exit define mode */
    {
      sprintf(errmsg,
              "Error: failed to complete definition for file id %d",
              exoid);
      ex_err("ex_put_elem_num_map",errmsg,exerrval);
    }
  return (EX_FATAL);
}

