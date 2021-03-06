#ifndef METRICEVOLUTION_H
#define METRICEVOLUTION_H

/* Stress Energy Tensor (SET) functionality */
void store_gws(simType **lij, IOData filedata);
void h_evolve(simType **hij, simType **lij, fftw_complex **fSTTij);
void fft_stt(simType **STTij, fftw_complex **fSTTij, fftw_plan p);
void set_stt(PointData *paq, simType **STTij, int i, int j, int k);

void StoSTT(fftw_complex **fSTTij);
void LtoLTT(simType **lij);

#endif
