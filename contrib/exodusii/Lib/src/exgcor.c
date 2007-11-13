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
* exgcor - ex_get_coord
*
* entry conditions - 
*   input parameters:
*       int     exoid                   exodus file id
*
* exit conditions - 
*       float*  x_coord                 X coord array
*       float*  y_coord                 y coord array
*       float*  z_coord                 z coord array
*
* revision history - 
*
*  $Id: exgcor.c,v 1.5 2007/10/08 15:01:39 gdsjaar Exp $
*
*****************************************************************************/

#include "exodusII.h"
#include "exodusII_int.h"

/*!
 * reads the coordinates of the nodes
 * Only fills in the 'non-null' arrays.
 */

int ex_get_coord (int exoid,
                  void *x_coor,
                  void *y_coor,
                  void *z_coor)
{
  int coordid;
  int coordidx, coordidy, coordidz;

  int numnoddim, ndimdim, i;
  long num_nod, num_dim, start[2], count[2];
  char errmsg[MAX_ERR_LENGTH];

  exerrval = 0;

  /* inquire id's of previously defined dimensions  */

  if ((numnoddim = ncdimid (exoid, DIM_NUM_NODES)) == -1)
    {
      /* If not found, then this file is storing 0 nodes.
         Return immediately */
      return (EX_NOERR);
    }

  if (ncdiminq (exoid, numnoddim, (char *) 0, &num_nod) == -1)
    {
      exerrval = ncerr;
      sprintf(errmsg,
              "Error: failed to get number of nodes in file id %d",
              exoid);
      ex_err("ex_get_coord",errmsg,exerrval);
      return (EX_FATAL);
    }


  if ((ndimdim = ncdimid (exoid, DIM_NUM_DIM)) == -1)
    {
      exerrval = ncerr;
      sprintf(errmsg,
              "Error: failed to locate number of dimensions in file id %d",
              exoid);
      ex_err("ex_get_coord",errmsg,exerrval);
      return (EX_FATAL);
    }

  if (ncdiminq (exoid, ndimdim, (char *) 0, &num_dim) == -1)
    {
      sprintf(errmsg,
              "Error: failed to get number of dimensions in file id %d",
              exoid);
      ex_err("ex_get_coord",errmsg,exerrval);
      return (EX_FATAL);
    }

  /* read in the coordinates  */
  if (ex_large_model(exoid) == 0) {
    if ((coordid = ncvarid (exoid, VAR_COORD)) == -1) {
      exerrval = ncerr;
      sprintf(errmsg,
              "Error: failed to locate nodal coordinates in file id %d", exoid);
      ex_err("ex_get_coord",errmsg,exerrval);
      return (EX_FATAL);
    } 

    for (i=0; i<num_dim; i++)
      {
        start[0] = i;
        start[1] = 0;

        count[0] = 1;
        count[1] = num_nod;

        if (i == 0 && x_coor != NULL)
          {
            if (ncvarget (exoid, coordid, start, count, 
                          ex_conv_array(exoid,RTN_ADDRESS,x_coor,(int)num_nod)) == -1)
              {
                exerrval = ncerr;
                sprintf(errmsg,
                        "Error: failed to get X coord array in file id %d", exoid);
                ex_err("ex_get_coord",errmsg,exerrval);
                return (EX_FATAL);
              }


            ex_conv_array( exoid, READ_CONVERT, x_coor, (int)num_nod );
          }
        else if (i == 1 && y_coor != NULL)
          {
            if (ncvarget (exoid, coordid, start, count,
                          ex_conv_array(exoid,RTN_ADDRESS,y_coor,(int)num_nod)) == -1)
              {
                exerrval = ncerr;
                sprintf(errmsg,
                        "Error: failed to get Y coord array in file id %d", exoid);
                ex_err("ex_get_coord",errmsg,exerrval);
                return (EX_FATAL);
              }


            ex_conv_array( exoid, READ_CONVERT, y_coor, (int)num_nod );
          }

        else if (i == 2 && z_coor != NULL) 
          {
            if (ncvarget (exoid, coordid, start, count,
                          ex_conv_array(exoid,RTN_ADDRESS,z_coor,(int)num_nod)) == -1)
              {
                exerrval = ncerr;
                sprintf(errmsg,
                        "Error: failed to get Z coord array in file id %d", exoid);
                ex_err("ex_get_coord",errmsg,exerrval);
                return (EX_FATAL);
              }


            ex_conv_array( exoid, READ_CONVERT, z_coor, (int)num_nod );
          }
      }
  } else {
    if ((coordidx = ncvarid (exoid, VAR_COORD_X)) == -1)
      {
        exerrval = ncerr;
        sprintf(errmsg,
                "Error: failed to locate x nodal coordinates in file id %d", exoid);
        ex_err("ex_get_coord",errmsg,exerrval);
        return (EX_FATAL);
      }

    if (num_dim > 1) {
      if ((coordidy = ncvarid (exoid, VAR_COORD_Y)) == -1)
        {
          exerrval = ncerr;
          sprintf(errmsg,
                  "Error: failed to locate y nodal coordinates in file id %d", exoid);
          ex_err("ex_get_coord",errmsg,exerrval);
          return (EX_FATAL);
        }
    } else {
      coordidy = 0;
    }

    if (num_dim > 2) {
      if ((coordidz = ncvarid (exoid, VAR_COORD_Z)) == -1)
        {
          exerrval = ncerr;
          sprintf(errmsg,
                  "Error: failed to locate z nodal coordinates in file id %d", exoid);
          ex_err("ex_get_coord",errmsg,exerrval);
          return (EX_FATAL);
        }
    } else {
      coordidz = 0;
    }

    /* write out the coordinates  */
    for (i=0; i<num_dim; i++)
      {
        const void *coor;
        char *which;
        int status;
       
        if (i == 0) {
          coor = x_coor;
          which = "X";
          coordid = coordidx;
        } else if (i == 1) {
          coor = y_coor;
          which = "Y";
          coordid = coordidy;
        } else if (i == 2) {
          coor = z_coor;
          which = "Z";
          coordid = coordidz;
        }

        if (coor != NULL) {
          if (nc_flt_code(exoid) == NC_FLOAT) {
            status = nc_get_var_float(exoid, coordid, 
                                      ex_conv_array(exoid,RTN_ADDRESS,
                                                    coor,(int)num_nod));
          } else {
            status = nc_get_var_double(exoid, coordid, 
                                       ex_conv_array(exoid,RTN_ADDRESS,
                                                     coor,(int)num_nod));
          }
          
          if (status == -1)
            {
              exerrval = ncerr;
              sprintf(errmsg,
                      "Error: failed to get %s coord array in file id %d", which, exoid);
              ex_err("ex_put_coord",errmsg,exerrval);
              return (EX_FATAL);
            }
          ex_conv_array( exoid, READ_CONVERT, coor, (int)num_nod );
        }
      }
  }
  return (EX_NOERR);
}
