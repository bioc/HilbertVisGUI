\name{makeRandomTestData}
\alias{makeRandomTestData}
\title{ generate a long vector of example data that is suitable to demonstrate
the Hilbert curve visualisation }
\description{
This function generates a long numeric vector and fills it with many narrow Gaussian
peaks of varying width and position. Around 30% of the vector's full length,
the distribution of peak width is changed to be substantially larger. This
feature is easily visible with the Hilbert curve visualization but much harder to
spot with conventional 1D plots.}
\usage{
makeRandomTestData(len = 1e+07, numPeaks = 500)
}
\arguments{
  \item{len}{ Length of the vector }
  \item{numPeaks}{ Number of peaks to be placed in the vector }
}
\value{
  A vector, of type 'numeric', with sample data.
}
\author{ Simon Anders, EMBL-EBI, sanders@fs.tum.de }
\examples{
# See the help page of function 'hilbertImage' for an example.
}
