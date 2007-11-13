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
* expmv - ex_put_nset_var
*
* entry conditions - 
*   input parameters:
*       int     exoid                   exodus file id
*       int     time_step               time step number
*       int     nset_var_index          nodeset variable index
*       int     nset_id                 nodeset id
*       int     num_nodes_this_nset     number of nodes in this nodeset
*
* exit conditions -
*
*
* exit conditions - 
*
* revision history - 
*
*  $Id: expnsv.c,v 1.4 2007/10/08 15:01:47 gdsjaar Exp $
*
*****************************************************************************/

#include <stdlib.h>
#include "exodusII.h"
#include "exodusII_int.h"

/*!
 * writes the values of a single nodeset variable for one nodeset at 
 * one time step to the database; assume the first time step and 
 * nodeset variable index are 1
 */

int ex_put_nset_var (int   exoid,
                     int   time_step,
                     int   nset_var_index,
                     int   nset_id,
                     int   num_nodes_this_nset,
                     const void *nset_var_vals)
{
  int varid, dimid,time_dim, numelbdim, dims[2], nset_id_ndx;
  long num_nsets, num_nset_var, start[2], count[2];
  int *nset_var_tab;
  char errmsg[MAX_ERR_LENGTH];

  exerrval = 0; /* clear error code */

  /* Determine index of nset_id in VAR_NS_ID array */
  nset_id_ndx = ex_id_lkup(exoid,VAR_NS_IDS,nset_id);
  if (exerrval != 0) 
  {
    if (exerrval == EX_NULLENTITY)
    {
      sprintf(errmsg,
              "Warning: no variables allowed for NULL nodeset %d in file id %d",
              nset_id,exoid);
      ex_err("ex_put_nset_var",errmsg,EX_MSG);
      return (EX_WARN);
    }
    else
    {
    sprintf(errmsg,
        "Error: failed to locate nodeset id %d in %s array in file id %d",
            nset_id, VAR_NS_IDS, exoid);
    ex_err("ex_put_nset_var",errmsg,exerrval);
    return (EX_FATAL);
    }
  }

  if ((varid = ncvarid (exoid,
                        VAR_NS_VAR(nset_var_index,nset_id_ndx))) == -1)
  {
    if (ncerr == NC_ENOTVAR) /* variable doesn't exist, create it! */
    {

/*    inquire previously defined dimensions */

      /* check for the existance of an nodeset variable truth table */
      if ((varid = ncvarid (exoid, VAR_NSET_TAB)) != -1)
      {
        /* find out number of nodesets and nodeset variables */
        if ((dimid = ncdimid (exoid, DIM_NUM_NS)) == -1)
        {
          exerrval = ncerr;
          sprintf(errmsg,
               "Error: failed to locate number of nodesets in file id %d",
                  exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }

        if (ncdiminq (exoid, dimid, (char *) 0, &num_nsets) == -1)
        {
          exerrval = ncerr;
          sprintf(errmsg,
                 "Error: failed to get number of nodesets in file id %d",
                  exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }

        if ((dimid = ncdimid (exoid, DIM_NUM_NSET_VAR)) == -1)
        {
          exerrval = EX_BADPARAM;
          sprintf(errmsg,
               "Error: no nodeset variables stored in file id %d",
                  exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }

        if (ncdiminq (exoid, dimid, (char *) 0, &num_nset_var) == -1)
        {
          exerrval = ncerr;
          sprintf(errmsg,
               "Error: failed to get number of nodeset variables in file id %d",
                  exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }

        if (!(nset_var_tab = malloc(num_nsets*num_nset_var*sizeof(int))))
        {
          exerrval = EX_MEMFAIL;
          sprintf(errmsg,
                 "Error: failed to allocate memory for nodeset variable truth table in file id %d",
                  exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }

        /*   read in the nodeset variable truth table */

        start[0] = 0;
        start[1] = 0;

        count[0] = num_nsets;
        count[1] = num_nset_var;

        if (ncvarget (exoid, varid, start, count, nset_var_tab) == -1)
        {
          exerrval = ncerr;
          sprintf(errmsg,
                 "Error: failed to get truth table from file id %d", exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }

        if(nset_var_tab[num_nset_var*(nset_id_ndx-1)+nset_var_index-1] 
           == 0L)
        {
          free(nset_var_tab);
          exerrval = EX_BADPARAM;
          sprintf(errmsg,
              "Error: Invalid nodeset variable %d, nodeset %d in file id %d",
                  nset_var_index, nset_id, exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
          return (EX_FATAL);
        }
        free(nset_var_tab);
      }

      if ((time_dim = ncdimid (exoid, DIM_TIME)) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
               "Error: failed to locate time dimension in file id %d", exoid);
        ex_err("ex_put_nset_var",errmsg,exerrval);
        goto error_ret;         /* exit define mode and return */
      }

      if ((numelbdim=ncdimid(exoid, DIM_NUM_NOD_NS(nset_id_ndx))) == -1)
      {
        if (ncerr == NC_EBADDIM)
        {
          exerrval = ncerr;
          sprintf(errmsg,
      "Error: number of nodes in nodeset %d not defined in file id %d",
                  nset_id, exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
        }
        else
        {
          exerrval = ncerr;
          sprintf(errmsg,
 "Error: failed to locate number of sides in nodeset %d in file id %d",
                  nset_id, exoid);
          ex_err("ex_put_nset_var",errmsg,exerrval);
        }
        goto error_ret;
      }

/*    variable doesn't exist so put file into define mode  */

      if (ncredef (exoid) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
               "Error: failed to put file id %d into define mode", exoid);
        ex_err("ex_put_nset_var",errmsg,exerrval);
        return (EX_FATAL);
      }


/*    define netCDF variable to store nodeset variable values */

      dims[0] = time_dim;
      dims[1] = numelbdim;
      if ((varid = ncvardef(exoid,VAR_NS_VAR(nset_var_index,nset_id_ndx),
                            nc_flt_code(exoid), 2, dims)) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
               "Error: failed to define nodeset variable %d in file id %d",
                nset_var_index,exoid);
        ex_err("ex_put_nset_var",errmsg,exerrval);
        goto error_ret;
      }


/*    leave define mode  */

      if (ncendef (exoid) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
       "Error: failed to complete nodeset variable %s definition to file id %d",
                VAR_NS_VAR(nset_var_index,nset_id_ndx), exoid);
        ex_err("ex_put_nset_var",errmsg,exerrval);
        return (EX_FATAL);
      }
    }
    else
    {
      exerrval = ncerr;
      sprintf(errmsg,
             "Error: failed to locate nodeset variable %s in file id %d",
              VAR_NS_VAR(nset_var_index,nset_id_ndx),exoid);
      ex_err("ex_put_nset_var",errmsg,exerrval);
      return (EX_FATAL);
    }
  }

/* store nodeset variable values */

  start[0] = --time_step;
  start[1] = 0;

  count[0] = 1;
  count[1] = num_nodes_this_nset;

  if (ncvarput (exoid, varid, start, count, 
                ex_conv_array(exoid,WRITE_CONVERT,nset_var_vals,
                num_nodes_this_nset)) == -1)
  {
    exerrval = ncerr;
    sprintf(errmsg,
           "Error: failed to store nodeset variable %d in file id %d", 
            nset_var_index,exoid);
    ex_err("ex_put_nset_var",errmsg,exerrval);
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
    ex_err("ex_put_nset_var",errmsg,exerrval);
  }
  return (EX_FATAL);
}
