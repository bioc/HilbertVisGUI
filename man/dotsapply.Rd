\name{dotsapply}
\alias{dotsapply}

\title{List apply for '...' arguments}

\description{A kludge to get around a certain problem in using \code{lapply}
with '\code{...}' ellipsis function arguments.}

\usage{
dotsapply( fun, ... )
}

\arguments{
\item{fun}{A function that takes one argument}
\item{...}{Arguments to which the function should be applied }
}

\value{A list of function values}

\details{ "\code{dotsapply(fun,...)}" gives the same result as \code{lapply(
list(...), fun)}. However, the construction with "\code{list}", when used for
variables enumerated explicitely, will result in their duplication. 

Assume, for example, that you have three very large vectors \code{a}, \code{b}, and
\code{c}, whose lengths you wish to know. If your write "\code{lapply( list(a,b,c),
length )}", R will duplicate all three vectors in memory when constructing the
list, which results in unnecessary use of memory. The alternative "\code{dotsapply(
length, a, b, c )}" avoids this. Of course, you could also write "\code{list(
length(a), length(b), length(c) )}", which neither causes duplication.

This last possibility is, however, not an option, if, instead of "\code{a,b,c}",
you have ellipsed function arguments, i.e. literally "\code{...}". In this special
case, \code{dotsapply} comes in handy, and as this case arose in
\code{\link{hilbertDisplay}}, I implemented \code{dotsapply} as a kludge and export
it from the package just in case somebody ahs use for it.
}

\author{Simon Anders, EMBL-EBI, \email{sanders@fs.tum.de}}
\keyword{manip}
