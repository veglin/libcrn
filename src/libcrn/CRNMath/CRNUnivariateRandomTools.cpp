/* Copyright 2008-2014 INSA Lyon
 * 
 * This file is part of libcrn.
 * 
 * libcrn is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libcrn is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcrn.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * file: CRNUnivariateRandomTools.cpp
 * \author Jean DUONG, Yann LEYDIER
 */

#include <CRNMath/CRNUnivariateRandomTools.h>
#include <CRNMath/CRNMatrixInt.h>
#include <CRNMath/CRNUnivariateGaussianPDF.h>
#include <CRNMath/CRNUnivariateGaussianMixture.h>
#include <CRNMath/CRNMatrixDouble.h>

using namespace crn;

/*****************************************************************************/
/*! Simulate a uniform random sample in [0;1]
*
* \param[in]	n	size of sample
* \param[in]	reseed shall the random seed be reinitialized?
*
* \return a set of patterns generated by the uniform law
*/
std::vector<double> UnivariateRandomTools::NewUniformSample(size_t n, bool reseed)
{
	if (reseed)
		srand((unsigned int)time(nullptr));
	
	std::vector<double> sample(n);
	
	for (size_t k = 0; k < n; ++k)
		sample[k] = ((double)(rand())) / ((double)(RAND_MAX));
	
	return sample;
}

/*****************************************************************************/
/*! Simulate a gaussian random sample
*
* With default values mu = 0 and sigma = 1, we obtain the normal random sample
*
* \param[in]	mu	mean of gaussian distribution (default = 0)
* \param[in]	sigma	deviation of gaussian distribution (default = 1)
* \param[in]	n	size of gaussian sample (default = 1)
* \param[in]	m	size of temporary uniform sample (default = 100)
* \param[in]	reseed	shall the random seed be reinitialized?
*
* \return a set of patterns generated by the Gaussian law
*/

std::vector<double> UnivariateRandomTools::NewGaussianSample(double mu, double sigma, size_t n, size_t m, bool reseed)
{
	if (reseed)
		srand((unsigned int)time(nullptr));
	
	std::vector<double> G(n);
	
	for (size_t k = 0; k < n; ++k)
	{		
		// cumulate a uniform sample with m numbers
		
		double s = 0.0;
		
		for (size_t i = 0; i < m; ++i)
			s += ((double)(rand())) / ((double)(RAND_MAX));
		
		// Gaussian random number from uniform distribution
		
		s -= double(m) / 2.0;
		s *= sigma * sqrt(12.0 / double(m));
		s += mu;
		
		G[k] = s;
	}

	return G;
}

/*****************************************************************************/
/*! Simulate a gaussian mixture random sample
*
* \param[in]	Mx		UnivariateGaussianMixture*
* \param[in]	n		size of gaussian sample (default = 1)
* \param[in]	m		size of temporary uniform sample (default = 100)
* \param[in]	reseed	shall the random seed be reinitialized?
*
* \return a set of patterns generated by the Gaussian law
*/
std::vector<double> UnivariateRandomTools::NewGaussianMixtureSample(const UnivariateGaussianMixture& Mx, size_t n, size_t m, bool reseed)
{
	size_t nbPDF = Mx.GetNbMembers();
	
	std::vector<double> Patterns(n);
	std::vector<double> CumulWeights(nbPDF);
	std::vector<double> IndexeSample(UnivariateRandomTools::NewUniformSample(n, reseed));
	std::vector<int> Pop(nbPDF, 0);
	
	// Cumulative weights from the mixture
	
	double Mass = 0.0;
		
	for (size_t k = 0; k < nbPDF; ++k)
	{
		Mass += Mx.GetWeight(k);
		
		CumulWeights[k] = Mass;
	}
	
    for (size_t k = 0; k < nbPDF; ++k)
        CumulWeights[k] /= Mass;
	
	// Build indexes to indicate which PDF of the mixture is used to generate each pattern
	
	for (size_t k = 0; k < n; ++k)
	{
		double d = IndexeSample[k];
		bool Continue = true;
		size_t Id = 0;
		
		while (Continue)
		{
			if ((CumulWeights[Id] >= d) || (Id == nbPDF - 1))
				Continue = false;
			else
                ++Id;
		}
		
		Pop[Id] += 1;
	}

	// Create patterns
	
	size_t PatternIndex = 0;
	
	for (size_t p = 0; p < nbPDF; ++p)
	{
		int SubSamplePop = Pop[p];
		
        std::vector<double> SubSample(NewGaussianSample(Mx.GetMember(p).GetMean(), Mx.GetMember(p).GetDeviation(), SubSamplePop, m, reseed));
		
		for (int r = 0; r < SubSamplePop; ++r)
		{
			Patterns[PatternIndex] = SubSample[r];
			++PatternIndex;
		}
	}

  return Patterns;
}
