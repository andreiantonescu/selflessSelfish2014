//
//  gaussian_1d.h
//  disertatiev3_2
//
//  Created by Andrei Antonescu on 16/05/14.
//
//

#ifndef disertatiev3_2_gaussian_1d_h
#define disertatiev3_2_gaussian_1d_h


//
//  gaussian_1d.h
//  image_integral_projection
//
//  Created by Andrei Antonescu on 21/01/14.
//  Copyright (c) 2014 Andrei Antonescu. All rights reserved.
//

/**
 * \brief Fast 1D Gaussian convolution IIR approximation
 * \param data the data to be convolved, modified in-place
 * \param length number of elements
 * \param sigma the standard deviation of the Gaussian in pixels
 * \param numsteps number of timesteps, more steps implies better accuracy
 *
 * Implements the fast Gaussian convolution algorithm of Alvarez and Mazorra,
 * where the Gaussian is approximated by a cascade of first-order infinite
 * impulsive response (IIR) filters.  Boundaries are handled with half-sample
 * symmetric extension.
 *
 * Gaussian convolution is approached as approximating the heat equation and
 * each timestep is performed with an efficient recursive computation.  Using
 * more steps yields a more accurate approximation of the Gaussian.  A
 * reasonable default value for \c numsteps is 4.
 *
 * Reference:
 * Alvarez, Mazorra, "Signal and Image Restoration using Shock Filters and
 * Anisotropic Diffusion," SIAM J. on Numerical Analysis, vol. 31, no. 2,
 * pp. 590-605, 1994.
 */

#include "gaussian1d.h"

void gaussianiir1d(float *data, long length, float sigma, int numsteps)
{
    double lambda, dnu;
    float nu, boundaryscale, postscale;
    long i;
    int step;
    
    if(!data || length < 1 || sigma <= 0 || numsteps < 0)
        return;
    
    lambda = (sigma*sigma)/(2.0*numsteps);
    dnu = (1.0 + 2.0*lambda - sqrt(1.0 + 4.0*lambda))/(2.0*lambda);
    nu = (float)dnu;
    boundaryscale = (float)(1.0/(1.0 - dnu));
    postscale = (float)(pow(dnu/lambda,numsteps));
    
    for(step = 0; step < numsteps; step++)
    {
        data[0] *= boundaryscale;
        
        /* Filter rightwards (causal) */
        for(i = 1; i < length; i++)
            data[i] += nu * data[i - 1];
        
        data[i = length - 1] *= boundaryscale;
        
        /* Filter leftwards (anti-causal) */
        for(; i > 0; i--)
            data[i - 1] += nu*data[i];
    }
    
    for(i = 0; i < length; i++)
        data[i] *= postscale;
    
    return;
}


#endif
