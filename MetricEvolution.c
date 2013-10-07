/* RNSE includes */
#include "defines.h"


/*
 * Calculate the actual gravitational wave spectrum from l
 */
void store_gws(simType **lij)
{

  simType array_out[(int)(1.73205*(POINTS/2))+1];
  int numpoints_gw[(int)(1.73205*(POINTS/2))+1]; // Number of points in each momentum bin
  simType p[(int)(1.73205*(POINTS/2))+1];
  simType f2_gw[(int)(1.73205*(POINTS/2))+1]; // Values for each bin: Momentum, |F-k|^2, n_k
  int numbins_gw = (int)(1.73205*(POINTS/2))+1; // Actual number of bins for the number of dimensions

  double pmagnitude_gw; // Total momentum (p) in units of lattice spacing, pmagnitude = Sqrt(px^2+py^2+pz^2). This also gives the bin index since bin spacing is set to equal lattice spacing.
  double fp2_gw;
  int i,j,k,px,py,pz; // px, py, and pz are components of momentum in units of grid spacing

  // Calculate magnitude of momentum in each bin
  for(i=0; i<numbins_gw; i++) {
    p[i] = (double) i;
    f2_gw[i] = 0.0;
    numpoints_gw[i] = 0;
  }
    
  // Perform average over all angles here.
  for(i=0; i<POINTS; i++)
  {
    px = (i<=POINTS/2 ? i : i-POINTS);
    for(j=0; j<POINTS; j++)
    {
      py = (j<=POINTS/2 ? j : j-POINTS);
      for(k=1; k<POINTS/2; k++)
      {
        pz = k;
        pmagnitude_gw = sqrt(pw2(px) + pw2(py) + pw2(pz));
        fp2_gw = pw2(lij[0][SINDEX(i,j,k)]) + pw2(lij[6][SINDEX(i,j,k)])
          + 2.*pw2(lij[1][SINDEX(i,j,k)]) + 2.*pw2(lij[7][SINDEX(i,j,k)])
          + 2.*pw2(lij[2][SINDEX(i,j,k)]) + 2.*pw2(lij[8][SINDEX(i,j,k)])
          + pw2(lij[3][SINDEX(i,j,k)]) + pw2(lij[9][SINDEX(i,j,k)])
          + 2.*pw2(lij[4][SINDEX(i,j,k)]) + 2.*pw2(lij[10][SINDEX(i,j,k)])
          + pw2(lij[5][SINDEX(i,j,k)]) + pw2(lij[11][SINDEX(i,j,k)]);
        numpoints_gw[(int)pmagnitude_gw] += 2;
        f2_gw[(int)pmagnitude_gw] += 2.*fp2_gw;
      }

      pz = 0;
      k = 0;
      pmagnitude_gw = sqrt(pw2(px) + pw2(py) + pw2( pz));
      fp2_gw = pw2(lij[0][SINDEX(i,j,k)]) + pw2(lij[6][SINDEX(i,j,k)])
        + 2.*pw2(lij[1][SINDEX(i,j,k)]) + 2.*pw2(lij[7][SINDEX(i,j,k)])
        + 2.*pw2(lij[2][SINDEX(i,j,k)]) + 2.*pw2(lij[8][SINDEX(i,j,k)])
        + pw2(lij[3][SINDEX(i,j,k)]) + pw2(lij[9][SINDEX(i,j,k)])
        + 2.*pw2(lij[4][SINDEX(i,j,k)]) + 2.*pw2(lij[10][SINDEX(i,j,k)])
        + pw2(lij[5][SINDEX(i,j,k)]) + pw2(lij[11][SINDEX(i,j,k)]);  
      numpoints_gw[(int)pmagnitude_gw] += 1;
      f2_gw[(int)pmagnitude_gw] += fp2_gw;
        
      pz = POINTS/2;
      k = POINTS/2;
      pmagnitude_gw=sqrt(pw2(px) + pw2(py) + pw2(pz));
      fp2_gw = pw2(lij[0][SINDEX(i,j,k)]) + pw2(lij[6][SINDEX(i,j,k)])
        + 2.*pw2(lij[1][SINDEX(i,j,k)]) + 2.*pw2(lij[7][SINDEX(i,j,k)])
        + 2.*pw2(lij[2][SINDEX(i,j,k)]) + 2.*pw2(lij[8][SINDEX(i,j,k)])
        + pw2(lij[3][SINDEX(i,j,k)]) + pw2(lij[9][SINDEX(i,j,k)])
        + 2.*pw2(lij[4][SINDEX(i,j,k)]) + 2.*pw2(lij[10][SINDEX(i,j,k)])
        + pw2(lij[5][SINDEX(i,j,k)]) + pw2(lij[11][SINDEX(i,j,k)]);
      numpoints_gw[(int)pmagnitude_gw] += 1;
      f2_gw[(int)pmagnitude_gw] += fp2_gw;
    }
  }
      
  for(i=0; i<numbins_gw; i++)
  {
    // Converts sums to averages. (numpoints[i] should always be greater than zero.)
    if(numpoints_gw[i]>0)
    {
        array_out[i] = f2_gw[i]/((double) numpoints_gw[i]);
    }
    else
    {
        array_out[i] = 0.;
    }
  }
  
  FILE *gwspec;
  gwspec = fopen("gwspec.txt", "a");  
  for(i=0;i<numbins_gw;i++) {
    fprintf(gwspec,"%10.10f %10.10f\n", p[i], array_out[i]);
  }
  fclose (gwspec);
    
  return;
}


/*
 * For evolving h_ij, we work in fourier space, as this has proven to be more stable in past work.
 */
void h_evolve(simType **hij, simType **lij, fftw_complex **fSTTij)
{
  int i, j, k, a, b;
  simType pz, px, py, p, p2;
  simType h, l, S, trs_RE, trs_IM;

  // evolve h_ij (the fourier transform of)
    // solving (in fourier space) d_t l = k^2 h + S
    //                            d_t h = h


  // only evolve the transverse/traceless part:
  // #pragma omp parallel for default(shared) private(i, j, k) num_threads(threads)
  for(int i=0; i<POINTS; i++)
  {
    px = (simType) (i <= POINTS/2 ? i : i - POINTS);
    for(int j=0; j<POINTS; j++)
    {
      py = (simType) (j <= POINTS/2 ? j : j - POINTS);
      for(int k=0; k<POINTS/2+1; k++)
      {
        pz = (simType) k;
        p2 = px*px + py*py + pz*pz; // p^2 (momentum)
        simType phat[3] = { px/sqrt(p2), py/sqrt(p2), pz/sqrt(p2) };

        // pre-calculate some stuff:          
          // Trace (S^a_a)
          trs_RE = C_RE(fSTTij[0][SINDEX(i,j,k)]) + C_RE(fSTTij[3][SINDEX(i,j,k)]) + C_RE(fSTTij[5][SINDEX(i,j,k)]);
          trs_IM = C_IM(fSTTij[0][SINDEX(i,j,k)]) + C_IM(fSTTij[3][SINDEX(i,j,k)]) + C_IM(fSTTij[5][SINDEX(i,j,k)]);
          // One contraction (k^m S_mi)
          simType k_mS_mi_RE[3] = { 
            phat[1]*C_RE(fSTTij[0][SINDEX(i,j,k)]) + phat[2]*C_RE(fSTTij[1][SINDEX(i,j,k)]) + phat[3]*C_RE(fSTTij[2][SINDEX(i,j,k)]),
            phat[1]*C_RE(fSTTij[1][SINDEX(i,j,k)]) + phat[2]*C_RE(fSTTij[3][SINDEX(i,j,k)]) + phat[3]*C_RE(fSTTij[4][SINDEX(i,j,k)]),
            phat[1]*C_RE(fSTTij[2][SINDEX(i,j,k)]) + phat[2]*C_RE(fSTTij[4][SINDEX(i,j,k)]) + phat[3]*C_RE(fSTTij[5][SINDEX(i,j,k)])
          };
          simType k_mS_mi_IM[3] = { 
            phat[1]*C_IM(fSTTij[0][SINDEX(i,j,k)]) + phat[2]*C_IM(fSTTij[1][SINDEX(i,j,k)]) + phat[3]*C_IM(fSTTij[2][SINDEX(i,j,k)]),
            phat[1]*C_IM(fSTTij[1][SINDEX(i,j,k)]) + phat[2]*C_IM(fSTTij[3][SINDEX(i,j,k)]) + phat[3]*C_IM(fSTTij[4][SINDEX(i,j,k)]),
            phat[1]*C_IM(fSTTij[2][SINDEX(i,j,k)]) + phat[2]*C_IM(fSTTij[4][SINDEX(i,j,k)]) + phat[3]*C_IM(fSTTij[5][SINDEX(i,j,k)])
          };
          // Two contractions (k^a k^b S_ab)
          simType kkS_RE = phat[1]*k_mS_mi_RE[1] + phat[2]*k_mS_mi_RE[2] + phat[3]*k_mS_mi_RE[3];
          simType kkS_IM = phat[1]*k_mS_mi_IM[1] + phat[2]*k_mS_mi_IM[2] + phat[3]*k_mS_mi_IM[3];

        for(a=1; a<=3; a++) {
          for(b=a; b<=3; b++) {
            // (7-a)*a/2-4+b formula maps indexes of h to sequential indices
            C_RE(fSTTij[(7-a)*a/2-4+b][SINDEX(i,j,k)]) +=
              (
                0.5*(phat[a]*phat[b] - (a==b))*trs_RE
                + 0.5*(phat[a]*phat[b] + (a==b))*kkS_RE
                - (phat[a]*k_mS_mi_RE[b] + phat[b]*k_mS_mi_RE[a])
              );
            C_IM(fSTTij[(7-a)*a/2-4+b][SINDEX(i,j,k)]) +=
              (
                0.5*(phat[a]*phat[b] - (a==b))*trs_RE
                + 0.5*(phat[a]*phat[b] + (a==b))*kkS_RE
                - (phat[a]*k_mS_mi_RE[b] + phat[b]*k_mS_mi_RE[a])
              );
          }
        }

      }
    }
  }

  for(a=0; a<12; a++) {
    // #pragma omp parallel for default(shared) private(i, j, k) num_threads(threads)
    for(int i=0; i<POINTS; i++)
    {
      px = (simType) (i <= POINTS/2 ? i : i - POINTS);
      for(int j=0; j<POINTS; j++)
      {
        py = (simType) (j <= POINTS/2 ? j : j - POINTS);
        for(int k=0; k<POINTS/2+1; k++)
        {
          pz = (simType) k;
          p = sqrt(px*px + py*py + pz*pz); // p (momentum)
          h = hij[a][SINDEX(i,j,k)];
          l = lij[a][SINDEX(i,j,k)];

          S;
          if(a >= 6) {
            S = C_IM(fSTTij[a-6][SINDEX(i,j,k)]);
          } else {
            S = C_RE(fSTTij[a][SINDEX(i,j,k)]);
          }

          // This is RK4. Promise.
          hij[a][SINDEX(i,j,k)] += dt*(l*(6.0 + p*p*dt*dt) + dt*(h*k*k + S)*(12.0 + dt*dt*p*p)/4.0)/6.0;
          lij[a][SINDEX(i,j,k)] += dt*(6.0*S + 3.0*p*p*l*dt + p*p*p*p*l*dt*dt*dt/4.0 + h*p*p*(6.0 + p*p*dt*dt))/6.0;
        }
      }
    }
  }

  return;
}


/*
 * FFT the STT
 */
void fft_stt(simType **STTij, fftw_complex **fSTTij)
{
  // transform STT components into fourier space
  int a;
  fftw_plan p;
  p = fftw_plan_dft_r2c_3d(POINTS, POINTS, POINTS,
                            STTij[0], fSTTij[0],
                            FFTW_ESTIMATE);

  // fourier transform stress-energy tensor
  for(a=1; a<6; a++) {
    fftw_execute_dft_r2c(p, STTij[a], fSTTij[a]);
  }
}


/*
 * Calculate S at a point (i,j,k).
 * This does not remove the transverse/traceless part.
 */
void set_stt(PointData *paq, simType **STTij, int i, int j, int k)
{
  // S_ij = T_ij - 1/3 \delta_ij * T_k^k.
  //      = (e+p)U^iU^j + d^ifd^jf - 1/3(\delta_ij)*( trace of previous terms )
  // The above is traceless, but not yet transverse.

  int a, b;
  for(a=1; a<=3; a++) {
    for(b=a; b<=3; b++) {
      // (7-a)*a/2-4+b formula maps indexes of h to sequential indices
      STTij[(7-a)*a/2-4+b][SINDEX(i,j,k)] =
          pow(paq->gradients[a][5],2)
            + W_EOSp1 * exp(paq->fields[0]) * paq->fields[a] * paq->fields[b];
    }
  }

  // project into transverse plane, and subtract trace - this is done during actual evolution.

  return;
}
